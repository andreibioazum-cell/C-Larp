/* Admin panel input and chrome: hit-testing, the touch handler, the "A" orb,
 * the panel frame, the two tabs, the cheats grid and the continuous cheat
 * tick. The players tab lives in admin_part3.inc. */

static void admin_panel_press(double x, double y) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double hh = admin_header_h();
    double tabs_y = admin_tabs_y();
    double tabs_h = admin_tabs_h();
    double cxx = px + pw - hh * 0.55;
    double dx = x - cxx;
    double dy = y - (py + hh / 2);
    if (sqrt(dx * dx + dy * dy) <= hh * 0.38 + 8) {
        admin_panel_open = 0;
        play_button_click();
        return;
    }
    if (hit_rect(x, y, px, py, pw, admin_panel_h()) == 0) {
        admin_panel_open = 0;
        return;
    }
    double tw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double t = 0;
    while (t < 2) {
        if (hit_rect(x, y, px + admin_pad() + t * (tw + admin_gap()), tabs_y, tw, tabs_h) == 1) {
            if (admin_tab != t) {
                admin_tab = t;
                admin_page = 0;
                play_button_click();
            }
            return;
        }
        t = t + 1;
    }
    if (admin_tab == 1) {
        admin_players_press(x, y);
        return;
    }
    double bw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double row = floor((y - admin_content_top()) / (admin_row_h() + admin_gap()));
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
    if (is_admin() == 0) {
        return 0;
    }
    /* The panel is modal: every touch while it is open goes to it, on top of
     * every screen, the transition curtain and the epilepsy warning. */
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
                admin_page = 0;
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

/* Black disc with a white rim and a white "A" - no gradients, no decorations. */
static void admin_draw_ball(void) {
    double r = admin_ball_r();
    double cx = admin_ball_x;
    double cy = admin_ball_y;
    double ih = text_ink_height("A");
    double sc = 0.5;
    circle(cx, cy, r + 3, admin_a(0x000000, 0.30));
    circle(cx, cy, r, admin_a(admin_body(), 0.98));
    ring(cx, cy, r, clamp(r * 0.11, 2, 3), admin_a(admin_frame(), 0.95));
    if (ih > 0) {
        sc = (r * 1.05) / ih;
    }
    double tw = text_ink_width("A") * sc;
    double th = text_ink_height("A") * sc;
    double tt = text_ink_top("A") * sc;
    text_scaled("A", cx - tw / 2, cy - th / 2 - tt, admin_a(admin_frame(), 1), sc);
}

static void admin_draw_toggle(double x, double y, double w, double h, const char *label, double on) {
    double sc = fit_text_scale(label, w - 16, h / 74.0);
    if (on == 1) {
        roundrect(x, y, w, h, 10, admin_a(admin_on(), 0.96));
        text_in_box(label, x, y, w, h, 0x000000, sc);
        return;
    }
    roundrect(x, y, w, h, 10, admin_a(admin_off(), 0.95));
    roundrect_outline(x, y, w, h, 10, 2, admin_a(admin_frame(), 0.45));
    text_in_box(label, x, y, w, h, admin_a(admin_dim_text(), 0.95), sc);
}

static void admin_draw_cheats(void) {
    double px = admin_panel_x() + admin_pad();
    double pw = admin_panel_w() - 2 * admin_pad();
    double bw = (pw - admin_gap()) / 2;
    double rh = admin_row_h();
    double idx = 0;
    while (idx < 6) {
        double row = floor(idx / 2);
        double col = idx - row * 2;
        double bx = px + col * (bw + admin_gap());
        double by = admin_content_top() + row * (rh + admin_gap());
        admin_draw_toggle(bx, by, bw, rh, admin_cheat_label(idx), admin_cheat_value(idx));
        idx = idx + 1;
    }
}

/* Rounded black button with a white rim; used by the Players footer. */
static void admin_draw_small_btn(const char *label, double x, double y, double w, double h, double enabled) {
    double sc = fit_text_scale(label, w - 14, h / 62.0);
    if (enabled == 1) {
        roundrect_outline(x, y, w, h, 9, 2, admin_a(admin_frame(), 0.85));
        text_in_box(label, x, y, w, h, admin_a(admin_frame(), 1), sc);
        return;
    }
    roundrect_outline(x, y, w, h, 9, 2, admin_a(admin_frame(), 0.25));
    text_in_box(label, x, y, w, h, admin_a(admin_dim_text(), 0.5), sc);
}

static void admin_draw_panel(void) {
    double px = admin_panel_x();
    double py = admin_panel_y();
    double pw = admin_panel_w();
    double ph = admin_panel_h();
    double hh = admin_header_h();
    double tabs_y = admin_tabs_y();
    double tabs_h = admin_tabs_h();
    rect(0, 0, screen_w, screen_h, admin_a(0x000000, 0.55));
    roundrect(px, py, pw, ph, 14, admin_a(admin_body(), 0.98));
    roundrect_outline(px, py, pw, ph, 14, 3, admin_a(admin_frame(), 0.9));
    /* Header: black band, white "ADMIN", white close orb. */
    roundrect(px, py, pw, hh, 14, admin_a(admin_header(), 0.98));
    rect(px, py + hh / 2, pw, hh / 2, admin_a(admin_header(), 0.98));
    line(px + 12, py + hh - 1, px + pw - 12, py + hh - 1, 2, admin_a(admin_frame(), 0.35));
    text_in_box("ADMIN", px + 16, py, pw * 0.5, hh, admin_a(admin_frame(), 1), hh / 61.0);
    double cxx = px + pw - hh * 0.55;
    double cr = hh * 0.3;
    circle(cxx, py + hh / 2, cr, admin_a(admin_body(), 1));
    ring(cxx, py + hh / 2, cr, clamp(cr * 0.14, 2, 3), admin_a(admin_frame(), 0.9));
    text_in_box("X", cxx - cr, py + hh / 2 - cr, cr * 2, cr * 2, admin_a(admin_frame(), 1), cr / 30.0);
    /* Tabs. */
    double tw = (pw - 2 * admin_pad() - admin_gap()) / 2;
    double t = 0;
    while (t < 2) {
        double tx = px + admin_pad() + t * (tw + admin_gap());
        const char *tl = admin_tab_label(t);
        double sc = fit_text_scale(tl, tw - 16, tabs_h / 57.0);
        if (admin_tab == t) {
            roundrect(tx, tabs_y, tw, tabs_h, 9, admin_a(admin_on(), 0.96));
            text_in_box(tl, tx, tabs_y, tw, tabs_h, 0x000000, sc);
        } else {
            roundrect(tx, tabs_y, tw, tabs_h, 9, admin_a(admin_off(), 0.95));
            roundrect_outline(tx, tabs_y, tw, tabs_h, 9, 2, admin_a(admin_frame(), 0.45));
            text_in_box(tl, tx, tabs_y, tw, tabs_h, admin_a(admin_dim_text(), 0.95), sc);
        }
        t = t + 1;
    }
    if (admin_tab == 1) {
        admin_draw_players();
        return;
    }
    admin_draw_cheats();
}

static void admin_draw(void) {
    if (is_admin() == 0) {
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
