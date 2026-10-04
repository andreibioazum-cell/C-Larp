static int flat_rsrv(DSFC *f, int n) {
    int need = f->count + n, cap = f->cap ? f->cap : 32;
    while (cap < need) cap *= 2;
    DSFPoint *p = (DSFPoint *)realloc(f->points, (size_t)cap * sizeof(*p));
    if (!p)
        return 0;
    f->points = p;
    f->cap = cap;
    return 1;
}
static int flat_push(DSFC *f, float x, float y) {
    if (!flat_rsrv(f, 1))
        return 0;
    f->points[f->count].x = x;
    f->points[f->count].y = y;
    f->count++;
    return 1;
}
static int flat_q(DSFC *f, DSFPoint from, DSPoint c, DSFPoint to) {
    for (int s = 1; s <= 8; s++) {
        float t = (float)s / 8.0f, u = 1 - t;
        if (!flat_push(f, u * u * from.x + 2 * u * t * c.x + t * t * to.x,
                       u * u * from.y + 2 * u * t * c.y + t * t * to.y))
            return 0;
    }
    return 1;
}
static int flatten(const DSPoint *p, int n, DSFC *flat) {
    if (n <= 0)
        return 1;
    int first_on = p[0].on_curve, idx, proc;
    DSFPoint start, cur;
    if (first_on) {
        start.x = p[0].x;
        start.y = p[0].y;
        idx = 1;
        proc = 1;
    } else if (p[n - 1].on_curve) {
        start.x = p[n - 1].x;
        start.y = p[n - 1].y;
        idx = 0;
        proc = 0;
    } else {
        start.x = (p[n - 1].x + p[0].x) * 0.5f;
        start.y = (p[n - 1].y + p[0].y) * 0.5f;
        idx = 0;
        proc = 0;
    }
    cur = start;
    if (!flat_push(flat, start.x, start.y))
        return 0;
    while (proc < n) {
        const DSPoint *one = &p[idx % n];
        if (one->on_curve) {
            cur.x = one->x;
            cur.y = one->y;
            if (!flat_push(flat, cur.x, cur.y))
                return 0;
            idx++;
            proc++;
        } else {
            const DSPoint *two = &p[(idx + 1) % n];
            DSFPoint end;
            if (two->on_curve) {
                end.x = two->x;
                end.y = two->y;
                idx += 2;
                proc += 2;
            } else {
                end.x = (one->x + two->x) * 0.5f;
                end.y = (one->y + two->y) * 0.5f;
                idx++;
                proc++;
            }
            if (!flat_q(flat, cur, *one, end))
                return 0;
            cur = end;
        }
    }
    if (fabsf(cur.x - start.x) > 0.001f || fabsf(cur.y - start.y) > 0.001f) {
        if (!flat_push(flat, start.x, start.y))
            return 0;
    }
    return 1;
}

