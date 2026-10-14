static const char *tr_music_state(void) {
    if (language == 1) {
        if (music_volume <= 0) {
            return "Музыка: выкл";
        }
        if (music_volume >= 100) {
            return "Музыка: вкл";
        }
        if (music_volume == 50) {
            return "Музыка: 50%";
        }
        return ds_concat(ds_concat("Музыка: ", ds_num_to_string((double)(music_volume))), "%");
    }
    if (music_volume <= 0) {
        return "Music: off";
    }
    if (music_volume >= 100) {
        return "Music: on";
    }
    if (music_volume == 50) {
        return "Music: 50%";
    }
    return ds_concat(ds_concat("Music: ", ds_num_to_string((double)(music_volume))), "%");
}

static const char *tr_hitboxes_toggle(void) {
    if (language == 1) {
        if (show_hitboxes == 1) {
            return "Хитбоксы: ВКЛ";
        }
        return "Хитбоксы: ВЫКЛ";
    }
    if (show_hitboxes == 1) {
        return "Hitboxes: ON";
    }
    return "Hitboxes: OFF";
}

static const char *tr_winter_toggle(void) {
    if (language == 1) {
        if (winter_theme == 1) {
            return "Зимняя тема: ВКЛ";
        }
        return "Зимняя тема: ВЫКЛ";
    }
    if (winter_theme == 1) {
        return "Winter theme: ON";
    }
    return "Winter theme: OFF";
}

/* Графика: две кнопки в одном ряду экрана «Играть». */
static const char *tr_gfx_low(void) {
    if (language == 1) {
        return "Низкая";
    }
    return "Low";
}

static const char *tr_gfx_ultra(void) {
    if (language == 1) {
        return "Ультра";
    }
    return "Ultra";
}

static const char *tr_newyear_banner(void) {
    if (language == 1) {
        return "С Новым годом!";
    }
    return "Happy New Year!";
}


static const char *tr_theme_label(void) {
    if (language == 1) {
        return new_theme == 1 ? "Тема: Новая" : "Тема: Старая";
    }
    return new_theme == 1 ? "Theme: New" : "Theme: Old";
}
