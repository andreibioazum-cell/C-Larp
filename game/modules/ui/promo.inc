static void card_roll(void) {
    if (rand() % 100 >= card_chance) {
        return;
    }
    card_spawn();
}

static void card_spawn(void) {
    double m = candy_margin;
    double k = 0;
    double cx = m + rand() % (int)(screen_w - 2 * m);
    double cy = m + rand() % (int)(screen_h - 2 * m);
    while (k < 24) {
        if (dist(cx, cy, player->x, player->y) < 140 || dist(cx, cy, enemy->x, enemy->y) < 140) {
            cx = m + rand() % (int)(screen_w - 2 * m);
            cy = m + rand() % (int)(screen_h - 2 * m);
            k = k + 1;
        } else {
            k = 24;
        }
    }
    card_x = cx;
    card_y = cy;
    card_t = 0;
    card_active = 1;
    ds_log("card dropped at %g %g", cx, cy);
}

static void card_collect(void) {
    if (finished != 0) {
        return;
    }
    card_active = 0;
    card_open = 1;
    card_open_t = 0;
    card_code = net_promo_new_code();
    ds_log("card collected");
}

static void card_close(void) {
    card_open = 0;
    card_open_t = 0;
}

static void card_update(void) {
    card_t = card_t + dt;
    if (card_open == 1) {
        card_open_t = card_open_t + dt;
        return;
    }
    if (finished != 0) {
        return;
    }
    if (card_active == 1) {
        if (dist(card_x, card_y, player->x, player->y) < player->size * 0.65 + 26) {
            card_collect();
        }
    }
}

static void draw_card_item(void) {
    if (card_active == 0) {
        return;
    }
    double bob = 4 * sin(card_t * 3);
    double x = card_x - 22;
    double y = card_y - 30 + bob;
    double a = 110 + 95 * sin(card_t * 5);
    double aa = floor(clamp(a, 0, 255));
    ring(card_x, card_y + bob, 44, 3, (((unsigned)(int)aa) << 24) | 0xFFB388FF);
    roundrect(x - 3, y - 3, 50, 66, 10, 0xFFB388FF);
    roundrect(x, y, 44, 60, 8, 0xFFF7F3FF);
    text_in_box("?", x, y + 8, 44, 44, 0xFF5F10A0, 1.1);
}

static void draw_card_open(void) {
    if (card_open == 0) {
        return;
    }
    rect(0, 0, screen_w, screen_h, 0xAA000000);
    double w = 430;
    double h = 380;
    double x = center_x(w);
    double y = screen_h / 2 - h / 2;
    roundrect(x - 8, y - 8, w + 16, h + 16, 36, card_ring);
    roundrect(x, y, w, h, 30, card_bg);
    ctext_scaled(tr_card_title(), y + 36, 0xFFB388FF, 1.05);
    ctext_scaled(tr_card_sub(), y + 88, text_color, 0.55);
    double sc = 0.85;
    if (text_ink_width(card_code) * sc > w - 56) {
        sc = (w - 56) / text_ink_width(card_code);
    }
    ctext_scaled(card_code, y + 150, 0xFF4FC3F7, sc);
    if (net_promo_used() == 1) {
        ctext_scaled(tr_card_used1(), y + 216, text_color, 0.5);
        ctext_scaled(tr_card_used2(), y + 240, text_color, 0.5);
    } else {
        ctext_scaled(tr_card_hint1(), y + 216, text_color, 0.5);
        ctext_scaled(tr_card_hint2(), y + 240, text_color, 0.5);
    }
    ctext_scaled(tr_tap_to_return(), y + h - 42, text_color, 0.55);
}

static double promo_field_y(void) {
    return content_top() + 158;
}

static double promo_btn_y(void) {
    return promo_field_y() + 56 + 20;
}

static double promo_msg_y(void) {
    return promo_btn_y() + btn_h + 16;
}

static void promo_set_msg(const char *text, double kind) {
    promo_msg_text = text;
    promo_msg_kind = kind;
    promo_msg_t = 4.0;
}

static void promo_leave(void) {
    promo_focus = 0;
    keyboard_hide();
}

