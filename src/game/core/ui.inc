static double shade(double rgb, double k) {
    double a = floor(rgb / 16777216);
    double body = rgb - a * 16777216;
    double r = floor(body / 65536);
    double g = floor(body / 256) - r * 256;
    double b = body - floor(body / 256) * 256;
    return a * 16777216 + floor(r * k) * 65536 + floor(g * k) * 256 + floor(b * k);
}

static double locked_bg(void) {
    return shade(ui_purple, locked_shade);
}

static double locked_text(void) {
    return locked_text_alpha * 16777216 + text_color;
}

static double fade_color(double rgb, double t, double fade) {
    double a = clamp(t / fade, 0, 1);
    return (1 + floor(a * 254)) * 0x1000000 + rgb;
}

static double hit_rect(double px, double py, double x, double y, double w, double h) {
    if (px >= x && px <= x + w && py >= y && py <= y + h) {
        return 1;
    }
    return 0;
}

static double hit_rect_pad(double px, double py, double x, double y, double w, double h) {
    return hit_rect(px, py, x - touch_pad_x, y - touch_pad_y, w + 2 * touch_pad_x, h + 2 * touch_pad_y);
}

static void ctext(const char *s, double y, uint32_t c) {
    text(s, (screen_w - text_ink_width(s)) / 2, y, c);
}

static void ctext_scaled(const char *s, double y, uint32_t c, double scale) {
    text_scaled(s, (screen_w - text_ink_width(s) * scale) / 2, y, c, scale);
}

static void ctext_fade(const char *s, double y, uint32_t rgb, double t, double fade) {
    ctext(s, y, fade_color(rgb, t, fade));
}

static void text_in_box(const char *label, double x, double y, double w, double h, uint32_t c, double scale) {
    double tw = text_ink_width(label) * scale;
    double th = text_ink_height(label) * scale;
    double tt = text_ink_top(label) * scale;
    text_scaled(label, x + (w - tw) / 2, y + (h - th) / 2 - tt + 1.5, c, scale);
}

static void text_in_field(const char *label, double x, double y, double h, uint32_t c, double scale) {
    double th = text_ink_height(label) * scale;
    double tt = text_ink_top(label) * scale;
    text_scaled(label, x, y + (h - th) / 2 - tt + 1.5, c, scale);
}

static double fit_text_scale(const char *s, double w, double max_scale) {
    double tw = text_ink_width(s);
    if (tw <= 0 || w <= 0) {
        return max_scale;
    }
    double sc = w / tw;
    if (sc > max_scale) {
        sc = max_scale;
    }
    return sc;
}

static void centered_text(const char *label, double x, double w, double y, uint32_t c, double scale) {
    double tw = text_ink_width(label) * scale;
    text_scaled(label, x + (w - tw) / 2, y, c, scale);
}

static void money_pair(const char *left, const char *right, double y, double scale, double gap) {
    double lw = text_ink_width(left) * scale;
    double rw = text_ink_width(right) * scale;
    double x = (screen_w - lw - rw - gap) / 2;
    text_scaled(left, x, y, text_color, scale);
    text_scaled(right, x + lw + gap, y, text_color, scale);
}

static void draw_nick_msg(const char *nick, const char *rest, double x, double y, uint32_t base, double scale) {
    double nw = text_ink_width(nick) * scale;
    text_scaled(nick, x, y, admin_color_of(nick, base), scale);
    text_scaled(rest, x + nw, y, base, scale);
}

static void button(const char *label, double x, double y, double w, double h) {
    roundrect(x, y, w, h, btn_radius, ui_purple);
    text_in_box(label, x, y, w, h, text_color, 1);
}

static void button_scaled(const char *label, double x, double y, double w, double h, double scale) {
    roundrect(x, y, w, h, btn_radius, ui_purple);
    text_in_box(label, x, y, w, h, text_color, scale);
}

