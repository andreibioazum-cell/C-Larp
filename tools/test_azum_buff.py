#!/usr/bin/env python3
"""Host tests for the live Firebase flag, without Firebase access or real threads."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HARNESS = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int DSMutex;
typedef int DSThread;
#define DS_MUTEX_INIT 0
#define ds_mutex_lock(m) ((void)(m))
#define ds_mutex_unlock(m) ((void)(m))
#define NET_LOGIN_OK 2
#define NET_FIREBASE_URL "https://test.invalid"
static long long clock_ms = 100;
static int login = 0, response_code = 200, thread_ok = 1, reads;
static const char *response = "1";
static void *(*pending)(void *);
static void *pending_arg;
static long long now_ms(void) { return clock_ms; }
static double net_login_status(void) { return login; }
static DSThread ds_thread_start(void *(*fn)(void *), void *arg) {
    assert(!pending);
    if (!thread_ok) return 0;
    pending = fn;
    pending_arg = arg;
    return 1;
}
static void ds_thread_detach(DSThread t) { (void)t; }
static int fb_http(const char *method, const char *url, const char *body, char *out, size_t cap) {
    assert(strcmp(method, "GET") == 0);
    assert(strcmp(url, NET_FIREBASE_URL "/AzumBuff.json") == 0);
    assert(body == NULL);
    reads++;
    snprintf(out, cap, "%s", response);
    return response_code;
}
#include "engine/network/azum_buff.inc"
static void complete(void) {
    void *(*fn)(void *) = pending;
    assert(fn);
    pending = NULL;
    fn(pending_arg);
}
static void poll(const char *json, int code) {
    response = json;
    response_code = code;
    clock_ms += AZUM_BUFF_POLL_MS;
    (void)net_azum_buff();
    assert(pending);
    complete();
}
int main(void) {
    assert(net_azum_buff() == 0 && !pending && reads == 0);
    login = NET_LOGIN_OK;
    assert(net_azum_buff() == 0 && pending);
    /* No parallel jobs and no blocking I/O on the game thread. */
    assert(net_azum_buff() == 0 && reads == 0);
    complete();
    assert(net_azum_buff() == 1 && reads == 1);
    poll("0", 200); assert(net_azum_buff() == 0);
    poll("null", 200); assert(net_azum_buff() == 0);
    poll("\"1\"", 200); assert(net_azum_buff() == 0);
    poll("true", 200); assert(net_azum_buff() == 0);
    poll("{\"AzumBuff\":1}", 200); assert(net_azum_buff() == 0);
    poll("1.1", 200); assert(net_azum_buff() == 0);
    poll("1 trailing", 200); assert(net_azum_buff() == 0);
    poll(" \n1.0\t", 200); assert(net_azum_buff() == 1);
    poll("1", 403); assert(net_azum_buff() == 0);
    poll("1", 200); assert(net_azum_buff() == 1);
    poll("", -1); assert(net_azum_buff() == 0);
    poll("1", 200); assert(net_azum_buff() == 1);
    /* A hung request cannot keep the last success alive indefinitely. */
    clock_ms += AZUM_BUFF_TTL_MS;
    assert(net_azum_buff() == 0 && pending);
    assert(net_azum_buff() == 0);
    /* An old successful reply cannot re-enable the flag after logout. */
    login = 0;
    assert(net_azum_buff() == 0);
    complete();
    assert(net_azum_buff() == 0);
    login = NET_LOGIN_OK;
    assert(net_azum_buff() == 0 && pending);
    complete();
    assert(net_azum_buff() == 1);
    thread_ok = 0;
    clock_ms += AZUM_BUFF_POLL_MS;
    assert(net_azum_buff() == 0 && !pending);
    puts("AzumBuff: async polling, exact value, errors, expiry and logout OK");
}
'''


def main():
    with tempfile.TemporaryDirectory(prefix="cb4-azum-") as directory:
        temp = Path(directory)
        source = temp / "test.c"
        source.write_text(HARNESS)
        executable = temp / "test"
        subprocess.run([
            os.environ.get("CC", "clang"), "-std=c99", "-Wall", "-Wextra",
            "-Werror", "-pedantic", "-I", str(ROOT / "src"),
            str(source), "-o", str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    main()
