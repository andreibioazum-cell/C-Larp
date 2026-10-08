/* ===========================================================================
 * Astra (Rework) - developer-only variant of Astra.
 *
 * It IS the same class as Astra (CLASS_ASTRA); only the numbers differ. The
 * variant travels in the *skin* slot Astra never used (SKIN_SPECIAL), so the
 * wire format is untouched: cls stays 4, skin becomes 1, and every client
 * already publishes, validates and caches that pair (see read_remotes(),
 * which snapshots cls+skin once per remote join). Nothing extra is sent, no
 * new JSON key is added and no per-frame network read is introduced.
 *
 * Deltas vs. regular Astra (every number lives right here):
 *   HP              -5%       astra_rw_hp_factor      <- astra_hp
 *   punch           -7%       astra_rw_punch_factor   <- astra punch damage
 *   ult hits 1..3   faster    astra_rw_interval_factor <- astra_hit_interval
 *   ult finisher    sooner    astra_rw_final_delay     <- astra_final_delay
 *     Three quick beats, then only ~0.4 s of wind-up before the throw, so the
 *     ultimate reads as "fast hits, short pause, throw". The whole grab is
 *     thus shorter than the regular one - deliberate: the variant is
 *     developer-only and every client resolves it from the synced class/skin
 *     pair (astra_rw_of_slot()), never from a timer of its own.
 *   ult throw       +35%      astra_rw_throw_factor   <- astra_throw_dist
 *   small lunge on ult cast   astra_rw_lunge_dist / astra_rw_lunge_time
 *     A small forward step, eased over astra_rw_lunge_time instead of an
 *     instant teleport. The grab origin (hitbox + published zone) is the end
 *     of the step, taken at cast time, so hitbox and network traffic stay
 *     exactly what they were.
 *
 * Only ADMIN_NICK / ADMIN_PASS (the developer account) can unlock or select
 * it; every other account sees Astra exactly as before.
 *
 * Porting into regular Astra later: move these numbers into the astra_*
 * constants in state/create/core/config.inc and delete this file, replacing
 * the astra_rw_*() calls in battle_astra*.inc with the plain astra_* values.
 * =========================================================================== */

static const double astra_rw_hp_factor = 0.95;
static const double astra_rw_punch_factor = 0.93;
static const double astra_rw_interval_factor = 0.8;
static const double astra_rw_final_delay = 0.4;
static const double astra_rw_throw_factor = 1.35;
static const double astra_rw_lunge_dist = 34;
static const double astra_rw_lunge_time = 0.16;
static const double astra_rw_tint_rgb = 0xE040FB;
static const double astra_rw_tint_a = 0.35;

/* The variant stored in the skin slot of Astra. */
static double astra_rw_of(double cls, double skin) {
    if (cls == CLASS_ASTRA && skin == SKIN_SPECIAL) {
        return 1;
    }
    return 0;
}

/* Variant of a remote fighter, read straight from the per-slot class/skin
 * snapshot read_remotes() took when the player joined. No network call. */
static double astra_rw_of_slot(double slot) {
    double b = slot * remote_fields;
    return astra_rw_of(arr_get(remotes, b + 10), arr_get(remotes, b + 11));
}

static double astra_rw_is_dev(void) {
    if (net_login_status() != 2) {
        return 0;
    }
    if (str_eq(net_login_nick(), ADMIN_NICK) == 1 && str_eq(net_login_pass(), ADMIN_PASS) == 1) {
        return 1;
    }
    return 0;
}

/* Cached permission bit. Refreshed when the account changes (login, profile
 * load, battle start), never while drawing or ticking, so the dev check does
 * not run a single time per frame. */
static void astra_rw_refresh(void) { astra_rw_ok = astra_rw_is_dev(); }

static double astra_rw_allowed(void) { return astra_rw_ok; }

/* Effective variant of the local player: only the developer account may use a
 * saved selection, so a copied progress file cannot fake the variant. */
static double astra_rw_on(void) {
    if (astra_rw_ok == 1 && astra_skin == 1) {
        return 1;
    }
    return 0;
}

static double astra_rw_local(void) {
    if (player_class == CLASS_ASTRA && astra_rw_on() == 1) {
        return 1;
    }
    return 0;
}

static void astra_rw_select(double on) {
    astra_rw_refresh();
    if (astra_rw_allowed() == 0) {
        astra_skin = 0;
        save_progress();
        return;
    }
    astra_skin = on == 1 ? 1 : 0;
    save_progress();
}

static const char *astra_rw_desc(void) { return "devdevdev"; }

static double astra_rw_hit_interval_for(double rw) {
    if (rw == 1) {
        return astra_hit_interval * astra_rw_interval_factor;
    }
    return astra_hit_interval;
}

static double astra_rw_catch_time_for(double rw) {
    (void)rw;
    return astra_catch_time;
}

static double astra_rw_beat_time_for(double i, double rw) {
    return astra_rw_catch_time_for(rw) + astra_rw_hit_interval_for(rw) * i;
}

static double astra_rw_final_delay_for(double rw) {
    if (rw == 1) {
        return astra_rw_final_delay;
    }
    return astra_final_delay;
}

