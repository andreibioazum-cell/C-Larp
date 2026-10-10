static const char *tr_top(void) {
    if (language == 1) {
        return "Топ";
    }
    return "Top";
}

static const char *tr_class_astra_rework(void) {
    if (language == 1) {
        return "Астра (Rework)";
    }
    return "Astra (Rework)";
}

static const char *tr_admin_ban_protected(void) {
    if (language == 1) {
        return "Администраторов нельзя банить";
    }
    return "Admin accounts cannot be banned";
}

static const char *tr_skins_note_for(double cls) {
    (void)cls;
    return tr_skins_need_legends();
}

/* Три прощания при выходе из игры. Третье собирается из двух частей, между
 * которыми подставляется ник игрока: "Пока, Дима, удачи!". */
static const char *tr_bye_sorry(void) {
    if (language == 1) {
        return "Нам жаль что вы вышли, удачи!";
    }
    return "Sorry that you left, good luck!";
}

static const char *tr_bye_cubes(void) {
    if (language == 1) {
        return "Пока, кубы будут вас ждать!";
    }
    return "Bye, the cubes will be waiting for you!";
}

static const char *tr_bye_nick_pre(void) {
    if (language == 1) {
        return "Пока, ";
    }
    return "Bye, ";
}

static const char *tr_bye_nick_post(void) {
    if (language == 1) {
        return ", удачи!";
    }
    return ", good luck!";
}
