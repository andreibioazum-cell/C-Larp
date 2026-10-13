/* Проверка кадрового пула строк рантайма. Пул обязан освобождать строки,
 * построенные в прошлых кадрах: иначе за сеанс он разрастается, под бой
 * память кончается, отрисовка падает с ошибкой копирования текста, игра
 * перезапускается - и игрока выбрасывает из боя в лобби. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Заглушка: в хостовой сборке нет реализации лога Android. */
void __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args);
void __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args) {
    (void)prio;
    (void)tag;
    (void)fmt;
    (void)args;
}

#include "engine/runtime/core.inc"

static long pool_size(void) {
    long n = 0;
    DSStringNode *node = ds_strings;
    while (node) {
        n = n + 1;
        node = node->next;
    }
    return n;
}

static void make_frame_strings(int frame) {
    int i;
    for (i = 0; i < 20; i++) {
        ds_num_to_string((double)(frame * 20 + i));
    }
    ds_concat(ds_num_to_string((double)frame), " hp");
    ds_concat("игрок ", ds_num_to_string((double)(frame % 7)));
}

int main(void) {
    long grown, swept, kept;
    int frame;

    ds_string_pool_reset();
    for (frame = 0; frame < 500; frame++) {
        make_frame_strings(frame);
    }
    grown = pool_size();
    if (grown < 10000) {
        printf("пул без уборки должен расти, а в нём %ld строк\n", grown);
        return 1;
    }

    ds_string_pool_reset();
    for (frame = 0; frame < 500; frame++) {
        ds_string_pool_tick();
        make_frame_strings(frame);
    }
    swept = pool_size();
    /* За один кадр строится 22 строки, история держится один кадр. */
    if (swept > 64) {
        printf("пул не освобождается: после 500 кадров в нём %ld строк\n", swept);
        return 1;
    }

    /* Строка, построенная в прошлом кадре, ещё жива, а построенная два кадра
     * назад - уже нет: именно столько histories нужно, чтобы ничего не
     * повисло у получателей вроде поля чата. */
    ds_string_pool_reset();
    ds_string_pool_tick();
    make_frame_strings(0);
    kept = pool_size();
    ds_string_pool_tick();
    make_frame_strings(1);
    if (pool_size() < kept) {
        printf("уборка забрала строку прошлого кадра (%ld -> %ld)\n", kept, pool_size());
        return 1;
    }
    ds_string_pool_tick();
    if (pool_size() > 64) {
        printf("уборка не выбросила строки двухкадавровой давности: %ld\n", pool_size());
        return 1;
    }

    printf("пул строк: без уборки %ld, с уборкой %ld - норма\n", grown, swept);
    return 0;
}
