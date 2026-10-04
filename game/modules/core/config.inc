static double admin_slot(const char *nick) {
    if (str_eq(nick, ADMIN_NICK) == 1) {
        return 1;
    }
    if (str_eq(nick, ADMIN2_NICK) == 1) {
        return 2;
    }
    return 0;
}

static const char *admin_nick_of(double slot) {
    if (slot == 2) {
        return ADMIN2_NICK;
    }
    return ADMIN_NICK;
}

static double admin_color_of(const char *nick, double base) {
    if (str_eq(nick, ADMIN_NICK) == 1) {
        return admin_color;
    }
    if (str_eq(nick, ADMIN2_NICK) == 1) {
        return admin2_color;
    }
    return base;
}

static double admin_head_color_of(const char *nick, double base) {
    if (str_eq(nick, ADMIN_NICK) == 1) {
        return admin_head_color;
    }
    if (str_eq(nick, ADMIN2_NICK) == 1) {
        return admin2_head_color;
    }
    return base;
}
