static double fighter_look_of(double slot) { return arr_get(fighter_looks, slot); }

static double fighter_pose_of(double slot, double punching) {
    if (punching == 1 && arr_get(fighter_swings, slot) == 1) {
        return 1;
    }
    return 0;
}

static void resolve_fighter_look(double slot, double cls, double skin) {
    double wanted = class_look(cls, skin);
    if (look_ready(wanted) == 1) {
        /* Publish the matching idle/punch pair atomically. */
        arr_set(fighter_looks, slot, wanted);
        arr_set(fighter_swings, slot, 1);
        return;
    }
    /* Never retain or substitute a different fighter after an upload failure.
     * The normal upload path completes at frame start, so this fallback is
     * reserved for a real asset/GPU error and remains visibly opaque. */
    arr_set(fighter_looks, slot, LOOK_NONE);
    arr_set(fighter_swings, slot, 0);
}

static void resolve_fighter_looks(void) {
    resolve_fighter_look(FLOOK_ME, player_class, current_skin());
    if (game_state == ST_ONLINE) {
        double i = 0;
        while (i < max_players) {
            double b = i * remote_fields;
            resolve_fighter_look(FLOOK_REMOTE + i, arr_get(remotes, b + 10), arr_get(remotes, b + 11));
            i = i + 1;
        }
        return;
    }
    resolve_fighter_look(FLOOK_BOT, enemy_class, SKIN_NORMAL);
}

static GAME_MAYBE_UNUSED const char *fighter_sprite(double cls, double punching, double skin) {
    double look = drawable_look(cls, skin);
    if (look == LOOK_NONE) {
        return "";
    }
    return look_tex(look, punching == 1 && look_swing_ready(look) == 1);
}

static GAME_MAYBE_UNUSED double fighter_sprite_visible(double cls, double punching, double skin) {
    (void)punching;
    if (drawable_look(cls, skin) == LOOK_NONE) {
        return 0;
    }
    return 1;
}

static void draw_fighter_shadow(double x, double y, double ang, double look, double punching) {
    double half_px = 25 * cube_scale * player_scale;
    double sprite_scale = cube_scale * player_scale;
    if (look == LOOK_NONE) {
        circle(x - shadow_off, y + shadow_off, half_px, (((unsigned)(int)shadow_alpha) << 24));
        return;
    }
    tex_tint(x - half_px - shadow_off, y - half_px + shadow_off, look_tex(look, punching), sprite_a(ang), sprite_scale,
             (((unsigned)(int)shadow_alpha) << 24));
}

static void draw_fighter_body(double x, double y, double ang, double look, double punching) {
    double half_px = 25 * cube_scale * player_scale;
    double sprite_scale = cube_scale * player_scale;
    if (look == LOOK_NONE) {
        rect(x - half_px, y - half_px, 2 * half_px, 2 * half_px, fighter_fallback_rgb);
        return;
    }
    tex(x - half_px, y - half_px, look_tex(look, punching), sprite_a(ang), sprite_scale);
}

static void draw_fighter_hurt(double x, double y, double ang, double look, double punching, double scale, double a) {
    if (a <= 0) {
        return;
    }
    double fa = floor(a * 255 * hurt_alpha);
    if (fa < 1) {
        return;
    }
    if (look == LOOK_NONE) {
        return;
    }
    double half_px = 25 * cube_scale * scale;
    tex_tint(x - half_px, y - half_px, look_tex(look, punching), sprite_a(ang), cube_scale * scale,
             (((unsigned)(int)fa) << 24) | ((unsigned)(int)hurt_rgb));
}
