/* Button slide-in / slide-out animation (new theme only).
 *
 * Flow:
 *   1. User taps a button → exit animation: buttons whoosh out to the right,
 *      travelling far enough to clear the screen edge completely
 *   2. Black fade covers the screen while buttons are sliding out
 *   3. Screen swap happens
 *   4. Fade-out: enter animation: buttons fly IN from far off-screen right
 *      with a small settle overshoot
 *
 * The travel distance is derived from the live screen size, so on entry the
 * buttons start past the right edge — never half-visible on screen.
 *
 * Old theme (new_theme == 0): no animation at all. */

static void btn_anim_begin_exit(void) {
    if (new_theme == 0) return;
    btn_anim_t = 0;
    btn_anim_active = 1;
    btn_anim_exiting = 1;
    btn_anim_entering = 0;
}

static void btn_anim_begin_enter(void) {
    if (new_theme == 0) return;
    btn_anim_t = 0;
    btn_anim_active = 1;
    btn_anim_exiting = 0;
    btn_anim_entering = 1;
}

/* Full travel: the button's left edge lands past the right screen edge. */
static double btn_anim_travel(void) {
    return screen_w + btn_w + 120;
}

/* Cubic ease-in for exit (accelerating away). */
static double btn_anim_exit_ease(double t) {
    return t * t * t;
}

/* Overshooting ease-out for enter (bouncy arrival). */
static double btn_anim_enter_ease(double t) {
    if (t < 0.85) {
        double u = t / 0.85;
        double cubic = 1 - (1 - u) * (1 - u) * (1 - u);
        return cubic * 1.15;
    }
    double s = (t - 0.85) / 0.15;
    return 1.15 - 0.15 * s * s * (3 - 2 * s);
}

static void btn_anim_tick(void) {
    if (btn_anim_active == 0) return;
    btn_anim_t = btn_anim_t + dt;
}

/* X offset for a button row.  Positive = to the right (off-screen). */
static double btn_anim_offset(double row, double row_count) {
    (void)row_count;
    if (new_theme == 0 || btn_anim_active == 0) return 0;
    double travel = btn_anim_travel();
    double delay = row * btn_anim_stagger;
    if (btn_anim_exiting == 1) {
        /* The exit is quicker than the enter and moves all rows together: the
         * whoosh is over right before the transition curtain turns the screen
         * black, so every button is seen leaving to the right. */
        double raw = btn_anim_t / (btn_anim_duration * 0.5);
        double t = clamp(raw, 0, 1);
        return btn_anim_exit_ease(t) * travel;
    }
    if (btn_anim_entering == 1) {
        double raw = (btn_anim_t - delay) / btn_anim_duration;
        if (raw < 0) return travel;
        double t = clamp(raw, 0, 1);
        double e = btn_anim_enter_ease(t);
        double p = e < 1 ? e : 1;
        double over = e > 1 ? (e - 1) : 0;
        /* The overshoot is measured against the button, not the travel
         * distance, so the bounce stays small no matter the screen width. */
        return (1 - p) * travel - over * btn_w * 0.7;
    }
    return 0;
}