static double promo_letter_ok(const char *ch) {
    return str_index_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ", ch) >= 0;
}

static double promo_digit_ok(const char *ch) {
    return str_index_of("0123456789", ch) >= 0;
}

static double promo_code_valid(const char *s) {
    if (str_len(s) != 4) {
        return 0;
    }
    double letters = 0;
    double digits = 0;
    double i = 0;
    while (i < 4) {
        const char *ch = str_sub(s, i, 1);
        if (promo_letter_ok(ch) == 1) {
            letters = letters + 1;
        } else if (promo_digit_ok(ch) == 1) {
            digits = digits + 1;
        } else {
            return 0;
        }
        i = i + 1;
    }
    if (letters == 3 && digits == 1) {
        return 1;
    }
    return 0;
}

static void promo_submit(void) {
    if (promo_focus == 1) {
        promo_input = keyboard_get_text();
    }
    const char *code = str_upper(str_trim(promo_input));
    if (str_len(code) == 0) {
        promo_set_msg(tr_promo_need_code(), 0);
        return;
    }
    if (net_login_status() != 2) {
        promo_set_msg(tr_promo_need_login(), 0);
        promo_leave();
        return;
    }
    if (promo_code_valid(code) == 0) {
        promo_set_msg(tr_promo_bad(), 0);
        return;
    }
    if (net_promo_used() == 1) {
        promo_set_msg(tr_promo_used_msg(), 0);
        return;
    }
    if (net_promo_check(code) == 0) {
        promo_set_msg(tr_promo_bad(), 0);
        return;
    }
    cups = cups + promo_reward_cups;
    candies = candies + promo_reward_candies;
    net_promo_mark_used();
    save_progress();
    ds_log("promo redeemed: +%g cups +%g candies", promo_reward_cups, promo_reward_candies);
    promo_leave();
    promo_set_msg(tr_promo_success(), 1);
}

static void update_promo(void) {
    if (promo_focus == 0) {
        return;
    }
    if (keyboard_visible() == 1) {
        promo_input = keyboard_get_text();
    }
    if (keyboard_enter_pressed() == 1) {
        promo_submit();
    }
}

static void draw_promo(void) {
    screen_fill();
    draw_back();
    ctext(tr_promo_title(), title_y(), text_color);
    double w = fit_w(560);
    double x = center_x(w);
    double top = content_top() + 8;
    panel(x, top, w, 76, 0);
    if (net_promo_used() == 1) {
        centered_text(tr_promo_used_msg(), x, w, top + 32, 0xFFC9B6FF, 0.55);
    } else {
        centered_text(tr_promo_need_card(), x, w, top + 32, text_color, 0.55);
    }
    ctext_scaled(tr_promo_how1(), top + 88, text_color, 0.48);
    ctext_scaled(tr_promo_how2(), top + 114, text_color, 0.48);
    double fy = promo_field_y();
    roundrect(x, fy, w, 56, field_radius, card_bg);
    double caret = 0;
    if (promo_focus == 1 && keyboard_visible() == 1) {
        caret = 1;
    }
    const char *shown = promo_input;
    if (caret == 1) {
        shown = ds_concat(shown, "|");
    }
    if (str_len(shown) == 0) {
        shown = tr_promo_enter_ph();
    }
    text_in_field(shown, x + field_text_pad, fy, 56, text_color, 0.6);
    button(tr_promo_submit(), x, promo_btn_y(), w, btn_h);
    promo_msg_t = tick_down(promo_msg_t);
    draw_msg(promo_msg_text, promo_msg_y(), promo_msg_t);
}

static void touch_promo(double x, double y, double action) {
    if (action != 0) {
        return;
    }
    if (back_hit(x, y) == 1) {
        promo_leave();
        start_transition(ST_MODES);
        return;
    }
    double w = fit_w(560);
    double fx = center_x(w);
    if (hit_rect(x, y, fx, promo_field_y(), w, 56) == 1) {
        promo_focus = 1;
        keyboard_show();
        return;
    }
    if (hit_rect_pad(x, y, fx, promo_btn_y(), w, btn_h) == 1) {
        promo_submit();
        return;
    }
    promo_leave();
}
