/* Temporary, online-only operator flag. Never persisted to the profile or disk.
 * Poll independently of rooms: the shop and bot fights need it too. */
#define AZUM_BUFF_POLL_MS 3000
#define AZUM_BUFF_TTL_MS 6000
static struct {
    DSMutex lock;
    int busy, enabled;
    unsigned epoch;
    long long next_poll, valid_until;
} azum_buff = {.lock = DS_MUTEX_INIT};

static int azum_buff_value(const char *json) {
    char *end;
    double value;
    if (!json)
        return 0;
    value = strtod(json, &end);
    if (end == json)
        return 0;
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
        end++;
    return value == 1 && !*end;
}

static void *azum_buff_fetch(void *arg) {
    unsigned epoch = (unsigned)(uintptr_t)arg;
    char resp[64] = "";
    int code = fb_http("GET", NET_FIREBASE_URL "/AzumBuff.json", NULL, resp, sizeof(resp));
    ds_mutex_lock(azum_buff.lock);
    if (epoch == azum_buff.epoch) {
        azum_buff.enabled = code == 200 && azum_buff_value(resp);
        azum_buff.valid_until = now_ms() + AZUM_BUFF_TTL_MS;
    }
    azum_buff.busy = 0;
    ds_mutex_unlock(azum_buff.lock);
    return NULL;
}

double net_azum_buff(void) {
    long long now = now_ms();
    double active;
    int logged_in = net_login_status() == NET_LOGIN_OK;
    ds_mutex_lock(azum_buff.lock);
    if (!logged_in) {
        azum_buff.enabled = 0;
        azum_buff.valid_until = 0;
        azum_buff.next_poll = 0;
        azum_buff.epoch++;
    } else if (!azum_buff.busy && now >= azum_buff.next_poll) {
        DSThread t;
        azum_buff.busy = 1;
        azum_buff.next_poll = now + AZUM_BUFF_POLL_MS;
        t = ds_thread_start(azum_buff_fetch, (void *)(uintptr_t)azum_buff.epoch);
        if (t)
            ds_thread_detach(t);
        else {
            azum_buff.busy = 0;
            azum_buff.enabled = 0;
        }
    }
    active = logged_in && azum_buff.enabled && now < azum_buff.valid_until;
    ds_mutex_unlock(azum_buff.lock);
    return active;
}
