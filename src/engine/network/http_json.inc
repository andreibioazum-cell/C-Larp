static int http(const char *method, const char *url, const char *body, char *out, size_t cap) {
    return http_ex(method, url, body, out, cap, NULL, NULL, NULL, 0);
}
static const char *skip_ws(const char *p) {
    while (p && *p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    return p;
}
static const char *skip_str(const char *p) {
    if (!p || *p != '\"')
        return NULL;
    for (p++; *p; p++) {
        if (*p == '\\' && p[1]) {
            p++;
            continue;
        }
        if (*p == '\"')
            return p + 1;
    }
    return NULL;
}
static const char *skip_box(const char *p, char open, char close) {
    int d = 0;
    if (!p || *p != open)
        return NULL;
    for (; *p; p++) {
        if (*p == '\"') {
            p = skip_str(p);
            if (!p)
                return NULL;
            p--;
            continue;
        }
        if (*p == open)
            d++;
        else if (*p == close && --d == 0)
            return p + 1;
    }
    return NULL;
}
static const char *skip_val(const char *p) {
    p = skip_ws(p);
    if (!p || !*p)
        return NULL;
    if (*p == '\"')
        return skip_str(p);
    if (*p == '{')
        return skip_box(p, '{', '}');
    if (*p == '[')
        return skip_box(p, '[', ']');
    while (*p && *p != ',' && *p != '}' && *p != ']') p++;
    return p;
}
static const char *member(const char *o, const char *key) {
    size_t n = strlen(key);
    const char *p = skip_ws(o);
    if (!p || *p != '{')
        return NULL;
    for (p = skip_ws(p + 1); p && *p && *p != '}';) {
        const char *name = p, *end = skip_str(p);
        if (!end)
            return NULL;
        p = skip_ws(end);
        if (*p != ':')
            return NULL;
        p = skip_ws(p + 1);
        if ((size_t)(end - name - 2) == n && strncmp(name + 1, key, n) == 0)
            return p;
        p = skip_val(p);
        p = skip_ws(p);
        if (p && *p == ',')
            p = skip_ws(p + 1);
    }
    return NULL;
}
static const char *element(const char *a, size_t wanted) {
    const char *p = skip_ws(a);
    size_t index = 0;
    if (!p || *p != '[')
        return NULL;
    for (p = skip_ws(p + 1); p && *p && *p != ']'; index++) {
        if (index == wanted)
            return p;
        p = skip_val(p);
        p = skip_ws(p);
        if (p && *p == ',')
            p = skip_ws(p + 1);
        else
            break;
    }
    return NULL;
}
static const char *path_val(const char *json, const char *path) {
    char part[48], *end;
    const char *v = json;
    while (path && *path && v) {
        const char *slash = strchr(path, '/');
        size_t n = slash ? (size_t)(slash - path) : strlen(path), index;
        if (!n || n >= sizeof(part))
            return NULL;
        memcpy(part, path, n);
        part[n] = 0;
        v = skip_ws(v);
        if (*v == '[') {
            index = strtoul(part, &end, 10);
            if (!*part || *end)
                return NULL;
            v = element(v, index);
        } else
            v = member(v, part);
        path = slash ? slash + 1 : path + n;
    }
    return v;
}
static double num(const char *json, const char *path, double fb) {
    const char *v = path_val(json, path);
    if (!v || !strncmp(v, "null", 4))
        return fb;
    if (*v == 't')
        return 1;
    if (*v == 'f')
        return 0;
    if (*v == '\"')
        v++;
    return atof(v);
}
static void strv(const char *json, const char *path, char *out, size_t cap) {
    const char *v = path_val(json, path);
    size_t i = 0;
    if (cap)
        out[0] = 0;
    if (!v || *v != '\"')
        return;
    for (v++; *v && *v != '\"' && i + 1 < cap; v++) {
        if (*v == '\\' && v[1])
            v++;
        out[i++] = *v;
    }
    out[i] = 0;
}
static void json_escape(const char *src, char *dst, size_t cap) {
    size_t o = 0;
    if (cap == 0)
        return;
    for (size_t i = 0; src[i] && o + 2 < cap; i++) {
        char c = src[i];
        if (c == '\"') {
            dst[o++] = '\\';
            dst[o++] = '\"';
        } else if (c == '\\') {
            dst[o++] = '\\';
            dst[o++] = '\\';
        } else if (c == '\n') {
            dst[o++] = '\\';
            dst[o++] = 'n';
        } else if (c == '\r') {
            dst[o++] = '\\';
            dst[o++] = 'r';
        } else if ((unsigned char)c < 0x20) {
        } else {
            dst[o++] = c;
        }
    }
    dst[o] = '\0';
}

#define FB_EMAIL_DOMAIN "@cb4.game"

#define FB_PASS_SUFFIX "|cb4v1"
typedef struct {
    DSMutex lock;
    char key[128];
    char id_token[FB_ID_MAX];
    char refresh_token[FB_REFRESH_MAX];
    long long expires_at;
} FirebaseAuth;
static FirebaseAuth fb = {.lock = DS_MUTEX_INIT};

void net_set_firebase_key(const char *key) {
    size_t n;
    if (!key)
        key = "";
    while (*key == ' ' || *key == '\t') key++;
    ds_mutex_lock(fb.lock);
    snprintf(fb.key, sizeof(fb.key), "%s", key);
    n = strlen(fb.key);
    while (n && (fb.key[n - 1] == ' ' || fb.key[n - 1] == '\t' || fb.key[n - 1] == '\r' || fb.key[n - 1] == '\n'))
        fb.key[--n] = 0;
    ds_mutex_unlock(fb.lock);
    if (fb.key[0])
        LOG("firebase auth enabled (key set)");
}
static int fb_enabled(void) {
    return fb.key[0] != 0;
}

static void fb_store_tokens(const char *id_token, const char *refresh_token, double expires_in_sec) {
    ds_mutex_lock(fb.lock);
    snprintf(fb.id_token, sizeof(fb.id_token), "%s", id_token ? id_token : "");
    snprintf(fb.refresh_token, sizeof(fb.refresh_token), "%s", refresh_token ? refresh_token : "");
    if (expires_in_sec < 60)
        expires_in_sec = 60;
    fb.expires_at = now_ms() + (long long)(expires_in_sec * 1000.0) - 60000;
    ds_mutex_unlock(fb.lock);
}

static void fb_auth_endpoint(char *dst, size_t cap, const char *path) {
    if (net.base[0] && !strncmp(net.base, "http://", 7)) {
        snprintf(dst, cap, "%s%s?key=%s", net.base, path, fb.key);
        return;
    }
    if (!strcmp(path, "/v1/token"))
        snprintf(dst, cap, "https://securetoken.googleapis.com/v1/token?key=%s", fb.key);
    else

        snprintf(dst, cap, "https://identitytoolkit.googleapis.com%s?key=%s", path, fb.key);
}
