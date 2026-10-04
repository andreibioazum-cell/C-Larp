static void universe_collapse(double dmg) {
    if (finished != 0) {
        return;
    }
    if (game_state != ST_ONLINE) {
        enemy_apply_damage(dmg);
    }
    take_damage(dmg);
}

static void universe_collapse_remote(double dmg) {
    if (finished != 0) {
        return;
    }
    take_damage(dmg);
}

static void tick_universe(void) {
    if (universe_active == 0) {
        return;
    }
    universe_t = universe_t + dt;
    universe_a = clamp(universe_t / ebuc_universe_collapse_time, 0, 1);
    if (universe_t >= ebuc_universe_collapse_time) {
        universe_active = 0;
        universe_hit = 1;
        universe_collapse(ebuc_universe_damage_for(player_level));
    }
}

static void update_remote_station(void) {
    if (net_slot() < 0) {
        return;
    }
    double s = 0;
    while (s < max_players) {
        double b = s * station_fields;
        if (s == net_slot() || net_player_online(s) != 1) {
            double f = 0;
            while (f < station_fields) {
                arr_set(remote_station, b + f, 0);
                f = f + 1;
            }
        } else {
            double i = 0;
            while (i < turret_max) {
                double o = b + 1 + i * 5;
                double hx = 0;
                double hy = 0;
                double hhp = 0;
                if (i == 0) {
                    hx = net_player_station_x(s) * screen_w;
                    hy = net_player_station_y(s) * screen_h;
                    hhp = net_player_station_hp(s);
                } else if (i == 1) {
                    hx = net_player_station2_x(s) * screen_w;
                    hy = net_player_station2_y(s) * screen_h;
                    hhp = net_player_station2_hp(s);
                } else {
                    hx = net_player_station3_x(s) * screen_w;
                    hy = net_player_station3_y(s) * screen_h;
                    hhp = net_player_station3_hp(s);
                }
                double st = arr_get(remote_station, o);
                if (hhp > 0) {
                    arr_set(remote_station, o, 1);
                    arr_set(remote_station, o + 1, hx);
                    arr_set(remote_station, o + 2, hy);
                    arr_set(remote_station, o + 3, hhp);
                    arr_set(remote_station, o + 4, 0);
                    if (st != 1) {
                        arr_set(remote_station, b, i + 1);
                    }
                } else {
                    if (st == 1) {
                        arr_set(remote_station, o, 2);
                        arr_set(remote_station, o + 4, station_boom_time);
                    } else if (st == 2) {
                        double bt = arr_get(remote_station, o + 4) - dt;
                        if (bt < 0) {
                            bt = 0;
                        }
                        arr_set(remote_station, o + 4, bt);
                        if (bt <= 0) {
                            arr_set(remote_station, o, 0);
                        }
                    }
                }
                i = i + 1;
            }
        }
        s = s + 1;
    }
}

static void update_remote_universe(void) {
    if (net_slot() < 0) {
        return;
    }
    double s = 0;
    while (s < max_players) {
        double b = s * universe_fields;
        if (s == net_slot() || net_player_online(s) != 1) {
            arr_set(remote_universe, b, 0);
            arr_set(remote_universe, b + 1, 0);
        } else {
            double ucnt = net_player_universe(s);
            double prev = arr_get(remote_universe, b);
            if (ucnt != prev && ucnt > 0) {
                arr_set(remote_universe, b, ucnt);
                arr_set(remote_universe, b + 1, 1);
                arr_set(remote_universe, b + 2, net_player_universe_x(s) * screen_w);
                arr_set(remote_universe, b + 3, net_player_universe_y(s) * screen_h);
                arr_set(remote_universe, b + 4, 0);
            }
            if (arr_get(remote_universe, b + 1) == 1) {
                double t = arr_get(remote_universe, b + 4) + dt;
                arr_set(remote_universe, b + 4, t);
                if (t >= ebuc_universe_collapse_time) {
                    arr_set(remote_universe, b + 1, 0);
                    arr_set(remote_universe, b, 0);
                    double rlevel = net_player_level(s);
                    universe_collapse_remote(ebuc_universe_damage_for(rlevel));
                    if (finished != 0) {
                        return;
                    }
                }
            }
        }
        s = s + 1;
    }
}

static void update_remote_dashes(void) {
    if (net_slot() < 0) {
        return;
    }
    double s = 0;
    while (s < max_players) {
        double b = s * dash_fields;
        if (s == net_slot() || net_player_online(s) != 1) {
            arr_set(remote_dash, b, 0);
            arr_set(remote_dash, b + 1, 0);
        } else {
            double gen = net_player_dash(s);
            if (gen != arr_get(remote_dash, b)) {
                arr_set(remote_dash, b, gen);
                if (gen > 0) {
                    arr_set(remote_dash, b + 1, 1);
                    arr_set(remote_dash, b + 6, azum_dash_time);
                    arr_set(remote_dash, b + 7, 0);
                } else {
                    arr_set(remote_dash, b + 1, 0);
                }
            }
            if (arr_get(remote_dash, b + 1) == 1) {
                double ox = net_player_dash_x(s) * screen_w;
                double oy = net_player_dash_y(s) * screen_h;
                double vx = net_player_x(s) * screen_w - ox;
                double vy = net_player_y(s) * screen_h - oy;
                double leg = sqrt(vx * vx + vy * vy);
                if (leg < 1) {
                    leg = 0;
                    arr_set(remote_dash, b + 4, 0);
                    arr_set(remote_dash, b + 5, 0);
                } else {
                    arr_set(remote_dash, b + 4, vx / leg);
                    arr_set(remote_dash, b + 5, vy / leg);
                }
                arr_set(remote_dash, b + 2, ox);
                arr_set(remote_dash, b + 3, oy);
                arr_set(remote_dash, b + 8, leg);
                double t = arr_get(remote_dash, b + 6) - dt;
                if (t < 0) {
                    t = 0;
                }
                arr_set(remote_dash, b + 6, t);
                if (t <= 0) {
                    arr_set(remote_dash, b + 1, 0);
                    arr_set(remote_dash, b + 7, 0);
                } else if (arr_get(remote_dash, b + 7) == 0 && finished == 0 && leg > 0) {
                    double target = dash_resolve_target(arr_get(remote_dash, b + 2), arr_get(remote_dash, b + 3),
                                                        arr_get(remote_dash, b + 4), arr_get(remote_dash, b + 5), leg);
                    if (target == 0) {
                        arr_set(remote_dash, b + 7, 1);
                        if (revive_prot <= 0) {
                            take_damage(azum_dash_damage);
                            stun_player(azum_dash_stun);
                            ds_log("remote dash hit player");
                            if (finished != 0) {
                                return;
                            }
                        }
                    } else if (target >= 1 && target <= turret_max) {
                        arr_set(remote_dash, b + 7, 1);
                        damage_turret(target - 1, azum_dash_damage);
                    }
                }
            }
        }
        s = s + 1;
    }
}

