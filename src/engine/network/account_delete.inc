/* Удаление аккаунта и всех данных игрока по его требованию.
 *
 * 152-ФЗ, ст. 9 ч. 2 и ст. 21 ч. 5: субъект вправе отозвать согласие, и тогда
 * оператор прекращает обработку и уничтожает персональные данные. В игре это
 * экран «Аккаунт → Удалить аккаунт»: игрок вводит пароль и проводит
 * ползунок вправо. Удаление идёт в своём потоке:
 *   0. введённый пароль должен совпасть с паролем сессии;
 *   1. свежий вход этим паролем (Firebase удаляет учётку только после
 *      недавнего входа; заодно сервер ещё раз проверяет пароль);
 *   2. профиль users/<ник> удаляется из базы; если на сервере ещё старые
 *      правила без права на самоудаление, профиль обнуляется;
 *   3. учётная запись Firebase Auth удаляется (accounts:delete);
 *   4. на устройстве стираются сессия, прогресс, достижения, промокод и
 *      отметка о согласии на обработку данных.
 * Статус: 0 — ничего не идёт, 1 — идёт, 2 — удалено, 3 — ошибка сети или
 * сервера (ничего не стёрто, можно повторить), 4 — неверный пароль. */

static volatile int s_acct_del_status = 0;
static char s_acct_del_pass[64];

static void acct_wipe_local(void) {
    static const char *const files[] = {SESSION_FILE, PROGRESS_FILE, ACHIEVEMENTS_FILE, PROGRESS_DIRTY_FILE,
                                        PROMO_FILE};
    char path[320];
    size_t i;
    net_logout();
    fb_store_tokens("", "", 0);
    for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
        data_file_path(path, sizeof(path), files[i]);
        remove(path);
    }
    pg_lock();
    pg.loaded = 0;
    pg_unlock();
    ds_mutex_lock(ach_lock);
    ach_loaded = 0;
    ach_flags = 0;
    ach_revives = 0;
    ds_mutex_unlock(ach_lock);
    pr_lock();
    pr.loaded = 0;
    pr.used = 0;
    pr.code[0] = 0;
    pr_unlock();
    settings_set_pd_consent(0);
}

static void *acct_delete_job(void *arg) {
    char nick[LOGIN_NICK_MAX + 2] = "", pass[64] = "", typed[64], url[URL], body[512], resp[1024];
    char email[96], epass[192], idt[FB_ID_MAX];
    int code;
    (void)arg;
    snprintf(typed, sizeof(typed), "%s", s_acct_del_pass);
    memset(s_acct_del_pass, 0, sizeof(s_acct_del_pass));
    lg_lock();
    snprintf(nick, sizeof(nick), "%s", lg.session_nick);
    snprintf(pass, sizeof(pass), "%s", lg.session_pass);
    lg_unlock();
    if (nick[0] && (!pass[0] || strcmp(pass, typed) != 0)) {
        memset(typed, 0, sizeof(typed));
        s_acct_del_status = 4;
        return NULL;
    }
    memset(typed, 0, sizeof(typed));
    if (nick[0] && net.base[0]) {
        if (fb_enabled()) {
            int r;
            fb_email_of(nick, email, sizeof(email));
            snprintf(epass, sizeof(epass), "%s%s", pass, net_pass_suffix());
            r = fb_try_sign_in(email, epass, NULL);
            memset(epass, 0, sizeof(epass));
            if (r != 0) {
                s_acct_del_status = r == 1 ? 4 : 3;
                return NULL;
            }
        }
        snprintf(url, sizeof(url), "%s/users/%s.json", net.base, nick);
        code = fb_http("DELETE", url, NULL, NULL, 0);
        if (code != 200) {
            /* Старые правила базы не дают удалить свой профиль: тогда в нём
             * не остаётся ничего, кроме ника и нулей. */
            if (code == 0 || put_default_user(url, nick, NULL) != 200) {
                if (net_log_ok())
                    LOGERR("account delete: profile HTTP %d", code);
                s_acct_del_status = 3;
                return NULL;
            }
            if (net_log_ok())
                LOG("account delete: profile zeroed (deploy firebase.rules.json for a full delete)");
        }
        if (fb_enabled()) {
            fb_id_token(idt, sizeof(idt));
            if (idt[0]) {
                fb_auth_endpoint(url, sizeof(url), "/v1/accounts:delete");
                snprintf(body, sizeof(body), "{\"idToken\":\"%s\"}", idt);
                code = http("POST", url, body, resp, sizeof(resp));
                if (code != 200 && net_log_ok())
                    LOGERR("account delete: auth HTTP %d %.160s", code, resp);
            }
        }
    }
    acct_wipe_local();
    if (net_log_ok())
        LOG("account and local data deleted");
    s_acct_del_status = 2;
    return NULL;
}

void net_delete_account(const char *password) {
    DSThread t;
    if (s_acct_del_status == 1)
        return;
    snprintf(s_acct_del_pass, sizeof(s_acct_del_pass), "%s", password ? password : "");
    s_acct_del_status = 1;
    t = ds_thread_start(acct_delete_job, NULL);
    if (t) {
        ds_thread_detach(t);
        return;
    }
    s_acct_del_status = 3;
}

/* Прочитать статус; готовый результат (2, 3 или 4) отдаётся один раз. */
double net_delete_account_status(void) {
    int s = s_acct_del_status;
    if (s >= 2)
        s_acct_del_status = 0;
    return (double)s;
}
