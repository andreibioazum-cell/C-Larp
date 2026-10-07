static void draw_menu_button(double row, const char *label) {
    button(label, menu_x(), menu_row(row), btn_w, btn_h);
}

static void draw_lobby_nick(void) {
    if (net_login_status() != 2) {
        return;
    }
    const char *nick = net_login_nick();
    if (str_len(nick) == 0) {
        return;
    }
    const char *pre = tr_hello_pre();
    const char *post = tr_hello_post();
    double scale = 0.5;
    double pw = text_ink_width(pre) * scale;
    double nw = text_ink_width(nick) * scale;
    double qw = text_ink_width(post) * scale;
    double x = (screen_w - pw - nw - qw) / 2;
    double y = lobby_hello_y();
    text_scaled(pre, x, y, text_color, scale);
    text_scaled(nick, x + pw, y, admin_color_of(nick, text_color), scale);
    text_scaled(post, x + pw + nw, y, text_color, scale);
}

static void draw_lobby_money(void) {
    const char *cup_s = ds_concat(ds_concat(tr_cups(), ": "), ds_num_to_string((double)(cups)));
    if (candy_enabled == 0) {
        ctext_scaled(cup_s, lobby_money_y(), text_color, money_scale);
        return;
    }
    const char *cd_s = ds_concat(ds_concat(tr_candies(), ": "), ds_num_to_string((double)(candies)));
    money_pair(cup_s, cd_s, lobby_money_y(), money_scale, money_gap);
}

static void draw_lobby(void) {
    check_achievements();
    screen_fill();
    ctext("Cubic Battle 4", lobby_title_y(), text_color);
    if (newyear_active() == 1) {
        ctext_scaled(tr_newyear_banner(), lobby_title_y() + 34, newyear_gold + 0xFF000000, 0.6);
    }
    draw_menu_button(0, tr_play());
    draw_menu_button(1, tr_shop());
    draw_menu_button(2, tr_settings());
    draw_menu_button(3, tr_account());
    draw_menu_button(4, tr_achievements());
    button(tr_quests(), lobby_quests_x(), lobby_quests_y(), btn_w, btn_h);
    draw_lobby_nick();
    draw_lobby_money();
    if (str_len(banned_msg) > 0) {
        ctext(banned_msg, lobby_msg_y(), text_color);
    }
}

static void draw_modes(void) {
    screen_fill();
    button(tr_back(), menu_x(), modes_row(0), btn_w, btn_h);
    button(tr_solo(), menu_x(), modes_row(1), btn_w, btn_h);
    button(tr_online(), menu_x(), modes_row(2), btn_w, btn_h);
    button(tr_top(), menu_x(), modes_row(3), btn_w, btn_h);
    button(tr_promo(), menu_x(), modes_row(4), btn_w, btn_h);
    button_scaled(tr_winter_toggle(), menu_x(), modes_row(5), btn_w, btn_h, 0.8);
}

static void draw_settings(void) {
    screen_fill();
    button(tr_back(), menu_x(), settings_row_y(0), btn_w, settings_btn_h());
    button("English", menu_x(), settings_row_y(1), btn_w, settings_btn_h());
    button("Русский", menu_x(), settings_row_y(2), btn_w, settings_btn_h());
    button(tr_hitboxes_toggle(), menu_x(), settings_row_y(3), btn_w, settings_btn_h());
    button(tr_music_state(), menu_x(), settings_row_y(4), btn_w, settings_btn_h());
    button_scaled(tr_fps_meter_toggle(), menu_x(), settings_row_y(5), btn_w, settings_btn_h(), 0.8);
    button_scaled(tr_privacy(), menu_x(), settings_row_y(6), btn_w, settings_btn_h(), 0.8);
}

static void draw_privacy(void) {
    screen_fill();
    draw_back();
    double top = screen_h / 2 - 160;
    if (top < 96) {
        top = 96;
    }
    ctext_scaled(tr_privacy_title(), top, text_color, 0.9);
    double y = top + 54;
    ctext_scaled(tr_privacy_line1(), y, text_color, 0.5);
    ctext_scaled(tr_privacy_line2(), y + 26, text_color, 0.5);
    ctext_scaled(tr_privacy_line3(), y + 52, text_color, 0.5);
    ctext_scaled(tr_privacy_line4(), y + 78, text_color, 0.5);
    ctext_scaled(tr_privacy_line5(), y + 104, text_color, 0.5);
    ctext_scaled(tr_privacy_line6(), y + 130, text_color, 0.5);
    ctext_scaled(tr_privacy_line7(), y + 156, text_color, 0.5);
    ctext_scaled(tr_privacy_line8(), y + 182, text_color, 0.5);
    ctext_scaled(tr_privacy_line9(), y + 208, text_color, 0.5);
    ctext_scaled(tr_privacy_line10(), y + 234, text_color, 0.5);
}

