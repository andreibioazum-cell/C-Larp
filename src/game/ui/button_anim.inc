/* Button exit animation.
 *
 * When the player taps a menu button to navigate away, all visible buttons
 * slide out to the right in a staggered wave.  When the new screen appears,
 * buttons are at their home positions instantly — no enter animation.
 *
 * start_transition() calls btn_anim_begin_exit();  enter_screen() calls
 * btn_anim_reset(). */

static void btn_anim_reset(void) {
    btn_anim_t = 0;
    btn_anim_active = 0;
    btn_anim_exiting = 0;
}

static void btn_anim_begin_exit(void) {
    btn_anim_t = 0;
    btn_anim_active = 1;
    btn_anim_exiting = 1;
}

/* Smooth ease-in for the exit slide (buttons accelerate away). */
static double btn_anim_exit_ease(double t) {
    return t * t * t;
}

static void btn_anim_tick(void) {
    if (btn_anim_active == 0) {
        return;
    }
    btn_anim_t = btn_anim_t + dt;
}

/* X offset for a given button row.  Positive = to the right (off-screen). */
static double btn_anim_offset(double row, double row_count) {
    (void)row_count;
    if (btn_anim_active == 0) {
        return 0;
    }
    double delay = row * btn_anim_stagger;
    double raw = (btn_anim_t - delay) / btn_anim_duration;
    double t = clamp(raw, 0, 1);
    return btn_anim_exit_ease(t) * btn_anim_offscreen;
}
