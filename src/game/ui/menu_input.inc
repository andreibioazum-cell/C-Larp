static const char *login_error_text(void) {
    if (login_status == 4) {
        return tr_no_connection();
    }
    if (login_status == 5) {
        return tr_bad_nick();
    }
    if (login_status == 6) {
        return tr_wrong_pass();
    }
    if (login_status == 7) {
        return tr_bad_pass();
    }
    return "";
}

static void draw_login_field(const char *label, const char *value, const char *placeholder, double y, double focused) {
    text(label, login_x() + login_lab_ox, y - login_lab, text_color);
    roundrect(login_x(), y, login_w, login_fh, field_radius, card_bg);
    double caret = 0;
    if (focused == 1 && keyboard_visible() == 1) {
        caret = 1;
    }
    const char *shown = value;
    if (str_len(value) == 0 && caret == 0) {
        shown = placeholder;
    }
    if (caret == 1) {
        shown = ds_concat(value, "|");
    }
    text_in_field(shown, login_x() + field_text_pad, y, login_fh, text_color, 0.6);
}

static void draw_login(void) {
    screen_fill();
    draw_login_field(tr_nick(), login_nick, tr_nick_ph(), login_nick_y(), login_field == 1);
    draw_login_field(tr_pass(), login_pass, tr_pass_ph(), login_pass_y(), login_field == 2);
    button_scaled(tr_login_btn(), login_btn_x(), login_btn_y(), login_btn_w, btn_h, 0.65);
    const char *err = login_error_text();
    if (str_len(err) > 0) {
        ctext(err, login_msg_y(), text_color);
    }
    draw_back();
}

static void login_sync_field(void) {
    if (keyboard_visible() == 0) {
        return;
    }
    if (login_field == 1) {
        login_nick = keyboard_get_text();
    } else {
        login_pass = keyboard_get_text();
    }
}

static void login_focus(double field, const char *value) {
    login_sync_field();
    login_field = field;
    keyboard_clear();
    if (str_len(value) > 0) {
        keyboard_type(value);
    }
    keyboard_show();
}

static void login_focus_nick(void) {
    login_focus(1, login_nick);
}

static void login_focus_pass(void) {
    login_focus(2, login_pass);
}

static void login_open(void) {
    login_field = 1;
    login_status = 0;
    login_nick = "";
    login_pass = "";
    keyboard_clear();
    keyboard_hide();
}

static void login_do(void) {
    login_sync_field();
    if (str_len(login_nick) == 0) {
        login_status = 5;
        keyboard_hide();
        return;
    }
    if (str_len(login_pass) == 0) {
        login_status = 7;
        keyboard_hide();
        return;
    }
    double res = net_auth(FIREBASE, login_nick, login_pass);
    if (res != 2) {
        login_status = res;
        return;
    }
    login_status = 0;
    keyboard_hide();
    keyboard_clear();
    progress_from_storage();
    settings_from_storage();
    start_transition(login_after);
}

static void update_login(void) {
    if (keyboard_visible() == 1) {
        login_sync_field();
    }
    if (keyboard_enter_pressed() == 1) {
        if (login_field == 1) {
            login_focus_pass();
        } else {
            login_do();
        }
    }
}

static double warn_rgb(double rgb) {
    return (1 + floor(warn_a * 254)) * 0x1000000 + rgb;
}

static void draw_warning(void) {
    rect(0, 0, screen_w, screen_h, warn_rgb(0));
    uint32_t body = warn_rgb(text_color);
    double y = screen_h / 2 - 196;
    ctext_scaled(tr_warn_title_en(), y, body, 1.15);
    ctext_scaled(tr_warn_en1(), y + 48, body, 0.52);
    ctext_scaled(tr_warn_en2(), y + 76, body, 0.52);
    ctext_scaled(tr_warn_en3(), y + 104, body, 0.52);
    ctext_scaled(tr_warn_en4(), y + 132, body, 0.52);
    ctext_scaled(tr_warn_title_ru(), y + 186, body, 1.15);
    ctext_scaled(tr_warn_ru1(), y + 234, body, 0.52);
    ctext_scaled(tr_warn_ru2(), y + 262, body, 0.52);
    ctext_scaled(tr_warn_ru3(), y + 290, body, 0.52);
    ctext_scaled(tr_warn_ru4(), y + 318, body, 0.52);
    const char *label = tr_legal_accept();
    if (warn_ready < 1) {
        label =
            ds_concat(ds_concat(ds_concat(label, " ("), ds_num_to_string((double)((warn_wait - floor(warn_t))))), ")");
    }
    double scale = fit_text_scale(label, warn_btn_w() - 24, 0.8);
    double bw = warn_btn_w();
    double bx = center_x(bw);
    double by = warn_btn_y();
    roundrect(bx, by, bw, btn_h, btn_radius, warn_rgb(0));
    roundrect_outline(bx, by, bw, btn_h, btn_radius, warn_frame_th, warn_rgb(0xFFFFFF));
    text_in_box(label, bx, by, bw, btn_h, warn_rgb(0xFFFFFF), scale);
}

