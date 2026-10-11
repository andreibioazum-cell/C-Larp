/* Admin panel: a floating orb that only the two admin accounts see. Drag it
 * anywhere, tap it to open a stylish cheat panel with three tabs. Everything
 * here is session-local and gated behind is_admin(), the same check the admin
 * chat commands use, so a copied progress file cannot summon the orb. */

static double admin_a(double rgb, double a) {
    return (1 + floor(a * 254)) * 0x1000000 + rgb;
}

static double admin_in_battle(void) {
    return game_state == ST_SOLO || game_state == ST_ONLINE;
}

static double admin_panel_w(void) {
    double w = 304;
    if (w > screen_w - 12) {
        w = screen_w - 12;
    }
    return w;
}

static double admin_panel_h(void) {
    double h = 160;
    if (h > screen_h - 8) {
        h = screen_h - 8;
    }
    return h;
}

static double admin_panel_x(void) { return (screen_w - admin_panel_w()) / 2; }

static double admin_panel_y(void) { return (screen_h - admin_panel_h()) / 2; }

static double admin_pad(void) { return 8; }

static double admin_gap(void) { return 5; }

static double admin_row_h(void) { return 24; }

static double admin_ball_r(void) { return 13; }

static void admin_flash(const char *m) {
    admin_msg = m;
    admin_msg_state = -1;
    admin_msg_t = 1.6;
}

static void admin_flash_state(const char *label, double v) {
    admin_msg = label;
    admin_msg_state = v;
    admin_msg_t = 1.6;
}

static const char *admin_on_off(double v) {
    if (v == 1) {
        if (language == 1) {
            return "вкл";
        }
        return "on";
    }
    if (language == 1) {
        return "выкл";
    }
    return "off";
}

static const char *admin_solo_only(void) {
    if (language == 1) {
        return "Только в соло";
    }
    return "Solo only";
}

static const char *admin_tab_label(double t) {
    if (language == 1) {
        if (t == 0) {
            return "ЧИТЫ";
        }
        if (t == 1) {
            return "БОЙ";
        }
        return "ИГРА";
    }
    if (t == 0) {
        return "CHEATS";
    }
    if (t == 1) {
        return "FIGHT";
    }
    return "GAME";
}

static const char *admin_cheat_label(double idx) {
    if (language == 1) {
        if (idx == 0) {
            return "Бессмертие";
        }
        if (idx == 1) {
            return "Урон x5";
        }
        if (idx == 2) {
            return "Скорость x2";
        }
        if (idx == 3) {
            return "Без КД";
        }
        if (idx == 4) {
            return "Анти-стан";
        }
        return "Стан врага";
    }
    if (idx == 0) {
        return "God mode";
    }
    if (idx == 1) {
        return "Damage x5";
    }
    if (idx == 2) {
        return "Speed x2";
    }
    if (idx == 3) {
        return "No cooldown";
    }
    if (idx == 4) {
        return "Anti-stun";
    }
    return "Stun enemy";
}

static const char *admin_fight_label(double idx) {
    if (language == 1) {
        if (idx == 0) {
            return "Хилка";
        }
        if (idx == 1) {
            return "Убить врага";
        }
        if (idx == 2) {
            return "Победа";
        }
        if (idx == 3) {
            return "Телепорт";
        }
        if (idx == 4) {
            return "Диско";
        }
        return "FPS";
    }
    if (idx == 0) {
        return "Heal";
    }
    if (idx == 1) {
        return "Kill enemy";
    }
    if (idx == 2) {
        return "Instant win";
    }
    if (idx == 3) {
        return "Teleport";
    }
    if (idx == 4) {
        return "Disco";
    }
    return "FPS";
}

static const char *admin_game_label(double idx) {
    if (language == 1) {
        if (idx == 0) {
            return "+100 кубков";
        }
        if (idx == 1) {
            return "+100 конфет";
        }
        if (idx == 2) {
            return "Макс. уровень";
        }
        if (idx == 3) {
            return "Открыть всё";
        }
        if (idx == 4) {
            return "Зима";
        }
        return "Хитбоксы";
    }
    if (idx == 0) {
        return "+100 cups";
    }
    if (idx == 1) {
        return "+100 candies";
    }
    if (idx == 2) {
        return "Max level";
    }
    if (idx == 3) {
        return "Unlock all";
    }
    if (idx == 4) {
        return "Winter";
    }
    return "Hitboxes";
}

