static double fit_w(double w) { return clamp(w, 0, screen_w - 2 * screen_margin); }

static double center_x(double w) { return (screen_w - w) / 2; }

static double title_y(void) { return back_y + btn_h + title_gap; }

static double content_top(void) { return title_y() + list_top_gap; }

static double menu_x(void) { return center_x(btn_w); }

static double menu_stride(void) { return btn_h + btn_gap; }

static double menu_top(void) { return screen_h / 2 - (lobby_rows * btn_h + (lobby_rows - 1) * btn_gap) / 2; }

static double menu_row(double i) { return menu_top() + i * menu_stride(); }

static double hit_menu_row(double px, double py, double i) {
    return hit_rect_pad(px, py, menu_x(), menu_row(i), btn_w, btn_h);
}

static double settings_stride(void) {
    double s = menu_stride();
    double fit = (screen_h - 16) / SETTINGS_ROWS;
    if (fit < s) {
        s = fit;
    }
    if (s < 24) {
        s = 24;
    }
    return s;
}

static double settings_btn_h(void) {
    double h = settings_stride();
    if (h > btn_h) {
        h = btn_h;
    }
    return h;
}

static double settings_top(void) {
    double top = screen_h / 2 - (settings_btn_h() + (SETTINGS_ROWS - 1) * settings_stride()) / 2;
    if (top < 8) {
        top = 8;
    }
    return top;
}

static double settings_row_y(double i) { return settings_top() + i * settings_stride(); }

static double hit_settings_row(double px, double py, double i) {
    return hit_rect_pad(px, py, menu_x(), settings_row_y(i), btn_w, settings_btn_h());
}

static double lobby_title_y(void) { return menu_top() - 70; }

static double lobby_hello_y(void) { return 18; }

static double lobby_money_y(void) { return menu_row(lobby_rows - 1) + btn_h + 12; }

static double lobby_msg_y(void) { return lobby_money_y() + money_line_h + 6; }

static double lobby_quests_x(void) { return screen_w - btn_w - 20; }

static double lobby_quests_y(void) { return screen_h - btn_h - 20; }

static double hit_lobby_quests(double px, double py) {
    return hit_rect_pad(px, py, lobby_quests_x(), lobby_quests_y(), btn_w, btn_h);
}

static double classes_card_w(void) {
    double n = classes_visible_count();
    if (n < 1) {
        n = 1;
    }
    double total_gap = class_gap * (n - 1);
    double w = (screen_w - 2 * screen_margin - total_gap) / n;
    return clamp(w, class_card_min_w, class_card_max_w);
}

static double classes_total_w(void) {
    double n = classes_visible_count();
    if (n < 1) {
        n = 1;
    }
    return classes_card_w() * n + class_gap * (n - 1);
}

static double classes_top(void) { return back_y + btn_h + title_gap; }

static double classes_card_x(double vi) { return center_x(classes_total_w()) + vi * (classes_card_w() + class_gap); }

/* On-screen size (in px) of the shop card render. Bumped up from the old
 * tiny 50*1.55 = 77.5px pixel sprite so the higher-resolution character art
 * in the shop reads as a proper picture rather than a cube icon;
 * class_title_y/class_desc_y/class_level_y below are shifted down to match,
 * keeping the same clearance to the buy button as before.
 *
 * classes_card_w() shrinks toward class_card_min_w on narrow screens (small
 * phones with a low-resolution screen_w), while the render kept a fixed
 * 108px box - on those phones it filled most of the card's width and almost
 * reached the name/description text below it. Scale the render down together
 * with the card width so small screens keep a comfortable gap above the
 * text, while wide/high-res screens still get the full-size 108px render. */
static double class_render_px(void) {
    double w = classes_card_w();
    double t = (w - class_card_min_w) / (class_card_max_w - class_card_min_w);
    t = clamp(t, 0, 1);
    return 84 + t * (108 - 84);
}

static double class_sprite_y(void) { return classes_top() + 4; }

static double class_title_scale(void) { return 0.85; }

static double class_title_y(void) { return classes_top() + 105; }

static double class_desc_y(void) { return classes_top() + 137; }

static double class_level_y(void) { return classes_top() + 159; }

static double class_pick_inset(void) { return 24; }

static double class_pick_w(void) { return classes_card_w() - 2 * class_pick_inset(); }

static double class_pick_y(void) { return classes_top() + class_card_h - class_pick_h - class_pick_pad; }

static double hit_class_card(double px, double py, double vi) {
    return hit_rect(px, py, classes_card_x(vi), classes_top(), classes_card_w(), class_card_h);
}

static double class_actions_y(void) { return classes_top() + class_card_h + class_gap; }

static double class_action_w(void) {
    double half = (screen_w - 2 * screen_margin - class_action_gap) / 2;
    return clamp(half, 0, btn_w);
}

static double levels_btn_x(void) {
    if (class_has_skins(player_class) == 0) {
        return screen_w / 2 - class_action_w() / 2;
    }
    return screen_w / 2 - class_action_gap / 2 - class_action_w();
}

