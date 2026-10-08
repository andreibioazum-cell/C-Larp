static void quest_init(void) {
    if (arr_len(quest_type) >= quest_count) {
        return;
    }
    arr_clear(quest_type);
    arr_clear(quest_prog);
    arr_clear(quest_need);
    arr_clear(quest_next);
    double i = 0;
    while (i < quest_count) {
        double t = rand() % 4;
        double need = 1;
        if (t == 0) {
            need = 1;
        } else if (t == 1) {
            need = 1;
        } else if (t == 2) {
            need = 5;
        } else {
            need = 3;
        }
        arr_push(quest_type, t);
        arr_push(quest_prog, 0);
        arr_push(quest_need, need);
        arr_push(quest_next, 0);
        i = i + 1;
    }
    quest_save();
}

static void quest_save(void) {
    net_save_quest_state(arr_get(quest_type, 0), arr_get(quest_prog, 0), arr_get(quest_need, 0), arr_get(quest_next, 0),
                         arr_get(quest_type, 1), arr_get(quest_prog, 1), arr_get(quest_need, 1), arr_get(quest_next, 1),
                         arr_get(quest_type, 2), arr_get(quest_prog, 2), arr_get(quest_need, 2),
                         arr_get(quest_next, 2));
}

static void quest_restore(void) {
    arr_clear(quest_type);
    arr_clear(quest_prog);
    arr_clear(quest_need);
    arr_clear(quest_next);
    double i = 0;
    while (i < quest_count) {
        arr_push(quest_type, net_load_quest_state(i, 0));
        arr_push(quest_prog, net_load_quest_state(i, 1));
        arr_push(quest_need, net_load_quest_state(i, 2));
        arr_push(quest_next, net_load_quest_state(i, 3));
        i = i + 1;
    }
}

static void quest_load(void) {
    if (net_quest_has_state() == 1) {
        quest_restore();
    } else if (arr_len(quest_type) < quest_count) {
        quest_init();
    }
    if (quest_fill_empty() == 1) {
        quest_save();
    }
}

static double quest_fill_empty(void) {
    double changed = 0;
    double i = 0;
    while (i < quest_count) {
        if (arr_get(quest_type, i) == -1) {
            quest_roll_new(i);
            changed = 1;
        }
        if (arr_get(quest_next, i) != 0) {
            arr_set(quest_next, i, 0);
            changed = 1;
        }
        i = i + 1;
    }
    return changed;
}

static void quest_tick(void) {
    if (quest_fill_empty() == 1) {
        quest_save();
    }
}

static const char *quest_title(double t) {
    if (t == 0) {
        return tr_quest_win_solo();
    }
    if (t == 1) {
        return tr_quest_win_online();
    }
    if (t == 2) {
        return tr_quest_collect_candy();
    }
    return tr_quest_play_matches();
}

static const char *quest_progress_text(double i) {
    if (arr_get(quest_type, i) == -1) {
        return "";
    }
    double p = arr_get(quest_prog, i);
    double n = arr_get(quest_need, i);
    const char *out =
        ds_concat(ds_concat(ds_concat("", ds_num_to_string((double)(p))), "/"), ds_num_to_string((double)(n)));
    return out;
}

static double quest_is_done(double i) {
    if (arr_get(quest_type, i) == -1) {
        return 0;
    }
    double p = arr_get(quest_prog, i);
    double n = arr_get(quest_need, i);
    if (p >= n) {
        return 1;
    }
    return 0;
}

static void quest_roll_new(double idx) {
    double t = rand() % 4;
    double need = 1;
    if (t == 0) {
        need = 1;
    } else if (t == 1) {
        need = 1;
    } else if (t == 2) {
        need = 5;
    } else {
        need = 3;
    }
    arr_set(quest_type, idx, t);
    arr_set(quest_prog, idx, 0);
    arr_set(quest_need, idx, need);
    ds_log("quest rolled new %g at %g", t, idx);
}

static void quest_complete(double idx) {
    if (quest_is_done(idx) == 0) {
        return;
    }
    cups = cups + quest_cups;
    candies = candies + quest_candies;
    save_progress();
    quest_msg_t = msg_time;
    ds_log("quest %g completed", idx);
    quest_roll_new(idx);
    arr_set(quest_next, idx, 0);
    quest_save();
}

static void quest_on_win(double result) {
    double i = 0;
    while (i < quest_count) {
        double t = arr_get(quest_type, i);
        if (quest_is_done(i) == 0) {
            if (t == 0 && result == 1 && game_state == ST_SOLO) {
                arr_set(quest_prog, i, arr_get(quest_prog, i) + 1);
            }
            if (t == 1 && result == 1 && game_state == ST_ONLINE) {
                arr_set(quest_prog, i, arr_get(quest_prog, i) + 1);
            }
            if (t == 3) {
                arr_set(quest_prog, i, arr_get(quest_prog, i) + 1);
            }
            if (quest_is_done(i) == 1) {
                quest_complete(i);
            }
        }
        i = i + 1;
    }
    quest_save();
}

static void quest_on_candy(void) {
    double i = 0;
    while (i < quest_count) {
        double t = arr_get(quest_type, i);
        if (t == 2 && quest_is_done(i) == 0) {
            arr_set(quest_prog, i, arr_get(quest_prog, i) + 1);
            if (quest_is_done(i) == 1) {
                quest_complete(i);
            }
        }
        i = i + 1;
    }
}

static void draw_quest_card(double i) {
    double x = quest_x();
    double y = quest_row_y(i);
    double w = quest_w();
    double done = quest_is_done(i);
    double locked = 0;
    panel(x, y, w, ach_card_h, locked);
    uint32_t c = panel_text(locked);
    double t = arr_get(quest_type, i);
    const char *title = quest_title(t);
    const char *prog = quest_progress_text(i);
    const char *rew = tr_quest_reward(quest_cups, quest_candies);
    if (done == 1) {
        rew = tr_quest_claimed();
    }
    text_scaled(title, x + card_text_pad, y + 14, c, 0.62);
    text_scaled(prog, x + card_text_pad, y + 46, c, 0.46);
    text_scaled(rew, x + card_text_pad, y + 70, c, 0.42);
}

static void draw_quests(void) {
    quest_load();
    screen_fill();
    ctext(tr_quests(), title_y(), text_color);
    double i = 0;
    while (i < quest_count) {
        draw_quest_card(i);
        i = i + 1;
    }
    quest_msg_t = tick_down(quest_msg_t);
    draw_msg(tr_quest_reward(quest_cups, quest_candies), quest_row_y(quest_count) + 12, quest_msg_t);
    draw_back();
}

static void touch_quests(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        start_transition(ST_LOBBY);
        return;
    }
}
