static FighterSpritePair look_sprite_pair(double look) {
    FighterSpritePair pair = {"", ""};
    if (look == LOOK_AZUM) {
        pair.idle = AZUM_TEX;
        pair.punch = AZUM_PUNCH_TEX;
    } else if (look == LOOK_ZOMBIE) {
        pair.idle = AZUM_ZOMBIE_TEX;
        pair.punch = AZUM_ZOMBIE_PUNCH_TEX;
    } else if (look == LOOK_SANTA) {
        pair.idle = SANTA_TEX;
        pair.punch = SANTA_PUNCH_TEX;
    } else if (look == LOOK_EBUC) {
        pair.idle = EBUC_TEX;
        pair.punch = EBUC_PUNCH_TEX;
    } else if (look == LOOK_ELF) {
        pair.idle = EBUC_ELF_TEX;
        pair.punch = EBUC_ELF_PUNCH_TEX;
    } else if (look == LOOK_ORDINARY) {
        pair.idle = ORDINARY_TEX;
        pair.punch = PUNCH_TEX;
    }
    return pair;
}

static const char *look_idle_tex(double look) { return look_sprite_pair(look).idle; }

static const char *look_punch_tex(double look) { return look_sprite_pair(look).punch; }

static const char *look_tex(double look, double punching) {
    if (punching == 1) {
        return look_punch_tex(look);
    }
    return look_idle_tex(look);
}

static double look_ready(double look) {
    if (look == LOOK_NONE) {
        return 0;
    }
    FighterSpritePair pair = look_sprite_pair(look);
    /* The idle and punch textures are resolved as one pair for the same look,
     * so changing pose can never change the fighter's class or skin. */
    return tex_ready(pair.idle) == 1 && tex_ready(pair.punch) == 1;
}

static double look_swing_ready(double look) { return look_ready(look); }

static double look_base(double look) {
    if (look == LOOK_ZOMBIE) {
        return LOOK_AZUM;
    }
    if (look == LOOK_ELF) {
        return LOOK_EBUC;
    }
    return look;
}

static double class_look(double cls, double skin) {
    if (skin_is_active(cls, skin) == 1) {
        if (cls == CLASS_EBUC) {
            return LOOK_ELF;
        }
        return LOOK_ZOMBIE;
    }
    if (cls == CLASS_AZUM) {
        return LOOK_AZUM;
    }
    if (cls == CLASS_SANTA) {
        return LOOK_SANTA;
    }
    if (cls == CLASS_EBUC) {
        return LOOK_EBUC;
    }
    return LOOK_ORDINARY;
}

static double drawable_look(double cls, double skin) {
    double want = class_look(cls, skin);
    if (look_ready(want) == 1) {
        return want;
    }
    double base = look_base(want);
    if (base != want && look_ready(base) == 1) {
        return base;
    }
    return LOOK_NONE;
}

static double card_look(double cls, double skin) {
    double look = class_look(cls, skin);
    if (look_ready(look) == 1) {
        return look;
    }
    return LOOK_NONE;
}

static GAME_MAYBE_UNUSED const char *class_idle_tex(double cls) { return look_idle_tex(class_look(cls, SKIN_NORMAL)); }

static GAME_MAYBE_UNUSED const char *class_punch_tex(double cls) {
    return look_punch_tex(class_look(cls, SKIN_NORMAL));
}

static GAME_MAYBE_UNUSED const char *skin_idle_tex_for(double cls, double skin) {
    return look_idle_tex(class_look(cls, skin));
}

static double class_has_super(double cls) {
    if (cls == CLASS_AZUM || cls == CLASS_SANTA || cls == CLASS_EBUC || cls == CLASS_ASTRA) {
        return 1;
    }
    return 0;
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
    if (cls == CLASS_ASTRA) {
        return astra_hp;
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

static double class_punch_damage(double cls) { return class_punch_damage_for(cls, player_level); }

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