static void draw_card_look(double look, double x, double y, double scale) {
    if (look != LOOK_NONE) {
        tex(x, y, look_idle_tex(look), 0, scale);
        return;
    }
    double side = 50 * scale;
    rect(x, y, side, side, fighter_fallback_rgb);
}

/* Draws the shop/lobby render for a class+skin combo into a target_px square
 * at (x, y), the same box the old tiny battle sprite used to occupy. Prefers
 * the high-resolution card art; while it hasn't finished uploading (or is
 * missing) it falls back to the familiar small pixel sprite scaled to fill
 * the same box, so the shop never shows a blank hole.
 *
 * The card art itself is a plain square PNG - tex_round() rounds its corners
 * at draw time (same silhouette the card background panel already uses),
 * so the source art can stay a simple square image. */
static void draw_class_render(double cls, double skin, double x, double y, double target_px) {
    if (class_card_render_ready(cls, skin) == 1) {
        tex_round(x, y, class_card_tex(cls, skin), target_px / CARD_RENDER_PX, target_px * card_render_radius_frac);
        return;
    }
    draw_card_look(card_look(cls, skin), x, y, target_px / 50);
}

static void draw_class_card(double vi) {
    double cls = visible_class_at(vi);
    double x = classes_card_x(vi);
    double w = classes_card_w();
    double selected = 0;
    if (player_class == cls && class_owned_of(cls) == 1) {
        selected = 1;
    }
    card(x, classes_top(), w, class_card_h, selected);
    double sprite = class_render_px();
    double sx = x + (w - sprite) / 2;
    /* The shop card always shows the class's base look, regardless of which
     * skin is currently equipped - picking a skin in the Skins screen should
     * not change what the character looks like in the class list. */
    draw_class_render(cls, SKIN_NORMAL, sx, class_sprite_y(), sprite);
    centered_text(class_name_of(cls), x, w, class_title_y(), text_color, class_title_scale());
    const char *motto = class_desc_of(cls);
    centered_text(motto, x, w, class_desc_y(), text_color, fit_text_scale(motto, w - 16, 0.5));
    centered_text(ds_concat(ds_concat(tr_level(), " "), ds_num_to_string((double)(class_level_of(cls)))), x, w,
                  class_level_y(), text_color, 0.45);
    button_scaled(class_pick_label(cls), x + class_pick_inset(), class_pick_y(), class_pick_w(), class_pick_h, 0.55);
}

static void draw_stat_row(const char *label, double x, double y, double w, double h, double cur, double maxv,
                          uint32_t c) {
    text_scaled(label, x, y - 26, text_color, 0.5);
    hud_bar(x, y, w, h, cur, maxv, c, 0);
}

static void draw_class_stats(void) {
    double w = stat_bar_w();
    double x = center_x(w);
    double hp = class_hp_of(player_class);
    double power = class_power_of(player_class);
    const char *smotto = class_desc_of(player_class);
    ctext_scaled(smotto, stat_motto_y(), text_color, fit_text_scale(smotto, screen_w - 2 * screen_margin, 0.8));
    draw_stat_row(ds_concat(ds_concat(tr_hp(), ": "), ds_num_to_string((double)(hp))), x, stat_hp_y(), w, stat_bar_h(),
                  hp, 20, stat_hp_color);
    draw_stat_row(ds_concat(ds_concat(tr_strength(), ": "), ds_num_to_string((double)(power))), x, stat_power_y(), w,
                  stat_bar_h(), power, 2, stat_power_color);
}

static void draw_classes(void) {
    screen_fill();
    double vi = 0;
    while (vi < classes_visible_count()) {
        draw_class_card(vi);
        vi = vi + 1;
    }
    button_scaled(tr_levels(), levels_btn_x(), class_actions_y(), class_action_w(), btn_h, 0.9);
    if (skins_btn_visible() == 1) {
        button_scaled(tr_skins(), skins_btn_x(), class_actions_y(), class_action_w(), btn_h, 0.9);
    }
    class_msg_t = tick_down(class_msg_t);
    draw_msg(class_msg_text(), class_msg_y(), class_msg_t);
    draw_class_stats();
    draw_back();
}

static void draw_level_row(double n) {
    double x = levels_x();
    double w = levels_w();
    double y = levels_row_y(n);
    double selected = 0;
    if (player_level == n && n <= levels_unlocked) {
        selected = 1;
    }
    card(x, y, w, level_row_h, selected);
    centered_text(ds_concat(ds_concat(tr_level(), " "), ds_num_to_string((double)(n))), x, w, y + 10, text_color, 0.62);
    centered_text(tr_class_level_effect(player_class, n), x, w, y + 34, text_color, 0.46);
    centered_text(level_status_of(player_class, n), x, w, y + 52, text_color, 0.42);
}

static void draw_levels(void) {
    screen_fill();
    double n = 1;
    while (n <= max_level) {
        draw_level_row(n);
        n = n + 1;
    }
    level_msg_t = tick_down(level_msg_t);
    draw_msg(level_msg_text(), levels_msg_y(), level_msg_t);
    draw_back();
}

