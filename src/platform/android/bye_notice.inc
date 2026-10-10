#ifdef __ANDROID__
/* Прощание при выходе из игры: текст уходит в Java, оттуда - в шторку
 * уведомлений. Всё остальное (шанс 50%, выбор строки) решает игра. */
void bye_notice_show(const char *text) { (void)kb_call_text_method("showByeNotice", text); }
#else
void bye_notice_show(const char *text) { (void)text; }
#endif
