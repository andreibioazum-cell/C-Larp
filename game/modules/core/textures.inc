static void load_textures(void) {
    png_load(GRASS);
    png_load(ORDINARY_TEX);
    ordinary_punch_tex_ok = png_load(PUNCH_TEX);
    azum_tex_ok = png_load(AZUM_TEX);
    azum_punch_tex_ok = png_load(AZUM_PUNCH_TEX);
    azum_zombie_tex_ok = png_load(AZUM_ZOMBIE_TEX);
    azum_zombie_punch_tex_ok = png_load(AZUM_ZOMBIE_PUNCH_TEX);
    png_load(SANTA_TEX);
    santa_punch_tex_ok = png_load(SANTA_PUNCH_TEX);
    png_load(EBUC_TEX);
    ebuc_punch_tex_ok = png_load(EBUC_PUNCH_TEX);
    png_load(DESPENSER_TEX);
    candy_tex_ok = png_load(CANDY_TEX);
    png_load(SNOWFLAKE_TEX);
    tex_reload_t = 0;
    tex_reload_left = 60;
}

static double textures_all_ok(void) {
    if (ordinary_punch_tex_ok == 0 || azum_tex_ok == 0 || azum_punch_tex_ok == 0) {
        return 0;
    }
    if (azum_zombie_tex_ok == 0 || azum_zombie_punch_tex_ok == 0) {
        return 0;
    }
    if (santa_punch_tex_ok == 0 || ebuc_punch_tex_ok == 0 || candy_tex_ok == 0) {
        return 0;
    }
    if (winter_theme == 1 && snow_tex_ok == 0) {
        return 0;
    }
    return 1;
}

static void reload_textures(void) {
    if (tex_reload_left <= 0 && textures_all_ok() == 1) {
        return;
    }
    if (tex_reload_t > 0) {
        tex_reload_t = tex_reload_t - dt;
        return;
    }
    if (tex_reload_left > 0) {
        tex_reload_t = 0.5;
        tex_reload_left = tex_reload_left - 1;
    } else {
        tex_reload_t = tex_reload_slow;
    }
    if (ordinary_punch_tex_ok == 0) {
        ordinary_punch_tex_ok = png_load(PUNCH_TEX);
    }
    if (azum_tex_ok == 0) {
        azum_tex_ok = png_load(AZUM_TEX);
    }
    if (azum_punch_tex_ok == 0) {
        azum_punch_tex_ok = png_load(AZUM_PUNCH_TEX);
    }
    if (azum_zombie_tex_ok == 0) {
        azum_zombie_tex_ok = png_load(AZUM_ZOMBIE_TEX);
    }
    if (azum_zombie_punch_tex_ok == 0) {
        azum_zombie_punch_tex_ok = png_load(AZUM_ZOMBIE_PUNCH_TEX);
    }
    if (santa_punch_tex_ok == 0) {
        santa_punch_tex_ok = png_load(SANTA_PUNCH_TEX);
    }
    if (ebuc_punch_tex_ok == 0) {
        ebuc_punch_tex_ok = png_load(EBUC_PUNCH_TEX);
    }
    if (candy_tex_ok == 0) {
        candy_tex_ok = png_load(CANDY_TEX);
    }
    if (winter_theme == 1 && snow_tex_ok == 0) {
        snow_tex_ok = png_load(SNOW_TEX);
    }
    if (textures_all_ok() == 1) {
        tex_reload_left = 0;
    }
}

static void apply_winter_theme(void) {
    if (winter_theme == 1) {
        snow_tex_ok = png_load(SNOW_TEX);
        return;
    }
    snow_tex_ok = 0;
}
