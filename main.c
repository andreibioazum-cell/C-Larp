#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

/* Android glue for the native C game. There is no script dispatcher here:
 * every frame calls the C game directly, and every frame is submitted to Vulkan. */
#include <android_native_app_glue.h>
#include "runtime.h"
#include <android/input.h>
#include <android/keycodes.h>
#include <android/native_activity.h>
#include <time.h>
#include <unistd.h>

static int renderer_ready;
static int game_started;
static int phys_w, phys_h;
static uint64_t previous_frame_ns;
static uint64_t previous_loop_ns;
static uint64_t retry_at_ns;
static int renderer_attempts;
static int failure_shown;

static uint64_t monotonic_ns(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0;
    return (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_nsec;
}

static void apply_screen_size(void) {
    if (phys_w > 0 && phys_h > 0) {
        screen_w = phys_w;
        screen_h = phys_h;
    }
}

static void renderer_failed(struct android_app *app, const char *why) {
    renderer_ready = 0;
    ++renderer_attempts;
    if (renderer_attempts < 4) {
        retry_at_ns = monotonic_ns() + (uint64_t)(500000000ull << (renderer_attempts - 1));
        ds_log_err("Vulkan start %d/4 failed (%s); retry scheduled", renderer_attempts,
                   why ? why : "unknown");
        return;
    }
    retry_at_ns = 0;
    failure_shown = ds_graphics_show_failure(app->window, renderer_attempts);
}

static void start_window(struct android_app *app) {
    if (!app || !app->window) {
        renderer_ready = 0;
        return;
    }
    phys_w = ANativeWindow_getWidth(app->window);
    phys_h = ANativeWindow_getHeight(app->window);
    if (phys_w <= 0 || phys_h <= 0) {
        renderer_failed(app, "window size is zero");
        return;
    }
    apply_screen_size();
    ds_set_activity(app->activity);
    if (!ds_graphics_init(app->activity ? app->activity->assetManager : NULL, app->window)) {
        renderer_failed(app, ds_graphics_failure());
        return;
    }
    renderer_attempts = 0;
    retry_at_ns = 0;
    failure_shown = 0;
    ds_sound_init(app->activity ? app->activity->assetManager : NULL);
    ds_sound_resume();
    renderer_ready = 1;
    previous_frame_ns = 0;
    previous_loop_ns = 0;
    if (!game_started) {
        init(app->activity ? app->activity->assetManager : NULL);
        game_started = 1;
    }
}

static void handle_command(struct android_app *app, int32_t command) {
    if (!app) return;
    switch (command) {
        case APP_CMD_INIT_WINDOW:
            renderer_attempts = 0;
            retry_at_ns = 0;
            failure_shown = 0;
            start_window(app);
            break;
        case APP_CMD_WINDOW_RESIZED:
        case APP_CMD_CONTENT_RECT_CHANGED:
        case APP_CMD_CONFIG_CHANGED:
            if (app->window) {
                int w = ANativeWindow_getWidth(app->window);
                int h = ANativeWindow_getHeight(app->window);
                if (w > 0 && h > 0) {
                    phys_w = w;
                    phys_h = h;
                    apply_screen_size();
                }
            }
            break;
        case APP_CMD_TERM_WINDOW:
            renderer_ready = 0;
            keyboard_hide();
            retry_at_ns = 0;
            failure_shown = 0;
            ds_graphics_window_lost();
            ds_sound_suspend();
            break;
        case APP_CMD_WINDOW_REDRAW_NEEDED:
            if (failure_shown && app->window)
                failure_shown = ds_graphics_show_failure(app->window, renderer_attempts);
            break;
        case APP_CMD_GAINED_FOCUS:
            ds_sound_resume();
            break;
        case APP_CMD_LOST_FOCUS:
            ds_sound_pause();
            break;
        default:
            break;
    }
}

typedef struct { float x, y; int action, id; } TouchCall;

static int32_t handle_input(struct android_app *app, AInputEvent *event) {
    (void)app;
    if (!event) return 0;
    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        size_t count = AMotionEvent_getPointerCount(event);
        if (count == 0) return 0;
        int raw = AMotionEvent_getAction(event);
        int action = raw & AMOTION_EVENT_ACTION_MASK;
        size_t index = (size_t)((raw & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
        if (action == AMOTION_EVENT_ACTION_POINTER_DOWN) action = AMOTION_EVENT_ACTION_DOWN;
        if (action == AMOTION_EVENT_ACTION_POINTER_UP) action = AMOTION_EVENT_ACTION_UP;
        if (index >= count) index = 0;
        if (action == AMOTION_EVENT_ACTION_MOVE) {
            for (size_t i = 0; i < count; ++i) {
                TouchCall call = { AMotionEvent_getX(event, i), AMotionEvent_getY(event, i),
                                   action, AMotionEvent_getPointerId(event, i) };
                touch(call.x, call.y, call.action, call.id);
            }
        } else {
            TouchCall call = { AMotionEvent_getX(event, index), AMotionEvent_getY(event, index),
                               action, AMotionEvent_getPointerId(event, index) };
            touch(call.x, call.y, call.action, call.id);
        }
        return 1;
    }
    if (type == AINPUT_EVENT_TYPE_KEY) {
        int32_t action = AKeyEvent_getAction(event);
        int32_t key = AKeyEvent_getKeyCode(event);
        int32_t meta = AKeyEvent_getMetaState(event);
        if (key == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_DOWN &&
            (keyboard_visible() || keyboard_uses_editor())) {
            keyboard_hide();
            return 1;
        }
        if (keyboard_visible() &&
            (action == AKEY_EVENT_ACTION_DOWN || action == AKEY_EVENT_ACTION_MULTIPLE) &&
            keyboard_handle_key(key, action, meta)) {
            return 1;
        }
        if (key == AKEYCODE_BACK) {
            if (action == AKEY_EVENT_ACTION_DOWN) return back_pressed();
            return 1;
        }
        return keyboard_uses_editor() ? 0 : 1;
    }
    return 0;
}

void android_main(struct android_app *app) {
    Buffer frame = {0};
    if (!app) return;
    srand((unsigned)(time(NULL) * 2654435761u) ^ ((unsigned)getpid() * 0x9e3779b9u));
    app->onAppCmd = handle_command;
    app->onInputEvent = handle_input;
    ds_sound_set_java_vm((void *)app->activity->vm);
    ds_log("C-Larp native C obby + Vulkan");

    for (;;) {
        struct android_poll_source *source = NULL;
        int timeout = (app->window && renderer_ready) ? 0 : (retry_at_ns ? 50 : 250);
        int ident;
        while ((ident = ALooper_pollOnce(timeout, NULL, NULL, (void **)&source)) >= 0) {
            (void)ident;
            if (source && source->process) source->process(app, source);
            if (app->destroyRequested) {
                renderer_ready = 0;
                keyboard_hide();
                ds_graphics_shutdown();
                ds_sound_shutdown();
                return;
            }
            timeout = 0;
        }
        if (app->window && !renderer_ready && retry_at_ns &&
            monotonic_ns() >= retry_at_ns && !app->destroyRequested) {
            retry_at_ns = 0;
            start_window(app);
        }
        if (!app->window || !renderer_ready || app->destroyRequested) continue;

        uint64_t frame_start = monotonic_ns();
        if (previous_loop_ns)
            ds_graphics_report_frame_interval((double)(frame_start - previous_loop_ns) / 1e9);
        previous_loop_ns = frame_start;
        apply_screen_size();
        dt = previous_frame_ns ? (double)(frame_start - previous_frame_ns) / 1000000000.0 : 0.0;
        if (dt < 0.0) dt = 0.0;
        if (dt > 0.1) dt = 0.1;
        previous_frame_ns = frame_start;
        update();

        frame.pixels = NULL;
        frame.width = screen_w;
        frame.height = screen_h;
        frame.stride = screen_w;
        if (frame.width > 0 && frame.height > 0 && ds_graphics_begin_frame(&frame)) {
            draw(&frame);
            ds_graphics_end_frame();
        }
    }
}

/* Keep the native build self-contained, as before, but the only gameplay source
 * is the handwritten C game above. */
#include "graphics.c"
#include "sound.c"
