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
        return;
    }
    if (hit_lobby_quests(x, y) == 1) {
        quest_load();
        start_transition(ST_QUESTS);
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

static void touch_classes(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        start_transition(ST_LOBBY);
        return;
    }
    if (hit_levels_btn(x, y) == 1) {
        start_transition(ST_LEVELS);
        return;
    }
    if (hit_skins_btn(x, y) == 1) {
        start_transition(ST_SKINS);
        return;
    }
    double vi = 0;
    while (vi < classes_visible_count()) {
        if (hit_class_card(x, y, vi) == 1) {
            pick_class(visible_class_at(vi));
            return;
        }
        vi = vi + 1;
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
