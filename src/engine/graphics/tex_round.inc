/* Draws an (unrotated) texture clipped to a rounded-rectangle silhouette, so
 * square card art can get soft corners without having to bake them into the
 * PNG itself. radius is in the same on-screen pixel units as x/y/scale. */
void tex_round(float x, float y, const char *name, float s, float r) {
    if (!frame_open)
        return;
    Texture *t = load_png(name);
    if (!t)
        return;
    DSCmd *p = push(DS_CMD_TEX_ROUND);
    if (!p)
        return;
    p->v.txr.x = x;
    p->v.txr.y = y;
    p->v.txr.sc = s;
    p->v.txr.r = r;
    p->v.txr.tx = t;
}

/* Texture drawn with a plain transparency: the picture keeps its own colours
 * (the vertex colour is white), only the alpha is scaled. Used where something
 * has to appear gradually instead of growing out of nothing. */
void tex_alpha(float x, float y, const char *name, float a, float s, double alpha) {
    if (!frame_open)
        return;
    Texture *t = load_png(name);
    if (!t)
        return;
    double k = alpha;
    if (k < 0) {
        k = 0;
    }
    if (k > 1) {
        k = 1;
    }
    unsigned int byte = (unsigned int)floor(k * 254) + 1;
    if (byte > 255) {
        byte = 255;
    }
    DSCmd *p = push(DS_CMD_TEX_FADE);
    if (!p)
        return;
    p->v.tx2.x = x;
    p->v.tx2.y = y;
    p->v.tx2.a = a;
    p->v.tx2.sc = s;
    p->v.tx2.tx = t;
    p->v.tx2.c = pack_c((byte << 24) | 0x00FFFFFF);
}
