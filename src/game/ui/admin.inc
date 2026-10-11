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

