static int state_ready;

void game_init(AAssetManager *assets) {
    ds_set_asset_manager(assets);
    if (!state_ready) {
        state_create();
        state_ready = 1;
    }
    logic_init();
}

void game_reset(void) {
    if (state_ready)
        state_destroy();
    state_create();
    state_ready = 1;
}

void game_update(void) {
    logic_update();
}

void game_draw(Buffer *buffer) {
    (void)buffer;
    logic_draw();
}

int game_back(void) {
    return logic_back() != 0;
}

void game_touch(float x, float y, int action, int pointer_id) {
    mouse_clicked = action == 0;
    if (action == 0) {
        ds_mouse_x = x;
        ds_mouse_y = y;
    }
    logic_touch((double)x, (double)y, (double)action, (double)pointer_id);
}
