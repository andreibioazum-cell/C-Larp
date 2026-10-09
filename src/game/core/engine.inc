static void start_transition(double target) {
    if (t_dir != 0) {
        return;
    }
    classes_swipe_reset();
    t_target = target;
    t_dir = 1;
}

static void leave_screen(double state) {
    if (state == ST_LOGIN || state == ST_PROMO) {
        keyboard_hide();
    }
    if (state == ST_ONLINE) {
        chat_set_open(0);
        net_disconnect();
        event_mode = 0;
        event_t = 0;
    }
    if (state == ST_SOLO || state == ST_ONLINE) {
        save_progress();
    }
}

static double enter_screen(double state) {
    if (state == ST_SOLO) {
        init_game();
        return 1;
    }
    if (state == ST_LOGIN) {
        login_open();
        return 1;
    }
    if (state == ST_LEADERBOARD) {
        open_leaderboard();
        return 1;
    }
    if (state == ST_ONLINE) {
        if (net_is_banned(net_login_nick()) == 1) {
            banned_msg = tr_banned();
            game_state = ST_LOBBY;
            t_dir = -1;
            t_fade = 1;
            return 0;
        }
        init_online();
    }
    return 1;
}

static void update_transition(void) {
    if (t_dir == 1) {
        t_fade = t_fade + dt / transition_duration;
        if (t_fade >= 1) {
            t_fade = 1;
            t_dir = 2;
        }
        return;
    }
    if (t_dir == 2) {
        leave_screen(game_state);
        game_state = t_target;
        if (enter_screen(t_target) == 1) {
            t_dir = -1;
        }
        return;
    }
    if (t_dir == -1) {
        t_fade = t_fade - dt / transition_duration;
        if (t_fade <= 0) {
            t_fade = 0;
            t_dir = 0;
        }
    }
}

static void play_button_click(void) {
    if (button_click_ok == 1) {
        /* The sample is preloaded and starts at frame zero, so touch feedback
         * never waits for file decoding or for the finger to be released. */
        snd_play(BUTTON_CLICK_SOUND);
    }
}

static void start_music(void) {
    music_ok = snd_load(LOBBY_MUSIC);
    if (music_ok == 0) {
        ds_log("init: no lobby music");
        return;
    }
    music_level = 1;
    snd_volume(LOBBY_MUSIC, music_volume / 100);
    snd_loop(LOBBY_MUSIC);
    ds_log("init: lobby music on");
}

static void logic_init(void) {
    ds_log("init: loading textures");
    load_textures();
    button_click_ok = snd_load(BUTTON_CLICK_SOUND);
    if (button_click_ok == 1) {
        snd_volume(BUTTON_CLICK_SOUND, 0.7);
    }
    net_set_firebase_key(FB_KEY);
    net_autologin(FIREBASE);
    progress_from_storage();
    settings_from_storage();
    apply_winter_theme();
    start_music();
    showdown_music_load();
    update_newyear_jingle();
    ds_log("init: ready");
}

static void update_music(void) {
    if (music_ok == 0) {
        return;
    }
    if (game_state == ST_SOLO || game_state == ST_ONLINE || newyear_music_active() == 1) {
        music_level = clamp(music_level - dt / music_fade_out_time, 0, 1);
        snd_volume(LOBBY_MUSIC, music_volume / 100 * music_level);
        if (music_level <= 0 && snd_playing(LOBBY_MUSIC) == 1) {
            snd_stop(LOBBY_MUSIC);
        }
        return;
    }
    if (snd_playing(LOBBY_MUSIC) == 0) {
        music_level = 0;
        snd_loop(LOBBY_MUSIC);
    }
    music_level = clamp(music_level + dt / music_fade_in_time, 0, 1);
    snd_volume(LOBBY_MUSIC, music_volume / 100 * music_level);
}

static void update_warning(void) {
    if (warn_open != 1) {
        return;
    }
    if (warn_closing == 1) {
        warn_close_t = clamp(warn_close_t + dt / warn_fade, 0, 1);
        double p = warn_close_t;
        warn_a = 1 - p * p * (3 - 2 * p);
        if (warn_close_t >= 1) {
            warn_a = 0;
            warn_open = 0;
            warn_closing = 0;
        }
        return;
    }
    warn_a = 1;
    warn_t = warn_t + dt;
    warn_ready = clamp(warn_t / warn_wait, 0, 1);
}