static double admin_cheat_value(double idx) {
    if (idx == 0) {
        return cheat_god;
    }
    if (idx == 1) {
        return cheat_damage;
    }
    if (idx == 2) {
        return cheat_speed;
    }
    if (idx == 3) {
        return cheat_nocd;
    }
    if (idx == 4) {
        return cheat_cc;
    }
    return cheat_estun;
}

static double admin_fight_enabled(double idx) {
    if (idx == 0) {
        return admin_in_battle() == 1 && finished == 0;
    }
    if (idx == 1 || idx == 2 || idx == 3) {
        return game_state == ST_SOLO && finished == 0;
    }
    return 1;
}

static void admin_max_level(void) {
    set_class_progress(player_class, max_level, max_level);
    sync_selected_class();
    if (game_state == ST_SOLO && finished == 0) {
        player_level = max_level;
    }
    save_progress();
    admin_flash(admin_game_label(2));
}

static void admin_unlock_all(void) {
    double c = 0;
    while (c < class_count) {
        if (c != CLASS_ASTRA_REWORK) {
            set_class_owned(c, 1);
            if (class_unlocked_of(c) < max_level) {
                set_class_progress(c, class_level_of(c), max_level);
            }
        }
        c = c + 1;
    }
    save_progress();
    admin_flash(admin_game_label(3));
}

static void admin_press_cheat(double idx) {
    double v = admin_cheat_value(idx);
    if (v == 1) {
        v = 0;
    } else {
        v = 1;
    }
    if (idx == 0) {
        cheat_god = v;
    }
    if (idx == 1) {
        cheat_damage = v;
    }
    if (idx == 2) {
        cheat_speed = v;
    }
    if (idx == 3) {
        cheat_nocd = v;
    }
    if (idx == 4) {
        cheat_cc = v;
    }
    if (idx == 5) {
        cheat_estun = v;
    }
    admin_flash_state(admin_cheat_label(idx), v);
}

static void admin_press_fight(double idx) {
    if (admin_fight_enabled(idx) != 1) {
        admin_flash(admin_solo_only());
        return;
    }
    if (idx == 0) {
        player->hp = player->max_hp;
        admin_flash("HP");
        return;
    }
    if (idx == 1) {
        enemy_apply_damage(enemy->hp + 1);
        return;
    }
    if (idx == 2) {
        finish_game(1);
        return;
    }
    if (idx == 3) {
        player->x = enemy->x;
        player->y = enemy->y;
        admin_flash("TP");
        return;
    }
    if (idx == 4) {
        if (net_event() == 0) {
            net_event_set(1);
        } else {
            net_event_set(0);
        }
        admin_flash("DISCO");
        return;
    }
    show_fps = 1 - show_fps;
    save_settings();
    admin_flash_state(admin_fight_label(5), show_fps);
}

static void admin_press_game(double idx) {
    if (idx == 0) {
        cups = cups + 100;
        save_progress();
        admin_flash("+100");
        return;
    }
    if (idx == 1) {
        candies = candies + 100;
        save_progress();
        admin_flash("+100");
        return;
    }
    if (idx == 2) {
        admin_max_level();
        return;
    }
    if (idx == 3) {
        admin_unlock_all();
        return;
    }
    if (idx == 4) {
        winter_theme = 1 - winter_theme;
        apply_winter_theme();
        save_settings();
        admin_flash_state(admin_game_label(4), winter_theme);
        return;
    }
    show_hitboxes = 1 - show_hitboxes;
    save_settings();
    admin_flash_state(admin_game_label(5), show_hitboxes);
}

static void admin_press(double idx) {
    play_button_click();
    if (admin_tab == 0) {
        admin_press_cheat(idx);
        return;
    }
    if (admin_tab == 1) {
        admin_press_fight(idx);
        return;
    }
    admin_press_game(idx);
}

static void admin_panel_press(double x, double y) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    double cxx = px + pw - 14;
    double dx = x - cxx;
    double dy = y - (py + 15);
    if (sqrt(dx * dx + dy * dy) <= 13) {
        admin_panel_open = 0;
        play_button_click();
        return;
    }
    if (hit_rect(x, y, px, py, pw, ph) == 0) {
        admin_panel_open = 0;
        return;
    }
    double tw = (pw - 2 * admin_pad() - 2 * admin_gap()) / 3;
    double t = 0;
    while (t < 3) {
        if (hit_rect(x, y, px + admin_pad() + t * (tw + admin_gap()), py + 36, tw, 20) == 1) {
            if (admin_tab != t) {
                admin_tab = t;
                play_button_click();
            }
            return;
        }
        t = t + 1;
    }
    double bw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double row = floor((y - (py + 62)) / (admin_row_h() + admin_gap()));
    if (row < 0 || row > 2 || x < px + admin_pad() || x > px + pw - admin_pad()) {
        return;
    }
    double col = 0;
    if (x >= px + admin_pad() + bw + admin_gap()) {
        col = 1;
    }
    admin_press(row * 2 + col);
}

