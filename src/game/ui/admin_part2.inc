/* Admin panel screen half: hit-testing, the touch handler, all drawing and
 * the continuous cheat tick. Split out of admin.inc to keep sources under
 * the 450-line project limit; helpers and labels live in admin.inc above. */

static void admin_panel_press(double x, double y) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    double cxx = px + pw - 14;
    double dx = x - cxx;
    double dy = y - (py + 15);
    if (sqrt(dx * dx + dy * dy) <= 13) {
        admin_panel_open = 0;
        play_button_click();
        return;
    }
    if (hit_rect(x, y, px, py, pw, ph) == 0) {
        admin_panel_open = 0;
        return;
    }
    double tw = (pw - 2 * admin_pad() - 2 * admin_gap()) / 3;
    double t = 0;
    while (t < 3) {
        if (hit_rect(x, y, px + admin_pad() + t * (tw + admin_gap()), py + 36, tw, 20) == 1) {
            if (admin_tab != t) {
                admin_tab = t;
                play_button_click();
            }
            return;
        }
        t = t + 1;
    }
    double bw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double row = floor((y - (py + 62)) / (admin_row_h() + admin_gap()));
    if (row < 0 || row > 2 || x < px + admin_pad() || x > px + pw - admin_pad()) {
        return;
    }
    double col = 0;
    if (x >= px + admin_pad() + bw + admin_gap()) {
        col = 1;
    }
    admin_press(row * 2 + col);
}

static double admin_ball_hit(double x, double y) {
    double dx = x - admin_ball_x;
    double dy = y - admin_ball_y;
    return sqrt(dx * dx + dy * dy) <= admin_ball_r() + 9;
}

static double admin_touch(double x, double y, double action, double id) {
    (void)id;
    if (is_admin() == 0 || warn_open == 1 || studio_open == 1) {
        return 0;
    }
    /* The panel is modal: every touch while it is open goes to it. */
    if (admin_panel_open == 1) {
        if (action == 0) {
            admin_panel_press(x, y);
        }
        return 1;
    }
    if (admin_ball_grab == 1) {
        if (action == 2 || action == 1) {
            admin_ball_x = clamp(x - admin_ball_grab_dx, 16, screen_w - 16);
            admin_ball_y = clamp(y - admin_ball_grab_dy, 18, screen_h - 18);
        }
        if (action == 1 || action == 3) {
            admin_ball_grab = 0;
            double dx = x - admin_ball_px;
            double dy = y - admin_ball_py;
            if (action == 1 && sqrt(dx * dx + dy * dy) <= 11) {
                admin_panel_open = 1;
                play_button_click();
            }
        }
        return 1;
    }
    if (action == 0 && admin_ball_hit(x, y) == 1) {
        admin_ball_grab = 1;
        admin_ball_px = x;
        admin_ball_py = y;
        admin_ball_grab_dx = x - admin_ball_x;
        admin_ball_grab_dy = y - admin_ball_y;
        return 1;
    }
    return 0;
}

static void admin_draw_toggle(double x, double y, double w, double h, const char *label, double on) {
    if (on == 1) {
        roundrect(x, y, w, h, 9, admin_a(0x00E5FF, 0.88));
        text_in_box(label, x, y, w, h, 0x0D0D22, fit_text_scale(label, w - 12, 0.42));
        return;
    }
    roundrect(x, y, w, h, 9, admin_a(0x1D1D3C, 0.92));
    roundrect_outline(x, y, w, h, 9, 1, admin_a(0x4A4A7A, 0.9));
    text_in_box(label, x, y, w, h, 0x9AA3CC, fit_text_scale(label, w - 12, 0.42));
}

static void admin_draw_action(double x, double y, double w, double h, const char *label, double enabled) {
    if (enabled == 1) {
        roundrect(x, y, w, h, 9, admin_a(0x7C4DFF, 0.92));
        text_in_box(label, x, y, w, h, 0xFFFFFF, fit_text_scale(label, w - 12, 0.42));
        return;
    }
    roundrect(x, y, w, h, 9, locked_bg());
    text_in_box(label, x, y, w, h, locked_text(), fit_text_scale(label, w - 12, 0.42));
}

static void admin_draw_ball(void) {
    double r = admin_ball_r() + sin(admin_ball_t * 2.6) * 1.3;
    double cx = admin_ball_x;
    double cy = admin_ball_y;
    circle(cx, cy, r + 8, admin_a(0x00E5FF, 0.10));
    circle(cx, cy, r + 4, admin_a(0x7C4DFF, 0.16));
    circle(cx, cy, r, admin_a(0x141432, 0.92));
    circle(cx - r * 0.22, cy - r * 0.26, r * 0.62, admin_a(0x7C4DFF, 0.60));
    circle(cx + r * 0.30, cy + r * 0.34, r * 0.48, admin_a(0x00E5FF, 0.30));
    ring(cx, cy, r + 1, 1.6, admin_a(0x00E5FF, 0.85));
    circle(cx - r * 0.40, cy - r * 0.44, r * 0.20, admin_a(0xFFFFFF, 0.85));
    double sc = 0.5;
    double tw = text_ink_width("A") * sc;
    double th = text_ink_height("A") * sc;
    double tt = text_ink_top("A") * sc;
    text_scaled("A", cx - tw / 2, cy - th / 2 - tt, 0xFFD54F, sc);
}