static void update_studio(void) {
    if (studio_open != 1) {
        return;
    }
    studio_t = studio_t + dt;
    if (studio_t < studio_in) {
        studio_a = studio_t / studio_in;
        return;
    }
    if (studio_t <= studio_in + studio_hold) {
        studio_a = 1;
        studio_bg_a = 1;
        return;
    }
    studio_a = 1 - (studio_t - studio_in - studio_hold) / studio_fade;
    studio_bg_a = studio_a;
    if (studio_a <= 0) {
        studio_a = 0;
        studio_bg_a = 0;
        studio_open = 0;
    }
}

static void update_alpha_notice(void) {
    if (alpha_notice_done == 1) {
        return;
    }
    if (warn_open == 1 || studio_open == 1) {
        alpha_notice_t = 0;
        return;
    }
    alpha_notice_t = alpha_notice_t + dt;
    if (alpha_notice_t < alpha_notice_delay) {
        return;
    }
    alpha_notice_done = 1;
    alpha_notice_show(language);
}

static void tick_achievement_toast(void) {
    ach_toast_t = tick_down(ach_toast_t);
}

static void tick_fps_meter(void) {
    if (show_fps == 0) {
        fps_acc = 0;
        fps_frames = 0;
        return;
    }
    fps_acc = fps_acc + dt;
    fps_frames = fps_frames + 1;
    if (fps_acc < fps_interval) {
        return;
    }
    if (fps_acc > 0) {
        fps_value = fps_frames / fps_acc;
    }
    fps_acc = 0;
    fps_frames = 0;
}

static const char *achievement_toast_text(void) {
    if (ach_toast_bit == ACH_FIRST_WIN) {
        return tr_achievement_first_win();
    }
    if (ach_toast_bit == ACH_ALL_CHARACTERS) {
        return tr_achievement_grass();
    }
    return tr_achievement_legends();
}

static void draw_achievement_toast(void) {
    if (ach_toast_bit <= 0 || ach_toast_t <= 0) {
        return;
    }
    double elapsed = ACH_TOAST_DURATION - ach_toast_t;
    double w = fit_w(460);
    double h = 70;
    double x = center_x(w);
    double home_y = 64;
    double off = home_y + h + 8;
    double y = home_y;
    if (elapsed < ACH_TOAST_FADE_IN) {
        double k = elapsed / ACH_TOAST_FADE_IN;
        double ease = 1 - (1 - k) * (1 - k);
        y = home_y - off * (1 - ease);
    } else if (elapsed > ACH_TOAST_FADE_IN + ACH_TOAST_HOLD) {
        double k2 = (elapsed - ACH_TOAST_FADE_IN - ACH_TOAST_HOLD) / ACH_TOAST_EXIT;
        y = home_y - off * clamp(k2, 0, 1) * clamp(k2, 0, 1);
    }
    if (y + h <= 0) {
        return;
    }
    roundrect(x, y, w, h, btn_radius, ui_purple);
    const char *head = tr_achievement_toast_title();
    const char *body = achievement_toast_text();
    double hs = 0.42;
    double bs = 0.62;
    double gap = 6;
    double hh = text_ink_height(head) * hs;
    double ht = text_ink_top(head) * hs;
    double bh = text_ink_height(body) * bs;
    double bt = text_ink_top(body) * bs;
    double top = y + (h - hh - gap - bh) / 2;
    ctext_scaled(head, top - ht, text_color, hs);
    ctext_scaled(body, top + hh + gap - bt, text_color, bs);
}

static void logic_update(void) {
    update_warning();
    update_studio();
    update_alpha_notice();
    update_transition();
    update_music();
    update_showdown_music();
    reload_textures();
    tick_achievement_toast();
    tick_fps_meter();
    if (game_state == ST_SOLO) {
        update_game();
    } else if (game_state == ST_ONLINE) {
        update_online();
    } else if (game_state == ST_LOGIN) {
        update_login();
    } else if (game_state == ST_PROMO) {
        update_promo();
    }
    update_newyear();
    update_weather();
}

static void draw_screen(void) {
    if (game_state == ST_LOBBY) {
        draw_lobby();
    } else if (game_state == ST_SOLO) {
        draw_game();
    } else if (game_state == ST_MODES) {
        draw_modes();
    } else if (game_state == ST_SETTINGS) {
        draw_settings();
    } else if (game_state == ST_LOGIN) {
        draw_login();
    } else if (game_state == ST_CLASSES) {
        draw_classes();
    } else if (game_state == ST_LEVELS) {
        draw_levels();
    } else if (game_state == ST_SKINS) {
        draw_skins();
    } else if (game_state == ST_LEADERBOARD) {
        draw_leaderboard();
    } else if (game_state == ST_ACHIEVEMENTS) {
        draw_achievements();
    } else if (game_state == ST_PROMO) {
        draw_promo();
    } else if (game_state == ST_PRIVACY) {
        draw_privacy();
    } else {
        draw_online();
    }
}

