static int http_patch_once(HttpJob *j) {
    int code;
    if (fb_enabled()) {
        fb_ensure_token();
        fb_url(j->url, sizeof(j->url), j->plain);
        code = http("PATCH", j->url, j->body, NULL, 0);
        if (code == 401 && fb_refresh_token_now()) {
            fb_url(j->url, sizeof(j->url), j->plain);
            code = http("PATCH", j->url, j->body, NULL, 0);
        }
    } else {
        code = http("PATCH", j->url, j->body, NULL, 0);
    }
    return code;
}

#define PATCH_RETRIES 4

static DSMutex patch_seq_lock = DS_MUTEX_INIT;
static int patch_seq = 0;
static void *http_patch_job(void *arg) {
    HttpJob *j = (HttpJob *)arg;
    if (!j)
        return NULL;
    int code = 0, attempt = 0, wait = 1000;
    for (;;) {
        code = http_patch_once(j);
        if (code == 200)
            break;
        int transient = (code == 0 || code == 429 || code >= 500);
        if (!transient || ++attempt >= PATCH_RETRIES)
            break;
        sleep_ms(wait);
        if (wait < 4000)
            wait *= 2;
    }
    if (code != 200) {
        if (j->progress)
            cloud_dirty_set(1);
        if (net_log_ok())
            LOGERR("cloud PATCH failed: HTTP %d%s", code,
                   j->progress ? " (progress will be resent on next login)" : "");
    } else {

        if (j->progress) {
            int latest;
            ds_mutex_lock(patch_seq_lock);
            latest = (j->progress == patch_seq);
            ds_mutex_unlock(patch_seq_lock);
            if (latest)
                cloud_dirty_set(0);
        }
        if (attempt > 0)
            LOG("cloud PATCH ok after %d retry(ies)", attempt);
    }
    free(j);
    return NULL;
}
static void http_patch_async_ex(const char *url, const char *body, int progress) {
    HttpJob *j = (HttpJob *)malloc(sizeof(*j));
    DSThread t;
    if (!j)
        return;
    snprintf(j->plain, sizeof(j->plain), "%s", url);
    snprintf(j->url, sizeof(j->url), "%s", url);
    snprintf(j->body, sizeof(j->body), "%s", body);
    j->progress = 0;
    if (progress) {
        ds_mutex_lock(patch_seq_lock);
        j->progress = ++patch_seq;
        ds_mutex_unlock(patch_seq_lock);
    }
    t = ds_thread_start(http_patch_job, j);
    if (t) {
        ds_thread_detach(t);
        return;
    }
    if (progress)
        cloud_dirty_set(1);
    free(j);
}
static void http_patch_async(const char *url, const char *body) {
    http_patch_async_ex(url, body, 0);
}

void net_save_progress(double cups, double candies, double cls, double azum, double santa, double ebuc, double level,
                       double levels_unlocked) {
    int k = (int)cls;
    int ol, ou, al, au, sl, su, el, eu, bp, sk;
    progress_read();
    pg_lock();
    ol = pg.ordinary_level;
    ou = pg.ordinary_levels_unlocked;
    al = pg.azum_level;
    au = pg.azum_levels_unlocked;
    sl = pg.santa_level;
    su = pg.santa_levels_unlocked;
    el = pg.ebuc_level;
    eu = pg.ebuc_levels_unlocked;
    bp = pg.bp_level;
    sk = pg.azum_skin;
    pg_unlock();
    if (k == 0) {
        ol = (int)level;
        ou = (int)levels_unlocked;
    } else if (k == 1) {
        al = (int)level;
        au = (int)levels_unlocked;
    } else if (k == 2) {
        sl = (int)level;
        su = (int)levels_unlocked;
    } else if (k == 3) {
        el = (int)level;
        eu = (int)levels_unlocked;
    }
    net_save_progress_all(cups, candies, cls, azum, santa, ebuc, level, levels_unlocked, ol, ou, al, au, sl, su, el, eu,
                          bp, sk);
}

static DSMutex ach_lock = DS_MUTEX_INIT;
static int ach_loaded = 0;
static unsigned int ach_flags = 0;
static unsigned int ach_revives = 0;

