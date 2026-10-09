/* Pure part of the background presence check: no I/O and no globals.
 * rooms/main/players holds one object per slot with the account "nick".
 * The per-connection "uid" is random (make_uid), so our own slot is found by
 * nick, compared without case, the same way accounts are keyed in Firebase. */

static int presence_same_nick(const char *slot, size_t len, const char *own) {
    size_t i;
    if (strlen(own) != len)
        return 0;
    for (i = 0; i < len; i++) {
        char a = slot[i];
        char b = own[i];
        if (a >= 'A' && a <= 'Z')
            a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z')
            b = (char)(b - 'A' + 'a');
        if (a != b)
            return 0;
    }
    return 1;
}

/* 2 = our account is in the room, 1 = another account is, 0 = room is empty,
 * -1 = bad input. Our own slot wins even when other players are present, since
 * then the player is already online and needs no call. */
static int presence_classify(const char *json, const char *own_nick) {
    static const char key[] = "\"nick\":\"";
    const char *p = json;
    int other = 0;
    if (!json || !own_nick || !own_nick[0])
        return -1;
    while ((p = strstr(p, key)) != NULL) {
        const char *value = p + sizeof(key) - 1;
        const char *end = strchr(value, '"');
        size_t len;
        if (!end)
            break;
        len = (size_t)(end - value);
        if (len > 0) {
            if (presence_same_nick(value, len, own_nick))
                return 2;
            other = 1;
        }
        p = end + 1;
    }
    return other ? 1 : 0;
}
