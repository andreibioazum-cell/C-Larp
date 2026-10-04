#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <android/input.h>
#include <android/keycodes.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <stdint.h>
#include <time.h>

#include "runtime.h"

#define GRAPHICS_START_TRIES 4

static int renderer_ready;
static int game_initialized;
static int physical_width;
static int physical_height;
static int graphics_start_tries;
static int graphics_failure_shown;
static int back_consumed;
static uint64_t graphics_retry_at;
static uint64_t previous_frame_at;
static uint64_t previous_loop_at;

static uint64_t monotonic_ns(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0;
    return (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_nsec;
}

static void apply_screen_size(void) {
    if (physical_width > 0 && physical_height > 0) {
        screen_w = physical_width;
        screen_h = physical_height;
    }
}

static void graphics_start_failed(struct android_app *app, const char *reason) {
    renderer_ready = 0;
    ++graphics_start_tries;
    if (graphics_start_tries < GRAPHICS_START_TRIES) {
        uint64_t delay = 500000000ull << (graphics_start_tries - 1);
        graphics_retry_at = monotonic_ns() + delay;
        ds_log_err("renderer start %d/%d failed (%s), retrying",
                   graphics_start_tries, GRAPHICS_START_TRIES,
                   reason ? reason : "unknown");
        return;
    }
    graphics_retry_at = 0;
    ds_log_err("renderer could not start: %s", reason ? reason : "unknown");
    graphics_failure_shown = ds_graphics_show_failure(app->window, graphics_start_tries);
}

static void start_window(struct android_app *app) {
    AAssetManager *assets;

    if (!app || !app->window) {
        renderer_ready = 0;
        return;
    }
    physical_width = ANativeWindow_getWidth(app->window);
    physical_height = ANativeWindow_getHeight(app->window);
    if (physical_width <= 0 || physical_height <= 0) {
        graphics_start_failed(app, "window has no size");
        return;
    }
    apply_screen_size();
    assets = app->activity ? app->activity->assetManager : NULL;
    if (!ds_graphics_init(assets, app->window)) {
        graphics_start_failed(app, ds_graphics_failure());
        return;
    }

    graphics_start_tries = 0;
    graphics_retry_at = 0;
    graphics_failure_shown = 0;
    renderer_ready = 1;
    previous_frame_at = 0;
    previous_loop_at = 0;

    if (!game_initialized) {
        ds_clear_runtime_error();
        game_init(assets);
        game_initialized = 1;
    }
    ds_log("window ready: %dx%d", physical_width, physical_height);
}

static void handle_command(struct android_app *app, int32_t command) {
    switch (command) {
        case APP_CMD_INIT_WINDOW:
            graphics_start_tries = 0;
            graphics_retry_at = 0;
            graphics_failure_shown = 0;
            start_window(app);
            break;
        case APP_CMD_WINDOW_RESIZED:
        case APP_CMD_CONTENT_RECT_CHANGED:
        case APP_CMD_CONFIG_CHANGED:
            if (app && app->window) {
                int width = ANativeWindow_getWidth(app->window);
                int height = ANativeWindow_getHeight(app->window);
                if (width > 0 && height > 0) {
                    physical_width = width;
                    physical_height = height;
                    apply_screen_size();
                }
            }
            break;
        case APP_CMD_TERM_WINDOW:
            renderer_ready = 0;
            graphics_retry_at = 0;
            graphics_failure_shown = 0;
            ds_graphics_window_lost();
            break;
        case APP_CMD_WINDOW_REDRAW_NEEDED:
            if (graphics_failure_shown && app && app->window) {
                (void)ds_graphics_show_failure(app->window, graphics_start_tries);
            }
            break;
        default:
            break;
    }
}

static int32_t handle_input(struct android_app *app, AInputEvent *event) {
    int32_t type;
    (void)app;
    if (!event || !renderer_ready || !game_initialized) return 0;

    type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        size_t count = AMotionEvent_getPointerCount(event);
        int raw;
        int action;
        size_t index;
        size_t first;
        size_t end;
        size_t i;

        if (count == 0) return 0;
        raw = AMotionEvent_getAction(event);
        action = raw & AMOTION_EVENT_ACTION_MASK;
        if (action == AMOTION_EVENT_ACTION_POINTER_DOWN) action = AMOTION_EVENT_ACTION_DOWN;
        else if (action == AMOTION_EVENT_ACTION_POINTER_UP) action = AMOTION_EVENT_ACTION_UP;
        index = (size_t)((raw & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                         AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
        if (index >= count) index = 0;
        first = action == AMOTION_EVENT_ACTION_MOVE ? 0 : index;
        end = action == AMOTION_EVENT_ACTION_MOVE ? count : index + 1;

        for (i = first; i < end; ++i) {
            float x = AMotionEvent_getX(event, i);
            float y = AMotionEvent_getY(event, i);
            if (screen_w > 0) {
                if (x < 0.0f) x = 0.0f;
                if (x > (float)(screen_w - 1)) x = (float)(screen_w - 1);
            }
            if (screen_h > 0) {
                if (y < 0.0f) y = 0.0f;
                if (y > (float)(screen_h - 1)) y = (float)(screen_h - 1);
            }
            game_touch(x, y, action, AMotionEvent_getPointerId(event, i));
        }
        return 1;
    }

    if (type == AINPUT_EVENT_TYPE_KEY && AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
        int32_t action = AKeyEvent_getAction(event);
        if (action == AKEY_EVENT_ACTION_DOWN) {
            back_consumed = game_back() ? 1 : 0;
            return back_consumed;
        }
        if (action == AKEY_EVENT_ACTION_UP) {
            int consumed = back_consumed;
            back_consumed = 0;
            return consumed;
        }
    }
    return 0;
}

void android_main(struct android_app *app) {
    Buffer frame = {0};

    if (!app) return;
    app_dummy();
    renderer_ready = 0;
    game_initialized = 0;
    app->onAppCmd = handle_command;
    app->onInputEvent = handle_input;
    ds_log("Cubic Battle 4: pure C game with Vulkan renderer");

    for (;;) {
        struct android_poll_source *source = NULL;
        int ident;
        int timeout = app->window && renderer_ready ? 0 : (graphics_retry_at ? 50 : 250);

        while ((ident = ALooper_pollOnce(timeout, NULL, NULL, (void **)&source)) >= 0) {
            if (source && source->process) source->process(app, source);
            if (app->destroyRequested) {
                renderer_ready = 0;
                ds_graphics_shutdown();
                return;
            }
            timeout = 0;
        }

        if (app->window && !renderer_ready && graphics_retry_at &&
            monotonic_ns() >= graphics_retry_at) {
            graphics_retry_at = 0;
            start_window(app);
        }
        if (!app->window || !renderer_ready || app->destroyRequested) continue;

        {
            uint64_t frame_start = monotonic_ns();
            if (previous_loop_at) {
                ds_graphics_report_frame_interval((double)(frame_start - previous_loop_at) / 1e9);
            }
            previous_loop_at = frame_start;
            apply_screen_size();
            dt = previous_frame_at ? (double)(frame_start - previous_frame_at) / 1e9 : 0.0;
            if (dt < 0.0) dt = 0.0;
            if (dt > 0.1) dt = 0.1;
            previous_frame_at = frame_start;
            if (!ds_runtime_has_error()) game_update();
        }

        frame.pixels = NULL;
        frame.width = screen_w;
        frame.height = screen_h;
        frame.stride = screen_w;
        if (frame.width > 0 && frame.height > 0 && ds_graphics_begin_frame(&frame)) {
            if (ds_runtime_has_error()) {
                ds_graphics_error_screen(ds_runtime_error_message());
            } else {
                game_draw();
                if (ds_runtime_has_error()) {
                    ds_graphics_error_screen(ds_runtime_error_message());
                }
            }
            ds_graphics_end_frame();
        }
    }
}

/* The renderer is included in this translation unit so its internal modules can
 * share state without exposing a large platform API. */
#include "graphics.c"