static void draw_skin_card(double skin) {
    double x = skins_card_x(skin);
    double w = skin_card_w;
    double locked = 1 - skin_unlocked(skin);
    double selected = 0;
    if (current_skin() == skin) {
        selected = 1;
    }
    card_state(x, skins_top(), w, skin_card_h, selected, locked);
    double sprite = 50 * skin_preview_scale();
    double sx = x + (w - sprite) / 2;
    draw_class_render(player_class, skin, sx, skin_preview_y(), sprite);
    uint32_t c = panel_text(locked);
    const char *status = skin_hint_for(player_class);
    if (locked == 0) {
        status = tr_class_pick();
        if (selected == 1) {
            status = tr_class_selected();
        }
    }
    centered_text(skin_name_of(skin), x, w, skin_name_y(), c, 0.8);
    centered_text(status, x, w, skin_status_y(), c, 0.45);
}

static void draw_skins(void) {
    screen_fill();
    double skin = 0;
    while (skin < SKIN_COUNT) {
        draw_skin_card(skin);
        skin = skin + 1;
    }
    skin_msg_t = tick_down(skin_msg_t);
    draw_msg(tr_skins_need_legends(), skins_msg_y(), skin_msg_t);
    draw_back();
}

static double achievement_count(void) {
    return 3;
}

static double achievement_bit(double i) {
    if (i == 1) {
        return ACH_FIRST_WIN;
    }
    if (i == 2) {
        return ACH_ALL_CHARACTERS;
    }
    return ACH_LEGENDS;
}

static const char *achievement_title(double i) {
    if (i == 1) {
        return tr_achievement_first_win();
    }
    if (i == 2) {
        return tr_achievement_grass();
    }
    return tr_achievement_legends();
}

static const char *achievement_desc(double i) {
    if (i == 1) {
        return tr_achievement_first_win_desc();
    }
    if (i == 2) {
        return tr_achievement_grass_desc();
    }
    return tr_achievement_legends_desc();
}

static void draw_achievement_card(double i) {
    double locked = 1 - has_achievement(achievement_bit(i));
    double x = ach_x();
    double y = ach_card_y(i);
    panel(x, y, ach_w(), ach_card_h, locked);
    uint32_t c = panel_text(locked);
    const char *status = tr_achievement_open();
    if (locked == 1) {
        status = tr_achievement_locked();
    }
    text_scaled(achievement_title(i), x + card_text_pad, y + 14, c, 0.62);
    text_scaled(achievement_desc(i), x + card_text_pad, y + 46, c, 0.46);
    text_scaled(status, x + card_text_pad, y + 70, c, 0.42);
}

static void draw_achievements(void) {
    check_achievements();
    screen_fill();
    double i = 0;
    while (i < achievement_count()) {
        draw_achievement_card(i);
        i = i + 1;
    }
    draw_back();
}

static double lb_place_bg(double i) {
    if (i == 0) {
        return 0xFF3D3215;
    }
    if (i == 1) {
        return 0xFF2E2E30;
    }
    if (i == 2) {
        return 0xFF3A2414;
    }
    return card_bg;
}

static void draw_lb_row(double i) {
    double x = lb_x();
    double y = lb_row_y(i);
    double w = lb_w();
    roundrect(x, y, w, lb_row_h, 12, lb_place_bg(i));
    const char *place = ds_concat("#", ds_num_to_string((double)((i + 1))));
    const char *nick = net_leaderboard_nick(i);
    const char *cup_s = ds_concat("", ds_num_to_string((double)(floor(net_leaderboard_cups(i) + 0.5))));
    double scale = 0.6;
    double th = text_ink_height(place) * scale;
    double tt = text_ink_top(place) * scale;
    double ty = y + (lb_row_h - th) / 2 - tt + 1;
    text_scaled(place, x + card_text_pad, ty, text_color, scale);
    text_scaled(nick, x + 70, ty, text_color, scale);
    double cw = text_ink_width(cup_s) * scale;
    text_scaled(cup_s, x + w - card_text_pad - cw, ty, text_color, scale);
}

static void draw_leaderboard(void) {
    screen_fill();
    double st = net_leaderboard_status();
    if (st == 1) {
        lb_wait = lb_wait + dt;
        if (lb_wait > lb_wait_max) {
            ctext(tr_no_connection(), screen_h / 2, text_color);
        } else {
            ctext(tr_loading(), screen_h / 2, text_color);
        }
        draw_back();
        return;
    }
    if (st == 4) {
        ctext(tr_no_connection(), screen_h / 2, text_color);
        draw_back();
        return;
    }
    double count = net_leaderboard_count();
    if (count == 0) {
        ctext(tr_no_data(), screen_h / 2, text_color);
    } else {
        double i = 0;
        while (i < count) {
            draw_lb_row(i);
            i = i + 1;
        }
    }
    draw_back();
}
