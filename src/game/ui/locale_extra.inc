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
