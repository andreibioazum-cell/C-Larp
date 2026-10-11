/* Admin panel: a small black-and-white "A" orb that only the two admin
 * accounts see. Drag it anywhere, tap it to open a compact monochrome panel
 * with two tabs - cheats and players. The panel is drawn above every screen,
 * above the transition curtain and above the epilepsy warning, so it never
 * hides itself, and nothing here prints a status line under the panel.
 * Everything is session-local and gated behind is_admin(), the same check the
 * admin chat commands use, so a copied progress file cannot summon the orb. */

static double admin_a(double rgb, double a) {
    return (1 + floor(a * 254)) * 0x1000000 + rgb;
}

static double admin_in_battle(void) {
    return game_state == ST_SOLO || game_state == ST_ONLINE;
}

/* Compact panel: a bit over half of the screen wide, and only as tall as the
 * cheat grid needs, so it never stretches over the whole display. */
static double admin_pad(void) { return 10; }

static double admin_gap(void) { return 6; }

static double admin_header_h(void) { return clamp(screen_h * 0.055, 34, 56); }

static double admin_tabs_h(void) { return clamp(screen_h * 0.045, 26, 44); }

/* Three cheat rows, two buttons per row. */
static double admin_row_h(void) { return clamp(screen_h * 0.075, 30, 76); }

static double admin_panel_w(void) {
    double lim = screen_w - 24;
    double w = screen_w * 0.58;
    double cap = screen_h * 1.1;
    if (w > cap) {
        w = cap;
    }
    if (w > lim) {
        w = lim;
    }
    if (w < 180) {
        w = 180;
    }
    return w;
}

/* Header, tabs and the three cheat rows decide the height. */
static double admin_panel_h(void) {
    double lim = screen_h - 32;
    double h = admin_header_h() + admin_gap() + admin_tabs_h() + 2 * admin_gap() + 3 * admin_row_h() + 2 * admin_gap() +
               admin_pad();
    if (h > lim) {
        h = lim;
    }
    if (h < 180) {
        h = 180;
    }
    if (h > lim) {
        h = lim;
    }
    return h;
}

static double admin_panel_x(void) { return (screen_w - admin_panel_w()) / 2; }

static double admin_panel_y(void) { return (screen_h - admin_panel_h()) / 2; }

static double admin_tabs_y(void) { return admin_panel_y() + admin_header_h() + admin_gap(); }

static double admin_content_top(void) { return admin_tabs_y() + admin_tabs_h() + 2 * admin_gap(); }

static double admin_content_bottom(void) { return admin_panel_y() + admin_panel_h() - admin_pad(); }

/* The Players tab keeps a footer with its page controls. */
static double admin_footer_h(void) { return clamp(admin_panel_h() * 0.11, 26, 42); }

static double admin_list_bottom(void) { return admin_content_bottom() - admin_footer_h() - admin_gap(); }

static double admin_player_row_h(void) { return clamp(screen_h * 0.05, 28, 44); }

static double admin_players_per_page(void) {
    double avail = admin_list_bottom() - admin_content_top();
    double n = floor(avail / (admin_player_row_h() + admin_gap()));
    if (n < 1) {
        n = 1;
    }
    return n;
}

static double admin_ball_r(void) { return clamp(screen_h / 34.0, 15, 26); }

/* Monochrome palette: black surfaces, white outlines, grey secondary text. */
static double admin_body(void) { return 0x000000; }

static double admin_header(void) { return 0x0D0D0D; }

static double admin_frame(void) { return 0xFFFFFF; }

static double admin_on(void) { return 0xFFFFFF; }

static double admin_off(void) { return 0x000000; }

static double admin_dim_text(void) { return 0xB4B4B4; }

static const char *admin_tab_label(double t) {
    if (t == 0) {
        return tr_admin_cheats_tab();
    }
    return tr_admin_players_tab();
}

static const char *admin_cheat_label(double idx) {
    if (language == 1) {
        if (idx == 0) {
            return "Бессмертие";
        }
        if (idx == 1) {
            return "Урон x5";
        }
        if (idx == 2) {
            return "Скорость x2";
        }
        if (idx == 3) {
            return "Без КД";
        }
        if (idx == 4) {
            return "Анти-стан";
        }
        return "Стан врага";
    }
    if (idx == 0) {
        return "God mode";
    }
    if (idx == 1) {
        return "Damage x5";
    }
    if (idx == 2) {
        return "Speed x2";
    }
    if (idx == 3) {
        return "No cooldown";
    }
    if (idx == 4) {
        return "Anti-stun";
    }
    return "Stun enemy";
}

static double admin_cheat_value(double idx) {
    if (idx == 0) {
        return cheat_god;
    }
    if (idx == 1) {
        return cheat_damage;
    }
    if (idx == 2) {
        return cheat_speed;
    }
    if (idx == 3) {
        return cheat_nocd;
    }
    if (idx == 4) {
        return cheat_cc;
    }
    return cheat_estun;
}

static void admin_press_cheat(double idx) {
    double v = admin_cheat_value(idx);
    if (v == 1) {
        v = 0;
    } else {
        v = 1;
    }
    if (idx == 0) {
        cheat_god = v;
    }
    if (idx == 1) {
        cheat_damage = v;
    }
    if (idx == 2) {
        cheat_speed = v;
    }
    if (idx == 3) {
        cheat_nocd = v;
    }
    if (idx == 4) {
        cheat_cc = v;
    }
    if (idx == 5) {
        cheat_estun = v;
    }
}

static void admin_press(double idx) {
    play_button_click();
    admin_press_cheat(idx);
}

/* Cheats read by the fight code every frame. */
static double admin_cheat_god(void) {
    return cheat_god;
}

static double admin_cheat_damage_mult(void) {
    if (cheat_damage == 1) {
        return 5;
    }
    return 1;
}

static double admin_cheat_speed_mult(void) {
    if (cheat_speed == 1) {
        return 2;
    }
    return 1;
}

static double admin_cheat_nocd(void) {
    return cheat_nocd;
}
