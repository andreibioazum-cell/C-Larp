/* Event 4: the map is gradually infected, then everything explodes into a
 * black-and-white aftermath that heals back to normal after infect_gray_time. */

static void update_infect(void) {
    if (event_mode != 4) {
        if (infect_t >= 0) {
            infect_t = -1;
            ds_gfx_gray(0);
        }
        return;
    }
    if (infect_t < 0) {
        infect_t = 0;
    } else {
        infect_t = infect_t + dt;
    }
    if (infect_t >= infect_spread_time && infect_t - dt < infect_spread_time) {
        /* Boom: a heavy shake marks the moment the infection detonates. */
        ds_gfx_shake(12, 0.5);
    }
    if (infect_t >= infect_spread_time && infect_t < infect_spread_time + infect_gray_time) {
        ds_gfx_gray(1);
    } else {
        ds_gfx_gray(0);
    }
}

static void draw_infect(void) {
    if (infect_t < 0 || event_mode != 4) {
        return;
    }
    double k = clamp(infect_t / infect_spread_time, 0, 1);
    if (k < 1) {
        /* Green goo creeps out of fixed spots until it blankets the arena. */
        double i = 0;
        while (i < 8) {
            double px = screen_w * (0.5 + 0.42 * sin(i * 2.4 + 1.7));
            double py = screen_h * (0.5 + 0.42 * cos(i * 1.9 + 0.6));
            double r = (90 + 45 * sin(i * 3.1)) * (0.2 + 1.9 * k);
            circle(px, py, r, 0xB0123F16);
            circle(px * 0.6 + screen_w * 0.2, py * 0.6 + screen_h * 0.2, r * 0.55, 0x992E7D32);
            i = i + 1;
        }
        unsigned tint = (unsigned)(int)(46 * k);
        rect(0, 0, screen_w, screen_h, (tint << 24) | 0x1B5E20);
        return;
    }
    double ft = (infect_t - infect_spread_time) / 0.35;
    if (ft <= 1) {
        /* The detonation flash and a fast expanding shock ring. */
        unsigned flash = (unsigned)(int)(230 * (1 - ft));
        rect(0, 0, screen_w, screen_h, (flash << 24) | 0xFFFFFF);
        ring(screen_w / 2, screen_h / 2, 80 + screen_w * ft * 1.2, 12, ((unsigned)(int)(200 * (1 - ft)) << 24) | 0xFFFFFF);
    }
}