static double astra_rw_final_time_for(double rw) {
    return astra_rw_catch_time_for(rw) + astra_rw_hit_interval_for(rw) * astra_hit_count + astra_rw_final_delay_for(rw);
}

static double astra_rw_grab_time_for(double rw) { return astra_rw_final_time_for(rw) + astra_throw_time; }

static double astra_rw_beats_done_for(double t, double rw) {
    double n = 0;
    double i = 1;
    while (i <= astra_hit_count) {
        if (t >= astra_rw_beat_time_for(i, rw)) {
            n = n + 1;
        }
        i = i + 1;
    }
    return n;
}

static double astra_rw_zone_live_for(double t, double caught, double rw) {
    if (t <= astra_rw_catch_time_for(rw)) {
        return 1;
    }
    if (caught != 1) {
        return 0;
    }
    double i = 1;
    while (i <= astra_hit_count) {
        double bt = astra_rw_beat_time_for(i, rw);
        if (t >= bt && t <= bt + astra_hit_show) {
            return 1;
        }
        i = i + 1;
    }
    double ft = astra_rw_final_time_for(rw);
    if (t >= ft && t <= ft + astra_hit_show) {
        return 1;
    }
    return 0;
}

static double astra_rw_throw_dist_for(double lv, double rw) {
    double d = astra_throw_dist_for(lv);
    if (rw == 1) {
        d = d * astra_rw_throw_factor;
    }
    return d;
}

static double astra_rw_throw_stun_for(double lv, double rw) {
    (void)rw;
    return astra_throw_stun_for(lv);
}

static double astra_rw_max_hp_for(double cls, double rw) {
    double hp = player_max_hp_for(cls);
    if (rw == 1 && cls == CLASS_ASTRA) {
        hp = hp * astra_rw_hp_factor;
    }
    return hp;
}

static double astra_rw_punch_damage_for(double cls, double lv, double rw) {
    double d = class_punch_damage_for(cls, lv);
    if (rw == 1 && cls == CLASS_ASTRA) {
        d = d * astra_rw_punch_factor;
    }
    return d;
}

/* Punch damage of a fighter as the receiving client must apply it: cls + skin
 * come from the same synced snapshot the rest of the fight is built on. */
static double astra_rw_punch_damage_net(double cls, double lv, double skin) {
    return astra_rw_punch_damage_for(cls, lv, astra_rw_of(cls, skin));
}

static double astra_rw_self_damage(void) {
    return astra_rw_punch_damage_for(player_class, player_level, astra_rw_local());
}

/* The tiny forward step Astra (Rework) takes when the ultimate is pressed.
 *
 * It is small and smooth: the body glides to the target over
 * astra_rw_lunge_time with a smoothstep ease instead of jumping there in one
 * frame. The grab origin - the hitbox and the zone published to the other
 * clients - is the end of the step, captured at cast time, exactly where the
 * old instant hop put it, so the grab stays identical for everyone and no
 * extra network write is needed. The glide itself is ordinary local movement
 * (a few pixels per frame, movement is locked during the grab anyway). */
static void astra_rw_lunge_start(void) {
    astra_rw_lunge_t = 0;
    astra_rw_lunge_fx = player->x;
    astra_rw_lunge_fy = player->y;
    astra_rw_lunge_tx = player->x;
    astra_rw_lunge_ty = player->y;
    if (astra_rw_local() == 0) {
        return;
    }
    double dx = cos(player->angle);
    double dy = sin(player->angle);
    astra_rw_lunge_tx = clamp(player->x + dx * astra_rw_lunge_dist, player->size, screen_w - player->size);
    astra_rw_lunge_ty = clamp(player->y + dy * astra_rw_lunge_dist, player->size, screen_h - player->size);
    dust_trail(astra_rw_lunge_tx, astra_rw_lunge_ty, dx, dy);
}

static void astra_rw_lunge_tick(void) {
    if (astra_rw_lunge_fx == astra_rw_lunge_tx && astra_rw_lunge_fy == astra_rw_lunge_ty) {
        return;
    }
    astra_rw_lunge_t = astra_rw_lunge_t + dt;
    if (astra_rw_lunge_t >= astra_rw_lunge_time) {
        astra_rw_lunge_t = astra_rw_lunge_time;
        astra_rw_lunge_fx = astra_rw_lunge_tx;
        astra_rw_lunge_fy = astra_rw_lunge_ty;
        player->x = astra_rw_lunge_tx;
        player->y = astra_rw_lunge_ty;
        return;
    }
    double p = astra_rw_lunge_t / astra_rw_lunge_time;
    p = p * p * (3 - 2 * p);
    player->x = astra_rw_lunge_fx + (astra_rw_lunge_tx - astra_rw_lunge_fx) * p;
    player->y = astra_rw_lunge_fy + (astra_rw_lunge_ty - astra_rw_lunge_fy) * p;
}

static double astra_rw_tint(void) {
    return floor(astra_rw_tint_a * 255) * 0x1000000 + astra_rw_tint_rgb;
}

static double astra_rw_tint_local(void) {
    if (astra_rw_local() == 1) {
        return astra_rw_tint();
    }
    return 0;
}

static double astra_rw_tint_slot(double slot) {
    if (astra_rw_of_slot(slot) == 1) {
        return astra_rw_tint();
    }
    return 0;
}