static void flush_queued_action(void) {
    if (queued_punch == 0 && queued_super == 0) {
        return;
    }
    if (punch_left > 0 || punch_cd > 0 || ultimate_visual_busy() == 1) {
        return;
    }
    if (queued_super == 1) {
        if (player_class == CLASS_AZUM) {
            if (dash_cd <= 0) {
                queued_super = 0;
                fire_super();
                return;
            }
        } else {
            if (super_cd <= 0) {
                queued_super = 0;
                fire_super();
                return;
            }
        }
    }
    if (queued_punch == 1) {
        queued_punch = 0;
        fire_punch();
    }
}

static void tick_dash(void) {
    if (dash_active > 0) {
        dash_t = dash_t - dt;
        if (dash_t <= 0) {
            dash_t = 0;
            dash_active = 0;
            dash_hit = 0;
        }
    }
    if (dash_cd > 0) {
        dash_cd = dash_cd - dt;
        if (dash_cd < 0) {
            dash_cd = 0;
        }
    }
}

static void tick_candies(void) {
    if (candy_enabled == 1) {
        double ci = 0;
        while (ci < candy_count) {
            double cp = arr_get(candy_pick, ci);
            if (cp > 0) {
                cp = 0;
                candy_rand_pos(ci);
                arr_set(candy_pick, ci, 0);
            } else {
                arr_set(candy_t, ci, arr_get(candy_t, ci) + dt);
                if (dist(arr_get(candy_x, ci), arr_get(candy_y, ci), player->x, player->y) < player->size * 0.65 + 18) {
                    candies = candies + candy_give;
                    save_progress();
                    quest_on_candy();
                    ds_log("candy picked -> %g", candies);
                    candy_rand_pos(ci);
                    arr_set(candy_pick, ci, 0);
                    arr_set(candy_t, ci, 0);
                }
            }
            ci = ci + 1;
        }
    }
}

static void update_game(void) {
    card_update();
    tick_alert_hold();
    tick_revive();
    tick_freeze();
    tick_player_stun();
    tick_player_poison();
    tick_turrets();
    tick_universe();
    tick_enemy_class();
    tick_hitbox_fades();
    tick_status_fades();
    tick_hurt_fades();
    update_aim();
    if (finished != 0) {
        return;
    }
    if (player->hp <= 0) {
        if (try_revive() == 0) {
            finish_game(2);
            return;
        }
    }
    move_player();
    tick_dash_solo_hit();
    tick_dash();
    tick_candies();
    follow_punch();
    super_cd = tick_down(super_cd);
    boom_t = tick_down(boom_t);
    tick_super_windup();
    tick_santa_follow();
    if (punch->active == 1 && punch->hit == 0) {
        double enemy_turret_target = enemy_punch_turret_target(punch->x, punch->y, punch->dx, punch->dy);
        if (enemy_turret_target >= 1) {
            punch->hit = 1;
            enemy_damage_turret(enemy_turret_target - 1, class_punch_damage(player_class));
            if (revive_prot <= 0) {
                if (take_damage(turret_counter_damage) == 1) {
                    return;
                }
            }
            heal_after_attack();
        } else if (enemy_in_punch() == 1) {
            punch->hit = 1;
            if (enemy_revive_prot <= 0) {
                if (player_class == 2 && santa_follow > 0) {
                    if (player_level >= 3) {
                        if (enemy->poison <= 0) {
                            enemy->poison_tick = santa_poison_tick_interval;
                            enemy->poison_damage = santa_poison_damage_for(player_level);
                        } else {
                            enemy->poison_damage = enemy->poison_damage + santa_poison_damage_for(player_level);
                        }
                        enemy->poison = santa_poison_time_for(player_level);
                    } else {
                        if (enemy->poison <= 0) {
                            enemy->poison_tick = santa_poison_tick_interval;
                        }
                        enemy->poison = santa_poison_time_for(player_level);
                    }
                }
                heal_after_attack();
                if (enemy_apply_damage(class_punch_damage(player_class)) == 1) {
                    return;
                }
            }
        }
    }
    if (punch->active == 1 && punch_left <= 0) {
        punch->active = 0;
    }
    punch_left = tick_down(punch_left);
    update_dust();
    update_gift();
    flush_queued_action();
    if (finished != 0) {
        return;
    }
    update_enemy();
}