static void admin_draw_panel(void) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    rect(0, 0, screen_w, screen_h, admin_a(0x000000, 0.55));
    roundrect(px - 3, py - 3, pw + 6, ph + 6, 17, admin_a(0x7C4DFF, 0.20));
    roundrect(px, py, pw, ph, 14, admin_a(0x0B0B1E, 0.97));
    roundrect_outline(px, py, pw, ph, 14, 1.5, admin_a(0x00E5FF, 0.55));
    /* Header: violet band, golden title, the admin's own nick, red close orb. */
    roundrect(px, py, pw, 30, 14, admin_a(0x241A4D, 0.98));
    rect(px, py + 15, pw, 15, admin_a(0x241A4D, 0.98));
    line(px + 10, py + 29, px + pw - 10, py + 29, 1.2, admin_a(0xFFD54F, 0.45));
    text_in_box("ADMIN", px + 8, py, 70, 30, 0xFFD54F, 0.52);
    const char *nick = net_login_nick();
    text_in_box(nick, px + 80, py, pw - 110, 30, admin_color_of(nick, text_color), 0.3);
    double cxx = px + pw - 14;
    circle(cxx, py + 15, 10, admin_a(0xE94560, 0.95));
    text_in_box("X", cxx - 10, py + 5, 20, 20, 0xFFFFFF, 0.36);
    /* Tabs. */
    double tw = (pw - 2 * admin_pad() - 2 * admin_gap()) / 3;
    double t = 0;
    while (t < 3) {
        double tx = px + admin_pad() + t * (tw + admin_gap());
        const char *tl = admin_tab_label(t);
        if (admin_tab == t) {
            roundrect(tx, py + 36, tw, 20, 8, admin_a(0x00E5FF, 0.9));
            text_in_box(tl, tx, py + 36, tw, 20, 0x0D0D22, fit_text_scale(tl, tw - 10, 0.4));
        } else {
            roundrect(tx, py + 36, tw, 20, 8, admin_a(0x1D1D3C, 0.92));
            text_in_box(tl, tx, py + 36, tw, 20, 0x9AA3CC, fit_text_scale(tl, tw - 10, 0.4));
        }
        t = t + 1;
    }
    /* Two-column grid of controls. */
    double bw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double idx = 0;
    while (idx < 6) {
        double row = floor(idx / 2);
        double col = idx - row * 2;
        double bx = px + admin_pad() + col * (bw + admin_gap());
        double by = py + 62 + row * (admin_row_h() + admin_gap());
        if (admin_tab == 0) {
            admin_draw_toggle(bx, by, bw, admin_row_h(), admin_cheat_label(idx), admin_cheat_value(idx));
        } else if (admin_tab == 1) {
            admin_draw_action(bx, by, bw, admin_row_h(), admin_fight_label(idx), admin_fight_enabled(idx));
        } else {
            admin_draw_action(bx, by, bw, admin_row_h(), admin_game_label(idx), 1);
        }
        idx = idx + 1;
    }
    if (admin_msg_t > 0 && admin_msg != 0 && str_len(admin_msg) > 0) {
        const char *m = admin_msg;
        if (admin_msg_state >= 0) {
            m = ds_concat(ds_concat(admin_msg, ": "), admin_on_off(admin_msg_state));
        }
        ctext_scaled(m, py + ph - 13, 0xFFD54F, 0.34);
    }
}

static void admin_draw(void) {
    if (is_admin() == 0 || warn_open == 1 || studio_open == 1 || t_dir != 0) {
        return;
    }
    if (admin_ball_x == 0 && admin_ball_y == 0) {
        admin_ball_x = screen_w - 30;
        admin_ball_y = screen_h * 0.42;
    }
    if (admin_panel_open == 1) {
        admin_draw_panel();
    }
    admin_draw_ball();
}

/* Continuous cheats: ticked from logic_update so anti-stun and the frozen
 * enemy survive every re-application the fight throws at the player. */
static void admin_tick(void) {
    admin_ball_t = admin_ball_t + dt;
    if (admin_msg_t > 0) {
        admin_msg_t = tick_down(admin_msg_t);
    }
    if ((cheat_cc != 1 && cheat_estun != 1) || is_admin() == 0 || admin_in_battle() == 0 || finished != 0) {
        return;
    }
    if (cheat_cc == 1) {
        player_stun = 0;
        player_freeze = 0;
        player_poison = 0;
    }
    if (cheat_estun == 1 && game_state == ST_SOLO) {
        if (enemy->stun < 0.25) {
            enemy->stun = 0.25;
        }
    }
}

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
