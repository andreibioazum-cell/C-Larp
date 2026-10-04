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
