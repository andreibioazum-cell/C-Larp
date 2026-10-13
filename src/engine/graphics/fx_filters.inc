/* Black-and-white filter and screen shake helpers. The state lives in types.inc
 * so the color packer and the geometry layer can see it too. */

static void gray_texture_pixels(uint32_t *px, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint32_t c = px[i];
        uint32_t y = (299 * (c & 0xff) + 587 * ((c >> 8) & 0xff) + 114 * ((c >> 16) & 0xff)) / 1000;
        px[i] = (c & 0xff000000u) | (y << 16) | (y << 8) | y;
    }
}

/* Та же коррекция, но по пикселям самой картинки: спрайты бойцов получают
 * её вместе со всеми цветами кадра. Прозрачные пиксели не трогаем. */
static void grade_texture_pixels(uint32_t *px, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint32_t c = px[i];
        if (!(c & 0xff000000u))
            continue;
        uint32_t r = c & 0xff, g = (c >> 8) & 0xff, b = (c >> 16) & 0xff;
        grade_channels(&r, &g, &b);
        px[i] = (c & 0xff000000u) | (r << 16) | (g << 8) | b;
    }
}

/* Оригинал текстуры прячем в gray_backup, чтобы фильтр можно было снять. */
static void texture_backup(Texture *t, size_t n) {
    if (t->gray_backup)
        return;
    t->gray_backup = (uint32_t *)malloc(n * 4);
    if (!t->gray_backup)
        return;
    memcpy(t->gray_backup, t->pixels, n * 4);
}

/* Что именно сейчас наложить на пиксели: ч/б важнее, он временный. */
static void texture_filters_apply(Texture *t, size_t n) {
    if (gfx_gray_on)
        gray_texture_pixels(t->pixels, n);
    else if (gfx_grade_on)
        grade_texture_pixels(t->pixels, n);
}

/* Desaturate a freshly decoded texture when a filter is already on, keeping
 * its original colors for the eventual restore. */
static void texture_gray_apply(Texture *t) {
    if (!t || !t->pixels || (!gfx_gray_on && !gfx_grade_on))
        return;
    size_t n = (size_t)t->w * (size_t)t->h;
    texture_backup(t, n);
    texture_filters_apply(t, n);
}

/* Переключение любого фильтра: вернуть оригинал, наложить то, что осталось
 * включённым, и заново залить текстуру в GPU. */
static void texture_filters_refresh(void) {
    for (Texture *t = textures; t; t = t->next) {
        if (!t->pixels)
            continue;
        size_t n = (size_t)t->w * (size_t)t->h;
        if (t->gray_backup) {
            memcpy(t->pixels, t->gray_backup, n * 4);
            free(t->gray_backup);
            t->gray_backup = NULL;
        }
        if (gfx_gray_on || gfx_grade_on) {
            texture_backup(t, n);
            texture_filters_apply(t, n);
        }
        if (t->gpu.uploaded) {
            ds_vk_texture_gpu_release(t);
            ds_vk_pending_texture(t);
        }
    }
}

void ds_gfx_gray(double on) {
    int want = on != 0;
    if (want == gfx_gray_on)
        return;
    gfx_gray_on = want;
    /* Swap the CPU-side texture pixels and re-upload them: the grayscale
     * happens on the stored image data, not in the prebuilt shaders. */
    texture_filters_refresh();
    ds_log("gfx: gray filter %s", want ? "on" : "off");
}

/* Цветокоррекция кадра: считается в pack_c() для каждого цвета и в пикселях
 * текстур, поэтому это честная коррекция, а не полупрозрачная плёнка. */
void ds_gfx_grade(double on) {
    int want = on != 0;
    if (want == gfx_grade_on)
        return;
    gfx_grade_on = want;
    texture_filters_refresh();
    ds_log("gfx: color grade %s", want ? "on" : "off");
}

void ds_gfx_shake(double amp, double seconds) {
    if (seconds <= 0 || amp <= 0) {
        gfx_shake_amp = 0;
        gfx_shake_left = 0;
        gfx_shake_dur = 0;
        gfx_shake_dx = 0;
        gfx_shake_dy = 0;
        return;
    }
    /* An ongoing stronger shake is not cut short by a weaker trigger. */
    if (gfx_shake_left > 0 && gfx_shake_amp >= (float)amp)
        return;
    gfx_shake_amp = (float)amp;
    gfx_shake_dur = (float)seconds;
    gfx_shake_left = (float)seconds;
}
