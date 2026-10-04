static const char *tr_card_used1(void) {
    if (language == 1) {
        return "Награда за промокод уже получена";
    }
    return "The promo reward was already claimed";
}

static const char *tr_card_used2(void) {
    if (language == 1) {
        return "на этом аккаунте";
    }
    return "on this account";
}

static const char *tr_card_hint1(void) {
    if (language == 1) {
        return "Открой экран «Промокод» в меню";
    }
    return "Open the 'Promo code' screen in the menu";
}

static const char *tr_card_hint2(void) {
    if (language == 1) {
        return "и введи код, чтобы получить награду";
    }
    return "and enter the code to claim your reward";
}