static double admin_ball_hit(double x, double y) {
    double dx = x - admin_ball_x;
    double dy = y - admin_ball_y;
    return sqrt(dx * dx + dy * dy) <= admin_ball_r() + 9;
}

static double admin_touch(double x, double y, double action, double id) {
    (void)id;
    if (is_admin() == 0 || warn_open == 1 || studio_open == 1) {
        return 0;
    }
    /* The panel is modal: every touch while it is open goes to it. */
    if (admin_panel_open == 1) {
        if (action == 0) {
            admin_panel_press(x, y);
        }
        return 1;
    }
    if (admin_ball_grab == 1) {
        if (action == 2 || action == 1) {
            admin_ball_x = clamp(x - admin_ball_grab_dx, 16, screen_w - 16);
            admin_ball_y = clamp(y - admin_ball_grab_dy, 18, screen_h - 18);
        }
        if (action == 1 || action == 3) {
            admin_ball_grab = 0;
            double dx = x - admin_ball_px;
            double dy = y - admin_ball_py;
            if (action == 1 && sqrt(dx * dx + dy * dy) <= 11) {
                admin_panel_open = 1;
                play_button_click();
            }
        }
        return 1;
    }
    if (action == 0 && admin_ball_hit(x, y) == 1) {
        admin_ball_grab = 1;
        admin_ball_px = x;
        admin_ball_py = y;
        admin_ball_grab_dx = x - admin_ball_x;
        admin_ball_grab_dy = y - admin_ball_y;
        return 1;
    }
    return 0;
}

static void admin_draw_toggle(double x, double y, double w, double h, const char *label, double on) {
    if (on == 1) {
        roundrect(x, y, w, h, 9, admin_a(0x00E5FF, 0.88));
        text_in_box(label, x, y, w, h, 0x0D0D22, fit_text_scale(label, w - 12, 0.42));
        return;
    }
    roundrect(x, y, w, h, 9, admin_a(0x1D1D3C, 0.92));
    roundrect_outline(x, y, w, h, 9, 1, admin_a(0x4A4A7A, 0.9));
    text_in_box(label, x, y, w, h, 0x9AA3CC, fit_text_scale(label, w - 12, 0.42));
}

static void admin_draw_action(double x, double y, double w, double h, const char *label, double enabled) {
    if (enabled == 1) {
        roundrect(x, y, w, h, 9, admin_a(0x7C4DFF, 0.92));
        text_in_box(label, x, y, w, h, 0xFFFFFF, fit_text_scale(label, w - 12, 0.42));
        return;
    }
    roundrect(x, y, w, h, 9, locked_bg());
    text_in_box(label, x, y, w, h, locked_text(), fit_text_scale(label, w - 12, 0.42));
}

static void admin_draw_ball(void) {
    double r = admin_ball_r() + sin(admin_ball_t * 2.6) * 1.3;
    double cx = admin_ball_x;
    double cy = admin_ball_y;
    circle(cx, cy, r + 8, admin_a(0x00E5FF, 0.10));
    circle(cx, cy, r + 4, admin_a(0x7C4DFF, 0.16));
    circle(cx, cy, r, admin_a(0x141432, 0.92));
    circle(cx - r * 0.22, cy - r * 0.26, r * 0.62, admin_a(0x7C4DFF, 0.60));
    circle(cx + r * 0.30, cy + r * 0.34, r * 0.48, admin_a(0x00E5FF, 0.30));
    ring(cx, cy, r + 1, 1.6, admin_a(0x00E5FF, 0.85));
    circle(cx - r * 0.40, cy - r * 0.44, r * 0.20, admin_a(0xFFFFFF, 0.85));
    double sc = 0.5;
    double tw = text_ink_width("A") * sc;
    double th = text_ink_height("A") * sc;
    double tt = text_ink_top("A") * sc;
    text_scaled("A", cx - tw / 2, cy - th / 2 - tt, 0xFFD54F, sc);
}

