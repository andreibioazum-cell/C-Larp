/* Сеть на вебе: полная заглушка (offline-режим, без Java VM, без JNI).
 * Сгенерирована как минимальный набор no-op, чтобы линковка прошла. */
#include <stddef.h>
#define W0(name) void name(void){}
#define W0v(name) void name(double v){(void)v;}
#define W0s(name) void name(const char *s){(void)s;}
#define W1(name) void name(double a){(void)a;}
#define W2(name) void name(double a,double b){(void)a;(void)b;}
#define W2s(name) void name(const char*a,const char*b){(void)a;(void)b;}
#define W5(name) void name(double a,double b,double c,double d,double e){(void)a;(void)b;(void)c;(void)d;(void)e;}
#define W9(name) void name(double a,double b,double c,double d,double e,double f,double g,double h,double i){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;}
#define D0(name) double name(void){return 0;}
#define D1(name) double name(double a){(void)a;return 0;}
#define D1s(name) double name(const char*a){(void)a;return 0;}
#define S0(name) const char*name(void){return "";}
#define S1(name) const char*name(double a){(void)a;return "";}
#define S1s(name) const char*name(const char*a){(void)a;return "";}

void net_set_java_vm(void *vm){(void)vm;}
void net_set_data_path(const char *p){(void)p;}
void net_set_system_language(const char *c){(void)c;}
void net_set_firebase_key(const char *k){(void)k;}
W2s(net_connect)
W0(net_disconnect)
W0s(net_autologin)
D1s(net_presence_check)
double net_auth(const char *u,const char *n,const char *p){(void)u;(void)n;(void)p;return -1;}
W0(net_logout)
D0(net_login_status)
S0(net_login_nick) S0(net_login_pass)
W5(net_publish)
void net_publish_punch(double x,double y,double dx,double dy,double p){(void)x;(void)y;(void)dx;(void)dy;(void)p;}
void net_publish_snow(double x,double y,double dx,double dy,double s){(void)x;(void)y;(void)dx;(void)dy;(void)s;}
W9(net_publish_turrets)
void net_publish_dash(double x,double y,double dx,double dy,double d){(void)x;(void)y;(void)dx;(void)dy;(void)d;}
void net_publish_grab(double x,double y,double dx,double dy,double g){(void)x;(void)y;(void)dx;(void)dy;(void)g;}
void net_publish_universe(double x,double y,double c){(void)x;(void)y;(void)c;}
W1(net_publish_thud) W1(net_publish_block) W1(net_publish_codecracker_super)
W1(net_set_class)
D0(net_status) D0(net_slot) D0(net_event) W1(net_event_set)
D1(net_player_online) D1(net_player_x) D1(net_player_y) D1(net_player_angle) D1(net_player_hp) D1(net_player_alive)
S1(net_player_nick)
D1(net_player_punch_x) D1(net_player_punch_y) D1(net_player_punch_dx) D1(net_player_punch_dy) D1(net_player_punch)
D1(net_player_snow_x) D1(net_player_snow_y) D1(net_player_snow_dx) D1(net_player_snow_dy) D1(net_player_snow)
D1(net_player_station_x) D1(net_player_station_y) D1(net_player_station_hp)
D1(net_player_station2_x) D1(net_player_station2_y) D1(net_player_station2_hp)
D1(net_player_station3_x) D1(net_player_station3_y) D1(net_player_station3_hp)
D1(net_player_dash_x) D1(net_player_dash_y) D1(net_player_dash_dx) D1(net_player_dash_dy) D1(net_player_dash)
D1(net_player_grab_x) D1(net_player_grab_y) D1(net_player_grab_dx) D1(net_player_grab_dy) D1(net_player_grab)
D0(net_universe_x) D0(net_universe_y) D0(net_universe_counter) D0(net_thud_counter)
D0(net_block) D0(net_codecracker_super) D1(net_class)
W0(net_snow_reset) W0s(net_send_chat)
D0(net_chat_count) S1(net_chat_line) D1(net_chat_system)
D1s(net_ban_check)
D0(net_leaderboard_entry_count) S1(net_leaderboard_name) D1(net_leaderboard_score)
S0(net_error_message)
void net_account_delete(const char *p){(void)p;}
void net_account_password(const char *o,const char *n){(void)o;(void)n;}
W0s(net_room_create) W0s(net_room_join) W0(net_room_leave)
D0(net_room_count) S1(net_room_name) D1(net_room_players) S1(net_room_id)
D0(net_in_room) S0(net_my_room) S0(net_moderation_message) D0(net_banned)

