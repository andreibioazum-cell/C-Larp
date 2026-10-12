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
    apply_theme();
    apply_winter_theme();
    start_music();
    showdown_music_load();
    update_newyear_jingle();
    cube_npc_tex_ok = png_load(CUBE_NPC_TEX);
    cube_npc_d0 = "Привет! Я Куб-НПС!";
    cube_npc_d1 = "Добро пожаловать в Cubic Battle 4!";
    cube_npc_d2 = "Попробуй режим Solo или Online!";
    cube_npc_d3 = "В магазине можно купить персонажей!";
    cube_npc_d4 = "Удачи в битве!";
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

static void logic_update(void) {
    update_warning();
    update_studio();
    update_alpha_notice();
    update_transition();
    update_music();
    update_showdown_music();
    reload_textures();
    tick_achievement_toast();
    admin_tick();
    if (game_state == ST_LOBBY) {
        cube_npc_bob_t = cube_npc_bob_t + dt;
        cube_npc_tick();
        if (cube_npc_dialogue_active == 1) {
            double max_chars = utf8_len(cube_npc_dialogue_text());
            if (cube_npc_dialogue_chars < max_chars) {
                cube_npc_dialogue_chars = cube_npc_dialogue_chars + cube_npc_dialogue_speed * dt;
                if (cube_npc_dialogue_chars > max_chars) {
                    cube_npc_dialogue_chars = max_chars;
                }
            }
        }
    }
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
            /* Smoothstep the curtain alpha: the fade eases in and out instead
             * of ramping linearly. */
            double a = t_fade * t_fade * (3 - 2 * t_fade);
            rect(0, 0, screen_w, screen_h, (1 + floor(a * 254)) * 0x1000000);
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
    /* Last of all: the admin orb and its panel are never covered, not by the
     * transition curtain, not by the epilepsy warning or the splash. */
    admin_draw();
}

/* Saying goodbye when the game is left: half of the exits stay silent, the rest
 * post one of the three lines into the notification tray. Called from game_bye()
 * when the activity stops: back button, swipe-up, swipe out of recents. The
 * nick line needs an account, otherwise the two fixed lines are used. */
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
    /* The admin panel is on top of everything, so it also gets the back key
     * first: the epilepsy warning and the splash never hide it. */
    if (admin_panel_open == 1) {
        admin_panel_close();
        return 1;
    }
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
    /* Track the held pointer for button press feedback before anything can
     * consume or reject the event: 0 = down, 1 = up, 2 = move, 3 = cancel. */
    if (action == 0) {
        press_down = 1;
        press_x = x;
        press_y = y;
    } else if (action == 2) {
        press_x = x;
        press_y = y;
    } else if (action == 1 || action == 3) {
        press_down = 0;
    }
    /* The admin orb and its panel sit above every screen - including the
     * transition curtain and the epilepsy warning - and consume their own
     * touches before anything else, so they never disappear mid-transition. */
    if (admin_touch(x, y, action, id) == 1) {
        return;
    }
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
     * release so a horizontal drag can page without selecting a character.
     * The click sound covers every screen, battles included, so even the
     * in-game back button gives feedback. */
    double class_swipe = game_state == ST_CLASSES && classes_swipe_zone(x, y);
    if (action == 0 && class_swipe == 0 && menu_button_hit(x, y) == 1) {
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