static void admin_draw_panel(void) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    rect(0, 0, screen_w, screen_h, admin_a(0x000000, 0.55));
    roundrect(px - 3, py - 3, pw + 6, ph + 6, 17, admin_a(0x7C4DFF, 0.20));
    roundrect(px, py, pw, ph, 14, admin_a(0x0B0B1E, 0.97));
    roundrect_outline(px, py, pw, ph, 14, 1.5, admin_a(0x00E5FF, 0.55));
    /* Header: violet band, golden title, the admin's own nick, red close orb. */
    roundrect(px, py, pw, 30, 14, admin_a(0x241A4D, 0.98));
    rect(px, py + 15, pw, 15, admin_a(0x241A4D, 0.98));
    line(px + 10, py + 29, px + pw - 10, py + 29, 1.2, admin_a(0xFFD54F, 0.45));
    text_in_box("ADMIN", px + 8, py, 70, 30, 0xFFD54F, 0.52);
    const char *nick = net_login_nick();
    text_in_box(nick, px + 80, py, pw - 110, 30, admin_color_of(nick, text_color), 0.3);
    double cxx = px + pw - 14;
    circle(cxx, py + 15, 10, admin_a(0xE94560, 0.95));
    text_in_box("X", cxx - 10, py + 5, 20, 20, 0xFFFFFF, 0.36);
    /* Tabs. */
    double tw = (pw - 2 * admin_pad() - 2 * admin_gap()) / 3;
    double t = 0;
    while (t < 3) {
        double tx = px + admin_pad() + t * (tw + admin_gap());
        const char *tl = admin_tab_label(t);
        if (admin_tab == t) {
            roundrect(tx, py + 36, tw, 20, 8, admin_a(0x00E5FF, 0.9));
            text_in_box(tl, tx, py + 36, tw, 20, 0x0D0D22, fit_text_scale(tl, tw - 10, 0.4));
        } else {
            roundrect(tx, py + 36, tw, 20, 8, admin_a(0x1D1D3C, 0.92));
            text_in_box(tl, tx, py + 36, tw, 20, 0x9AA3CC, fit_text_scale(tl, tw - 10, 0.4));
        }
        t = t + 1;
    }
    /* Two-column grid of controls. */
    double bw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double idx = 0;
    while (idx < 6) {
        double row = floor(idx / 2);
        double col = idx - row * 2;
        double bx = px + admin_pad() + col * (bw + admin_gap());
        double by = py + 62 + row * (admin_row_h() + admin_gap());
        if (admin_tab == 0) {
            admin_draw_toggle(bx, by, bw, admin_row_h(), admin_cheat_label(idx), admin_cheat_value(idx));
        } else if (admin_tab == 1) {
            admin_draw_action(bx, by, bw, admin_row_h(), admin_fight_label(idx), admin_fight_enabled(idx));
        } else {
            admin_draw_action(bx, by, bw, admin_row_h(), admin_game_label(idx), 1);
        }
        idx = idx + 1;
    }
    if (admin_msg_t > 0 && admin_msg != 0 && str_len(admin_msg) > 0) {
        const char *m = admin_msg;
        if (admin_msg_state >= 0) {
            m = ds_concat(ds_concat(admin_msg, ": "), admin_on_off(admin_msg_state));
        }
        ctext_scaled(m, py + ph - 13, 0xFFD54F, 0.34);
    }
}

static void admin_draw(void) {
    if (is_admin() == 0 || warn_open == 1 || studio_open == 1 || t_dir != 0) {
        return;
    }
    if (admin_ball_x == 0 && admin_ball_y == 0) {
        admin_ball_x = screen_w - 30;
        admin_ball_y = screen_h * 0.42;
    }
    if (admin_panel_open == 1) {
        admin_draw_panel();
    }
    admin_draw_ball();
}

/* Continuous cheats: ticked from logic_update so anti-stun and the frozen
 * enemy survive every re-application the fight throws at the player. */
static void admin_tick(void) {
    admin_ball_t = admin_ball_t + dt;
    if (admin_msg_t > 0) {
        admin_msg_t = tick_down(admin_msg_t);
    }
    if ((cheat_cc != 1 && cheat_estun != 1) || is_admin() == 0 || admin_in_battle() == 0 || finished != 0) {
        return;
    }
    if (cheat_cc == 1) {
        player_stun = 0;
        player_freeze = 0;
        player_poison = 0;
    }
    if (cheat_estun == 1 && game_state == ST_SOLO) {
        if (enemy->stun < 0.25) {
            enemy->stun = 0.25;
        }
    }
}

static double admin_cheat_god(void) {
    return cheat_god;
}

static double admin_cheat_damage_mult(void) {
    if (cheat_damage == 1) {
        return 5;
    }
    return 1;
}

static double admin_cheat_speed_mult(void) {
    if (cheat_speed == 1) {
        return 2;
    }
    return 1;
}

static double admin_cheat_nocd(void) {
    return cheat_nocd;
}
