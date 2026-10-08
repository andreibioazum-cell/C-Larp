static double class_level_of(double cls) { return arr_get(class_level_tbl, cls); }

static double class_unlocked_of(double cls) { return arr_get(class_unlocked_tbl, cls); }

static double class_owned_of(double cls) {
    if (cls == CLASS_ORDINARY) {
        return 1;
    }
    return arr_get(class_owned_tbl, cls);
}

static void set_class_owned(double cls, double owned) { arr_set(class_owned_tbl, cls, owned); }

static void set_class_progress(double cls, double lv, double un) {
    arr_set(class_level_tbl, cls, lv);
    arr_set(class_unlocked_tbl, cls, un);
}

static double class_cost_of(double cls) {
    if (cls == CLASS_AZUM) {
        return azum_cost;
    }
    if (cls == CLASS_ASTRA) {
        return astra_cost;
    }
    if (cls == CLASS_SANTA) {
        return santa_candy_cost;
    }
    if (cls == CLASS_EBUC) {
        return ebuc_cost;
    }
    return 0;
}

static double class_pays_candies(double cls) {
    if (cls == CLASS_SANTA) {
        return 1;
    }
    return 0;
}

static const char *class_currency_of(double cls) {
    if (class_pays_candies(cls) == 1) {
        return tr_candies_unit();
    }
    return tr_cups_unit();
}

static const char *class_name_of(double cls) {
    if (cls == CLASS_AZUM) {
        return tr_class_azum();
    }
    if (cls == CLASS_ASTRA) {
        return tr_class_astra();
    }
    if (cls == CLASS_SANTA) {
        return tr_class_santa();
    }
    if (cls == CLASS_EBUC) {
        return tr_class_ebuc();
    }
    return tr_class_ordinary();
}

static const char *class_desc_of(double cls) {
    if (cls == CLASS_AZUM) {
        return tr_class_azum_desc();
    }
    if (cls == CLASS_ASTRA) {
        return tr_class_astra_desc();
    }
    if (cls == CLASS_SANTA) {
        return tr_class_santa_desc();
    }
    if (cls == CLASS_EBUC) {
        return tr_class_ebuc_desc();
    }
    return tr_class_ordinary_desc();
}

/* Kept (and used by tools/test_game_state.c) together with the star rating:
 * the raw numbers are still the source of truth for the fight, they are just
 * no longer printed on the classes screen. */
static GAME_MAYBE_UNUSED double class_hp_of(double cls) {
    return astra_rw_max_hp_for(cls, astra_rw_of(cls, class_skin_of(cls)));
}

static GAME_MAYBE_UNUSED double class_power_of(double cls) {
    return astra_rw_punch_damage_for(cls, class_level_of(cls), astra_rw_of(cls, class_skin_of(cls)));
}

/* How imbalanced (overpowered) each class is judged to be, 0..5 - the five
 * stars under the class cards. Astra (Rework) is the deliberately broken
 * developer variant, so it tops the scale while the plain Astra is already a
 * strong class. Tune the numbers here and nothing else has to change. */
static double class_imbalance_of(double cls) {
    if (cls == CLASS_ASTRA) {
        if (astra_rw_of(cls, class_skin_of(cls)) == 1) {
            return 5;
        }
        return 4;
    }
    if (cls == CLASS_AZUM || cls == CLASS_EBUC) {
        return 3;
    }
    if (cls == CLASS_SANTA) {
        return 2;
    }
    return 1;
}

static const char *class_pick_label(double cls) {
    if (player_class == cls) {
        return tr_class_selected();
    }
    if (class_owned_of(cls) == 0) {
        return tr_buy_for(class_cost_of(cls), class_currency_of(cls));
    }
    return tr_class_pick();
}

static const char *class_msg_text(void) {
    if (class_msg_kind == 1) {
        return tr_class_need_candies();
    }
    return tr_class_need_cups();
}

static double skin_is_active(double cls, double skin) {
    if (skin != SKIN_SPECIAL) {
        return 0;
    }
    if (cls == CLASS_AZUM || cls == CLASS_EBUC) {
        return 1;
    }
    return 0;
}

static double class_skin_of(double cls) {
    if (cls == CLASS_EBUC) {
        return ebuc_skin;
    }
    if (cls == CLASS_AZUM) {
        return azum_skin;
    }
    if (cls == CLASS_ASTRA) {
        /* Astra's skin slot carries the Rework variant (SKIN_SPECIAL) and is
         * what gets published as the player's skin over the network. */
        return astra_rw_on() == 1 ? SKIN_SPECIAL : SKIN_NORMAL;
    }
    return SKIN_NORMAL;
}

static double current_skin(void) { return class_skin_of(player_class); }

static double skin_unlocked_for(double cls, double skin) {
    if (skin == SKIN_NORMAL) {
        return 1;
    }
    if (cls == CLASS_EBUC) {
        return 1;
    }
    if (cls == CLASS_AZUM && has_achievement(ACH_LEGENDS) == 1) {
        return 1;
    }
    if (cls == CLASS_ASTRA && astra_rw_allowed() == 1) {
        return 1;
    }
    return 0;
}

