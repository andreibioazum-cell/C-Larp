/* Read-only view of the ban list for the admin Players tab: the panel has to
 * show who is already banned so a tap can lift the ban, and it has to list the
 * bans that survived on this device even when the cloud is unreachable.
 * ban_sync() talks HTTP, so the refresh runs on a worker thread - the UI only
 * ever reads the cache. */

#define BAN_LIST_RET_SLOTS 8

static char s_ban_ret[BAN_LIST_RET_SLOTS][LOGIN_NICK_MAX + 1];
static int s_ban_ret_pos = 0;
static volatile sig_atomic_t s_ban_local_loaded = 0;
static volatile sig_atomic_t s_ban_syncing = 0;

/* bans.dat is what keeps bans alive between sessions; fold it into the cache
 * once so old bans show up even with no connection. */
static void ban_list_load_local(void) {
    char path[320], line[64];
    FILE *f;
    if (s_ban_local_loaded) {
        return;
    }
    s_ban_local_loaded = 1;
    bans_file_path(path, sizeof(path));
    f = fopen(path, "r");
    if (!f) {
        return;
    }
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        char *p = line;
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r' || line[len - 1] == ' ' ||
                       line[len - 1] == '\t')) {
            line[--len] = 0;
        }
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p) {
            ban_cache_put(p);
        }
    }
    fclose(f);
}

double net_ban_list_count(void) {
    double v;
    ban_list_load_local();
    ban_lock();
    v = (double)banlist.count;
    ban_unlock();
    return v;
}

const char *net_ban_list_nick(double idx) {
    int i = (int)idx;
    char *slot = s_ban_ret[s_ban_ret_pos];
    s_ban_ret_pos = (s_ban_ret_pos + 1) % BAN_LIST_RET_SLOTS;
    slot[0] = 0;
    ban_list_load_local();
    ban_lock();
    if (i >= 0 && i < banlist.count) {
        snprintf(slot, LOGIN_NICK_MAX + 1, "%s", banlist.nick[i]);
    }
    ban_unlock();
    return slot;
}

double net_ban_list_have(const char *nick) {
    int found = 0;
    if (!nick || !*nick) {
        return 0;
    }
    ban_list_load_local();
    ban_lock();
    for (int i = 0; i < banlist.count; i++) {
        if (strcmp(banlist.nick[i], nick) == 0) {
            found = 1;
            break;
        }
    }
    ban_unlock();
    if (found) {
        return 1;
    }
    return is_banned_in_file(nick) ? 1 : 0;
}

double net_ban_list_busy(void) {
    return s_ban_syncing == 1 ? 1 : 0;
}

static void *ban_list_sync_job(void *arg) {
    (void)arg;
    ban_sync();
    s_ban_syncing = 0;
    return NULL;
}

void net_ban_list_refresh(void) {
    DSThread t;
    if (s_ban_syncing == 1 || !net.base[0]) {
        return;
    }
    s_ban_syncing = 1;
    t = ds_thread_start(ban_list_sync_job, NULL);
    if (t) {
        ds_thread_detach(t);
        return;
    }
    s_ban_syncing = 0;
}
