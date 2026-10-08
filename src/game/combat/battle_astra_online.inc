static void push_astra_grab(void) {
    if (player_class != CLASS_ASTRA) {
        return;
    }
    if (astra_state != 0 && astra_pub == 0) {
        net_publish_grab(astra_x / screen_w, astra_y / screen_h, astra_dx, astra_dy, astra_seq);
        astra_pub = 1;
    } else if (astra_state == 0 && astra_pub == 1) {
        net_publish_grab(0, 0, 0, 0, 0);
        astra_pub = 0;
    }
}

static double remote_grab_get(double slot, double field) {
    return arr_get(remote_grab, slot * grab_fields + field);
}

static void remote_grab_set(double slot, double field, double value) {
    arr_set(remote_grab, slot * grab_fields + field, value);
}

static void remote_grab_stop(double slot) {
    remote_grab_set(slot, GRAB_ACTIVE, 0);
    remote_grab_set(slot, GRAB_CAUGHT, 0);
    remote_grab_set(slot, GRAB_HITS, 0);
    remote_grab_set(slot, GRAB_T, 0);
    remote_grab_set(slot, GRAB_RW, 0);
    if (pgrab_active == 1 && pgrab_slot == slot) {
        pgrab_active = 0;
        pgrab_slot = -1;
        pgrab_t = 0;
        pgrab_hits = 0;
    }
}

static void remote_grab_begin(double slot) {
    remote_grab_set(slot, GRAB_ACTIVE, 1);
    remote_grab_set(slot, GRAB_T, 0);
    remote_grab_set(slot, GRAB_CAUGHT, 0);
    remote_grab_set(slot, GRAB_HITS, 0);
    remote_grab_set(slot, GRAB_X, net_player_grab_x(slot) * screen_w);
    remote_grab_set(slot, GRAB_Y, net_player_grab_y(slot) * screen_h);
    remote_grab_set(slot, GRAB_DX, net_player_grab_dx(slot));
    remote_grab_set(slot, GRAB_DY, net_player_grab_dy(slot));
    /* Resolve the attacker's Astra variant once, here, from the class/skin
     * snapshot read_remotes() already keeps: every later frame of this grab
     * (beats, hitboxes, throw) reads the cached bit instead of touching the
     * network again. */
    remote_grab_set(slot, GRAB_RW, astra_rw_of_slot(slot));
}

static double update_remote_grab(double slot) {
    double gen = net_player_grab(slot);
    if (gen != remote_grab_get(slot, GRAB_GEN)) {
        remote_grab_set(slot, GRAB_GEN, gen);
        if (gen > 0) {
            remote_grab_begin(slot);
        } else {
            remote_grab_stop(slot);
        }
    }
    if (remote_grab_get(slot, GRAB_ACTIVE) != 1) {
        return 0;
    }
    double rw = remote_grab_get(slot, GRAB_RW);
    double t = remote_grab_get(slot, GRAB_T) + dt;
    remote_grab_set(slot, GRAB_T, t);
    double lv = net_player_level(slot);
    double gx = remote_grab_get(slot, GRAB_X);
    double gy = remote_grab_get(slot, GRAB_Y);
    double gdx = remote_grab_get(slot, GRAB_DX);
    double gdy = remote_grab_get(slot, GRAB_DY);
    if (remote_grab_get(slot, GRAB_CAUGHT) == 0) {
        if (t <= astra_rw_catch_time_for(rw)) {
            if (t >= astra_rw_lunge_time_for(rw) && pgrab_active == 0 && revive_prot <= 0 && player->hp > 0) {
                if (astra_zone_hits_player(gx, gy, gdx, gdy, lv) == 1) {
                    remote_grab_set(slot, GRAB_CAUGHT, 1);
                    pgrab_active = 1;
                    pgrab_slot = slot;
                    pgrab_t = t;
                    pgrab_hits = 0;
                    grab_cancel_actions();
                    ds_log("caught by astra of slot %g", slot);
                }
            }
        } else {
            if (t >= astra_rw_grab_time_for(rw)) {
                remote_grab_stop(slot);
            }
            return 0;
        }
    }
    if (remote_grab_get(slot, GRAB_CAUGHT) != 1) {
        return 0;
    }
    pgrab_t = t;
    grab_quiet_player(astra_rw_hit_interval_for(rw));
    double due = astra_rw_beats_done_for(t, rw);
    while (remote_grab_get(slot, GRAB_HITS) < due) {
        remote_grab_set(slot, GRAB_HITS, remote_grab_get(slot, GRAB_HITS) + 1);
        pgrab_hits = remote_grab_get(slot, GRAB_HITS);
        remote_grab_swing(slot);
        if (take_damage(astra_beat_damage(player->max_hp)) == 1) {
            remote_grab_stop(slot);
            return 1;
        }
        if (player->hp <= 0 || finished != 0) {
            remote_grab_stop(slot);
            return 0;
        }
    }
    if (t >= astra_rw_final_time_for(rw)) {
        remote_grab_swing(slot);
        /* remote_grab_stop() clears the grab immediately, so explicitly pulse
         * the final damage zone and let the normal fade-out render it. */
        arr_set(rgrab_box_a, slot, 1);
        double dead = take_damage(astra_final_damage(player->max_hp));
        remote_grab_stop(slot);
        if (dead == 1) {
            return 1;
        }
        if (player->hp > 0 && finished == 0) {
            throw_player(gdx, gdy, astra_rw_throw_dist_for(lv, rw), astra_rw_throw_stun_for(lv, rw));
        }
    }
    return 0;
}

static void update_remote_grabs(void) {
    if (net_slot() < 0) {
        return;
    }
    for (double slot = 0; slot <= max_players - 1; slot += 1) {
        if (slot == net_slot() || net_player_online(slot) != 1) {
            remote_grab_set(slot, GRAB_GEN, 0);
            remote_grab_stop(slot);
        } else if (update_remote_grab(slot) == 1) {
            return;
        }
    }
}

static double remote_grab_punching(double slot) {
    if (arr_get(rgrab_punch_t, slot) > 0) {
        return 1;
    }
    return 0;
}

static void remote_grab_swing(double slot) {
    arr_set(rgrab_punch_t, slot, astra_hit_show);
}
