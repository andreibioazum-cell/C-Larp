/* Admin panel screen half: hit-testing, the touch handler, all drawing and
 * the continuous cheat tick. Helpers and labels live in admin.inc above. */

static void admin_panel_press(double x, double y) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    double hh = clamp(ph * 0.11, 44, 120);
    double tabs_y = py + hh + 8;
    double tabs_h = clamp(hh * 0.62, 30, 74);
    double cxx = px + pw - hh * 0.42;
    double dx = x - cxx;
    double dy = y - (py + hh / 2);
    if (sqrt(dx * dx + dy * dy) <= hh * 0.34 + 6) {
        admin_panel_open = 0;
        play_button_click();
        return;
    }
    if (hit_rect(x, y, px, py, pw, ph) == 0) {
        admin_panel_open = 0;
        return;
    }
    double tw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double t = 0;
    while (t < 2) {
        if (hit_rect(x, y, px + admin_pad() + t * (tw + admin_gap()), tabs_y, tw, tabs_h) == 1) {
            if (admin_tab != t) {
                admin_tab = t;
                play_button_click();
            }
            return;
        }
        t = t + 1;
    }
    double bw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double row = floor((y - (tabs_y + tabs_h + 10)) / (admin_row_h() + admin_gap()));
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
    return sqrt(dx * dx + dy * dy) <= admin_ball_r() + 14;
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
            double br = admin_ball_r();
            admin_ball_x = clamp(x - admin_ball_grab_dx, br + 4, screen_w - br - 4);
            admin_ball_y = clamp(y - admin_ball_grab_dy, br + 6, screen_h - br - 6);
        }
        if (action == 1 || action == 3) {
            admin_ball_grab = 0;
            double dx = x - admin_ball_px;
            double dy = y - admin_ball_py;
            if (action == 1 && sqrt(dx * dx + dy * dy) <= 13) {
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
    double sc = fit_text_scale(label, w - 20, h / 79.0);
    if (on == 1) {
        roundrect(x, y, w, h, 12, admin_a(admin_on(), 0.95));
        roundrect_outline(x, y, w, h, 12, 3, admin_a(0xE86E90, 0.9));
        text_in_box(label, x, y, w, h, 0xFFFFFF, sc);
        return;
    }
    roundrect(x, y, w, h, 12, admin_a(admin_off(), 0.94));
    roundrect_outline(x, y, w, h, 12, 3, admin_a(admin_frame(), 0.75));
    text_in_box(label, x, y, w, h, admin_a(admin_dim_text(), 0.95), sc);
}

static void admin_draw_action(double x, double y, double w, double h, const char *label, double enabled) {
    double sc = fit_text_scale(label, w - 20, h / 79.0);
    if (enabled == 1) {
        roundrect(x, y, w, h, 12, admin_a(admin_action(), 0.96));
        roundrect_outline(x, y, w, h, 12, 3, admin_a(admin_frame(), 0.9));
        text_in_box(label, x, y, w, h, 0xFFFFFF, sc);
        return;
    }
    roundrect(x, y, w, h, 12, admin_a(admin_off(), 0.94));
    text_in_box(label, x, y, w, h, 0x6E4A58, sc);
}

static void admin_draw_ball(void) {
    double r = admin_ball_r() + sin(admin_ball_t * 2.2) * (admin_ball_r() * 0.06);
    double cx = admin_ball_x;
    double cy = admin_ball_y;
    circle(cx, cy, r + r * 0.4, admin_a(0x5A1028, 0.16));
    circle(cx, cy, r, admin_a(0x34091A, 0.95));
    circle(cx - r * 0.24, cy - r * 0.26, r * 0.60, admin_a(0x7E1E3E, 0.85));
    circle(cx + r * 0.30, cy + r * 0.32, r * 0.42, admin_a(0x43101F, 0.9));
    ring(cx, cy, r + 1.5, r * 0.1, admin_a(0xC04A6A, 0.95));
    circle(cx - r * 0.38, cy - r * 0.42, r * 0.16, admin_a(0xFFE1EA, 0.9));
    double sc = r * 0.03;
    double tw = text_ink_width("A") * sc;
    double th = text_ink_height("A") * sc;
    double tt = text_ink_top("A") * sc;
    text_scaled("A", cx - tw / 2, cy - th / 2 - tt, 0xFFFFFF, sc);
}

static void admin_draw_panel(void) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    double hh = clamp(ph * 0.11, 44, 120);
    double tabs_y = py + hh + 8;
    double tabs_h = clamp(hh * 0.62, 30, 74);
    double grid_top = tabs_y + tabs_h + 10;
    double msg_h = clamp(ph * 0.06, 20, 44);
    double row_h = admin_row_h();
    rect(0, 0, screen_w, screen_h, admin_a(0x000000, 0.6));
    roundrect(px - 4, py - 4, pw + 8, ph + 8, 20, admin_a(admin_frame(), 0.35));
    roundrect(px, py, pw, ph, 16, admin_a(admin_body(), 0.98));
    roundrect_outline(px, py, pw, ph, 16, 3, admin_a(admin_frame(), 0.95));
    /* Header: deep burgundy band, white title, the admin's nick, close orb. */
    roundrect(px, py, pw, hh, 16, admin_a(admin_header(), 0.98));
    rect(px, py + hh / 2, pw, hh / 2, admin_a(admin_header(), 0.98));
    line(px + 12, py + hh - 1, px + pw - 12, py + hh - 1, 2, admin_a(0xC04A6A, 0.5));
    text_in_box("ADMIN", px + 16, py, pw * 0.3, hh, 0xFFFFFF, hh / 61.0);
    const char *nick = net_login_nick();
    text_in_box(nick, px + pw * 0.34, py, pw * 0.44, hh, admin_color_of(nick, 0xD8AABB), hh / 105.0);
    double cxx = px + pw - hh * 0.42;
    double cr = hh * 0.34;
    circle(cxx, py + hh / 2, cr, admin_a(0x8E1E36, 0.98));
    ring(cxx, py + hh / 2, cr + 1.5, 2.5, admin_a(0xE07090, 0.9));
    text_in_box("X", cxx - cr, py + hh / 2 - cr, cr * 2, cr * 2, 0xFFFFFF, cr / 33.0);
    /* Tabs. */
    double tw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double t = 0;
    while (t < 2) {
        double tx = px + admin_pad() + t * (tw + admin_gap());
        const char *tl = admin_tab_label(t);
        double sc = fit_text_scale(tl, tw - 20, tabs_h / 57.0);
        if (admin_tab == t) {
            roundrect(tx, tabs_y, tw, tabs_h, 10, admin_a(admin_on(), 0.96));
            text_in_box(tl, tx, tabs_y, tw, tabs_h, 0xFFFFFF, sc);
        } else {
            roundrect(tx, tabs_y, tw, tabs_h, 10, admin_a(admin_off(), 0.94));
            roundrect_outline(tx, tabs_y, tw, tabs_h, 10, 3, admin_a(admin_frame(), 0.75));
            text_in_box(tl, tx, tabs_y, tw, tabs_h, admin_a(admin_dim_text(), 0.95), sc);
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
        double by = grid_top + row * (row_h + admin_gap());
        if (admin_tab == 0) {
            admin_draw_toggle(bx, by, bw, row_h, admin_cheat_label(idx), admin_cheat_value(idx));
        } else {
            admin_draw_action(bx, by, bw, row_h, admin_fight_label(idx), admin_fight_enabled(idx));
        }
        idx = idx + 1;
    }
    if (admin_msg_t > 0 && admin_msg != 0 && str_len(admin_msg) > 0) {
        const char *m = admin_msg;
        if (admin_msg_state >= 0) {
            m = ds_concat(ds_concat(admin_msg, ": "), admin_on_off(admin_msg_state));
        }
        ctext_scaled(m, py + ph - msg_h, 0xFF9EB4, msg_h / 38.0);
    }
}

static void admin_draw(void) {
    if (is_admin() == 0 || warn_open == 1 || studio_open == 1 || t_dir != 0) {
        return;
    }
    if (admin_ball_x == 0 && admin_ball_y == 0) {
        admin_ball_x = screen_w - (admin_ball_r() + 14);
        admin_ball_y = screen_h * 0.40;
    }
    if (admin_panel_open == 1) {
        admin_draw_panel();
    }
    admin_draw_ball();
}

/* Continuous cheats: ticked from logic_update so anti-stun, the frozen enemy
 * and the zeroed cooldowns survive every re-application the fight makes. */
static void admin_tick(void) {
    admin_ball_t = admin_ball_t + dt;
    if (admin_msg_t > 0) {
        admin_msg_t = tick_down(admin_msg_t);
    }
    if (is_admin() == 0 || admin_in_battle() == 0 || finished != 0) {
        return;
    }
    if (cheat_nocd == 1) {
        /* Everything ability-related refills instantly, solo and online. */
        punch_cd = 0;
        super_cd = 0;
        dash_cd = 0;
        astra_cd = 0;
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