static double skin_unlocked(double skin) { return skin_unlocked_for(player_class, skin); }

static const char *skin_name_for(double cls, double skin) {
    if (skin != SKIN_SPECIAL) {
        return tr_skin_normal();
    }
    if (cls == CLASS_EBUC) {
        return tr_skin_elf();
    }
    if (cls == CLASS_ASTRA) {
        return tr_skin_astra_rw();
    }
    return tr_skin_zombie();
}

static const char *skin_name_of(double skin) { return skin_name_for(player_class, skin); }

static const char *skin_hint_for(double cls) {
    if (cls == CLASS_EBUC) {
        return tr_skin_elf_hint();
    }
    if (cls == CLASS_ASTRA) {
        return astra_rw_desc();
    }
    return tr_skin_legends_hint();
}

static void pick_skin(double skin) {
    if (skin < 0 || skin >= SKIN_COUNT) {
        return;
    }
    if (class_has_skins(player_class) == 0) {
        return;
    }
    if (skin_unlocked(skin) == 0) {
        skin_msg_t = msg_time;
        return;
    }
    if (player_class == CLASS_EBUC) {
        ebuc_skin = skin;
    } else if (player_class == CLASS_ASTRA) {
        astra_rw_select(skin);
        return;
    } else {
        azum_skin = skin;
    }
    save_progress();
}

static double class_visible(double cls) {
    if (cls == CLASS_SANTA && candy_enabled == 0 && class_owned_of(CLASS_SANTA) == 0) {
        return 0;
    }
    return 1;
}

static double class_at_order(double i) {
    if (i == 2) {
        return CLASS_EBUC;
    }
    if (i == 3) {
        return CLASS_SANTA;
    }
    if (i == 4) {
        return CLASS_ASTRA;
    }
    return i;
}

static double classes_visible_count(void) {
    double n = 0;
    double i = 0;
    while (i < class_count) {
        if (class_visible(class_at_order(i)) == 1) {
            n = n + 1;
        }
        i = i + 1;
    }
    return n;
}

static double visible_class_at(double vi) {
    double n = 0;
    double i = 0;
    while (i < class_count) {
        double cls = class_at_order(i);
        if (class_visible(cls) == 1) {
            if (n == vi) {
                return cls;
            }
            n = n + 1;
        }
        i = i + 1;
    }
    return CLASS_ORDINARY;
}

static GAME_MAYBE_UNUSED double class_visible_index(double cls) {
    double n = 0;
    double i = 0;
    while (i < class_count) {
        double c = class_at_order(i);
        if (class_visible(c) == 1) {
            if (c == cls) {
                return n;
            }
            n = n + 1;
        }
        i = i + 1;
    }
    return -1;
}

static double level_cost(double n) { return level_base_cost + (n - 1) * level_cost_step; }

static void sync_selected_class(void) {
    player_level = class_level_of(player_class);
    levels_unlocked = class_unlocked_of(player_class);
}

static void select_level(double cls, double n) {
    set_class_progress(cls, n, class_unlocked_of(cls));
    if (cls == player_class) {
        sync_selected_class();
    }
    save_progress();
}

static double buy_level(double cls, double n) {
    double cost = level_cost(n);
    if (cups < cost) {
        level_msg_kind = 0;
        level_msg_t = msg_time;
        return 0;
    }
    cups = cups - cost;
    set_class_progress(cls, n, n);
    if (cls == player_class) {
        sync_selected_class();
    }
    level_msg_t = 0;
    save_progress();
    return 1;
}

static void level_action(double cls, double n) {
    if (n < 1 || n > max_level) {
        return;
    }
    if (class_owned_of(cls) == 0) {
        class_msg_kind = 0;
        class_msg_t = msg_time;
        return;
    }
    double un = class_unlocked_of(cls);
    if (n <= un) {
        select_level(cls, n);
        return;
    }
    if (n == un + 1) {
        buy_level(cls, n);
        return;
    }
    level_msg_kind = 1;
    level_msg_t = msg_time;
}

static const char *level_status_of(double cls, double n) {
    if (n <= class_unlocked_of(cls)) {
        return tr_level_status_open();
    }
    if (n == class_unlocked_of(cls) + 1) {
        return tr_level_unlock_cost(level_cost(n));
    }
    return tr_level_status_locked();
}

static const char *level_msg_text(void) {
    if (level_msg_kind == 1) {
        return tr_level_locked();
    }
    return tr_level_need_cups();
}

static void select_class(double cls) {
    player_class = cls;
    sync_selected_class();
    save_progress();
}

static void pick_class(double cls) {
    if (cls < 0 || cls >= class_count) {
        return;
    }
    if (class_owned_of(cls) == 1) {
        select_class(cls);
        return;
    }
    double cost = class_cost_of(cls);
    class_msg_kind = class_pays_candies(cls);
    if (class_pays_candies(cls) == 1) {
        if (candies < cost) {
            class_msg_t = msg_time;
            return;
        }
        candies = candies - cost;
    } else {
        if (cups < cost) {
            class_msg_t = msg_time;
            return;
        }
        cups = cups - cost;
    }
    set_class_owned(cls, 1);
    select_class(cls);
    ds_log("bought class %g", cls);
}