static void logic_draw(void) {
    if (t_fade >= 1) {
        rect(0, 0, screen_w, screen_h, 0xFF000000);
    } else {
        draw_screen();
        /* Snow beneath the transition curtain. */
        draw_newyear_under_notices();
        if (t_fade > 0) {
            rect(0, 0, screen_w, screen_h, (1 + floor(t_fade * 254)) * 0x1000000);
        }
    }
    if (warn_open == 1) {
        draw_warning();
    }
    if (studio_open == 1) {
        draw_studio();
    }
    /* Snow above the studio splash, but never above the curtain or the
     * epilepsy warning. */
    draw_newyear_over_notices();
    draw_achievement_toast();
}

/* Saying goodbye when the game is left: half of the exits stay silent, the
 * other half post one of the three lines into the Android notification tray.
 * Called from game_bye() once the activity pauses or stops - that covers the
 * back button, the swipe-up gesture and the app being swiped out of recents.
 * The nick line needs a logged-in account, otherwise the two fixed lines are
 * used, so the text never reads "Пока, , удачи!". */
static void bye_notice_roll(void) {
    const char *nick;
    double pick;
    if (rand() % 100 >= 50) {
        return;
    }
    nick = net_login_nick();
    pick = rand() % 3;
    if (pick >= 2 && str_len(nick) == 0) {
        pick = rand() % 2;
    }
    if (pick >= 2) {
        bye_notice_show(ds_concat(ds_concat(tr_bye_nick_pre(), nick), tr_bye_nick_post()));
        return;
    }
    if (pick == 1) {
        bye_notice_show(tr_bye_cubes());
        return;
    }
    bye_notice_show(tr_bye_sorry());
}

static double logic_back(void) {
    if (warn_open == 1) {
        return 0;
    }
    if (studio_open == 1) {
        return 1;
    }
    if (t_dir != 0) {
        return 1;
    }
    if (game_state == ST_ONLINE && chat_open == 1) {
        chat_set_open(0);
        return 1;
    }
    if (game_state == ST_SOLO || game_state == ST_ONLINE) {
        if (finished != 0 && finish_t < alert_hold) {
            return 1;
        }
        start_transition(ST_LOBBY);
        return 1;
    }
    if (game_state == ST_LEVELS || game_state == ST_SKINS) {
        start_transition(ST_CLASSES);
        return 1;
    }
    if (game_state == ST_LEADERBOARD || game_state == ST_PROMO) {
        start_transition(ST_MODES);
        return 1;
    }
    if (game_state == ST_PRIVACY) {
        start_transition(ST_SETTINGS);
        return 1;
    }
    if (game_state == ST_LOGIN) {
        keyboard_hide();
        keyboard_clear();
        if (login_after == ST_ONLINE) {
            start_transition(ST_MODES);
        } else {
            start_transition(ST_LOBBY);
        }
        return 1;
    }
    if (game_state != ST_LOBBY) {
        start_transition(ST_LOBBY);
        return 1;
    }
    /* Back in the lobby closes the game; the farewell is said when the
     * activity actually stops, so every way of leaving is covered. */
    return 0;
}

static void logic_touch(double x, double y, double action, double id) {
    if (t_dir != 0) {
        classes_swipe_reset();
        return;
    }
    if (game_state != ST_CLASSES || warn_open == 1 || studio_open == 1) {
        classes_swipe_reset();
    }
    if (warn_open == 1) {
        touch_warn(x, y, action);
        return;
    }
    if (studio_open == 1) {
        return;
    }
    /* Lobby has its own input handler, while the other screens use
     * touch_menu(). Detect controls on touch-down; class-card taps wait for
     * release so a horizontal drag can page without selecting a character. */
    double class_swipe = game_state == ST_CLASSES && classes_swipe_zone(x, y);
    if (action == 0 && game_state != ST_SOLO && game_state != ST_ONLINE && class_swipe == 0 &&
        menu_button_hit(x, y) == 1) {
        play_button_click();
    }
    if (game_state == ST_LOBBY) {
        touch_lobby(x, y, action);
    } else if (game_state == ST_SOLO || game_state == ST_ONLINE) {
        touch_game(x, y, action, id);
    } else {
        touch_menu(x, y, action, id);
    }
}
