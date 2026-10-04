static const char *tr_super(void) {
    if (language == 1) {
        return "Снежинка";
    }
    return "Snowflake";
}

static const char *tr_super_ebuc(void) {
    if (language == 1) {
        return "Ульта";
    }
    return "Ult";
}

static const char *tr_super_azum(void) {
    if (language == 1) {
        return "Рывок";
    }
    return "Dash";
}

static const char *tr_class_selected(void) {
    if (language == 1) {
        return "Выбран";
    }
    return "Selected";
}

static const char *tr_class_pick(void) {
    if (language == 1) {
        return "Выбрать";
    }
    return "Select";
}

static const char *tr_levels(void) {
    if (language == 1) {
        return "Уровни";
    }
    return "Levels";
}

static const char *tr_level(void) {
    if (language == 1) {
        return "Уровень";
    }
    return "Level";
}

static const char *tr_level_status_open(void) {
    if (language == 1) {
        return "Открыт";
    }
    return "Opened";
}

static const char *tr_level_status_locked(void) {
    if (language == 1) {
        return "Закрыт";
    }
    return "Locked";
}

static const char *tr_level_unlock_cost(double cost) {
    const char *out = "";
    if (language == 1) {
        out = ds_concat(ds_concat(ds_concat("Открыть · ", ds_num_to_string((double)(cost))), " "), tr_cups_unit());
    } else {
        out = ds_concat(ds_concat(ds_concat("Unlock · ", ds_num_to_string((double)(cost))), " "), tr_cups_unit());
    }
    return out;
}

static const char *tr_class_level_effect(double cls, double n) {
    if (cls == 1 && n == 1) {
        if (language == 1) {
            return "Вторая жизнь: урон -10%";
        }
        return "Second life: damage -10%";
    }
    if (cls == 1 && n == 2) {
        if (language == 1) {
            return "Скорость +3%";
        }
        return "Speed +3%";
    }
    if (cls == 1 && n == 3) {
        if (language == 1) {
            return "Атака лечит +0.1 HP";
        }
        return "Attack heals +0.1 HP";
    }
    if (cls == 2 && n == 1) {
        if (language == 1) {
            return "Тик яда +0.15 урона";
        }
        return "Poison tick +0.15 damage";
    }
    if (cls == 2 && n == 2) {
        if (language == 1) {
            return "Тик яда +0.15 урона (x2)";
        }
        return "Poison tick +0.15 damage (x2)";
    }
    if (cls == 2 && n == 3) {
        if (language == 1) {
            return "Яд стакается, снежинка наносит урон";
        }
        return "Poison stacks, snowflake deals damage";
    }
    if (cls == 3 && n == 1) {
        if (language == 1) {
            return "Турель +2 HP";
        }
        return "Turret +2 HP";
    }
    if (cls == 3 && n == 2) {
        if (language == 1) {
            return "Спавн турели лечит +0.5 HP";
        }
        return "Placing a turret heals +0.5 HP";
    }
    if (cls == 3 && n == 3) {
        if (language == 1) {
            return "Луч турели лечит +0.1 HP/сек";
        }
        return "Turret beam heals +0.1 HP/sec";
    }
    if (cls == 0 && n == 1) {
        if (language == 1) {
            return "Урон атаки +5%";
        }
        return "Attack damage +5%";
    }
    if (cls == 0 && n == 2) {
        if (language == 1) {
            return "Скорость +2%";
        }
        return "Speed +2%";
    }
    if (language == 1) {
        return "Атака лечит +0.05 HP";
    }
    return "Attack heals +0.05 HP";
}

static const char *tr_cups_unit(void) {
    if (language == 1) {
        return "кубков";
    }
    return "cups";
}

static const char *tr_candies_unit(void) {
    if (language == 1) {
        return "конфет";
    }
    return "candies";
}

static const char *tr_level_locked(void) {
    if (language == 1) {
        return "Купите предыдущий уровень";
    }
    return "Buy the previous level";
}

static const char *tr_level_need_cups(void) {
    if (language == 1) {
        return "Не хватает кубков для уровня";
    }
    return "Not enough cups for this level";
}

static const char *tr_cups(void) {
    if (language == 1) {
        return "Кубки";
    }
    return "Cups";
}

static const char *tr_buy_for(double cost, const char *currency) {
    const char *out = "";
    if (language == 1) {
        out = ds_concat(ds_concat(ds_concat("", ds_num_to_string((double)(cost))), " "), currency);
    } else {
        out = ds_concat(ds_concat(ds_concat("", ds_num_to_string((double)(cost))), " "), currency);
    }
    return out;
}

