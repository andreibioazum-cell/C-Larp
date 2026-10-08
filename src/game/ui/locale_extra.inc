static const char *tr_top(void) {
    if (language == 1) {
        return "Топ";
    }
    return "Top";
}

static const char *tr_skin_astra_rw(void) { return "Astra (Rework)"; }

static const char *tr_admin_ban_protected(void) {
    if (language == 1) {
        return "Администраторов нельзя банить";
    }
    return "Admin accounts cannot be banned";
}

static const char *tr_skins_note_for(double cls) {
    if (cls == CLASS_ASTRA) {
        if (language == 1) {
            return "Rework-вариант: только для разработчика";
        }
        return "Rework variant: developer build only";
    }
    return tr_skins_need_legends();
}
