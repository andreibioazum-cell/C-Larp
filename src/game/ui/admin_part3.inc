/* Players tab of the admin panel: a nick field with its own ban / unban
 * buttons, then everyone in the online room, then the nicks that only exist in
 * the ban list. One tap on a row toggles that player. The panel replaces the
 * old Fight tab entirely - no instant win, no teleport. */

/* Do not offer actions for the two admin accounts; the engine refuses them
 * anyway, this just keeps the list honest. */
static double admin_players_locked(const char *nick) {
    return admin_slot(nick) != 0 ? 1 : 0;
}

static double admin_players_room_count(void) {
    double n = 0;
    double s = 0;
    while (s < max_players) {
        if (s != net_slot() && net_player_online(s) == 1 && str_len(net_player_nick(s)) > 0) {
            n = n + 1;
        }
        s = s + 1;
    }
    return n;
}

static const char *admin_players_room_nick(double row) {
    double n = 0;
    double s = 0;
    while (s < max_players) {
        if (s != net_slot() && net_player_online(s) == 1) {
            const char *nick = net_player_nick(s);
            if (str_len(nick) > 0) {
                if (n == row) {
                    return nick;
                }
                n = n + 1;
            }
        }
        s = s + 1;
    }
    return "";
}

static double admin_players_in_room(const char *nick) {
    double s = 0;
    if (str_len(nick) == 0) {
        return 0;
    }
    while (s < max_players) {
        if (s != net_slot() && net_player_online(s) == 1) {
            const char *rn = net_player_nick(s);
            if (str_len(rn) > 0 && admin_nick_equal(rn, nick) == 1) {
                return 1;
            }
        }
        s = s + 1;
    }
    return 0;
}

static double admin_players_ban_only_count(void) {
    double n = 0;
    double total = net_ban_list_count();
    double i = 0;
    while (i < total) {
        const char *nick = net_ban_list_nick(i);
        if (str_len(nick) > 0 && admin_players_in_room(nick) == 0) {
            n = n + 1;
        }
        i = i + 1;
    }
    return n;
}

static const char *admin_players_ban_only_nick(double row) {
    double n = 0;
    double total = net_ban_list_count();
    double i = 0;
    while (i < total) {
        const char *nick = net_ban_list_nick(i);
        if (str_len(nick) > 0 && admin_players_in_room(nick) == 0) {
            if (n == row) {
                return nick;
            }
            n = n + 1;
        }
        i = i + 1;
    }
    return "";
}

static double admin_players_total(void) {
    return admin_players_room_count() + admin_players_ban_only_count();
}

static double admin_players_here(double row) { return row < admin_players_room_count() ? 1 : 0; }

static const char *admin_players_nick(double row) {
    double room = admin_players_room_count();
    if (row < room) {
        return admin_players_room_nick(row);
    }
    return admin_players_ban_only_nick(row - room);
}

static double admin_players_pages(void) {
    double per = admin_players_per_page();
    double n = 1;
    if (per > 0) {
        n = ceil(admin_players_total() / per);
    }
    if (n < 1) {
        n = 1;
    }
    return n;
}

static void admin_players_clamp_page(void) {
    double last = admin_players_pages() - 1;
    if (admin_page < 0) {
        admin_page = 0;
    } else if (admin_page > last) {
        admin_page = last;
    }
}

/* Nick field: whatever the engine keyboard has typed. */
static void admin_draw_players_field(void) {
    double px = admin_panel_x() + admin_pad();
    double pw = admin_panel_w() - 2 * admin_pad();
    double y = admin_players_field_y();
    double h = admin_players_field_h();
    const char *text = str_len(admin_input_nick) > 0 ? admin_input_nick : tr_admin_nick_hint();
    double sc = fit_text_scale(text, pw - 24, h / 58.0);
    roundrect(px, y, pw, h, admin_btn_r(), admin_off());
    roundrect_outline(px, y, pw, h, admin_btn_r(), 2, admin_frame());
    text_in_box(text, px + 12, y, pw - 24, h, str_len(admin_input_nick) > 0 ? admin_frame() : admin_dim_text(), sc);
}

/* The two action buttons work on the typed nick, so a player can be banned or
 * freed without being in the room or on the visible page. */
static void admin_draw_players_actions(void) {
    double px = admin_panel_x() + admin_pad();
    double pw = admin_panel_w() - 2 * admin_pad();
    double y = admin_players_act_y();
    double h = admin_players_field_h();
    double bw = (pw - admin_gap()) / 2;
    const char *unban = tr_admin_do_unban();
    const char *ban = tr_admin_do_ban();
    admin_btn(unban, px, y, bw, h, 0, fit_text_scale(unban, bw - 12, h / 62.0));
    admin_btn(ban, px + bw + admin_gap(), y, bw, h, 0, fit_text_scale(ban, bw - 12, h / 62.0));
}

