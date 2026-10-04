#define DS_FB_FORMAT_RGB565 4
#define DS_FB_LINES 12

typedef struct {
    uint8_t *bits;
    int w, h, stride, bpp;
} DsFbTarget;

static void ds_fb_put(DsFbTarget *t, int x, int y, uint32_t rgb, int a) {
    if (x < 0 || y < 0 || x >= t->w || y >= t->h || a <= 0)
        return;
    if (a > 255)
        a = 255;
    int r = (int)((rgb >> 16) & 255), g = (int)((rgb >> 8) & 255), b = (int)(rgb & 255);
    if (t->bpp == 4) {
        uint8_t *p = t->bits + ((size_t)y * (size_t)t->stride + (size_t)x) * 4;
        p[0] = (uint8_t)(p[0] + ((r - p[0]) * a) / 255);
        p[1] = (uint8_t)(p[1] + ((g - p[1]) * a) / 255);
        p[2] = (uint8_t)(p[2] + ((b - p[2]) * a) / 255);
        p[3] = 255;
    } else {
        uint16_t *p = (uint16_t *)(void *)(t->bits + ((size_t)y * (size_t)t->stride + (size_t)x) * 2);
        int pr = ((*p >> 11) & 31) * 255 / 31, pg = ((*p >> 5) & 63) * 255 / 63, pb = (*p & 31) * 255 / 31;
        pr += ((r - pr) * a) / 255;
        pg += ((g - pg) * a) / 255;
        pb += ((b - pb) * a) / 255;
        *p = (uint16_t)(((pr * 31 / 255) << 11) | ((pg * 63 / 255) << 5) | (pb * 31 / 255));
    }
}

static void ds_fb_fill(DsFbTarget *t, uint32_t rgb) {
    for (int y = 0; y < t->h; y++)
        for (int x = 0; x < t->w; x++) ds_fb_put(t, x, y, rgb, 255);
}

static float ds_fb_text_width(const char *s, float sc) {
    float pen = 0;
    for (const char *c = s; *c;) {
        const DSFontGlyph *g = ds_font_glyph(font, (uint32_t)utf8_dec(&c));
        if (g)
            pen += g->advance * sc;
    }
    return pen;
}

static int ds_fb_text(DsFbTarget *t, const char *s, float x, float y, float sc, uint32_t rgb) {
    if (!font)
        return 0;
    const uint8_t *atlas = ds_font_alpha(font);
    int aw = ds_font_aw(font), ah = ds_font_ah(font);
    if (!atlas || aw <= 0 || ah <= 0)
        return 0;
    float asc = ds_font_ascent(font), lb = 0;
    const DSFontGlyph *ref = ds_font_glyph(font, 'S');
    if (ref) {
        asc = ref->bearing_top;
        lb = ref->bearing_x;
    }
    float pen = x - lb * sc, base = y + asc * sc;
    for (const char *cur = s; *cur;) {
        const DSFontGlyph *g = ds_font_glyph(font, (uint32_t)utf8_dec(&cur));
        if (!g)
            continue;
        if (g->width > 0 && g->height > 0) {
            int x0 = (int)floorf(pen + g->bearing_x * sc), y0 = (int)floorf(base - g->bearing_top * sc);
            int w = (int)ceilf(g->width * sc), h = (int)ceilf(g->height * sc);
            for (int dy = 0; dy < h; dy++) {
                int ay = (int)((g->v0 + (g->v1 - g->v0) * ((float)dy + 0.5f) / (float)h) * (float)ah);
                if (ay < 0 || ay >= ah)
                    continue;
                for (int dx = 0; dx < w; dx++) {
                    int ax = (int)((g->u0 + (g->u1 - g->u0) * ((float)dx + 0.5f) / (float)w) * (float)aw);
                    if (ax < 0 || ax >= aw)
                        continue;
                    ds_fb_put(t, x0 + dx, y0 + dy, rgb, atlas[(size_t)ay * (size_t)aw + (size_t)ax]);
                }
            }
        }
        pen += g->advance * sc;
    }
    return 1;
}

static void ds_fb_prop(const char *name, char *out, size_t cap) {
    if (!out || cap == 0)
        return;
    out[0] = '\0';
#ifdef __ANDROID__
    typedef int (*PropGetFn)(const char *, char *);
    PropGetFn get = (PropGetFn)dlsym(RTLD_DEFAULT, "__system_property_get");
    char value[96] = "";
    if (get && get(name, value) > 0)
        snprintf(out, cap, "%s", value);
#else
    (void)name;
#endif
}

