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

/* Admin panel tabs and the Players actions. */
/* The panel calls itself a content-creator tool, and the orb wears the
 * short version of the same name. */
static const char *tr_admin_title(void) {
    return "CONTENT CREATOR PANEL";
}

static const char *tr_admin_cheats_tab(void) {
    if (language == 1) {
        return "ЧИТЫ";
    }
    return "CHEATS";
}

static const char *tr_admin_players_tab(void) {
    if (language == 1) {
        return "ИГРОКИ";
    }
    return "PLAYERS";
}

static const char *tr_admin_ban(void) {
    if (language == 1) {
        return "БАН";
    }
    return "BAN";
}

static const char *tr_admin_unban(void) {
    if (language == 1) {
        return "РАЗБАН";
    }
    return "UNBAN";
}

static const char *tr_admin_bans(void) {
    if (language == 1) {
        return "БАН-ЛИСТ";
    }
    return "BAN LIST";
}

static const char *tr_admin_no_players(void) {
    if (language == 1) {
        return "Список пуст";
    }
    return "Nothing here";
}

static const char *tr_admin_nick_hint(void) {
    if (language == 1) {
        return "Ник игрока";
    }
    return "Player nick";
}

static const char *tr_admin_do_ban(void) {
    if (language == 1) {
        return "ЗАБАНИТЬ";
    }
    return "BAN PLAYER";
}

static const char *tr_admin_do_unban(void) {
    if (language == 1) {
        return "РАЗБАНИТЬ";
    }
    return "UNBAN PLAYER";
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
