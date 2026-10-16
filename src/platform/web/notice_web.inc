#ifndef CB_NOTICE_WEB_INC
#define CB_NOTICE_WEB_INC
/* Activity hooks for web: ds_set_activity stub + start-page alert.
 * (Android-only notice branches are empty when __ANDROID__ is not defined.) */
#include "platform/web/js_bridge.inc"
static int web_start_alert_shown = 0;
void ds_set_activity(void *a) { (void)a; }
void web_show_start_alert_once(void) {
    if (web_start_alert_shown) return;
    web_start_alert_shown = 1;
    js_alert("это та табличка в начале игры ^_^");
}
#endif
