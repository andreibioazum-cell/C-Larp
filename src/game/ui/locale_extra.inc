static const char *tr_top(void) {
    if (language == 1) {
        return "Топ";
    }
    return "Top";
}

static const char *tr_quests(void) {
    if (language == 1) {
        return "Задания";
    }
    return "Quests";
}

static const char *tr_quest_win_solo(void) {
    if (language == 1) {
        return "Выиграй соло";
    }
    return "Win solo";
}

static const char *tr_quest_win_online(void) {
    if (language == 1) {
        return "Выиграй онлайн";
    }
    return "Win online";
}

static const char *tr_quest_collect_candy(void) {
    if (language == 1) {
        return "Собери леденцы";
    }
    return "Collect candies";
}

static const char *tr_quest_play_matches(void) {
    if (language == 1) {
        return "Сыграй боев";
    }
    return "Play matches";
}

static const char *tr_quest_reward(double c, double cd) {
    const char *out = "";
    if (language == 1) {
        out = ds_concat(ds_concat(ds_concat(ds_concat("+", ds_num_to_string((double)(c))), " кубков, +"),
                                  ds_num_to_string((double)(cd))),
                        " леденцов");
    } else {
        out = ds_concat(ds_concat(ds_concat(ds_concat("+", ds_num_to_string((double)(c))), " cups, +"),
                                  ds_num_to_string((double)(cd))),
                        " candies");
    }
    return out;
}

static const char *tr_quest_claimed(void) {
    if (language == 1) {
        return "Получено";
    }
    return "Claimed";
}
