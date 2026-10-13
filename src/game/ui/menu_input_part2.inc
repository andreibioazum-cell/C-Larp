static void classes_swipe_reset(void) {
    classes_swipe_id = -1;
    classes_swipe_card = -1;
}

static double classes_swipe_threshold(void) { return clamp(screen_w * 0.08, 48, 88); }

static double classes_button_hit(double x, double y) {
    return back_hit(x, y) == 1 || hit_levels_btn(x, y) == 1 || hit_skins_btn(x, y) == 1 ||
           hit_classes_prev(x, y) == 1 || hit_classes_next(x, y) == 1;
}

static void touch_classes(double x, double y, double action, double id) {
    if (action == 3) {
        classes_swipe_reset();
        return;
    }
    if (action == 2) {
        /* A drag far enough sideways pages the list there and then; one swipe
         * is one page, so the finger has to come back for the next one. */
        if (classes_swipe_id != id || classes_swipe_id == -1) {
            return;
        }
        double dx = x - classes_swipe_start_x;
        double dy = y - classes_swipe_start_y;
        if (fabs(dx) < classes_swipe_threshold() || fabs(dx) < fabs(dy) * 1.2) {
            return;
        }
        classes_swipe_reset();
        if (dx < 0) {
            classes_page_next();
        } else {
            classes_page_prev();
        }
        return;
    }
    if (action == 1) {
        /* The card body picks when the finger lifts on it: that way a swipe
         * started on a card pages the list instead of buying a class. */
        double vi = classes_swipe_card;
        classes_swipe_reset();
        if (vi >= 0 && hit_class_card(x, y, vi) == 1) {
            pick_class(visible_class_at(vi));
        }
        return;
    }
    /* action == 0: every button acts on the touch itself. */
    if (classes_button_hit(x, y) == 1) {
        play_button_click();
        if (back_hit(x, y) == 1) {
            start_transition(ST_LOBBY);
        } else if (hit_levels_btn(x, y) == 1) {
            start_transition(ST_LEVELS);
        } else if (hit_skins_btn(x, y) == 1) {
            start_transition(ST_SKINS);
        } else if (hit_classes_prev(x, y) == 1) {
            classes_page_prev();
        } else {
            classes_page_next();
        }
        return;
    }
    double vi = classes_page_start();
    double last = vi + classes_page_items();
    while (vi < last) {
        if (hit_class_card(x, y, vi) == 1) {
            if (hit_class_pill(x, y, vi) == 1) {
                /* The pill is a button: it buys or picks there and then. */
                play_button_click();
                pick_class(visible_class_at(vi));
            } else {
                /* The rest of the card only takes the press as feedback; the
                 * pick waits for the release so swiping still pages. */
                play_button_click();
                classes_swipe_id = id;
                classes_swipe_start_x = x;
                classes_swipe_start_y = y;
                classes_swipe_card = vi;
            }
            return;
        }
        vi = vi + 1;
    }
    if (classes_swipe_zone(x, y) == 1) {
        classes_swipe_id = id;
        classes_swipe_start_x = x;
        classes_swipe_start_y = y;
        classes_swipe_card = -1;
    }
}


static double menu_button_hit(double x, double y) {
    double i = 0;
    if (game_state == ST_SOLO || game_state == ST_ONLINE) {
        /* Battles: only real buttons click - the intro card dismiss, the
         * joysticks and the attack pads stay silent, while the back button,
         * the chat controls and the results tap give feedback. */
        if (card_open != 0) {
            return 0;
        }
        if (finished != 0) {
            return finish_t >= alert_hold;
        }
        if (game_state == ST_ONLINE && chat_open == 1) {
            return chat_button_hit(x, y);
        }
        return battle_back_hit(x, y);
    }
    if (game_state == ST_LOBBY) {
        while (i <= 4) {
            if (hit_menu_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_MODES) {
        while (i <= 5) {
            if (hit_modes_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_SETTINGS) {
        while (i <= 7) {
            if (hit_settings_row(x, y, i) == 1) return 1;
            i = i + 1;
        }
        return 0;
    }
    if (game_state == ST_CLASSES) {
        if (back_hit(x, y) == 1 || hit_levels_btn(x, y) == 1 || hit_skins_btn(x, y) == 1 ||
            hit_classes_prev(x, y) == 1 || hit_classes_next(x, y) == 1) return 1;
        i = classes_page_start();
        double last = i + classes_page_items();
        while (i < last) {
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
    if (game_state == ST_ACHIEVEMENTS || game_state == ST_LEADERBOARD || game_state == ST_PRIVACY) {
        return back_hit(x, y);
    }
    return 0;
}

static void touch_menu(double x, double y, double action, double id) {
    if (game_state == ST_CLASSES) {
        touch_classes(x, y, action, id);
        return;
    }
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
    if (game_state == ST_PRIVACY) {
        if (back_hit(x, y) == 1) {
            start_transition(ST_SETTINGS);
        }
    }
}
