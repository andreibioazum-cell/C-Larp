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

/* Один отступ размытия. Прозрачность 0 в движке означает «совсем
 * непрозрачный», поэтому пустые отступы просто не рисуем. */
static void shadow_tap_circle(double x, double y, double r, double a) {
    unsigned ai = (unsigned)(int)(a + 0.5);
    if (ai < 1) {
        return;
    }
    circle(x, y, r, ((uint32_t)ai) << 24);
}

static void shadow_tap_tex(double x, double y, double half_px, double scale, const char *name, double ang, double a) {
    unsigned ai = (unsigned)(int)(a + 0.5);
    if (ai < 1) {
        return;
    }
    tex_tint(x - half_px, y - half_px, name, ang, scale, ((uint32_t)ai) << 24);
}

/* Настоящее размытие тени - свёртка силуэта: центр, кольцо из шести отступов
 * поближе и кольцо из шести подальше. Каждый отступ берёт свою долю общей
 * плотности, поэтому край тени мягкий по-настоящему, а не выглядит вторым
 * увеличенным пятном. */
static void draw_shadow_blur_circle(double cx, double cy, double r, double alpha, double blur) {
    const double kw[3] = {0.25, 0.09, 0.06};
    const double kr[3] = {0.0, 0.55, 1.0};
    int ring = 0;
    while (ring < 3) {
        double a = alpha * kw[ring];
        double rr = blur * kr[ring];
        if (ring == 0) {
            shadow_tap_circle(cx, cy, r, a);
        } else {
            double i = 0;
            while (i < 6) {
                double t = i * pi / 3;
                shadow_tap_circle(cx + cos(t) * rr, cy + sin(t) * rr, r, a);
                i = i + 1;
            }
        }
        ring = ring + 1;
    }
}

static void draw_shadow_blur_tex(double cx, double cy, double half_px, double scale, const char *name, double ang,
                                 double alpha, double blur) {
    const double kw[3] = {0.25, 0.09, 0.06};
    const double kr[3] = {0.0, 0.55, 1.0};
    int ring = 0;
    while (ring < 3) {
        double a = alpha * kw[ring];
        double rr = blur * kr[ring];
        if (ring == 0) {
            shadow_tap_tex(cx, cy, half_px, scale, name, ang, a);
        } else {
            double i = 0;
            while (i < 6) {
                double t = i * pi / 3;
                shadow_tap_tex(cx + cos(t) * rr, cy + sin(t) * rr, half_px, scale, name, ang, a);
                i = i + 1;
            }
        }
        ring = ring + 1;
    }
}

/* Смаз движения: на скорости за бойцом остаются полупрозрачные копии его же
 * спрайта по пути, по которому он только что прошёл. Это размытие самого
 * движения, а не всего кадра. */
static void draw_motion_ghosts(double x, double y, double dx, double dy, double half_px, double scale,
                               const char *name, double ang) {
    double len = sqrt(dx * dx + dy * dy);
    double lim = half_px * 1.1;
    if (len < 2) {
        return;
    }
    if (len > lim) {
        dx = dx / len * lim;
        dy = dy / len * lim;
        len = lim;
    }
    /* Корень из скорости: обычная ходьба уже заметна, рывок - сильный шлейф. */
    double a = 0.40 * sqrt(len / lim);
    if (a < 0.02) {
        return;
    }
    int i = 1;
    while (i <= 4) {
        double t = i / 5.0;
        double ga = a * (1 - t);
        unsigned ai = (unsigned)(int)(ga * 255.0 + 0.5);
        if (ai >= 1) {
            tex_tint(x - dx * t - half_px, y - dy * t - half_px, name, ang, scale, ((uint32_t)ai) << 24);
        }
        i = i + 1;
    }
}

static void draw_fighter_shadow(double x, double y, double ang, double look, double punching) {
    double half_px = 25 * cube_scale * player_scale;
    double sprite_scale = cube_scale * player_scale;
    double sx = x - shadow_off;
    double sy = y + shadow_off;
    if (look == LOOK_NONE) {
        draw_shadow_blur_circle(sx, sy, half_px, shadow_alpha, half_px * 0.13);
        return;
    }
    draw_shadow_blur_tex(sx, sy, half_px, sprite_scale, look_tex(look, punching), sprite_a(ang), shadow_alpha,
                         half_px * 0.13);
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

/* Flat-colour overlay drawn on top of a fighter silhouette. Astra (Rework)
 * uses it to stay recognisable for every player in online matches without a
 * second sprite sheet: one extra quad, only for a rework fighter. A tint of 0
 * (any other fighter) draws nothing. */
static void draw_fighter_tint(double x, double y, double ang, double look, double punching, double scale, double tint) {
    if (tint <= 0 || look == LOOK_NONE) {
        return;
    }
    double half_px = 25 * cube_scale * scale;
    tex_tint(x - half_px, y - half_px, look_tex(look, punching), sprite_a(ang), cube_scale * scale,
             (uint32_t)(int)tint);
}
