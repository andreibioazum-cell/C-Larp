#ifndef CB_NOTICE_WEB_INC
#define CB_NOTICE_WEB_INC
/* Web notice hooks: ds_set_activity + alpha-уведомление в начале игры,
 * повторяющее текст Android-версии (AlphaNotice.java). */
#include "platform/web/js_bridge.inc"

static int web_start_alert_shown = 0;
void ds_set_activity(void *a) { (void)a; }

/* Реальный текст альфа-уведомления — такой же, как на Android.
 * Название: «Cubic Battle 4 — альфа» (с той самой рожицей ^_^),
 * тело — альфа-предупреждение. */
#define CB_ALPHA_TITLE "Cubic Battle 4 — альфа ^_^"
#define CB_ALPHA_TEXT  "Игра находится в альфа-версии.\n\n" \
                       "Здесь есть баги, часть контента ещё не готова, а баланс " \
                       "и прогресс могут меняться между обновлениями. Спасибо, " \
                       "что играете и помогаете её делать лучше!"

void web_show_start_alert_once(void) {
    if (web_start_alert_shown) return;
    web_start_alert_shown = 1;
    js_alert_title(CB_ALPHA_TITLE, CB_ALPHA_TEXT);
}

/* alpha_notice_show вызывается движком после логотипа — на вебе мы уже показали
 * алерт при старте, ничего не делаем (bye_notice_show — в network_web.inc). */
void alpha_notice_show(double russian) { (void)russian; }

#endif
