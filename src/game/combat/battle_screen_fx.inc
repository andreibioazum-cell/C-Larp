/* Обработка кадра боя: виньетка, красный отлив при уроне и смаз движения.
 * Живёт отдельно, чтобы battle_render.inc не разрастался. */

/* Ультра: виньетка затемняет именно края экрана - восемь полос по периметру,
 * каждая ближе к центру светлее. Прямоугольник по форме экрана, а не круг. */
static void draw_vignette(void) {
    int steps = 8;
    double sx = screen_w * 0.07 / steps;
    double sy = screen_h * 0.07 / steps;
    double tw = sx * 1.7;
    double th = sy * 1.7;
    int i = 0;
    while (i < steps) {
        double k = (steps - 1 - i) / (double)(steps - 1);
        unsigned a = (unsigned)(int)(0.22 * k * k * 255.0 + 0.5);
        if (a >= 1) {
            double x = i * sx;
            double y = i * sy;
            double w = screen_w - 2 * x;
            double h = screen_h - 2 * y;
            uint32_t c = ((uint32_t)a) << 24;
            rect(x, y, w, th, c);
            rect(x, y + h - th, w, th, c);
            rect(x, y, tw, h, c);
            rect(x + w - tw, y, tw, h, c);
        }
        i = i + 1;
    }
}

/* Полученный урон: экран слегка краснеет, не сильно. Прозрачность 0 в движке
 * означает «совсем непрозрачный», поэтому пустой отлив не рисуем. */
static void draw_hurt_flash(void) {
    if (gfx_ultra == 0) {
        return;
    }
    unsigned a = (unsigned)(int)(0.22 * player_hurt_a * 255.0 + 0.5);
    if (a < 1) {
        return;
    }
    rect(0, 0, screen_w, screen_h, (((uint32_t)a) << 24) | 0xD81F1F);
}

/* Обработка кадра до интерфейса: только на ультра, на низкой её нет.
 * Цветокоррекция считается в самом движке для каждого цвета кадра. */
static void draw_screen_fx(void) {
    if (gfx_ultra == 0) {
        return;
    }
    draw_vignette();
}

/* Смаз движения считается по фактическому смещению за кадр: рывок даёт
 * длинный шлейф, спокойная ходьба - короткий. */
static void motion_blur_track(void) {
    if (mb_ok == 0) {
        mb_px = player->x;
        mb_py = player->y;
        mb_ex = enemy->x;
        mb_ey = enemy->y;
        mb_dx = 0;
        mb_dy = 0;
        mb_edx = 0;
        mb_edy = 0;
        mb_ok = 1;
        return;
    }
    mb_dx = player->x - mb_px;
    mb_dy = player->y - mb_py;
    mb_edx = enemy->x - mb_ex;
    mb_edy = enemy->y - mb_ey;
    mb_px = player->x;
    mb_py = player->y;
    mb_ex = enemy->x;
    mb_ey = enemy->y;
}