void net_save_progress_all(double cups, double candies, double cls, double azum, double santa, double ebuc,
                           double level, double levels_unlocked, double ordinary_level, double ordinary_levels_unlocked,
                           double azum_level, double azum_levels_unlocked, double santa_level,
                           double santa_levels_unlocked, double ebuc_level, double ebuc_levels_unlocked,
                           double bp_level, double azum_skin) {
    int c = (int)cups, cd = (int)candies, k = (int)cls, a = azum ? 1 : 0, sn = santa ? 1 : 0, eu = ebuc ? 1 : 0;
    int lv = (int)level, lu = (int)levels_unlocked;
    int ol = (int)ordinary_level, ou = (int)ordinary_levels_unlocked;
    int al = (int)azum_level, au = (int)azum_levels_unlocked;
    int sl = (int)santa_level, su = (int)santa_levels_unlocked;
    int el = (int)ebuc_level, eul = (int)ebuc_levels_unlocked;
    int bp = (int)bp_level, sk = azum_skin ? 1 : 0;
    if (c < 0)
        c = 0;
    if (cd < 0)
        cd = 0;
    if (k != 1 && k != 2 && k != 3)
        k = 0;

    progress_read();
    pg_lock();
    if (pg.loaded) {
        if (pg.azum == 1)
            a = 1;
        if (pg.santa == 1)
            sn = 1;
        if (pg.ebuc == 1)
            eu = 1;

        if (pg.ordinary_levels_unlocked > ou)
            ou = pg.ordinary_levels_unlocked;
        if (pg.azum_levels_unlocked > au)
            au = pg.azum_levels_unlocked;
        if (pg.santa_levels_unlocked > su)
            su = pg.santa_levels_unlocked;
        if (pg.ebuc_levels_unlocked > eul)
            eul = pg.ebuc_levels_unlocked;
        if (pg.ordinary_level > ol)
            ol = pg.ordinary_level;
        if (pg.azum_level > al)
            al = pg.azum_level;
        if (pg.santa_level > sl)
            sl = pg.santa_level;
        if (pg.ebuc_level > el)
            el = pg.ebuc_level;
    }
    pg_unlock();
    if (k == 1 && !a)
        k = 0;
    if (k == 2 && !sn)
        k = 0;
    if (k == 3 && !eu)
        k = 0;
    if (bp < 0)
        bp = 0;
    if (bp > BP_MAX)
        bp = BP_MAX;
    normalize_level_pair(&lv, &lu);
    normalize_level_pair(&ol, &ou);
    normalize_level_pair(&al, &au);
    normalize_level_pair(&sl, &su);
    normalize_level_pair(&el, &eul);
    if (k == 0) {
        lv = ol;
        lu = ou;
    } else if (k == 1) {
        lv = al;
        lu = au;
    } else if (k == 2) {
        lv = sl;
        lu = su;
    } else {
        lv = el;
        lu = eul;
    }
    pg_lock();
    pg.cups = c;
    pg.candies = cd;
    pg.cls = k;
    pg.azum = a;
    pg.santa = sn;
    pg.ebuc = eu;
    pg.level = lv;
    pg.levels_unlocked = lu;
    pg.loaded = 1;
    pg.ordinary_level = ol;
    pg.ordinary_levels_unlocked = ou;
    pg.azum_level = al;
    pg.azum_levels_unlocked = au;
    pg.santa_level = sl;
    pg.santa_levels_unlocked = su;
    pg.ebuc_level = el;
    pg.ebuc_levels_unlocked = eul;
    pg.bp_level = bp;
    pg.azum_skin = sk;
    pg_unlock();
    pending_cls = k;
    pending_level = lv;
    pending_skin = sk;
    progress_write(c, cd, k, a, sn, eu, lv, lu, ol, ou, al, au, sl, su, el, eul, bp, sk);

    char nick[LOGIN_NICK_MAX + 1] = "";
    lg_lock();
    if (lg.status == NET_LOGIN_OK && lg.session_nick[0]) {
        snprintf(nick, sizeof(nick), "%s", lg.session_nick);
    }
    lg_unlock();
    if (nick[0] && net.base[0]) {
        char url[URL], body[BODY * 3];

        unsigned int af = 0, ar = 0;
        ds_mutex_lock(ach_lock);
        af = ach_flags;
        ar = ach_revives;
        ds_mutex_unlock(ach_lock);
        snprintf(url, sizeof(url), "%s/users/%s.json", net.base, nick);
        snprintf(body, sizeof(body),
                 "{\"cups\":%d,\"candies\":%d,\"cls\":%d,\"azum\":%d,\"santa\":%d,\"ebuc\":%d,\"level\":%d,\"levels\":%"
                 "d,\"ordinary_level\":%d,\"ordinary_levels\":%d,\"azum_level\":%d,\"azum_levels\":%d,\"santa_level\":%"
                 "d,\"santa_levels\":%d,\"ebuc_level\":%d,\"ebuc_levels\":%d,\"bp\":%d,\"azum_skin\":%d,\"ach_flags\":%"
                 "u,\"ach_revives\":%u}",
                 c, cd, k, a, sn, eu, lv, lu, ol, ou, al, au, sl, su, el, eul, bp, sk, af, ar);

        cloud_dirty_set(1);
        http_patch_async_ex(url, body, 1);
    } else {

        cloud_dirty_set(1);
    }
}
