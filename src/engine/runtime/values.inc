double clamp(double v, double lo, double hi) {
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}
double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}
double dist(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1, dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

double ds_min(double a, double b) {
    return a < b ? a : b;
}
double ds_max(double a, double b) {
    return a > b ? a : b;
}
double ds_abs(double v) {
    return v < 0 ? -v : v;
}
double ds_round(double v) {
    return v < 0 ? -floor(-v + 0.5) : floor(v + 0.5);
}
double ds_sign(double v) {
    return v > 0 ? 1 : (v < 0 ? -1 : 0);
}

double ds_mod(double a, double b) {
    if (b == 0)
        return 0;
    return fmod(a, b);
}
double ds_trunc(double v) {
    return v < 0 ? ceil(v) : floor(v);
}
double str_len(const char *s) {
    return s ? (double)strlen(s) : 0;
}
int str_eq(const char *a, const char *b) {
    if (a == b)
        return 1;
    if (!a || !b)
        return 0;
    return strcmp(a, b) == 0;
}
int str_contains(const char *hay, const char *needle) {
    if (!hay || !needle)
        return 0;
    if (!*needle)
        return 1;
    return strstr(hay, needle) != NULL ? 1 : 0;
}
double str_index_of(const char *hay, const char *needle) {
    if (!hay || !needle)
        return -1;
    const char *p = strstr(hay, needle);
    if (!p)
        return -1;
    return (double)(p - hay);
}
int str_starts_with(const char *s, const char *pref) {
    if (!s || !pref)
        return 0;
    size_t ls = strlen(s), lp = strlen(pref);
    if (lp > ls)
        return 0;
    return strncmp(s, pref, lp) == 0 ? 1 : 0;
}
int str_ends_with(const char *s, const char *suf) {
    if (!s || !suf)
        return 0;
    size_t ls = strlen(s), lf = strlen(suf);
    if (lf > ls)
        return 0;
    return strcmp(s + ls - lf, suf) == 0 ? 1 : 0;
}
const char *str_sub(const char *s, double start, double len) {
    if (!s)
        return ds_track_string(ds_strdup(""));
    size_t sl = strlen(s);
    long st = (long)start;
    long ln = (long)len;
    if (st < 0)
        st = 0;
    if ((size_t)st > sl)
        st = sl;
    if (ln < 0)
        ln = 0;
    if ((size_t)(st + ln) > sl)
        ln = sl - st;
    char *out = (char *)malloc((size_t)ln + 1);
    if (!out)
        return ds_track_string(ds_strdup(""));
    memcpy(out, s + st, (size_t)ln);
    out[ln] = '\0';
    return ds_track_string(out);
}
double str_to_num(const char *s) {
    if (!s)
        return 0;
    char *end = NULL;
    double v = strtod(s, &end);
    if (end == s)
        return 0;
    return v;
}
const char *str_trim(const char *s) {
    if (!s)
        return ds_track_string(ds_strdup(""));
    const char *a = s;
    while (*a && (*a == ' ' || *a == '\t' || *a == '\n' || *a == '\r')) a++;
    const char *b = s + strlen(s);
    while (b > a && (b[-1] == ' ' || b[-1] == '\t' || b[-1] == '\n' || b[-1] == '\r')) b--;
    size_t ln = b - a;
    char *out = (char *)malloc(ln + 1);
    if (!out)
        return ds_track_string(ds_strdup(""));
    memcpy(out, a, ln);
    out[ln] = '\0';
    return ds_track_string(out);
}
const char *str_lower(const char *s) {
    if (!s)
        return ds_track_string(ds_strdup(""));
    size_t l = strlen(s);
    char *out = (char *)malloc(l + 1);
    if (!out)
        return ds_track_string(ds_strdup(""));
    for (size_t i = 0; i < l; i++) {
        char c = s[i];
        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a';
        out[i] = c;
    }
    out[l] = '\0';
    return ds_track_string(out);
}
const char *str_upper(const char *s) {
    if (!s)
        return ds_track_string(ds_strdup(""));
    size_t l = strlen(s);
    char *out = (char *)malloc(l + 1);
    if (!out)
        return ds_track_string(ds_strdup(""));
    for (size_t i = 0; i < l; i++) {
        char c = s[i];
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';
        out[i] = c;
    }
    out[l] = '\0';
    return ds_track_string(out);
}
