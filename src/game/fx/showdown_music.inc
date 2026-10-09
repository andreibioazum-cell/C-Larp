static double showdown_classes(double first, double second) {
    if (class_is_astra(first) == 1 && second == CLASS_AZUM) {
        return 1;
    }
    if (first == CLASS_AZUM && class_is_astra(second) == 1) {
        return 1;
    }
    return 0;
}

static double showdown_music_matchup(void) {
    if (finished != 0 || player->hp <= 0) {
        return 0;
    }
    if (game_state == ST_SOLO) {
        if (enemy->hp <= 0) {
            return 0;
        }
        return showdown_classes(player_class, enemy_class);
    }
    if (game_state != ST_ONLINE || online_ready == 0) {
        return 0;
    }
    double local_slot = net_slot();
    if (local_slot < 0) {
        return 0;
    }
    double living = 0;
    double rival_class = -1;
    double slot = 0;
    while (slot < max_players) {
        if (slot != local_slot) {
            double base = slot * remote_fields;
            if (arr_get(remotes, base + 5) == 1) {
                living = living + 1;
                rival_class = arr_get(remotes, base + 10);
            }
        }
        slot = slot + 1;
    }
    if (living != 1) {
        return 0;
    }
    return showdown_classes(player_class, rival_class);
}

static void showdown_music_load(void) {
    showdown_music_ok = snd_load(SHOWDOWN_MUSIC);
    if (showdown_music_ok == 0) {
        ds_log("showdown: no astra_azum_showdown.wav");
    }
}

static void showdown_music_reset(void) {
    if (snd_playing(SHOWDOWN_MUSIC) == 1) {
        snd_stop(SHOWDOWN_MUSIC);
    }
    showdown_music_started = 0;
    showdown_music_done = 0;
}

static void update_showdown_music(void) {
    if (showdown_music_matchup() == 0) {
        if (showdown_music_started == 1) {
            if (snd_playing(SHOWDOWN_MUSIC) == 1) {
                snd_stop(SHOWDOWN_MUSIC);
            }
            showdown_music_done = 1;
        }
        return;
    }
    if (showdown_music_done == 1 || showdown_music_ok == 0) {
        return;
    }
    if (showdown_music_started == 0) {
        if (snd_playing(LOBBY_MUSIC) == 1) {
            snd_stop(LOBBY_MUSIC);
        }
        if (snd_playing(WINTER_JINGLE) == 1) {
            snd_stop(WINTER_JINGLE);
        }
        snd_volume(SHOWDOWN_MUSIC, music_volume / 100 * showdown_music_volume);
        if (snd_play(SHOWDOWN_MUSIC) == 1) {
            showdown_music_started = 1;
            return;
        }
        showdown_music_done = 1;
        return;
    }
    if (snd_playing(SHOWDOWN_MUSIC) == 0) {
        showdown_music_done = 1;
    }
}