const char *ds_graphics_failure(void) {
    return vk_fail_step;
}

static int ds_fb_lines(char lines[DS_FB_LINES][160], int attempts) {
    int n = 0;
    char model[96], maker[96], release[96], sdk[96];
    ds_fb_prop("ro.product.model", model, sizeof(model));
    ds_fb_prop("ro.product.manufacturer", maker, sizeof(maker));
    ds_fb_prop("ro.build.version.release", release, sizeof(release));
    ds_fb_prop("ro.build.version.sdk", sdk, sizeof(sdk));
    snprintf(lines[n++], 160, "Графика Vulkan не запустилась");
    snprintf(lines[n++], 160, "Сделай скриншот этого экрана и отправь разработчику.");
    snprintf(lines[n++], 160, " ");
    snprintf(lines[n++], 160, "Шаг: %s", vk_fail_step[0] ? vk_fail_step : "неизвестно");
    snprintf(lines[n++], 160, "Код Vulkan: %d", vk_fail_code);
    if (vk_diag_gpu[0])
        snprintf(lines[n++], 160, "GPU: %s, Vulkan %u.%u.%u, драйвер 0x%x, вендор 0x%x", vk_diag_gpu,
                 VK_API_VERSION_MAJOR(vk_diag_api), VK_API_VERSION_MINOR(vk_diag_api),
                 VK_API_VERSION_PATCH(vk_diag_api), (unsigned)vk_diag_driver, (unsigned)vk_diag_vendor);
    else
        snprintf(lines[n++], 160, "GPU: не найден (Vulkan-драйвера нет или он не отвечает)");
    if (model[0] || release[0])
        snprintf(lines[n++], 160, "Телефон: %s %s, Android %s (API %s)", maker, model, release, sdk);
    snprintf(lines[n++], 160, "Попыток запуска: %d", attempts);
    snprintf(lines[n++], 160, " ");
    snprintf(lines[n++], 160, "Закрой игру и открой её снова, чтобы попробовать ещё раз.");
    return n;
}

int ds_graphics_show_failure(ANativeWindow *window, int attempts) {
    if (!window)
        return 0;

    ds_vk_shutdown_backend();
    char lines[DS_FB_LINES][160];
    int n = ds_fb_lines(lines, attempts);
    for (int i = 0; i < n; i++) ds_log_err("vulkan start failure screen: %s", lines[i]);
    (void)ensure_font();
    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buf;
    memset(&buf, 0, sizeof(buf));
    if (ANativeWindow_lock(window, &buf, NULL) != 0 || !buf.bits || buf.width <= 0 || buf.height <= 0) {
        ds_log_err("vulkan start failure screen: the window cannot be locked by the CPU either");
        return 0;
    }
    DsFbTarget t = {(uint8_t *)buf.bits, buf.width, buf.height, buf.stride, buf.format == DS_FB_FORMAT_RGB565 ? 2 : 4};
    ds_fb_fill(&t, 0x1c0b10);
    int shown = 0;
    if (font) {

        float lh = ds_font_lineh(font);
        float margin = (float)t.w * 0.05f;
        float sc = ((float)t.h / 14.0f) / (lh > 0 ? lh : 1.0f);
        float widest = 0;
        for (int i = 0; i < n; i++) {
            float w = ds_fb_text_width(lines[i], 1.0f) * (i == 0 ? 1.4f : 1.0f);
            if (w > widest)
                widest = w;
        }
        if (widest > 0 && widest * sc > (float)t.w - 2 * margin)
            sc = ((float)t.w - 2 * margin) / widest;
        float y = (float)t.h * 0.08f;
        for (int i = 0; i < n; i++) {
            float s = i == 0 ? sc * 1.4f : sc;
            uint32_t color = i == 0 ? 0xffffff : (i == 1 ? 0xffd27a : 0xffb0b0);
            if (ds_fb_text(&t, lines[i], margin, y, s, color))
                shown = 1;
            y += lh * s * 1.15f;
        }
    }
    if (!shown) {

        for (int y = t.h / 3; y < t.h * 2 / 3; y++)
            for (int x = t.w / 3; x < t.w * 2 / 3; x++) ds_fb_put(&t, x, y, 0xc03030, 255);
    }
    ANativeWindow_unlockAndPost(window);
    return 1;
}
