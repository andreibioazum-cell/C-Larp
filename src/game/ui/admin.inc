/* Admin panel: a small black-and-white "CCP" orb that only the two admin
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

/* Compact panel: a bit over half of the screen wide and only as tall as its
 * content needs, so it never stretches over the whole display. Every button in
 * it shares one flat black-and-white style. */
static double admin_pad(void) { return 10; }

static double admin_gap(void) { return 6; }

/* Chrome heights are measured from the screen, never from the panel, so the
 * panel height stays a plain function of the content. */
static double admin_header_h(void) { return clamp(screen_h * 0.03, 36, 56); }

static double admin_tabs_h(void) { return clamp(screen_h * 0.025, 28, 44); }

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

/* Base row sizes: one cheat button, one list row, one field. */
static double admin_row_h(void) { return clamp(screen_h * 0.055, 30, 64); }

static double admin_player_row_h(void) { return clamp(screen_h * 0.04, 26, 44); }

static double admin_players_field_h(void) { return clamp(admin_row_h() * 0.9, 30, 56); }

static double admin_footer_h(void) { return clamp(screen_h * 0.028, 26, 40); }

/* Six cheats sit in three rows of two. */
static double admin_grid_h(void) { return 3 * admin_row_h() + 2 * admin_gap(); }

/* Players tab needs the nick field, the ban / unban buttons, two rows and the
 * footer under them. */
static double admin_players_min_h(void) {
    double fh = admin_players_field_h();
    double rows = 2 * admin_player_row_h() + admin_gap();
    return fh + admin_gap() + fh + admin_gap() + rows + admin_gap() + admin_footer_h();
}

/* Both tabs share one content height, so switching tabs never resizes the
 * panel; the cheat grid is centred in whatever room the Players tab asks for. */
static double admin_content_h(void) {
    double grid = admin_grid_h();
    double players = admin_players_min_h();
    return grid > players ? grid : players;
}

static double admin_cheat_row_h(void) {
    double h = (admin_content_h() - 2 * admin_gap()) / 3;
    double cap = admin_row_h();
    return h > cap ? cap : h;
}

/* Header, tabs and the taller tab decide the height. */
static double admin_panel_h(void) {
    double lim = screen_h - 32;
    double h = admin_header_h() + admin_gap() + admin_tabs_h() + 2 * admin_gap() + admin_content_h() + admin_pad();
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

/* The cheat grid is centred while the Players tab fills the same box. */
static double admin_grid_top(void) { return admin_content_top() + (admin_content_h() - admin_grid_h()) / 2; }

/* The Players tab: nick field, then the ban / unban row, then the list. */
static double admin_players_field_y(void) { return admin_content_top(); }

static double admin_players_act_y(void) { return admin_players_field_y() + admin_players_field_h() + admin_gap(); }

static double admin_list_top(void) { return admin_players_act_y() + admin_players_field_h() + admin_gap(); }

static double admin_list_bottom(void) { return admin_content_bottom() - admin_footer_h() - admin_gap(); }

static double admin_players_per_page(void) {
    double avail = admin_list_bottom() - admin_list_top();
    double n = floor((avail + admin_gap()) / (admin_player_row_h() + admin_gap()));
    if (n < 1) {
        n = 1;
    }
    return n;
}

static double admin_ball_r(void) { return clamp(screen_h / 34.0, 15, 26); }

/* Monochrome palette: black surfaces, white outlines, grey for "on". */
static double admin_body(void) { return 0x000000; }

static double admin_header(void) { return 0x0D0D0D; }

static double admin_frame(void) { return 0xFFFFFF; }

/* Switched on / selected: the fill only turns grey. Nothing turns white, so
 * flipping a switch is barely noticeable at a glance. */
static double admin_on(void) { return 0x333333; }

static double admin_off(void) { return 0x000000; }

static double admin_dim_text(void) { return 0xB4B4B4; }

/* One flat style for every button of the panel: same radius, same 2 px white
 * outline, fully opaque fills and white text in both states. Off and on differ
 * in nothing but the fill colour - black against grey - so a toggle or a tab
 * has no flash, fade or highlight to look at. */
static double admin_btn_r(void) { return 9; }

static void admin_btn(const char *label, double x, double y, double w, double h, double on, double scale) {
    roundrect(x, y, w, h, admin_btn_r(), on == 1 ? admin_on() : admin_off());
    roundrect_outline(x, y, w, h, admin_btn_r(), 2, admin_frame());
    text_in_box(label, x, y, w, h, admin_frame(), scale);
}

/* The orb wears the short name of the panel. */
static const char *admin_orb_label(void) { return "CCP"; }

static const char *admin_tab_label(double t) {
    if (t == 0) {
        return tr_admin_cheats_tab();
    }
    return tr_admin_players_tab();
}

/* Players tab: the nick field talks to the engine keyboard directly. The nick
 * is copied into our own buffer, because the keyboard buffer is shared with
 * the chat and login fields and outlives the panel. */
static char admin_nick_buf[64];

static void admin_nick_copy(const char *src) {
    int i = 0;
    if (src == 0) {
        src = "";
    }
    while (i < 63 && src[i] != 0) {
        admin_nick_buf[i] = src[i];
        i = i + 1;
    }
    admin_nick_buf[i] = 0;
}

static void admin_input_sync(void) {
    if (admin_input_active == 1 && keyboard_visible() == 1) {
        char text[64];
        keyboard_get_text_to(text, sizeof(text));
        admin_nick_copy(text);
        admin_input_nick = admin_nick_buf;
    }
}

static void admin_input_focus(void) {
    admin_input_active = 1;
    keyboard_clear();
    if (str_len(admin_input_nick) > 0) {
        keyboard_type(admin_input_nick);
    }
    keyboard_show();
}

static void admin_input_blur(void) {
    if (admin_input_active == 0) {
        return;
    }
    admin_input_sync();
    admin_input_active = 0;
    keyboard_hide();
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

/* True while at least one cheat switch is on. */
static double admin_any_cheat_on(void) {
    if (cheat_god == 1 || cheat_damage == 1 || cheat_speed == 1 || cheat_nocd == 1 || cheat_cc == 1 ||
        cheat_estun == 1) {
        return 1;
    }
    return 0;
}

/* Called when a battle starts: entering a fight with a cheat already on marks
 * it just like flipping one on during the fight does. */
static void admin_battle_stain_reset(void) { cheat_stain = admin_any_cheat_on(); }

/* A stained battle pays no Cups at the end. */
static double admin_cups_allowed(void) { return cheat_stain == 0 ? 1 : 0; }

static void admin_press_cheat(double idx) {
    double v = admin_cheat_value(idx);
    if (v == 1) {
        v = 0;
    } else {
        v = 1;
    }
    if (v == 1) {
        /* Switching any cheat on leaves a mark on the current battle: a win
         * then pays no Cups, even if the switch is flipped back off at once. */
        cheat_stain = 1;
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
