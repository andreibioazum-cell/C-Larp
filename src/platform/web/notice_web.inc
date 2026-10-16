/* Activity-хуки на вебе: ds_set_activity — заглушка; alpha/bye notices используют
 * встроенные не-Android ветки из android/*.inc (они уже пустые при !__ANDROID__).
 * Здесь только ds_set_activity и начальный alert(...) при старте страницы. */
#ifndef CB_NOTICE_WEB_INC
#define CB_NOTICE_WEB_INC
#include "platform/web/js_bridge.inc"
static int web_start_alert_shown = 0;
void ds_set_activity(void *a) { (void)a; }
void web_show_start_alert_once(void) {
    if (web_start_alert_shown) return;
    web_start_alert_shown = 1;
    js_alert("это та табличка в начале игры ^_^");
}
#endif
