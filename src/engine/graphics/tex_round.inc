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
