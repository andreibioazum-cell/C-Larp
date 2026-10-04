#include "runtime.h"

#include <android/log.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TAG "CubicBattle"
#define CONSOLE_LINES 64
#define CONSOLE_LINE_SIZE 192
#define ERROR_SIZE 512

int screen_w;
int screen_h;
double dt;

static char console_text[CONSOLE_LINES][CONSOLE_LINE_SIZE];
static unsigned char console_errors[CONSOLE_LINES];
static int console_head;
static int console_used;
static char last_error[ERROR_SIZE];
static int has_error;

static void add_console_line(const char *line, int is_error) {
    size_t i;
    size_t write_pos = 0;
    char *target = console_text[console_head];

    if (!line) line = "";
    for (i = 0; line[i] && write_pos + 1 < CONSOLE_LINE_SIZE; ++i) {
        char c = line[i];
        target[write_pos++] = (c == '\n' || c == '\r') ? ' ' : c;
    }
    target[write_pos] = '\0';
    console_errors[console_head] = is_error ? 1u : 0u;
    console_head = (console_head + 1) % CONSOLE_LINES;
    if (console_used < CONSOLE_LINES) ++console_used;
}

static void write_log(int is_error, const char *format, va_list arguments) {
    char message[CONSOLE_LINE_SIZE];
    vsnprintf(message, sizeof(message), format ? format : "", arguments);
    __android_log_print(is_error ? ANDROID_LOG_ERROR : ANDROID_LOG_INFO,
                        LOG_TAG, "%s", message);
    add_console_line(message, is_error);
}

void ds_log(const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    write_log(0, format, arguments);
    va_end(arguments);
}

void ds_log_err(const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    write_log(1, format, arguments);
    va_end(arguments);
}

void ds_runtime_error(const char *format, ...) {
    va_list arguments;
    va_list copy;

    va_start(arguments, format);
    va_copy(copy, arguments);
    vsnprintf(last_error, sizeof(last_error), format ? format : "game error", copy);
    va_end(copy);
    write_log(1, format, arguments);
    va_end(arguments);
    has_error = 1;
}

const char *ds_runtime_error_message(void) {
    return last_error[0] ? last_error : "unknown game error";
}

int ds_runtime_has_error(void) {
    return has_error;
}

void ds_clear_runtime_error(void) {
    has_error = 0;
    last_error[0] = '\0';
}

int console_count(void) {
    return console_used;
}

const char *console_line(int index) {
    int first;
    int slot;
    if (index < 0 || index >= console_used) return "";
    first = (console_head - console_used + CONSOLE_LINES) % CONSOLE_LINES;
    slot = (first + index) % CONSOLE_LINES;
    return console_text[slot];
}

int console_type(int index) {
    int first;
    int slot;
    if (index < 0 || index >= console_used) return 0;
    first = (console_head - console_used + CONSOLE_LINES) % CONSOLE_LINES;
    slot = (first + index) % CONSOLE_LINES;
    return console_errors[slot] ? 1 : 0;
}

void console_clear(void) {
    console_head = 0;
    console_used = 0;
}