static void raster_glyph(DSFont *f, const DSFC *flat, int fc, int mnx, int mxy, int ax, int ay, int w, int h) {
    int *accum = (int *)calloc((size_t)w * h, sizeof(int));
    float xs[128];
    if (!accum)
        return;
    float sxs = f->scale * DS_FONT_SS;
    int rows = h * DS_FONT_SS, max_i = w * DS_FONT_SS - 1;
    for (int row = 0; row < rows; row++) {
        float fy = (float)mxy - ((float)row + 0.5f) / sxs;
        int nx = 0;
        for (int cn = 0; cn < fc; cn++) {
            const DSFC *poly = &flat[cn];
            for (int k = 0, j = poly->count - 1; k < poly->count; j = k++) {
                float yi = poly->points[k].y, yj = poly->points[j].y;
                if ((yi > fy) == (yj > fy))
                    continue;
                float xh = poly->points[j].x + (poly->points[k].x - poly->points[j].x) * (fy - yj) / (yi - yj);
                if (nx < (int)(sizeof(xs) / sizeof(xs[0])))
                    xs[nx++] = (xh - (float)mnx) * sxs;
            }
        }
        for (int a = 1; a < nx; a++) {
            float v = xs[a];
            int b = a;
            while (b > 0 && xs[b - 1] > v) {
                xs[b] = xs[b - 1];
                b--;
            }
            xs[b] = v;
        }
        int py = row / DS_FONT_SS;
        for (int k = 0; k + 1 < nx; k += 2) {
            int i0 = (int)ceilf(xs[k] - 0.5f), i1 = (int)floorf(xs[k + 1] - 0.5f);
            if (i0 < 0)
                i0 = 0;
            if (i1 > max_i)
                i1 = max_i;
            for (int i = i0; i <= i1; i++) accum[py * w + i / DS_FONT_SS]++;
        }
    }
    for (int py = 0; py < h; py++)
        for (int px = 0; px < w; px++)
            f->alpha[(size_t)(ay + py) * f->aw + (ax + px)] =
                (uint8_t)((accum[py * w + px] * 255) / (DS_FONT_SS * DS_FONT_SS));
    free(accum);
}
static int g_metrics(const DSFont *f, int g, int *adv, int *lsb) {
    if (!f || g < 0 || g >= f->ng || !f->hmtx)
        return 0;
    int m = g < f->nhm ? g : f->nhm - 1;
    size_t o = (size_t)f->hmtx + (size_t)m * 4;
    if (adv)
        *adv = bu16(f, o);
    if (lsb) {
        size_t lo = g < f->nhm ? o + 2 : (size_t)f->hmtx + (size_t)f->nhm * 4 + (size_t)(g - f->nhm) * 2;
        *lsb = bs16(f, lo);
    }
    return 1;
}
static int cmap4_lk(const DSFont *f, uint32_t cp) {
    if (!f->cmap4 || cp > 0xFFFF)
        return 0;
    size_t b = f->cmap4;
    uint16_t sc = bu16(f, b + 6) / 2;
    for (uint16_t i = 0; i < sc; i++) {
        uint16_t en = bu16(f, b + 14 + i * 2);
        uint16_t st = bu16(f, b + 16 + sc * 2 + i * 2);
        if (cp < st || cp > en)
            continue;
        int16_t dlt = bs16(f, b + 16 + sc * 4 + i * 2);
        uint16_t rng = bu16(f, b + 16 + sc * 6 + i * 2);
        if (rng == 0)
            return ((int)cp + dlt) & 0xFFFF;
        size_t ga = b + 16 + sc * 6 + i * 2 + rng + (cp - st) * 2;
        uint16_t g = bu16(f, ga);
        return g ? ((int)g + dlt) & 0xFFFF : 0;
    }
    return 0;
}
static int cmap12_lk(const DSFont *f, uint32_t cp) {
    if (!f->cmap12)
        return 0;
    size_t b = f->cmap12;
    uint32_t n = bu32(f, b + 12);
    for (uint32_t i = 0; i < n; i++) {
        size_t at = b + 16 + i * 12;
        uint32_t f1 = bu32(f, at), l1 = bu32(f, at + 4);
        if (cp >= f1 && cp <= l1)
            return (int)(bu32(f, at + 8) + cp - f1);
    }
    return 0;
}
static int g_for(const DSFont *f, uint32_t cp) {
    int g = cmap12_lk(f, cp);
    if (!g)
        g = cmap4_lk(f, cp);
    if (g < 0 || g >= f->ng)
        g = 0;
    return g;
}
static int bake_glyph(DSFont *f, DSFontGlyph *g, int ax, int ay, int rh) {
    int gi = g_for(f, g->codepoint);
    int adv_u = 0;
    DSOutline ol;
    int mnx = 0, mxx = 0, mny = 0, mxy = 0, has = 0;
    int st = 0;
    int w, h;
    DSFC *flat = NULL;
    int fc = 0;
    float s = f->scale;
    g_metrics(f, gi, &adv_u, NULL);
    g->advance = adv_u * s;
    g->bearing_x = 0;
    g->bearing_top = 0;
    g->width = 0;
    g->height = 0;
    g->u0 = g->u1 = (float)ax / f->aw;
    g->v0 = g->v1 = (float)ay / f->ah;
    ol_init(&ol);
    if (!read_outline(f, gi, 0, 1, 0, 0, 1, 0, 0, &ol)) {
        ol_free(&ol);
        return rh;
    }
    for (int cn = 0; cn < ol.cc; cn++) {
        int en = ol.ends[cn];
        for (int p = st; p <= en; p++) {
            int x = (int)lrintf(ol.points[p].x);
            int y = (int)lrintf(ol.points[p].y);
            if (!has || x < mnx)
                mnx = x;
            if (!has || x > mxx)
                mxx = x;
            if (!has || y < mny)
                mny = y;
            if (!has || y > mxy)
                mxy = y;
            has = 1;
        }
        st = en + 1;
    }
    if (!has) {
        ol_free(&ol);
        return rh;
    }
    w = (int)ceilf((mxx - mnx) * s) + 2;
    h = (int)ceilf((mxy - mny) * s) + 2;
    if (w < 1)
        w = 1;
    if (h < 1)
        h = 1;
    g->bearing_x = mnx * s;
    g->bearing_top = mxy * s;
    g->width = w;
    g->height = h;
    flat = (DSFC *)calloc((size_t)ol.cc, sizeof(*flat));
    if (!flat) {
        ol_free(&ol);
        return rh;
    }
    st = 0;
    for (int cn = 0; cn < ol.cc; cn++) {
        int en = ol.ends[cn];
        if (!flatten(ol.points + st, en - st + 1, &flat[fc])) {
            for (int i = 0; i <= fc; i++) free(flat[i].points);
            free(flat);
            ol_free(&ol);
            return rh;
        }
        fc++;
        st = en + 1;
    }
    raster_glyph(f, flat, fc, mnx, mxy, ax, ay, w, h);
    g->u0 = (float)ax / f->aw;
    g->v0 = (float)ay / f->ah;
    g->u1 = (float)(ax + w) / f->aw;
    g->v1 = (float)(ay + h) / f->ah;
    for (int cn = 0; cn < fc; cn++) free(flat[cn].points);
    free(flat);
    ol_free(&ol);
    return h > rh ? h : rh;
}
static int init_tables(DSFont *f) {
    uint32_t l;
    if (!tbound(f, TAG('h', 'e', 'a', 'd'), &f->head, &l) || l < 54 ||
        !tbound(f, TAG('h', 'h', 'e', 'a'), &f->hhea, &l) || l < 36 ||
        !tbound(f, TAG('h', 'm', 't', 'x'), &f->hmtx, &l) || !tbound(f, TAG('m', 'a', 'x', 'p'), &f->maxp, &l) ||
        l < 6 || !tbound(f, TAG('l', 'o', 'c', 'a'), &f->loca, &l) ||
        !tbound(f, TAG('g', 'l', 'y', 'f'), &f->glyf, &l) || !tbound(f, TAG('c', 'm', 'a', 'p'), &f->cmap, &l) || l < 4)
        return 0;
    f->upem = bu16(f, f->head + 18);
    f->loc_format = bs16(f, f->head + 50);
    f->ng = bu16(f, f->maxp + 4);
    f->nhm = bu16(f, f->hhea + 34);
    f->asc_u = bs16(f, f->hhea + 4);
    f->desc_u = bs16(f, f->hhea + 6);
    f->lg_u = bs16(f, f->hhea + 8);
    if (f->upem <= 0 || f->ng <= 0 || f->nhm <= 0)
        return 0;
    uint16_t n = bu16(f, f->cmap + 2);
    for (uint16_t i = 0; i < n; i++) {
        size_t r = f->cmap + 4 + (size_t)i * 8;
        uint16_t p = bu16(f, r), e = bu16(f, r + 2);
        uint32_t sub = f->cmap + bu32(f, r + 4);
        uint16_t fmt = bu16(f, sub);
        if (fmt == 12 && (p == 3 || p == 0)) {
            if (!f->cmap12 || (p == 3 && e == 10))
                f->cmap12 = sub;
        } else if (fmt == 4 && (p == 3 || p == 0)) {
            if (!f->cmap4 || (p == 3 && e == 1))
                f->cmap4 = sub;
        }
    }
    return f->cmap4 || f->cmap12;
}
static int add_cp(DSFont *f, uint32_t cp) {
    for (int i = 0; i < f->gcount; i++)
        if (f->glyphs[i].codepoint == cp)
            return 1;
    if (f->gcount >= DS_FONT_MAX)
        return 0;
    f->glyphs[f->gcount++].codepoint = cp;
    return 1;
}
static int add_utf8(DSFont *f, const char *text) {
    const uint8_t *p = (const uint8_t *)text;
    while (p && *p) {
