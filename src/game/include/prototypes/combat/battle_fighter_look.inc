static double fighter_look_of(double slot);
static double fighter_pose_of(double slot, double punching);
static void resolve_fighter_look(double slot, double cls, double skin);
static void resolve_fighter_looks(void);
static GAME_MAYBE_UNUSED const char *fighter_sprite(double cls, double punching, double skin);
static double fighter_sprite_visible(double cls, double punching, double skin);
static void draw_fighter_shadow(double x, double y, double ang, double look, double punching);
static void draw_fighter_body(double x, double y, double ang, double look, double punching);
static void draw_fighter_hurt(double x, double y, double ang, double look, double punching, double scale, double a);
