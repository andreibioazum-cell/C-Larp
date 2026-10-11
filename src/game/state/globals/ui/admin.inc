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
double admin_tab;
double admin_page;
const char *admin_input_nick;
double admin_input_active;
/* Cheat stain: once any switch is flipped on, the current battle pays no
 * Cups at the end - even if the switch goes straight back off. */
double cheat_stain;
double cheat_god;
double cheat_damage;
double cheat_speed;
double cheat_nocd;
double cheat_cc;
double cheat_estun;
