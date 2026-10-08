#!/usr/bin/env python3
from pathlib import Path
import os
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from test_settings_storage import ANDROID_LOG_H, JNI_H

HARNESS = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "engine/network.c"

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    (void)prio; (void)tag; (void)fmt;
    return 0;
}
void ds_console_log(int is_error, const char *format, ...) { (void)is_error; (void)format; }

static const char *CLOUD_NO_EBUC =
    "{\"nick\":\"tester\",\"cups\":100,\"candies\":200,\"cls\":0,\"azum\":1,\"santa\":0,"
    "\"level\":0,\"levels\":0}";

static const char *CLOUD_FRESH =
    "{\"nick\":\"tester\",\"cups\":120,\"candies\":50,\"cls\":1,\"azum\":1,\"santa\":0,"
    "\"ebuc\":1,\"level\":0,\"levels\":0}";

static const char *CLOUD_NO_ASTRA =
    "{\"nick\":\"tester\",\"cups\":100,\"candies\":20,\"cls\":0,\"azum\":0,\"santa\":0,"
    "\"ebuc\":0,\"level\":0,\"levels\":0}";

static int run_first_save(const char *dir) {
    net_set_data_path(dir);

    net_save_progress_all(7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    assert(net_load_ebuc() == 1);
    assert(net_load_azum() == 1);
    assert(net_load_cups() == 7);
    puts("first save: classes bought earlier survive a save issued before any read");
    return 0;
}

static int run_offline_buy(const char *dir) {
    net_set_data_path(dir);
    assert(net_load_ebuc() == 0);
    net_save_progress_all(100, 50, 3, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    assert(cloud_dirty_get() == 1);
    int push = apply_user_json_keep_local(CLOUD_NO_EBUC);
    assert(push == 1);
    assert(net_load_ebuc() == 1);
    assert(net_load_class() == 3);
    assert(net_load_candies() == 50);
    assert(net_load_cups() == 100);
    puts("offline buy: the ebuC, its selection and the spent candies come back from the device");
    return 0;
}

static int run_clean(const char *dir) {
    net_set_data_path(dir);
    net_save_progress_all(100, 50, 3, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    cloud_dirty_set(0);
    int push = apply_user_json_keep_local(CLOUD_FRESH);
    assert(push == 0);
    assert(net_load_class() == 1);
    assert(net_load_cups() == 120);
    assert(net_load_ebuc() == 1);
    puts("clean: with no dirty mark a fresher cloud record wins, ownership intact");
    return 0;
}

static int run_astra(const char *dir) {
    net_set_data_path(dir);
    assert(net_load_astra() == 0);
    net_save_astra(1, 3, 3);
    net_save_progress_all(90, 20, 4, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    assert(net_load_astra() == 1);
    assert(net_load_astra_level() == 3);
    assert(net_load_astra_levels_unlocked() == 3);
    assert(net_load_class() == 4);
    assert(apply_user_json_keep_local(CLOUD_NO_ASTRA) == 1);
    assert(net_load_astra() == 1);
    assert(net_load_astra_level() == 3);
    assert(net_load_class() == 4);
    puts("astra: an offline purchase, selection and levels survive an older cloud profile");
    return 0;
}

static int run_astra_rework(const char *dir) {
    net_set_data_path(dir);
    assert(net_load_astra_rw() == 0);
    net_save_astra(1, 3, 3);
    net_save_astra_rw(1);
    assert(net_load_astra_rw() == 1);
    /* The developer switch stays on the device: a full profile save and a
     * cloud sync must not carry it away or bring it in from another phone. */
    net_save_progress_all(90, 20, 4, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    assert(net_load_astra_rw() == 1);
    assert(apply_user_json_keep_local(CLOUD_NO_ASTRA) == 1);
    assert(net_load_astra_rw() == 1);
    net_save_astra(1, 3, 3);
    assert(net_load_astra_rw() == 1);
    net_save_astra_rw(0);
    assert(net_load_astra_rw() == 0);
    puts("astra rework: the developer-only variant is local, and survives every save path");
    return 0;
}

static int run_admin_ban_guard(const char *dir) {
    net_set_data_path(dir);
    net_ban_set("Dimasi4ek229", 1);
    net_ban_set("QWERTYUIOPAJ1234", 1);
    assert(net_is_banned("Dimasi4ek229") == 0);
    assert(net_is_banned("qwertyuiopaj1234") == 0);
    char path[320];
    snprintf(path, sizeof(path), "%s/bans.dat", dir);
    FILE *f = fopen(path, "r");
    assert(f == NULL);

    net_ban_set("ordinary_player", 1);
    assert(net_is_banned("ordinary_player") == 1);
    net_ban_set("ordinary_player", 0);
    assert(net_is_banned("ordinary_player") == 0);
    puts("moderation: neither administrator can be banned; ordinary bans still work");
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: test <mode> <dir>\n"); return 2; }
    if (strcmp(argv[1], "first-save") == 0) return run_first_save(argv[2]);
    if (strcmp(argv[1], "offline-buy") == 0) return run_offline_buy(argv[2]);
    if (strcmp(argv[1], "clean") == 0) return run_clean(argv[2]);
    if (strcmp(argv[1], "astra") == 0) return run_astra(argv[2]);
    if (strcmp(argv[1], "astra-rework") == 0) return run_astra_rework(argv[2]);
    if (strcmp(argv[1], "admin-ban-guard") == 0) return run_admin_ban_guard(argv[2]);
    fprintf(stderr, "unknown mode '%s'\n", argv[1]);
    return 2;
}
'''

def main():
    with tempfile.TemporaryDirectory(prefix="cubic-cloud-") as directory:
        temp = Path(directory)
        android = temp / "android"
        android.mkdir()
        (android / "log.h").write_text(ANDROID_LOG_H)
        (android / "asset_manager.h").write_text(
            "typedef struct AAssetManager AAssetManager;\n")
        (android / "native_window.h").write_text("typedef struct ANativeWindow ANativeWindow;\n")
        (temp / "jni.h").write_text(JNI_H)
        (temp / "test.c").write_text(HARNESS)
        subprocess.run([
            *shlex.split(os.environ.get("CC", "clang")), "-std=gnu99", "-O0",
            "-D_POSIX_C_SOURCE=200809L", "-D__ANDROID__",
            "-Werror=implicit-function-declaration",
            "-I", str(temp), "-I", str(ROOT / "src"), "-I", str(ROOT),
            str(temp / "test.c"), "-lm", "-lpthread", "-o", str(temp / "test"),
        ], check=True)
        run = [str(temp / "test")]


        first = temp / "first"
        first.mkdir()
        (first / "progress.dat").write_text(
            "10 3 1 0 5 0 0 0 0 0 0 0 0 0 0 1 0 0\n", encoding="utf-8")
        subprocess.run([*run, "first-save", str(first)], check=True)
        saved = (first / "progress.dat").read_text(encoding="utf-8").split()
        assert saved[2] == "1" and saved[15] == "1", saved

        offline = temp / "offline"
        offline.mkdir()
        subprocess.run([*run, "offline-buy", str(offline)], check=True)
        assert (offline / "progress.dirty").exists()

        clean = temp / "clean"
        clean.mkdir()
        subprocess.run([*run, "clean", str(clean)], check=True)
        assert not (clean / "progress.dirty").exists()

        astra = temp / "astra"
        astra.mkdir()
        subprocess.run([*run, "astra", str(astra)], check=True)
        saved = (astra / "progress.dat").read_text(encoding="utf-8").split()
        assert len(saved) == 22 and saved[18:] == ["1", "3", "3", "0"], saved

        rework = temp / "rework"
        rework.mkdir()
        subprocess.run([*run, "astra-rework", str(rework)], check=True)
        saved = (rework / "progress.dat").read_text(encoding="utf-8").split()
        assert len(saved) == 22 and saved[18:] == ["1", "3", "3", "0"], saved

        bans = temp / "bans"
        bans.mkdir()
        subprocess.run([*run, "admin-ban-guard", str(bans)], check=True)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
