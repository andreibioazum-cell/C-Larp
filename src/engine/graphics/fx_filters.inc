/* Black-and-white filter and screen shake helpers. The state lives in types.inc
 * so the color packer and the geometry layer can see it too. */

static void gray_texture_pixels(uint32_t *px, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint32_t c = px[i];
        uint32_t y = (299 * (c & 0xff) + 587 * ((c >> 8) & 0xff) + 114 * ((c >> 16) & 0xff)) / 1000;
        px[i] = (c & 0xff000000u) | (y << 16) | (y << 8) | y;
    }
}

/* Та же коррекция, но по пикселям самой картинки. Порядок байтов здесь
 * такой же, как его даёт декодер: младший байт - красный, старший из трёх -
 * синий (иначе красное и синее меняются местами). */
static void grade_texture_pixels(uint32_t *px, size_t n) {
    for (size_t i = 0; i < n; i++) {
        uint32_t c = px[i];
        if (!(c & 0xff000000u))
            continue;
        uint32_t r = c & 0xff, g = (c >> 8) & 0xff, b = (c >> 16) & 0xff;
        grade_channels(&r, &g, &b);
        px[i] = (c & 0xff000000u) | r | ((uint32_t)g << 8) | ((uint32_t)b << 16);
    }
}

/* Текстуры, которые встречаются только в бою: земля арены и боевые предметы.
 * Их можно корригировать смело - в лобби они не показываются, а весят они
 * немного, поэтому пересчёт при входе и выходе из боя ничего не стоит.
 * Спрайты бойцов и карточки сюда не входят: красная шапка обязана остаться
 * красной и в бою, и в магазине. */
static int texture_is_battle_only(const char *name) {
    if (!name)
        return 0;
    return strcmp(name, "grass.png") == 0 || strcmp(name, "snow.png") == 0 || strcmp(name, "candy.png") == 0 ||
           strcmp(name, "despenser.png") == 0 || strcmp(name, "dice.png") == 0;
}

/* Вернуть оригинал пикселей, наложить то, что включено сейчас, и заново
 * залить текстуру в GPU. Оригинал прячется один раз и не перезаписывается,
 * чтобы два фильтра не съели друг друга. */
static void texture_refresh(Texture *t) {
    size_t n = (size_t)t->w * (size_t)t->h;
    if (t->gray_backup) {
        memcpy(t->pixels, t->gray_backup, n * 4);
    } else {
        t->gray_backup = (uint32_t *)malloc(n * 4);
        if (!t->gray_backup)
            return;
        memcpy(t->gray_backup, t->pixels, n * 4);
    }
    if (gfx_gray_on)
        gray_texture_pixels(t->pixels, n);
    else if (gfx_grade_on)
        grade_texture_pixels(t->pixels, n);
    if (!gfx_gray_on && !gfx_grade_on) {
        free(t->gray_backup);
        t->gray_backup = NULL;
    }
    if (t->gpu.uploaded) {
        ds_vk_texture_gpu_release(t);
        ds_vk_pending_texture(t);
    }
}

/* Desaturate a freshly decoded texture when a filter is already on, keeping
 * its original colors for the eventual restore. */
static void texture_gray_apply(Texture *t) {
    if (!t || !t->pixels)
        return;
    if (!gfx_gray_on && !(gfx_grade_on && texture_is_battle_only(t->name)))
        return;
    size_t n = (size_t)t->w * (size_t)t->h;
    t->gray_backup = (uint32_t *)malloc(n * 4);
    if (!t->gray_backup)
        return;
    memcpy(t->gray_backup, t->pixels, n * 4);
    if (gfx_gray_on)
        gray_texture_pixels(t->pixels, n);
    else
        grade_texture_pixels(t->pixels, n);
}

void ds_gfx_gray(double on) {
    int want = on != 0;
    if (want == gfx_gray_on)
        return;
    gfx_gray_on = want;
    /* Swap the CPU-side texture pixels and re-upload them: the grayscale
     * happens on the stored image data, not in the prebuilt shaders. */
    for (Texture *t = textures; t; t = t->next) {
        if (!t->pixels)
            continue;
        texture_refresh(t);
    }
    ds_log("gfx: gray filter %s", want ? "on" : "off");
}

/* Цветокоррекция: pack_c() применяет её к каждому цвету кадра, а здесь
 * пересчитываются пиксели боевых текстур - земля арены занимает весь экран,
 * и без неё коррекции просто не видно. Спрайты и карточки не трогаем. Вызов
 * дешёвый: пока значение не меняется, он ничего не делает. */
void ds_gfx_grade(double on) {
    int want = on != 0;
    if (want == gfx_grade_on)
        return;
    gfx_grade_on = want;
    for (Texture *t = textures; t; t = t->next) {
        if (!t->pixels || !texture_is_battle_only(t->name))
            continue;
        texture_refresh(t);
    }
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
