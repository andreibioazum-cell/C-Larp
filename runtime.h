#ifndef CUBIC_BATTLE_RUNTIME_H
#define CUBIC_BATTLE_RUNTIME_H

#include <stddef.h>
#include <stdint.h>
#include <android/asset_manager.h>
#include <android/native_window.h>

typedef struct Buffer {
    uint32_t *pixels; /* Unused: drawing goes through Vulkan. */
    int width;
    int height;
    int stride;
} Buffer;

extern int screen_w;
extern int screen_h;
extern double dt;

/* Logging and the small error buffer shared by the native loop and renderer. */
void ds_log(const char *format, ...);
void ds_log_err(const char *format, ...);
void ds_runtime_error(const char *format, ...);
const char *ds_runtime_error_message(void);
int ds_runtime_has_error(void);
void ds_clear_runtime_error(void);
int console_count(void);
const char *console_line(int index);
int console_type(int index);
void console_clear(void);

/* Immediate-mode drawing API used by game/game.c. Colors are 0xAARRGGBB. */
void rect(float x, float y, float w, float h, uint32_t color);
void roundrect(float x, float y, float w, float h, float radius, uint32_t color);
void rect_rot(float x, float y, float w, float h, float angle, uint32_t color);
void circle(float x, float y, float radius, uint32_t color);
void ring(float x, float y, float radius, float thickness, uint32_t color);
void line(float x1, float y1, float x2, float y2, float thickness, uint32_t color);
void clear_screen(uint32_t color);
void ds_set_asset_manager(AAssetManager *assets);
void ds_release_assets(void);
int png_load(const char *name);
void tex(float x, float y, const char *name, float angle, float scale);
void tex_tint(float x, float y, const char *name, float angle, float scale, uint32_t color);
void text(const char *string, float x, float y, uint32_t color);
void text_scaled(const char *string, float x, float y, uint32_t color, float scale);
int text_width(const char *string);
int text_height(const char *string);
int text_ink_width(const char *string);
int text_ink_height(const char *string);
int text_ink_top(const char *string);

#define DS_FONT_ASSET "fonts/ComicRelief-Regular.ttf"
#define DS_FONT_PIXEL_HEIGHT 32

/* Vulkan renderer lifecycle. */
void ds_graphics_report_frame_interval(double seconds);
int ds_graphics_pixel_scale(void);
int ds_graphics_init(AAssetManager *assets, ANativeWindow *window);
int ds_graphics_begin_frame(Buffer *buffer);
void ds_graphics_end_frame(void);
void ds_graphics_cancel_frame(void);
void ds_graphics_window_lost(void);
void ds_graphics_shutdown(void);
void ds_graphics_error_screen(const char *message);
const char *ds_graphics_failure(void);
int ds_graphics_show_failure(ANativeWindow *window, int attempts);

/* Pure-C game hooks. */
void game_init(AAssetManager *assets);
void game_reset(void);
void game_update(void);
void game_draw(void);
void game_touch(float x, float y, int action, int pointer_id);
int game_back(void);

#endif