static const char *tr_class_need_cups(void) {
    if (language == 1) {
        return "Не хватает кубков";
    }
    return "Not enough cups";
}

static const char *tr_class_need_candies(void) {
    if (language == 1) {
        return "Не хватает леденцов";
    }
    return "Not enough candies";
}

static const char *tr_skins(void) {
    if (language == 1) {
        return "Скины";
    }
    return "Skins";
}

static const char *tr_skin_normal(void) {
    if (language == 1) {
        return "Обычный";
    }
    return "Normal";
}

static const char *tr_skin_zombie(void) {
    if (language == 1) {
        return "Зомби";
    }
    return "Zombie";
}

static const char *tr_skin_legends_hint(void) {
    if (language == 1) {
        return "Легенды не умирают...";
    }
    return "Legends never die...";
}

static const char *tr_skins_need_legends(void) {
    if (language == 1) {
        return "Скин «Зомби»: достижение «Легенды не умирают...»";
    }
    return "Zombie skin: \"Legends never die...\" achievement";
}

static const char *tr_cups_reward(double amount) {
    const char *out = "";
    if (language == 1) {
        out = ds_concat(ds_concat("+", ds_num_to_string((double)(amount))), " кубков");
    } else {
        out = ds_concat(ds_concat("+", ds_num_to_string((double)(amount))), " Cups");
    }
    return out;
}

static const char *tr_cups_lose(double amount) {
    const char *out = "";
    if (language == 1) {
        out = ds_concat(ds_concat("", ds_num_to_string((double)(amount))), " кубков");
    } else {
        out = ds_concat(ds_concat("", ds_num_to_string((double)(amount))), " Cups");
    }
    return out;
}

static const char *tr_banned(void) {
    if (language == 1) {
        return "Вы забанены";
    }
    return "You are banned";
}

static const char *tr_promo(void) {
    if (language == 1) {
        return "Промокод";
    }
    return "Promo code";
}

static const char *tr_promo_title(void) {
    if (language == 1) {
        return "Промокод";
    }
    return "Promo code";
}

static const char *tr_promo_need_login(void) {
    if (language == 1) {
        return "Войди, чтобы получить свой код";
    }
    return "Log in to get your own code";
}

static const char *tr_promo_enter_ph(void) {
    if (language == 1) {
        return "Введи код с карточки...";
    }
    return "Enter the code from the card...";
}

static const char *tr_promo_submit(void) {
    if (language == 1) {
        return "Активировать";
    }
    return "Redeem";
}

static const char *tr_promo_how1(void) {
    if (language == 1) {
        return "Карточка выпадает в начале соло-боя, шанс 5%";
    }
    return "A card drops at the start of a solo battle, 5% chance";
}

static const char *tr_promo_need_card(void) {
    if (language == 1) {
        return "Код есть только на карточке из соло-боя";
    }
    return "The code is only on a card from a solo battle";
}

static const char *tr_promo_how2(void) {
    if (language == 1) {
        return "Код на ней случайный, награда одна на аккаунт";
    }
    return "Its code is random, one reward per account";
}

static const char *tr_promo_need_code(void) {
    if (language == 1) {
        return "Введи код";
    }
    return "Enter a code";
}

static const char *tr_promo_bad(void) {
    if (language == 1) {
        return "Неверный код";
    }
    return "Invalid code";
}

static const char *tr_promo_used_msg(void) {
    if (language == 1) {
        return "Промокод уже активирован";
    }
    return "Promo code already redeemed";
}

static const char *tr_promo_success(void) {
    if (language == 1) {
        return ds_concat(
            ds_concat(ds_concat(ds_concat("+", ds_num_to_string((double)(promo_reward_cups))), " кубков и +"),
                      ds_num_to_string((double)(promo_reward_candies))),
            " леденцов!");
    }
    return ds_concat(ds_concat(ds_concat(ds_concat("+", ds_num_to_string((double)(promo_reward_cups))), " cups and +"),
                               ds_num_to_string((double)(promo_reward_candies))),
                     " candies!");
}

static const char *tr_card_title(void) {
    if (language == 1) {
        return "КАРТОЧКА";
    }
    return "CARD";
}

static const char *tr_card_sub(void) {
    if (language == 1) {
        return "Запомни код: больше он нигде не покажется";
    }
    return "Remember the code: it is not shown anywhere else";
}

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
