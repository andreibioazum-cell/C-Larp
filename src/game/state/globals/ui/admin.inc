/* Admin orb + panel state. Session-local: nothing here is saved to storage,
 * every reset starts with the orb in its default corner and cheats off. */
double admin_ball_x;
double admin_ball_y;
double admin_ball_grab;
double admin_ball_px;
double admin_ball_py;
double admin_ball_grab_dx;
double admin_ball_grab_dy;
double admin_ball_t;
double admin_panel_open;
/* Cheat stain: once any switch is flipped on, the current battle pays no
 * Cups at the end - even if the switch goes straight back off. */
double cheat_stain;
double cheat_god;
double cheat_damage;
double cheat_speed;
double cheat_nocd;
double cheat_cc;
double cheat_estun;
/* Метка: стан врагу поставила панель админки, а не игра. Такой стан не
 * рисует оранжевый кружок - ни пока чит включён, ни после выключения,
 * пока доставшийся от него стан ещё тикает. Снимается настоящим станом
 * (stun_enemy) и началом нового боя. */
double cheat_estun_mark;
