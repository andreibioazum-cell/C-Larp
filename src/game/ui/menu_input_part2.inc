static double menu_button_hit(double x, double y) {
    double i = 0;
    if (game_state == ST_LOBBY) {
        while (i <= 4) {
            if (hit_menu_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return hit_lobby_quests(x, y);
    }
    if (game_state == ST_MODES) {
        while (i <= 5) {
            if (hit_modes_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_SETTINGS) {
        while (i <= 6) {
            if (hit_settings_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_CLASSES) {
        if (back_hit(x, y) == 1 || hit_levels_btn(x, y) == 1 || hit_skins_btn(x, y) == 1) return 1;
        while (i < classes_visible_count()) {
            if (hit_class_card(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_LEVELS) {
        if (back_hit(x, y) == 1) return 1;
        i = 1;
        while (i <= max_level) {
            if (hit_level_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_SKINS) {
        if (back_hit(x, y) == 1) return 1;
        while (i < SKIN_COUNT) {
            if (hit_skin_card(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_LOGIN) {
        return hit_login_btn(x, y) == 1 || back_hit(x, y) == 1;
    }
    if (game_state == ST_PROMO) {
        double w = fit_w(560);
        double fx = center_x(w);
        return back_hit(x, y) == 1 || hit_rect_pad(x, y, fx, promo_btn_y(), w, btn_h) == 1;
    }
    if (game_state == ST_ACHIEVEMENTS || game_state == ST_LEADERBOARD || game_state == ST_PRIVACY ||
        game_state == ST_QUESTS) {
        return back_hit(x, y);
    }
    return 0;
}

static void touch_menu(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (game_state == ST_MODES) {
        touch_modes(x, y);
        return;
    }
    if (game_state == ST_SETTINGS) {
        touch_settings(x, y);
        return;
    }
    if (game_state == ST_LOGIN) {
        touch_login(x, y, action);
        return;
    }
    if (game_state == ST_CLASSES) {
        touch_classes(x, y, action);
        return;
    }
    if (game_state == ST_LEVELS) {
        touch_levels(x, y, action);
        return;
    }
    if (game_state == ST_SKINS) {
        touch_skins(x, y, action);
        return;
    }
    if (game_state == ST_ACHIEVEMENTS) {
        touch_achievements(x, y, action);
        return;
    }
    if (game_state == ST_LEADERBOARD) {
        touch_leaderboard(x, y, action);
        return;
    }
    if (game_state == ST_PROMO) {
        touch_promo(x, y, action);
        return;
    }
    if (game_state == ST_QUESTS) {
        touch_quests(x, y, action);
        return;
    }
    if (game_state == ST_PRIVACY) {
        if (back_hit(x, y) == 1) {
            start_transition(ST_SETTINGS);
        }
    }
}