static double skins_btn_x(void) { return screen_w / 2 + class_action_gap / 2; }

static double class_has_skins(double cls) {
    if (cls == CLASS_AZUM || cls == CLASS_EBUC) {
        return 1;
    }
    return 0;
}

static double skins_btn_visible(void) { return class_has_skins(player_class); }

static double hit_levels_btn(double px, double py) {
    return hit_rect_pad(px, py, levels_btn_x(), class_actions_y(), class_action_w(), btn_h);
}

static double hit_skins_btn(double px, double py) {
    if (skins_btn_visible() == 0) {
        return 0;
    }
    return hit_rect_pad(px, py, skins_btn_x(), class_actions_y(), class_action_w(), btn_h);
}

static double class_msg_y(void) { return class_actions_y() + btn_h + 14; }

static double stat_bar_w(void) { return fit_w(520); }

static double stat_bar_h(void) { return 26; }

static double stat_hp_y(void) { return screen_h - 110; }

static double stat_power_y(void) { return screen_h - 46; }

static double stat_motto_y(void) { return stat_hp_y() - 72; }

static double levels_w(void) { return fit_w(list_w); }

static double levels_x(void) { return center_x(levels_w()); }

static double levels_top(void) { return content_top() + 30; }

static double levels_row_y(double n) { return levels_top() + (n - 1) * (level_row_h + level_row_gap); }

static double levels_msg_y(void) { return levels_row_y(max_level) + level_row_h + 12; }

static double hit_level_row(double px, double py, double n) {
    return hit_rect(px, py, levels_x(), levels_row_y(n), levels_w(), level_row_h);
}

static double skins_total_w(void) { return skin_card_w * SKIN_COUNT + skin_card_gap * (SKIN_COUNT - 1); }

static double skins_card_x(double i) { return center_x(skins_total_w()) + i * (skin_card_w + skin_card_gap); }

static double skins_top(void) { return content_top() + 30; }

static double skin_preview_scale(void) { return 1.8; }

static double skin_preview_y(void) { return skins_top() + 16; }

static double skin_name_y(void) { return skins_top() + 118; }

static double skin_status_y(void) { return skins_top() + 146; }

static double skins_msg_y(void) { return skins_top() + skin_card_h + 16; }

static double hit_skin_card(double px, double py, double i) {
    return hit_rect(px, py, skins_card_x(i), skins_top(), skin_card_w, skin_card_h);
}

static double ach_w(void) { return fit_w(760); }

static double ach_x(void) { return center_x(ach_w()); }

static double ach_top(void) { return content_top(); }

static double ach_card_y(double i) { return ach_top() + i * (ach_card_h + ach_card_gap); }

static double lb_w(void) { return fit_w(lb_w_max); }

static double lb_x(void) { return center_x(lb_w()); }

static double lb_top(void) { return content_top(); }

static double lb_row_y(double i) { return lb_top() + i * (lb_row_h + lb_row_gap); }

static double login_x(void) { return center_x(login_w) + login_ox; }

static double login_nick_y(void) { return screen_h / 2 - 100 + login_oy; }

static double login_pass_y(void) { return login_nick_y() + login_fh + login_field_gap; }

static double login_btn_y(void) { return login_pass_y() + login_fh + login_btn_gap; }

static double login_btn_x(void) { return center_x(login_btn_w); }

static double login_msg_y(void) { return login_btn_y() + btn_h + 16; }

static double hit_login_field(double px, double py, double field_y) {
    return hit_rect(px, py, login_x(), field_y, login_w, login_fh);
}

static double hit_login_btn(double px, double py) {
    return hit_rect_pad(px, py, login_btn_x(), login_btn_y(), login_btn_w, btn_h);
}

static double modes_top(void) { return screen_h / 2 - (modes_rows * btn_h + (modes_rows - 1) * btn_gap) / 2; }

static double modes_row(double i) { return modes_top() + i * menu_stride(); }

static double hit_modes_row(double px, double py, double i) {
    return hit_rect_pad(px, py, menu_x(), modes_row(i), btn_w, btn_h);
}

static double quest_w(void) { return fit_w(760); }

static double quest_x(void) { return center_x(quest_w()); }

static double quest_top(void) { return content_top(); }

static double quest_row_y(double i) { return quest_top() + i * (ach_card_h + ach_card_gap); }

static GAME_MAYBE_UNUSED double hit_quest_row(double px, double py, double i) {
    return hit_rect(px, py, quest_x(), quest_row_y(i), quest_w(), ach_card_h);
}

static double warn_btn_w(void) { return fit_w(560); }

static double warn_btn_y(void) { return screen_h / 2 + 160; }

static double hit_warn_btn(double px, double py) {
    return hit_rect_pad(px, py, center_x(warn_btn_w()), warn_btn_y(), warn_btn_w(), btn_h);
}
