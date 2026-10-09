/* Background presence check, called from PresenceJobService without the game
 * loop. It reuses the saved login (auth.dat) and the game's HTTP helpers, but it
 * only signs in (never signs up) and only reads rooms/main/players. */

static int presence_sign_in(const char *nick, const char *pass) {
    char url[FB_URL_MAX], body[512], resp[2048], email[96], epass[192];
    char idt[FB_ID_MAX], rt[FB_REFRESH_MAX];
    int code;
    fb_email_of(nick, email, sizeof(email));
    snprintf(epass, sizeof(epass), "%s%s", pass, FB_PASS_SUFFIX);
    fb_auth_endpoint(url, sizeof(url), "/v1/accounts:signInWithPassword");
    snprintf(body, sizeof(body), "{\"email\":\"%s\",\"password\":\"%s\",\"returnSecureToken\":true}", email, epass);
    code = http("POST", url, body, resp, sizeof(resp));
    if (code != 200)
        return -1;
    idt[0] = 0;
    rt[0] = 0;
    strv(resp, "idToken", idt, sizeof(idt));
    strv(resp, "refreshToken", rt, sizeof(rt));
    if (!idt[0])
        return -1;
    fb_store_tokens(idt, rt, num(resp, "expiresIn", 3600));
    return 0;
}

/* Returns 2 = our account is in the online room, 1 = another player is there,
 * 0 = nobody is there, -1 = no saved login (or no password saved), -2 = network
 * or sign-in failed. Nothing is written to the database or to auth.dat. */
int net_presence_check(const char *data_dir) {
    char path[320], nick[LOGIN_NICK_MAX + 2] = "", pass[64] = "";
    char url[FB_URL_MAX], players[16384];
    FILE *f;
    int n, code, result;
    if (data_dir && *data_dir)
        net_set_data_path(data_dir);
    lg_lock();
    if (lg.path[0])
        snprintf(path, sizeof(path), "%s/%s", lg.path, SESSION_FILE);
    else
        snprintf(path, sizeof(path), "%s", SESSION_FILE);
    lg_unlock();
    f = fopen(path, "r");
    if (!f)
        return -1;
    n = fscanf(f, "%17s %63s", nick, pass);
    fclose(f);
    if (n < 2 || !nick_valid(nick))
        return -1;
    if (!fb_enabled())
        net_set_firebase_key(NET_FIREBASE_KEY);
    if (!net.base[0])
        snprintf(net.base, sizeof(net.base), "%s", NET_FIREBASE_URL);
    if (presence_sign_in(nick, pass) != 0)
        return -2;
    snprintf(url, sizeof(url), "%s/rooms/%s/players.json", net.base, NET_DEFAULT_ROOM);
    code = fb_http("GET", url, NULL, players, sizeof(players));
    if (code != 200)
        return -2;
    result = presence_classify(players, nick);
    return result < 0 ? -2 : result;
}
