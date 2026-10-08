static double take_damage(double dmg) {
    double real = dmg;
    if (player_class == CLASS_EBUC) {
        real = player_shield_absorb(real);
    }
    if (player_class == CLASS_AZUM && azum_revived == 1 && player_level >= 1) {
        real = real * (1 - clamp(azum_second_life_damage_reduction, 0, 0.9));
    }
    if (real > 0) {
        player_hurt_a = 1;
    }
    player->hp = clamp(player->hp - real, 0, player->max_hp);
    if (player->hp <= 0) {
        player->hp = 0;
        if (try_revive() == 0) {
            finish_game(2);
            return 1;
        }
    }
    return 0;
}

static void heal_after_attack(void) {
    double amount = 0;
    if (player_class == CLASS_AZUM && player_level >= 3) {
        amount = azum_attack_heal;
    } else if (player_class == CLASS_ORDINARY && player_level >= 3) {
        amount = ordinary_attack_heal;
    }
    if (amount > 0) {
        player->hp = clamp(player->hp + amount, 0, player->max_hp);
    }
}

static void stun_enemy(double t) {
    if (t > enemy->stun) {
        enemy->stun = t;
    }
    if (enemy->state >= 1 && enemy->state <= 3) {
        enemy->state = 0;
        enemy->state_time = 0;
        enemy->cooldown = 0.25;
        enemy->think = 0.15;
    }
    enemy_super_windup = 0;
    enemy_super_kind = 0;
}

static void stun_player(double t) {
    if (t > player_stun) {
        player_stun = t;
    }
}

static double try_revive(void) {
    if (player->hp > 0) {
        return 0;
    }
    if (player_class != 1) {
        return 0;
    }
    if (azum_revived == 1) {
        return 0;
    }
    azum_revived = 1;
    player->hp = player->max_hp;
    revive_prot = 1.2;
    double k = 0;
    while (k < 50) {
        double rx = 70 + rand() % (screen_w - 140);
        double ry = 70 + rand() % (screen_h - 140);
        if (dist(rx, ry, enemy->x, enemy->y) > 200) {
            k = 50;
            player->x = rx;
            player->y = ry;
        } else {
            k = k + 1;
        }
    }
    azum_revive_count = azum_revive_count + 1;
    net_save_azum_revives(azum_revive_count);
    ds_log("azum revived (%g/%g)", azum_revive_count, ZOMBIE_REVIVE_NEED);
    if (azum_revive_count >= ZOMBIE_REVIVE_NEED) {
        grant_unlock_flag(FLAG_ZOMBIE_SKIN);
    }
    return 1;
}

static void tick_revive(void) {
    if (revive_prot > 0) {
        revive_prot = revive_prot - dt;
        if (revive_prot < 0) {
            revive_prot = 0;
        }
    }
}

static void tick_hurt_fades(void) {
    player_hurt_a = fade_down_a(player_hurt_a, hurt_fade);
    enemy_hurt_a = fade_down_a(enemy_hurt_a, hurt_fade);
}

static void tick_alert_hold(void) {
    if (finished != 0 && finish_t < alert_hold) {
        finish_t = finish_t + dt;
        if (finish_t > alert_hold) {
            finish_t = alert_hold;
        }
    }
}