static void roundrect_outline(double x, double y, double w, double h, double rad, double t, uint32_t c) {
    double o = t / 2;
    double rr = rad + o;
    line(x + rad, y - o, x + w - rad, y - o, t, c);
    line(x + rad, y + h + o, x + w - rad, y + h + o, t, c);
    line(x - o, y + rad, x - o, y + h - rad, t, c);
    line(x + w + o, y + rad, x + w + o, y + h - rad, t, c);
    outline_arc(x + rad, y + rad, rr, t, c, -1, -1);
    outline_arc(x + w - rad, y + rad, rr, t, c, 1, -1);
    outline_arc(x + w - rad, y + h - rad, rr, t, c, 1, 1);
    outline_arc(x + rad, y + h - rad, rr, t, c, -1, 1);
}

static void outline_arc(double cx, double cy, double rr, double t, uint32_t c, double mx, double my) {
    line(cx + mx * rr, cy, cx + mx * 0.9239 * rr, cy + my * 0.3827 * rr, t, c);
    line(cx + mx * 0.9239 * rr, cy + my * 0.3827 * rr, cx + mx * 0.7071 * rr, cy + my * 0.7071 * rr, t, c);
    line(cx + mx * 0.7071 * rr, cy + my * 0.7071 * rr, cx + mx * 0.3827 * rr, cy + my * 0.9239 * rr, t, c);
    line(cx + mx * 0.3827 * rr, cy + my * 0.9239 * rr, cx, cy + my * rr, t, c);
}

static void draw_back(void) {
    button(tr_back(), (screen_w - btn_w) / 2, back_y, btn_w, btn_h);
}

static double back_hit(double x, double y) {
    return hit_rect_pad(x, y, (screen_w - btn_w) / 2, back_y, btn_w, btn_h);
}

static void screen_fill(void) {
    rect(0, 0, screen_w, screen_h, bg_screen);
}

static void card_state(double x, double y, double w, double h, double selected, double locked) {
    if (selected == 1) {
        roundrect(x - card_ring_pad, y - card_ring_pad, w + 2 * card_ring_pad, h + 2 * card_ring_pad, btn_radius + 4,
                  card_ring);
    }
    if (locked == 1) {
        roundrect(x, y, w, h, btn_radius, shade(card_bg, locked_shade));
        return;
    }
    roundrect(x, y, w, h, btn_radius, card_bg);
}

static void card(double x, double y, double w, double h, double selected) {
    card_state(x, y, w, h, selected, 0);
}

static void panel(double x, double y, double w, double h, double locked) {
    if (locked == 1) {
        roundrect(x, y, w, h, btn_radius, locked_bg());
        return;
    }
    roundrect(x, y, w, h, btn_radius, ui_purple);
}

static double panel_text(double locked) {
    if (locked == 1) {
        return locked_text();
    }
    return text_color;
}

static void hud_bar(double bx, double by, double w, double h, double cur, double max, uint32_t c, double from_right) {
    roundrect(bx, by, w, h, h / 2, 0xDD171923);
    double shown = clamp(cur, 0, max);
    double pad = 4;
    if (h <= 12) {
        pad = 1;
    }
    double fh = h - 2 * pad;
    if (fh < 1) {
        fh = 1;
    }
    if (max <= 0) {
        return;
    }
    double fill_w = (w - 2 * pad) * shown / max;
    if (fill_w <= 0) {
        return;
    }
    if (from_right == 1) {
        roundrect(bx + w - pad - fill_w, by + pad, fill_w, fh, fh / 2, c);
    } else {
        roundrect(bx + pad, by + pad, fill_w, fh, fh / 2, c);
    }
}

static double tick_down(double t) {
    if (t <= 0) {
        return 0;
    }
    if (t - dt <= 0) {
        return 0;
    }
    return t - dt;
}

static void draw_msg(const char *label, double y, double t) {
    if (t > 0) {
        ctext_fade(label, y, text_color, t, msg_fade);
    }
}