/* Прогресс и настройки */
D0(net_load_cups) D0(net_load_candies) D0(net_load_class) W1(net_save_class)
D0(net_load_azum_skin) W1(net_save_azum_skin)
D0(net_load_santa_level) D0(net_load_santa_levels_unlocked) W2(net_save_santa_level)
D0(net_load_ebuc_level) D0(net_load_ebuc_levels_unlocked) W2(net_save_ebuc_level)
D0(net_load_bp_level) D0(net_load_azum_revives) W1(net_save_azum_revives)
D0(net_load_astra) D0(net_load_astra_level) D0(net_load_astra_levels_unlocked)
void net_save_astra(double o,double l,double lu){(void)o;(void)l;(void)lu;}
void net_save_progress_all(double a,double b,double c,double d,double e,double f,double g,double h)
    {(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;}
D0(net_load_achievement_flags) W1(net_mark_achievement_flag)
D0(net_promo_2027_used) W0(net_promo_2027_mark_used)
W0s(net_leaderboard_fetch) D0(net_leaderboard_status) D0(net_leaderboard_count)
S1(net_leaderboard_nick) D1(net_leaderboard_cups)
D0(net_load_language) D0(net_load_hitboxes) W2(net_save_settings)
D0(net_load_music_volume) W1(net_save_music_volume)
D0(net_load_winter_theme) W1(net_save_winter_theme)
D0(net_load_new_theme) W1(net_save_new_theme)
W0(settings_mark_legal) D0(settings_legal_ts)
W1(settings_set_pd_consent) D0(settings_pd_consent_version) W0(net_pd_consent_upload)
W0s(net_delete_account) D0(net_delete_account_status)
void net_change_password(const char*o,const char*n){(void)o;(void)n;}
D0(net_change_password_status)
D1s(net_is_banned) void net_ban_set(const char*n,double b){(void)n;(void)b;}
D1s(net_chat_is_ban) D1s(net_chat_is_unban) S1s(net_chat_ban_target) S1s(net_chat_unban_target)
D1s(net_chat_is_text_cmd) S1s(net_chat_text_cmd_text) S1s(net_chat_text_cmd_color)
void net_banner_send(const char*t,const char*c){(void)t;(void)c;}
W0(net_banner_clear) D0(net_banner_ts) S0(net_banner_text) S0(net_banner_color)
W0s(net_chat_send) W1(net_chat_trim) D1(net_chat_count) S1(net_chat_text) S1(net_chat_uid) S1(net_chat_key)

/* Дополнительные no-op'ы для компоновки */
D1(net_player_universe_x) D1(net_player_universe_y) D1(net_player_universe)
D1(net_player_thud) D1(net_player_block) D1(net_player_codecracker_super)
D1(net_player_class) D1(net_player_level) W1(net_set_level) W1(net_set_skin) D1(net_player_skin)
D0(net_load_azum) D0(net_load_santa) D0(net_load_ebuc)
D0(net_load_level) D0(net_load_levels_unlocked)
D0(net_load_ordinary_level) D0(net_load_ordinary_levels_unlocked)
void net_save_santa_level(double level, double unlocked){(void)level;(void)unlocked;}
void net_save_ebuc_level(double level, double unlocked){(void)level;(void)unlocked;}
D0(net_load_azum_level) D0(net_load_azum_levels_unlocked)
D0(net_load_azum_skin) D1(net_save_azum_skin)
void net_save_class(double cls){(void)cls;}
D0(net_load_class)
