static int gfx_auto_scale = 1;
static int gfx_cooldown = 0;
static int gfx_finer_cooldown = 0;
static int gfx_probe_finer = 0;
static int gfx_probe_left = 0;
static int gfx_probe_fails = 0;
static int gfx_finer_off = 0;
static int gfx_clean = 0;
static int gfx_miss_streak = 0, gfx_win_frames = 0, gfx_win_misses = 0;

int ds_graphics_pixel_scale(void) {
    return gfx_auto_scale;
}

#define GFX_WARMUP_FRAMES 90

#define GFX_HITCH_SEC 0.100

static void gfx_autoscale_new_window(void) {
    if (gfx_auto_scale != 1)
        ds_log("graphics: internal resolution 1/1 (new window)");
    gfx_auto_scale = 1;
    gfx_cooldown = GFX_WARMUP_FRAMES;
    gfx_finer_cooldown = 0;
    gfx_probe_finer = 0;
    gfx_probe_left = 0;
    gfx_probe_fails = 0;
    gfx_finer_off = 0;
    gfx_miss_streak = 0;
    gfx_win_frames = 0;
    gfx_win_misses = 0;
    gfx_clean = 0;
}

static void gfx_autoscale_reset(void) {
    gfx_autoscale_new_window();
}

static void gfx_set_scale(int s, int cooldown) {
    if (s < 1)
        s = 1;

    if (s > 6)
        s = 6;
    if (s == gfx_auto_scale)
        return;
    gfx_auto_scale = s;
    gfx_cooldown = cooldown;
    gfx_miss_streak = 0;
    gfx_win_frames = 0;
    gfx_win_misses = 0;
    ds_log("graphics: internal resolution 1/%d (auto)", s);
}

static double gfx_report_acc;
static int gfx_report_frames;
#define GFX_REPORT_SEC 5.0

void ds_graphics_report_frame_interval(double sec) {
    if (!(sec > 0.0) || sec > 0.25)
        return;
    if (sec > GFX_HITCH_SEC) {
        gfx_clean = 0;
        return;
    }
    gfx_report_acc += sec;
    gfx_report_frames++;
    if (gfx_report_acc >= GFX_REPORT_SEC) {
        ds_log("graphics: %.1f fps, frame %.1f ms, internal 1/%d, present %s, swapchain %ux%u, recreates %u",
               (double)gfx_report_frames / gfx_report_acc, 1000.0 * gfx_report_acc / (double)gfx_report_frames,
               gfx_auto_scale, ds_vk_present_mode_name(vk_sw_present_mode), (unsigned)vk_sw_extent.width,
               (unsigned)vk_sw_extent.height, vk_sw_recreates);
        gfx_report_acc = 0;
        gfx_report_frames = 0;
    }
    if (gfx_finer_cooldown > 0)
        gfx_finer_cooldown--;

    int missed = sec > 0.020;
    gfx_clean = missed ? 0 : gfx_clean + 1;
    if (gfx_cooldown > 0) {
        gfx_cooldown--;
        return;
    }

    if (gfx_probe_finer && gfx_probe_left > 0) {
        gfx_probe_left--;
        if (gfx_probe_left == 0)
            gfx_probe_finer = 0;
    }
    if (missed) {
        if (gfx_probe_finer) {
            gfx_probe_finer = 0;
            gfx_probe_left = 0;
            gfx_probe_fails++;
            if (gfx_probe_fails >= 2)
                gfx_finer_off = 1;
            gfx_finer_cooldown = gfx_finer_off ? 1000000000 : 1800;
            gfx_set_scale(gfx_auto_scale + 1, 90);
            return;
        }
        gfx_miss_streak++;
    } else {
        gfx_miss_streak = 0;
    }
    gfx_win_frames++;
    if (missed)
        gfx_win_misses++;
    if (gfx_miss_streak >= 4 || gfx_win_frames >= 24) {
        int bad = gfx_miss_streak >= 4 || gfx_win_misses >= 4;
        if (bad) {

            if (gfx_auto_scale < 6)
                gfx_set_scale(gfx_auto_scale + 1, 12);
        } else if (gfx_win_misses == 0 && gfx_clean >= 300 && gfx_auto_scale > 1 && gfx_finer_cooldown == 0 &&
                   !gfx_finer_off) {
            gfx_probe_finer = 1;
            gfx_finer_cooldown = 900;
            gfx_set_scale(gfx_auto_scale - 1, 45);
            gfx_probe_left = 45 + 60;
        }
        gfx_win_frames = 0;
        gfx_win_misses = 0;
    }
}