static void legal_accept(void) {
    if (warn_ready < 1 || warn_closing == 1) {
        return;
    }
    warn_close_t = 0;
    warn_closing = 1;
    settings_mark_legal();
}

static void touch_warn(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (hit_warn_btn(x, y) == 1) {
        legal_accept();
    }
}

static double studio_rgb(double rgb) {
    return (1 + floor(studio_a * 254)) * 0x1000000 + rgb;
}

static double studio_bg_rgb(double rgb) {
    return (1 + floor(studio_bg_a * 254)) * 0x1000000 + rgb;
}

static void draw_studio(void) {
    rect(0, 0, screen_w, screen_h, studio_bg_rgb(0));
    uint32_t body = studio_rgb(text_color);
    double y = screen_h / 2 - 30;
    ctext_scaled(STUDIO_NAME, y, body, 1.1);
    double lw = fit_w(320);
    rect(center_x(lw), y + 34, lw, 2, body);
}

static void open_online(void) {
    if (net_login_status() != 2) {
        login_after = ST_ONLINE;
        start_transition(ST_LOGIN);
        return;
    }
    if (net_is_banned(net_login_nick()) == 1) {
        banned_msg = tr_banned();
        start_transition(ST_LOBBY);
        return;
    }
    start_transition(ST_ONLINE);
}

static void open_leaderboard(void) {
    lb_wait = 0;
    net_leaderboard_fetch(FIREBASE);
    start_transition(ST_LEADERBOARD);
}

static void touch_lobby(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (hit_menu_row(x, y, 0) == 1) {
        start_transition(ST_MODES);
        return;
    }
    if (hit_menu_row(x, y, 1) == 1) {
        start_transition(ST_CLASSES);
        return;
    }
    if (hit_menu_row(x, y, 2) == 1) {
        start_transition(ST_SETTINGS);
        return;
    }
    if (hit_menu_row(x, y, 3) == 1) {
        login_after = ST_LOBBY;
        start_transition(ST_LOGIN);
        return;
    }
    if (hit_menu_row(x, y, 4) == 1) {
        start_transition(ST_ACHIEVEMENTS);
    }
}

static void touch_modes(double x, double y) {
    if (hit_modes_row(x, y, 5) == 1) {
        winter_theme = 1 - winter_theme;
        apply_winter_theme();
        save_settings();
        return;
    }
    if (hit_modes_row(x, y, 0) == 1) {
        start_transition(ST_LOBBY);
        return;
    }
    if (hit_modes_row(x, y, 1) == 1) {
        start_transition(ST_SOLO);
        return;
    }
    if (hit_modes_row(x, y, 2) == 1) {
        open_online();
        return;
    }
    if (hit_modes_row(x, y, 3) == 1) {
        open_leaderboard();
        return;
    }
    if (hit_modes_row(x, y, 4) == 1) {
        start_transition(ST_PROMO);
    }
}

static void touch_settings(double x, double y) {
    if (hit_settings_row(x, y, 0) == 1) {
        start_transition(ST_LOBBY);
        return;
    }
    if (hit_settings_row(x, y, 1) == 1) {
        language = 0;
        save_settings();
        return;
    }
    if (hit_settings_row(x, y, 2) == 1) {
        language = 1;
        save_settings();
        return;
    }
    if (hit_settings_row(x, y, 3) == 1) {
        show_hitboxes = 1 - show_hitboxes;
        save_settings();
        return;
    }
    if (hit_settings_row(x, y, 4) == 1) {
        music_volume_cycle();
        return;
    }
    if (hit_settings_row(x, y, 5) == 1) {
        show_fps = 1 - show_fps;
        save_settings();
        return;
    }
    if (hit_settings_row(x, y, 6) == 1) {
        start_transition(ST_PRIVACY);
        return;
    }
}

static void touch_levels(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        start_transition(ST_CLASSES);
        return;
    }
    double n = 1;
    while (n <= max_level) {
        if (hit_level_row(x, y, n) == 1) {
            level_action(player_class, n);
            return;
        }
        n = n + 1;
    }
}

static void touch_skins(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        start_transition(ST_CLASSES);
        return;
    }
    double skin = 0;
    while (skin < SKIN_COUNT) {
        if (hit_skin_card(x, y, skin) == 1) {
            pick_skin(skin);
            return;
        }
        skin = skin + 1;
    }
}

static void touch_achievements(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        start_transition(ST_LOBBY);
    }
}

static void touch_leaderboard(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        start_transition(ST_MODES);
    }
}

static void touch_login(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (hit_login_field(x, y, login_nick_y()) == 1) {
        login_focus_nick();
        return;
    }
    if (hit_login_field(x, y, login_pass_y()) == 1) {
        login_focus_pass();
        return;
    }
    if (hit_login_btn(x, y) == 1) {
        login_do();
        keyboard_hide();
        return;
    }
    login_sync_field();
    keyboard_hide();
    if (back_hit(x, y) == 1) {
        keyboard_clear();
        if (login_after == ST_ONLINE) {
            start_transition(ST_MODES);
        } else {
            start_transition(ST_LOBBY);
        }
    }
}
