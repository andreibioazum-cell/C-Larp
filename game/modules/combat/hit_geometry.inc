static double fighter_half(double scale) {
    return body_px_half * cube_scale * scale;
}

static double player_half(void) {
    return fighter_half(player_scale);
}

static double enemy_half(void) {
    return fighter_half(enemy_scale);
}

static double turret_half(void) {
    return turret_body_px_half * station_scale;
}

static double sat_axis_ok(double nx, double ny, double tx, double ty, double aux, double auy, double ae0, double ae1,
                          double bux, double buy, double be0, double be1) {
    double ra = ae0 * fabs(aux * nx + auy * ny) + ae1 * fabs(auy * nx - aux * ny);
    double rb = be0 * fabs(bux * nx + buy * ny) + be1 * fabs(buy * nx - bux * ny);
    if (fabs(tx * nx + ty * ny) > ra + rb) {
        return 0;
    }
    return 1;
}

static double rects_overlap(double ax, double ay, double aux, double auy, double ae0, double ae1, double bx, double by,
                            double bux, double buy, double be0, double be1) {
    double tx = bx - ax;
    double ty = by - ay;
    if (sat_axis_ok(aux, auy, tx, ty, aux, auy, ae0, ae1, bux, buy, be0, be1) == 0) {
        return 0;
    }
    if (sat_axis_ok(0 - auy, aux, tx, ty, aux, auy, ae0, ae1, bux, buy, be0, be1) == 0) {
        return 0;
    }
    if (sat_axis_ok(bux, buy, tx, ty, aux, auy, ae0, ae1, bux, buy, be0, be1) == 0) {
        return 0;
    }
    if (sat_axis_ok(0 - buy, bux, tx, ty, aux, auy, ae0, ae1, bux, buy, be0, be1) == 0) {
        return 0;
    }
    return 1;
}

static double strip_hits_box(double sx, double sy, double dx, double dy, double off0, double off1, double half_w,
                             double bx, double by, double bang, double bh) {
    double len = off1 - off0;
    if (len <= 0 || half_w <= 0 || bh <= 0) {
        return 0;
    }
    double mid = (off0 + off1) / 2;
    return rects_overlap(sx + dx * mid, sy + dy * mid, dx, dy, len / 2, half_w, bx, by, cos(bang), sin(bang), bh, bh);
}

static double circle_hits_box(double cx, double cy, double r, double bx, double by, double bang, double bh) {
    if (r <= 0 || bh <= 0) {
        return 0;
    }
    double ux = cos(bang);
    double uy = sin(bang);
    double rx = cx - bx;
    double ry = cy - by;
    double lu = clamp(rx * ux + ry * uy, 0 - bh, bh);
    double lv = clamp(ry * ux - rx * uy, 0 - bh, bh);
    double px = bx + ux * lu - uy * lv;
    double py = by + uy * lu + ux * lv;
    double ex = cx - px;
    double ey = cy - py;
    if (ex * ex + ey * ey <= r * r) {
        return 1;
    }
    return 0;
}

static double strip_hits_player(double sx, double sy, double dx, double dy, double off0, double off1, double half_w) {
    return strip_hits_box(sx, sy, dx, dy, off0, off1, half_w, player->x, player->y, player->angle, player_half());
}

static double strip_hits_enemy(double sx, double sy, double dx, double dy, double off0, double off1, double half_w) {
    return strip_hits_box(sx, sy, dx, dy, off0, off1, half_w, enemy->x, enemy->y, enemy->angle, enemy_half());
}

static double strip_hits_turret(double sx, double sy, double dx, double dy, double off0, double off1, double half_w,
                                double tx, double ty) {
    return strip_hits_box(sx, sy, dx, dy, off0, off1, half_w, tx, ty, 0, turret_half());
}

static double circle_hits_player(double cx, double cy, double r) {
    return circle_hits_box(cx, cy, r, player->x, player->y, player->angle, player_half());
}

static double circle_hits_enemy(double cx, double cy, double r) {
    return circle_hits_box(cx, cy, r, enemy->x, enemy->y, enemy->angle, enemy_half());
}

static double circle_hits_turret(double cx, double cy, double r, double tx, double ty) {
    return circle_hits_box(cx, cy, r, tx, ty, 0, turret_half());
}

static double punch_hits_player(double px, double py, double pdx, double pdy, double reach, double width) {
    return strip_hits_player(px, py, pdx, pdy, 0, reach, width / 2);
}

static double punch_hits_enemy(double px, double py, double pdx, double pdy, double reach, double width) {
    return strip_hits_enemy(px, py, pdx, pdy, 0, reach, width / 2);
}

static double punch_hits_turret(double px, double py, double pdx, double pdy, double reach, double width, double tx,
                                double ty) {
    return strip_hits_turret(px, py, pdx, pdy, 0, reach, width / 2, tx, ty);
}

static double along(double px, double py, double pdx, double pdy, double tx, double ty) {
    return (tx - px) * pdx + (ty - py) * pdy;
}
