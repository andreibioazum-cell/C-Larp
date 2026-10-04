static const char *punch_tex_or_idle(const char *punch_tex, double ok, double cls) {
    if (ok == 1) {
        return punch_tex;
    }
    return class_idle_tex(cls);
}

static double azum_tex_pair_ok(void) {
    if (azum_tex_ok == 1 && azum_punch_tex_ok == 1) {
        return 1;
    }
    return 0;
}

static const char *azum_idle_tex(void) {
    if (azum_tex_pair_ok() == 1) {
        return AZUM_TEX;
    }
    return ORDINARY_TEX;
}

static const char *azum_punch_tex(void) {
    if (azum_tex_pair_ok() == 1) {
        return AZUM_PUNCH_TEX;
    }
    return punch_tex_or_idle(PUNCH_TEX, ordinary_punch_tex_ok, CLASS_ORDINARY);
}

static double zombie_tex_ok(void) {
    if (azum_zombie_tex_ok == 1 && azum_zombie_punch_tex_ok == 1) {
        return 1;
    }
    return 0;
}

static const char *zombie_idle_tex(void) {
    if (zombie_tex_ok() == 1) {
        return AZUM_ZOMBIE_TEX;
    }
    return azum_idle_tex();
}

static const char *zombie_punch_tex(void) {
    if (zombie_tex_ok() == 1) {
        return AZUM_ZOMBIE_PUNCH_TEX;
    }
    return azum_punch_tex();
}

static const char *class_idle_tex(double cls) {
    if (cls == CLASS_AZUM) {
        return azum_idle_tex();
    }
    if (cls == CLASS_SANTA) {
        return SANTA_TEX;
    }
    if (cls == CLASS_EBUC) {
        return EBUC_TEX;
    }
    return ORDINARY_TEX;
}

static const char *class_punch_tex(double cls) {
    if (cls == CLASS_AZUM) {
        return azum_punch_tex();
    }
    if (cls == CLASS_SANTA) {
        return punch_tex_or_idle(SANTA_PUNCH_TEX, santa_punch_tex_ok, cls);
    }
    if (cls == CLASS_EBUC) {
        return punch_tex_or_idle(EBUC_PUNCH_TEX, ebuc_punch_tex_ok, cls);
    }
    return punch_tex_or_idle(PUNCH_TEX, ordinary_punch_tex_ok, cls);
}

static double class_punch_damage_for(double cls, double lv) {
    if (cls == CLASS_SANTA) {
        return santa_damage;
    }
    if (cls == CLASS_EBUC) {
        return ebuc_damage;
    }
    if (cls == CLASS_ORDINARY && lv >= 1) {
        return punch_damage * (1 + ordinary_damage_bonus);
    }
    return punch_damage;
}

static double player_max_hp_for(double cls) {
    if (cls == CLASS_EBUC) {
        return ebuc_hp;
    }
    if (cls == CLASS_AZUM) {
        return azum_hp;
    }
    return 10;
}

static double ebuc_station_hp_for(double lv) {
    double h = ebuc_station_hp;
    if (lv >= 1) {
        h = h + ebuc_station_hp_bonus * 2;
    }
    return h;
}

static double ebuc_universe_damage_for(double lv) {
    double d = ebuc_universe_damage;
    if (lv >= 3) {
        d = d + ebuc_universe_damage_bonus;
    }
    return d;
}

static double class_punch_damage(double cls) {
    return class_punch_damage_for(cls, player_level);
}

static double santa_poison_time_for(double lv) {
    (void)lv;
    return santa_poison_time;
}

static double santa_poison_damage_for(double lv) {
    double d = santa_poison_dps;
    if (lv >= 1) {
        d = d + santa_poison_damage_bonus;
    }
    if (lv >= 2) {
        d = d + santa_poison_damage_bonus;
    }
    return d;
}

static double santa_freeze_time_for(double lv) {
    double t = super_freeze;
    if (lv >= 3) {
        t = t + santa_freeze_time_bonus;
    }
    return t;
}