static void admin_draw_player_row(double y, const char *nick, double here, double h) {
    double px = admin_panel_x() + admin_pad();
    double pw = admin_panel_w() - 2 * admin_pad();
    double chip = clamp(pw * 0.26, 66, 130);
    double banned = net_ban_list_have(nick);
    double cy = y + h / 2;
    const char *act = banned == 1 ? tr_admin_unban() : tr_admin_ban();
    double ax = px + pw - chip - 6;
    double as = fit_text_scale(act, chip - 14, h / 66.0);
    double lw = pw - chip - 44;
    double sc = fit_text_scale(nick, lw, h / 58.0);
    roundrect(px, y, pw, h, admin_btn_r(), admin_off());
    roundrect_outline(px, y, pw, h, admin_btn_r(), 2, admin_frame());
    /* Filled dot = in the room right now, hollow one = ban list only. */
    if (here == 1) {
        circle(px + 14, cy, clamp(h * 0.11, 3, 5), admin_frame());
    } else {
        ring(px + 14, cy, clamp(h * 0.11, 3, 5), 2, admin_frame());
    }
    /* Plain white on purpose: the panel stays black and white even for the
     * accounts that get a coloured nick everywhere else. */
    text_in_box(nick, px + 24, y, lw, h, admin_frame(), sc);
    if (admin_players_locked(nick) == 1) {
        return;
    }
    admin_btn(act, ax, y + 4, chip, h - 8, banned == 1, as);
}

static void admin_draw_players_footer(void) {
    double px = admin_panel_x() + admin_pad();
    double pw = admin_panel_w() - 2 * admin_pad();
    double h = admin_footer_h();
    double y = admin_content_bottom() - h;
    double bw = (pw - 2 * admin_gap()) / 3;
    const char *fresh = ds_concat(ds_concat(tr_admin_bans(), " "), ds_num_to_string(net_ban_list_count()));
    admin_draw_small_btn("<", px, y, bw, h, admin_page > 0);
    admin_draw_small_btn(">", px + 2 * (bw + admin_gap()), y, bw, h, admin_page + 1 < admin_players_pages());
    admin_draw_small_btn(fresh, px + bw + admin_gap(), y, bw, h, net_ban_list_busy() == 0);
}

static void admin_draw_players(void) {
    double rh = admin_player_row_h();
    double per = admin_players_per_page();
    double total = admin_players_total();
    double top = admin_list_top();
    double start = 0;
    admin_players_clamp_page();
    start = admin_page * per;
    admin_draw_players_field();
    admin_draw_players_actions();
    if (total == 0) {
        text_in_box(tr_admin_no_players(), admin_panel_x() + admin_pad(), top, admin_panel_w() - 2 * admin_pad(),
                    admin_list_bottom() - top, admin_dim_text(), 0.5);
    }
    double i = 0;
    while (i < per && start + i < total) {
        double row = start + i;
        double y = top + i * (rh + admin_gap());
        admin_draw_player_row(y, admin_players_nick(row), admin_players_here(row), rh);
        i = i + 1;
    }
    admin_draw_players_footer();
}

/* Ban or free a nick that was typed into the field by hand. */
static void admin_player_act_nick(const char *nick, double ban) {
    if (str_len(nick) == 0) {
        return;
    }
    play_button_click();
    if (admin_players_locked(nick) == 1) {
        return;
    }
    net_ban_set(nick, ban);
    if (game_state == ST_ONLINE && net_login_status() == 2) {
        net_chat_send(ds_concat(ds_concat(nick, ban == 1 ? " BANNED by " : " UNBANNED by "), admin_actor_nick()));
    }
    ds_log("admin panel: %s %s", ban == 1 ? "ban" : "unban", nick);
}

/* A tap on a listed row toggles that player. */
static void admin_player_act(double row) {
    const char *nick = admin_players_nick(row);
    double banned = net_ban_list_have(nick);
    admin_player_act_nick(nick, banned == 1 ? 0 : 1);
}

static void admin_players_apply(double ban) {
    admin_input_sync();
    admin_player_act_nick(admin_input_nick, ban);
}

static void admin_players_press(double x, double y) {
    double px = admin_panel_x() + admin_pad();
    double pw = admin_panel_w() - 2 * admin_pad();
    double fh = admin_footer_h();
    double fy = admin_content_bottom() - fh;
    double bw = (pw - 2 * admin_gap()) / 3;
    double field_h = admin_players_field_h();
    double abw = (pw - admin_gap()) / 2;
    double rh = admin_player_row_h();
    double top = admin_list_top();
    double rel = floor((y - top) / (rh + admin_gap()));
    double per = admin_players_per_page();
    double row = admin_page * per + rel;
    if (hit_rect(x, y, px, admin_players_field_y(), pw, field_h) == 1) {
        admin_input_focus();
        play_button_click();
        return;
    }
    if (hit_rect(x, y, px, admin_players_act_y(), abw, field_h) == 1) {
        admin_players_apply(0);
        return;
    }
    if (hit_rect(x, y, px + abw + admin_gap(), admin_players_act_y(), abw, field_h) == 1) {
        admin_players_apply(1);
        return;
    }
    if (hit_rect(x, y, px, fy, bw, fh) == 1) {
        if (admin_page > 0) {
            admin_page = admin_page - 1;
            play_button_click();
        }
        return;
    }
    if (hit_rect(x, y, px + 2 * (bw + admin_gap()), fy, bw, fh) == 1) {
        if (admin_page + 1 < admin_players_pages()) {
            admin_page = admin_page + 1;
            play_button_click();
        }
        return;
    }
    if (hit_rect(x, y, px + bw + admin_gap(), fy, bw, fh) == 1) {
        net_ban_list_refresh();
        play_button_click();
        return;
    }
    if (y < top || y > admin_list_bottom() || rel < 0 || x < px || x > px + pw) {
        return;
    }
    if (row < admin_players_total()) {
        admin_player_act(row);
    }
}
