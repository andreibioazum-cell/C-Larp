/* Сеть на вебе: полная заглушка (offline-режим, без Java VM, без JNI).
 * Каждая функция на отдельной строке (не в макрос-столбик), чтобы парсер C
 * не спотыкался при раскрытии EM_JS include guards'ов. */
#ifndef CB_NETWORK_WEB_INC
#define CB_NETWORK_WEB_INC
#include <stddef.h>

static void net_vnop(void){}
void net_set_java_vm(void *vm){(void)vm;}
void net_set_data_path(const char *p){(void)p;}
void net_set_system_language(const char *c){(void)c;}
void net_set_firebase_key(const char *k){(void)k;}
void net_connect(const char *a,const char *b){(void)a;(void)b;}
void net_disconnect(void){}
void net_autologin(const char *u){(void)u;}
double net_presence_check(const char *d){(void)d;return 0;}
double net_auth(const char *u,const char *n,const char *p){(void)u;(void)n;(void)p;return -1;}
void net_logout(void){}
double net_login_status(void){return 0;}
const char*net_login_nick(void){return "";}
const char*net_login_pass(void){return "";}
void net_publish(double a,double b,double c,double d,double e){(void)a;(void)b;(void)c;(void)d;(void)e;}
void net_publish_punch(double x,double y,double dx,double dy,double p){(void)x;(void)y;(void)dx;(void)dy;(void)p;}
void net_publish_snow(double x,double y,double dx,double dy,double s){(void)x;(void)y;(void)dx;(void)dy;(void)s;}
void net_publish_turrets(double a,double b,double c,double d,double e,double f,double g,double h,double i){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;}
void net_publish_dash(double x,double y,double dx,double dy,double d){(void)x;(void)y;(void)dx;(void)dy;(void)d;}
void net_publish_grab(double x,double y,double dx,double dy,double g){(void)x;(void)y;(void)dx;(void)dy;(void)g;}
void net_publish_universe(double x,double y,double c){(void)x;(void)y;(void)c;}
void net_publish_thud(double c){(void)c;}
void net_publish_block(double o){(void)o;}
void net_publish_codecracker_super(double o){(void)o;}
void net_set_class(double c){(void)c;}
double net_status(void){return 0;}
double net_slot(void){return 0;}
double net_event(void){return 0;}
void net_event_set(double m){(void)m;}
double net_player_online(double s){(void)s;return 0;}
double net_player_x(double s){(void)s;return 0;}
double net_player_y(double s){(void)s;return 0;}
double net_player_angle(double s){(void)s;return 0;}
double net_player_hp(double s){(void)s;return 0;}
double net_player_alive(double s){(void)s;return 0;}
const char*net_player_nick(double s){(void)s;return "";}
double net_player_punch_x(double s){(void)s;return 0;}
double net_player_punch_y(double s){(void)s;return 0;}
double net_player_punch_dx(double s){(void)s;return 0;}
double net_player_punch_dy(double s){(void)s;return 0;}
double net_player_punch(double s){(void)s;return 0;}
double net_player_snow_x(double s){(void)s;return 0;}
double net_player_snow_y(double s){(void)s;return 0;}
double net_player_snow_dx(double s){(void)s;return 0;}
double net_player_snow_dy(double s){(void)s;return 0;}
double net_player_snow(double s){(void)s;return 0;}
double net_player_station_x(double s){(void)s;return 0;}
double net_player_station_y(double s){(void)s;return 0;}
double net_player_station_hp(double s){(void)s;return 0;}
double net_player_station2_x(double s){(void)s;return 0;}
double net_player_station2_y(double s){(void)s;return 0;}
double net_player_station2_hp(double s){(void)s;return 0;}
double net_player_station3_x(double s){(void)s;return 0;}
double net_player_station3_y(double s){(void)s;return 0;}
double net_player_station3_hp(double s){(void)s;return 0;}
double net_player_dash_x(double s){(void)s;return 0;}
double net_player_dash_y(double s){(void)s;return 0;}
double net_player_dash_dx(double s){(void)s;return 0;}
double net_player_dash_dy(double s){(void)s;return 0;}
double net_player_dash(double s){(void)s;return 0;}
double net_player_grab_x(double s){(void)s;return 0;}
double net_player_grab_y(double s){(void)s;return 0;}
double net_player_grab_dx(double s){(void)s;return 0;}
double net_player_grab_dy(double s){(void)s;return 0;}
double net_player_grab(double s){(void)s;return 0;}
double net_universe_x(void){return 0;}
double net_universe_y(void){return 0;}
double net_universe_counter(void){return 0;}
double net_thud_counter(void){return 0;}
double net_block(void){return 0;}
double net_codecracker_super(void){return 0;}
double net_class(double s){(void)s;return 0;}
void net_snow_reset(void){}
double net_chat_count(void){return 0;}
const char*net_chat_line(double i){(void)i;return "";}
double net_chat_system(double i){(void)i;return 0;}
double net_ban_check(const char *m){(void)m;return 0;}
double net_leaderboard_entry_count(void){return 0;}
const char*net_leaderboard_name(double i){(void)i;return "";}
double net_leaderboard_score(double i){(void)i;return 0;}
const char*net_error_message(void){return "";}
void net_room_create(const char *n){(void)n;}
void net_room_join(const char *n){(void)n;}
void net_room_leave(void){}
double net_room_count(void){return 0;}
const char*net_room_name(double i){(void)i;return "";}
double net_room_players(double i){(void)i;return 0;}
const char*net_room_id(double i){(void)i;return "";}
double net_in_room(void){return 0;}
const char*net_my_room(void){return "";}
const char*net_moderation_message(void){return "";}
double net_banned(void){return 0;}
double net_load_cups(void){return 0;}
double net_load_candies(void){return 0;}
double net_load_class(void){return 0;}
void net_save_class(double v){(void)v;}
double net_load_azum_skin(void){return 0;}
void net_save_azum_skin(double v){(void)v;}
double net_load_santa(void){return 0;}
double net_load_santa_level(void){return 0;}
double net_load_santa_levels_unlocked(void){return 0;}
void net_save_santa_level(double l,double u){(void)l;(void)u;}
double net_load_ebuc(void){return 0;}
double net_load_ebuc_level(void){return 0;}
double net_load_ebuc_levels_unlocked(void){return 0;}
void net_save_ebuc_level(double l,double u){(void)l;(void)u;}
double net_load_bp_level(void){return 0;}
double net_load_azum_revives(void){return 0;}
void net_save_azum_revives(double v){(void)v;}
double net_load_astra(void){return 0;}
double net_load_astra_level(void){return 0;}
double net_load_astra_levels_unlocked(void){return 0;}
void net_save_astra(double o,double l,double lu){(void)o;(void)l;(void)lu;}
#include <stdarg.h>
static void net_save_progress_all_dummy(double first,...){(void)first;}
#define net_save_progress_all(...) net_save_progress_all_dummy(0.0,##__VA_ARGS__)
double net_load_achievement_flags(void){return 0;}
void net_mark_achievement_flag(double v){(void)v;}
double net_promo_2027_used(void){return 0;}
void net_promo_2027_mark_used(void){}
void net_leaderboard_fetch(const char *u){(void)u;}
double net_leaderboard_status(void){return 0;}
double net_leaderboard_count(void){return 0;}
const char*net_leaderboard_nick(double i){(void)i;return "";}
double net_leaderboard_cups(double i){(void)i;return 0;}
double net_load_language(void){return 0;}
double net_load_hitboxes(void){return 0;}
void net_save_settings(double a,double b){(void)a;(void)b;}
double net_load_music_volume(void){return 0;}
void net_save_music_volume(double v){(void)v;}
double net_load_winter_theme(void){return 0;}
void net_save_winter_theme(double v){(void)v;}
double net_load_new_theme(void){return 0;}
void net_save_new_theme(double v){(void)v;}
void settings_mark_legal(void){}
double settings_legal_ts(void){return 0;}
void settings_set_pd_consent(double v){(void)v;}
double settings_pd_consent_version(void){return 0;}
void net_pd_consent_upload(void){}
void net_delete_account(const char *p){(void)p;}
double net_delete_account_status(void){return 0;}
void net_change_password(const char *o,const char *n){(void)o;(void)n;}
double net_change_password_status(void){return 0;}
double net_is_banned(const char *n){(void)n;return 0;}
void net_ban_set(const char *n,double b){(void)n;(void)b;}
double net_chat_is_ban(const char *m){(void)m;return 0;}
double net_chat_is_unban(const char *m){(void)m;return 0;}
const char*net_chat_ban_target(const char *m){(void)m;return "";}
const char*net_chat_unban_target(const char *m){(void)m;return "";}
double net_chat_is_text_cmd(const char *m){(void)m;return 0;}
const char*net_chat_text_cmd_text(const char *m){(void)m;return "";}
const char*net_chat_text_cmd_color(const char *m){(void)m;return "";}
void net_banner_send(const char *t,const char *c){(void)t;(void)c;}
void net_banner_clear(void){}
double net_banner_ts(void){return 0;}
const char*net_banner_text(void){return "";}
const char*net_banner_color(void){return "";}
void net_chat_send(const char *t){(void)t;}
void net_chat_trim(double k){(void)k;}
const char*net_chat_text(double i){(void)i;return "";}
const char*net_chat_uid(double i){(void)i;return "";}
const char*net_chat_key(double i){(void)i;return "";}
double net_player_universe_x(double s){(void)s;return 0;}
double net_player_universe_y(double s){(void)s;return 0;}
double net_player_universe(double s){(void)s;return 0;}
double net_player_thud(double s){(void)s;return 0;}
double net_player_block(double s){(void)s;return 0;}
double net_player_codecracker_super(double s){(void)s;return 0;}
double net_player_class(double s){(void)s;return 0;}
double net_player_level(double s){(void)s;return 0;}
void net_set_level(double v){(void)v;}
void net_set_skin(double v){(void)v;}
double net_player_skin(double s){(void)s;return 0;}
double net_load_level(void){return 0;}
double net_load_levels_unlocked(void){return 0;}
double net_load_ordinary_level(void){return 0;}
double net_load_ordinary_levels_unlocked(void){return 0;}
double net_load_azum(void){return 0;}
double net_load_azum_level(void){return 0;}
double net_load_azum_levels_unlocked(void){return 0;}

#endif /* CB_NETWORK_WEB_INC */
