/* NPC cube in the lobby — typewriter dialogue system (new theme only). */

static const char *cube_npc_dialogue_text(void) {
    if (cube_npc_dialogue_line == 0) return cube_npc_d0;
    if (cube_npc_dialogue_line == 1) return cube_npc_d1;
    if (cube_npc_dialogue_line == 2) return cube_npc_d2;
    if (cube_npc_dialogue_line == 3) return cube_npc_d3;
    if (cube_npc_dialogue_line == 4) return cube_npc_d4;
    return "";
}

static void cube_npc_tap(void) {
    if (cube_npc_dialogue_active == 0) {
        cube_npc_dialogue_active = 1;
        cube_npc_dialogue_line = 0;
        cube_npc_dialogue_chars = 0;
        return;
    }
    double max_chars = str_len(cube_npc_dialogue_text());
    if (cube_npc_dialogue_chars < max_chars) {
        cube_npc_dialogue_chars = max_chars;
        return;
    }
    cube_npc_dialogue_line = cube_npc_dialogue_line + 1;
    if (cube_npc_dialogue_line >= cube_npc_dialogue_lines) {
        cube_npc_dialogue_active = 0;
        cube_npc_dialogue_line = 0;
        cube_npc_dialogue_chars = 0;
        return;
    }
    cube_npc_dialogue_chars = 0;
}

static double cube_npc_draw_sz = 60;
static double cube_npc_x;
static double cube_npc_y;
static double cube_npc_hit_r = 34;

static double dial_panel_h = 110;
static double dial_panel_margin = 10;
static double dial_text_pad = 16;
static double dial_portrait_sz = 64;
static double dial_text_sc = 0.55;

static void draw_lobby_npc_cube(void) {
    cube_npc_x = 20;
    cube_npc_y = screen_h - cube_npc_draw_sz - dial_panel_h - 14;
    double bob = sin(cube_npc_bob_t * 2.0) * 3;
    double draw_y = cube_npc_y + bob;
    /* Cube sprite */
    if (cube_npc_tex_ok == 1) {
        tex(cube_npc_x, draw_y, CUBE_NPC_TEX, 0, cube_npc_draw_sz / 512.0);
    } else {
        roundrect(cube_npc_x, draw_y, cube_npc_draw_sz, cube_npc_draw_sz, 6, ui_purple);
    }
    /* === Black & white dialogue panel at screen bottom === */
    if (cube_npc_dialogue_active != 1) {
        return;
    }
    double pm = dial_panel_margin;
    double pw = screen_w - 2 * pm;
    double ph = dial_panel_h;
    double px = pm;
    double py = screen_h - ph - pm;
    /* Panel background — dark gray */
    roundrect(px, py, pw, ph, 12, 0xFF1A1A1A);
    roundrect_outline(px, py, pw, ph, 12, 2, 0xFFCCCCCC);
    /* Portrait: NPC cube on the left */
    double ptm = (ph - dial_portrait_sz) / 2;
    double ptx = px + 10;
    double pty = py + ptm;
    if (cube_npc_tex_ok == 1) {
        tex(ptx, pty, CUBE_NPC_TEX, 0, dial_portrait_sz / 512.0);
    } else {
        roundrect(ptx, pty, dial_portrait_sz, dial_portrait_sz, 8, ui_purple);
    }
    /* Typewriter text — right of portrait, left-aligned, white text */
    const char *full = cube_npc_dialogue_text();
    double shown = floor(cube_npc_dialogue_chars);
    double len = str_len(full);
    if (shown > len) shown = len;
    const char *visible = str_sub(full, 0, shown);
    double sc = dial_text_sc;
    double text_area_x = ptx + dial_portrait_sz + 10;
    double text_area_w = px + pw - text_area_x - dial_text_pad;
    double tw = text_ink_width(visible) * sc;
    if (tw > text_area_w) {
        sc = sc * text_area_w / tw;
        tw = text_ink_width(visible) * sc;
    }
    double th = text_ink_height(visible) * sc;
    double tt = text_ink_top(visible) * sc;
    double text_y = py + (ph - th) / 2 - tt;
    text_scaled(visible, text_area_x, text_y, 0xFFFFFF, sc);
    /* Blinking cursor — white */
    if (shown < len) {
        double cx = text_area_x + tw + 2;
        double cy = py + (ph - th) / 2;
        double blink = fmod(cube_npc_bob_t * 4, 2);
        if (blink < 1) {
            rect(cx, cy, 2, th, 0xFFFFFF);
        }
    }
}

static double hit_npc_cube(double px, double py) {
    return hit_rect_pad(px, py, cube_npc_x - cube_npc_hit_r + cube_npc_draw_sz / 2,
                        cube_npc_y - cube_npc_hit_r + cube_npc_draw_sz / 2,
                        cube_npc_hit_r * 2, cube_npc_hit_r * 2);
}

static double hit_npc_panel(double px, double py) {
    if (cube_npc_dialogue_active != 1) return 0;
    double pm = dial_panel_margin;
    return hit_rect(px, py, pm, screen_h - dial_panel_h - pm, screen_w - 2 * pm, dial_panel_h);
}
