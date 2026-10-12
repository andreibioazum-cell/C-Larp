static double newyear_active(void) {
    return winter_theme_on() == 1;
}

/* Snow runs on the lobby-family screens for as long as winter is on. The
 * startup notice and transitions do not pause it: the draw passes at the end
 * of this file decide what is visible, and the animation clock never stops. */
static double newyear_menu_active(void) {
    if (newyear_active() == 0) {
        return 0;
    }
    return game_state == ST_LOBBY || game_state == ST_MODES || game_state == ST_SETTINGS;
}

static void update_newyear_snow(void) {
    if (newyear_menu_active() == 0) {
        return;
    }
    newyear_snow_t = newyear_snow_t + dt;
    if (newyear_snow_t > 100000) {
        newyear_snow_t = 0;
    }
}

static void newyear_flake_grow(double count) {
    while (newyear_flake_cached < count) {
        double k = 0;
        while (k < flake_fields) {
            arr_push(newyear_flake_cache, -1);
            k = k + 1;
        }
        newyear_flake_cached = newyear_flake_cached + 1;
    }
}

static void newyear_flake_refresh(double b, double i, double cyc) {
    arr_set(newyear_flake_cache, b, cyc);
    arr_set(newyear_flake_cache, b + 3, hash01(i * 7 + cyc * 131 + 5));
    arr_set(newyear_flake_cache, b + 4, hash01(i * 13 + cyc * 197 + 6));
    arr_set(newyear_flake_cache, b + 5, hash01(i * 29 + cyc * 57 + 7));
    arr_set(newyear_flake_cache, b + 6, hash01(i * 17 + cyc * 83 + 8));
}

static void draw_newyear_snow(void) {
    if (newyear_menu_active() == 0) {
        return;
    }
    newyear_flake_grow(newyear_menu_flakes);
    double i = 0;
    while (i < newyear_menu_flakes) {
        double b = i * flake_fields;
        double h1 = arr_get(newyear_flake_cache, b + 1);
        double h2 = arr_get(newyear_flake_cache, b + 2);
        if (h1 < 0) {
            h1 = hash01(i + 51);
            h2 = hash01(i + 142);
            arr_set(newyear_flake_cache, b + 1, h1);
            arr_set(newyear_flake_cache, b + 2, h2);
        }
        double fall_time = 3.2 + 3.6 * h1;
        double cyc = floor(newyear_snow_t / fall_time + h2);
        if (arr_get(newyear_flake_cache, b) != cyc) {
            newyear_flake_refresh(b, i, cyc);
        }
        double ph = frac01(newyear_snow_t / fall_time + h2);
        double hx = arr_get(newyear_flake_cache, b + 3);
        double hs = arr_get(newyear_flake_cache, b + 5);
        double hr = arr_get(newyear_flake_cache, b + 6);
        double sway = 14 * sin(newyear_snow_t * 0.6 + i * 2.1);
        double x = hx * screen_w + sway;
        double y = -40 + ph * (screen_h + 80);
        double s = 0.35 + 0.5 * hs;
        double hw = 26 * s;
        double spin_dir = 1;
        if (ds_mod(i, 2) == 0) {
            spin_dir = -1;
        }
        double spin = spin_dir * (0.9 + 1.4 * hr);
        double ang = (hr + newyear_snow_t * spin) * 2 * pi;
        /* The bundled flake texture is light blue; tinting it white keeps the
         * snow white in every theme instead of showing blue blobs. */
        tex_tint(x - hw, y - hw, SNOWFLAKE_TEX, ang, s, 0xFFFFFFFF);
        i = i + 1;
    }
}

static double newyear_music_active(void) {
    if (newyear_jingle_ok != 1) {
        return 0;
    }
    if (newyear_active() == 0) {
        return 0;
    }
    if (game_state == ST_SOLO || game_state == ST_ONLINE) {
        return 0;
    }
    return 1;
}

static void update_newyear_jingle(void) {
    if (newyear_jingle_tried == 0) {
        newyear_jingle_tried = 1;
        newyear_jingle_ok = snd_load(WINTER_JINGLE);
        if (newyear_jingle_ok == 0) {
            ds_log("newyear: no bundled winter_jingle.wav, the vibe stays visual-only");
        }
    }
    if (newyear_jingle_ok == 0) {
        return;
    }
    if (newyear_music_active() == 1) {
        /* The lobby loop used to keep this branch waiting, which left the
         * winter theme with ordinary music during the startup notice. Switch
         * tracks decisively; the regular track fades back in when winter is
         * disabled or gameplay begins. */
        if (snd_playing(LOBBY_MUSIC) == 1) {
            snd_stop(LOBBY_MUSIC);
        }
        music_level = 0;
        snd_volume(WINTER_JINGLE, music_volume / 100 * winter_jingle_volume);
        if (snd_playing(WINTER_JINGLE) == 0) {
            snd_loop(WINTER_JINGLE);
        }
        return;
    }
    if (snd_playing(WINTER_JINGLE) == 1) {
        snd_stop(WINTER_JINGLE);
    }
}

static void update_newyear(void) {
    update_newyear_snow();
    update_newyear_jingle();
}

/* Snowflakes are drawn around the lobby notices. The black transition curtain
 * must never be painted over flakes, and the epilepsy warning must never be
 * painted under them either - it is a solid black panel with the legal text on
 * it. So:
 *   - the under-pass runs as part of the screen backdrop (screen_fill), below
 *     every button, the dialogue and the notices: flakes fall across the
 *     whole width, and while the warning panel is fully opaque nothing shows
 *     through it anyway; as it fades the lobby - with its snow - is revealed
 *     underneath;
 *   - the over-pass only lifts flakes above the studio splash, which has no
 *     text to protect, and is skipped while the curtain is up (t_fade > 0).
 * Neither pass pauses the animation clock. */
static double newyear_snow_under_notice_active(void) {
    if (newyear_menu_active() == 0 || studio_open == 1) {
        return 0;
    }
    /* Under a fully opaque warning panel the flakes would be invisible: the
     * pass waits for the panel to start fading before drawing them. */
    if (warn_open == 1 && warn_a >= 1) {
        return 0;
    }
    return 1;
}

static double newyear_snow_over_notice_active(void) {
    return newyear_menu_active() == 1 && studio_open == 1 && t_fade <= 0;
}

static void draw_newyear_under_notices(void) {
    if (newyear_snow_under_notice_active() == 1) {
        draw_newyear_snow();
    }
}

static void draw_newyear_over_notices(void) {
    if (newyear_snow_over_notice_active() == 1) {
        draw_newyear_snow();
    }
}
