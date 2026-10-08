static double has_achievement(double bit) {
    double shifted = floor(achievement_mask / bit);
    return shifted - floor(shifted * 0.5) * 2;
}

static void show_achievement_toast(double bit) {
    ach_toast_bit = bit;
    ach_toast_t = ACH_TOAST_DURATION;
}

static double grant_achievement(double bit) {
    if (has_achievement(bit) == 1) {
        return 0;
    }
    achievement_mask = achievement_mask + bit;
    net_mark_achievement_flag(bit);
    show_achievement_toast(bit);
    return 1;
}

static void check_achievements(void) {
    if (class_owned_of(CLASS_AZUM) == 1 && class_owned_of(CLASS_SANTA) == 1 && class_owned_of(CLASS_EBUC) == 1) {
        grant_achievement(ACH_ALL_CHARACTERS);
    }
}

static void save_progress(void) {
    double bp_lv = net_load_bp_level();
    net_save_progress_all(cups, candies, player_class, class_owned_of(CLASS_AZUM), class_owned_of(CLASS_SANTA),
                          class_owned_of(CLASS_EBUC), player_level, levels_unlocked, class_level_of(CLASS_ORDINARY),
                          class_unlocked_of(CLASS_ORDINARY), class_level_of(CLASS_AZUM), class_unlocked_of(CLASS_AZUM),
                          class_level_of(CLASS_SANTA), class_unlocked_of(CLASS_SANTA), class_level_of(CLASS_EBUC),
                          class_unlocked_of(CLASS_EBUC), bp_lv, skin_flags());
    net_save_astra(class_owned_of(CLASS_ASTRA), class_level_of(CLASS_ASTRA), class_unlocked_of(CLASS_ASTRA));
    progress_push_to_net();
}

static double skin_flags(void) {
    double f = 0;
    if (azum_skin == SKIN_SPECIAL) {
        f = f + 1;
    }
    if (ebuc_skin == SKIN_SPECIAL) {
        f = f + 2;
    }
    return f;
}

static void progress_push_to_net(void) {
    net_set_class(player_class);
    net_set_level(player_level);
    net_set_skin(current_skin());
}

static void progress_normalize(void) {
    if (azum_skin < SKIN_NORMAL || azum_skin >= SKIN_COUNT) {
        azum_skin = SKIN_NORMAL;
    }
    if (ebuc_skin < SKIN_NORMAL || ebuc_skin >= SKIN_COUNT) {
        ebuc_skin = SKIN_NORMAL;
    }
    if (azum_skin == SKIN_ZOMBIE && has_achievement(ACH_LEGENDS) == 0) {
        azum_skin = SKIN_NORMAL;
    }
    double cls = CLASS_ORDINARY;
    while (cls < class_count) {
        double un = clamp(class_unlocked_of(cls), 0, max_level);
        double lv = class_level_of(cls);
        if (lv < 0 || lv > un) {
            lv = 0;
        }
        set_class_progress(cls, lv, un);
        cls = cls + 1;
    }
    if (player_class < 0 || player_class >= class_count || class_owned_of(player_class) == 0) {
        player_class = CLASS_ORDINARY;
    }
    levels_unlocked = clamp(levels_unlocked, 0, max_level);
    if (player_level < 0 || player_level > levels_unlocked) {
        player_level = 0;
    }
}

static void progress_from_storage(void) {
    cups = net_load_cups();
    candies = net_load_candies();
    achievement_mask = net_load_achievement_flags();
    azum_revive_count = net_load_azum_revives();
    player_class = net_load_class();
    set_class_owned(CLASS_AZUM, net_load_azum());
    set_class_owned(CLASS_SANTA, net_load_santa());
    set_class_owned(CLASS_EBUC, net_load_ebuc());
    set_class_owned(CLASS_ASTRA, net_load_astra());
    player_level = net_load_level();
    levels_unlocked = net_load_levels_unlocked();
    set_class_progress(CLASS_ORDINARY, net_load_ordinary_level(), net_load_ordinary_levels_unlocked());
    set_class_progress(CLASS_AZUM, net_load_azum_level(), net_load_azum_levels_unlocked());
    set_class_progress(CLASS_SANTA, net_load_santa_level(), net_load_santa_levels_unlocked());
    set_class_progress(CLASS_EBUC, net_load_ebuc_level(), net_load_ebuc_levels_unlocked());
    set_class_progress(CLASS_ASTRA, net_load_astra_level(), net_load_astra_levels_unlocked());
    double skins = net_load_azum_skin();
    ebuc_skin = SKIN_NORMAL;
    if (skins >= 2) {
        ebuc_skin = SKIN_SPECIAL;
        skins = skins - 2;
    }
    azum_skin = skins;
    if (azum_skin == SKIN_ZOMBIE && has_achievement(ACH_LEGENDS) == 0) {
        grant_achievement(ACH_LEGENDS);
    }
    progress_normalize();
    sync_selected_class();
    progress_push_to_net();
    check_achievements();
    quest_load();
}

static void settings_from_storage(void) {
    language = net_load_language();
    show_hitboxes = net_load_hitboxes();
    music_volume = net_load_music_volume();
    winter_theme = net_load_winter_theme();
    show_fps = net_load_fps_meter();
}

static void save_settings(void) {
    net_save_settings(language, show_hitboxes);
    net_save_winter_theme(winter_theme);
    net_save_fps_meter(show_fps);
}

static void music_volume_apply(void) {
    music_volume = clamp(music_volume, 0, 100);
    net_save_music_volume(music_volume);
    if (music_ok == 1) {
        snd_volume(LOBBY_MUSIC, music_volume / 100 * music_level);
    }
    if (newyear_jingle_ok == 1) {
        snd_volume(WINTER_JINGLE, music_volume / 100 * winter_jingle_volume);
    }
    if (showdown_music_ok == 1) {
        snd_volume(SHOWDOWN_MUSIC, music_volume / 100 * showdown_music_volume);
    }
}

static void music_volume_cycle(void) {
    if (music_volume <= 0) {
        music_volume = 50;
    } else if (music_volume >= 100) {
        music_volume = 0;
    } else {
        music_volume = 100;
    }
    music_volume_apply();
}
