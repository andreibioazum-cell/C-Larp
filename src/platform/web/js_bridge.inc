/* JavaScript-аналоги всего, что раньше было на Java (EM_JS мосты). */
#ifndef CB_JS_BRIDGE_INC
#define CB_JS_BRIDGE_INC
#include <emscripten.h>
#include <emscripten/html5.h>

EM_JS(void, js_alert, (const char *msg), {
    if (typeof alert !== "undefined") alert(UTF8ToString(msg));
});

/* alert с заголовком. В стандартном alert() нет заголовка, поэтому компонуем
 * его как «TITLE\n\nBODY» — ровно как выглядит системный диалог на Android,
 * который и так показывает title жирным отдельной строкой (на десктопе
 * заголовок будет просто первой строкой в жирном стиле нельзя, но всё
 * равно выглядит как обычная альфа-табличка). */
EM_JS(void, js_alert_title, (const char *title, const char *body), {
    var t = UTF8ToString(title), b = UTF8ToString(body);
    if (typeof alert !== "undefined") alert(t + "\n\n" + b);
});

EM_JS(void, js_console_log, (int err, const char *s), {
    var t = UTF8ToString(s);
    if (err) console.error(t); else console.log(t);
});

/* ---- Клавиатура: аналог Java-InputMethod на чистом JS ---- */
EM_JS(void, js_kb_show, (), {
    if (!Module._kbInput) {
        var i = document.createElement("input");
        i.id = "_cb4kb";
        i.style.cssText = "position:fixed;left:-9999px;top:-9999px;opacity:0;font-size:16px;";
        i.setAttribute("inputmode","text");
        i.autocomplete = "off";
        i.autocapitalize = "none";
        document.body.appendChild(i);
        Module._kbInput = i;
        i.addEventListener("input", function() {
            Module.__kb_text = i.value;
        });
        i.addEventListener("keydown", function(e) {
            if (e.key === "Enter") { Module.__kb_enter = 1; e.preventDefault(); }
        });
    }
    Module._kbInput.focus();
    Module.__kb_visible = 1;
});

EM_JS(void, js_kb_hide, (), {
    if (Module._kbInput) { Module._kbInput.blur(); Module._kbInput.style.left="-9999px"; }
    Module.__kb_visible = 0;
});

EM_JS(int, js_kb_visible, (), { return Module.__kb_visible ? 1 : 0; });
EM_JS(int, js_kb_enter, (), { var v = Module.__kb_enter ? 1 : 0; Module.__kb_enter = 0; return v; });

EM_JS(void, js_kb_gettext, (char *dst, int cap), {
    var s = (Module.__kb_text || "").substring(0, cap-1);
    for (var i = 0; i < s.length; i++) HEAP8[dst+i] = s.charCodeAt(i);
    HEAP8[dst+s.length] = 0;
});

EM_JS(void, js_kb_clear, (), {
    if (Module._kbInput) Module._kbInput.value = "";
    Module.__kb_text = "";
});

EM_JS(void, js_kb_type, (const char *t), {
    if (!Module._kbInput) return;
    Module._kbInput.value += UTF8ToString(t);
    Module.__kb_text = Module._kbInput.value;
});

EM_JS(void, js_kb_backspace, (), {
    if (!Module._kbInput) return;
    Module._kbInput.value = Module._kbInput.value.slice(0,-1);
    Module.__kb_text = Module._kbInput.value;
});

/* ---- Canvas 2D графика (вместо Vulkan/WebGPU — проще всего, меньше кода) ---- */
EM_JS(void, js_gfx_init, (const char *cid), {
    Module._canvas = document.getElementById(UTF8ToString(cid));
    if (!Module._canvas) { Module._canvas = document.createElement("canvas"); document.body.appendChild(Module._canvas); }
    Module._ctx = Module._canvas.getContext("2d");
    document.documentElement.style.margin=0; document.body.style.margin=0;
    document.body.style.overflow="hidden";
    document.body.style.width="100%"; document.body.style.height="100%";
    document.body.style.background="#000";
    document.documentElement.style.touchAction="none";
    document.body.style.touchAction="none";
    document.body.style.overscrollBehavior="none";
    Module._canvas.style.position="absolute";
    Module._canvas.style.left="0"; Module._canvas.style.top="0";
    Module._canvas.style.margin="0"; Module._canvas.style.padding="0";
    Module._canvas.style.transformOrigin="top left";
    Module._canvas.style.imageRendering="pixelated";
    Module._canvas.style.imageRendering="-webkit-crisp-edges";
    Module._canvas.style.imageRendering="crisp-edges";
    Module._canvas.style.touchAction="none";
    Module._canvas.style.webkitTapHighlightColor="transparent";
    Module._canvas.style.outline="none"; Module._canvas.style.border="none";
    Module._canvas.style.display="block";
    Module._canvas.oncontextmenu = function(e){ e.preventDefault(); return false; };
    var opts = { passive:false };
    Module._canvas.addEventListener("touchstart", function(e){ e.preventDefault(); }, opts);
    Module._canvas.addEventListener("touchmove",  function(e){ e.preventDefault(); }, opts);
    Module._canvas.addEventListener("touchend",   function(e){ e.preventDefault(); }, opts);
    Module._canvas.addEventListener("touchcancel",function(e){ e.preventDefault(); }, opts);
});

EM_JS(void, js_gfx_resize, (int w, int h), {
    if (!Module._canvas) return;
    var c = Module._canvas;
    c.width = w; c.height = h;
    Module._cw = w; Module._ch = h;
    /* Вписать канвас во viewport letterbox-ом с сохранением соотношения.
     * Используем CSS transform: scale + offset, чтобы координаты
     * getBoundingClientRect() автоматически учитывали масштаб и сдвиг,
     * а js_map_x/y просто умножал canvasX на (width / rect.width). */
    var vw = window.innerWidth  || document.documentElement.clientWidth;
    var vh = window.innerHeight || document.documentElement.clientHeight;
    var sx = vw / w, sy = vh / h;
    var s  = sx < sy ? sx : sy;
    var cw = w*s, ch = h*s;
    var ox = (vw - cw) / 2;
    var oy = (vh - ch) / 2;
    c.style.width  = w+"px";
    c.style.height = h+"px";
    c.style.transform = "translate("+ox+"px,"+oy+"px) scale("+s+")";
    Module._cb4_scale = s;
});

EM_JS(double, js_map_x, (double cx), {
    var c = Module._canvas; if (!c) return cx;
    var r = c.getBoundingClientRect();
    var cw = r.width  || 1;
    return cx * (c.width  / cw);
});
EM_JS(double, js_map_y, (double cy), {
    var c = Module._canvas; if (!c) return cy;
    var r = c.getBoundingClientRect();
    var ch = r.height || 1;
    return cy * (c.height / ch);
});

EM_JS(void, js_gfx_begin, (), {
    if (!Module._ctx) return;
    var c = Module._ctx; c.save(); c.setTransform(1,0,0,1,0,0);
    c.clearRect(0,0,Module._cw,Module._ch);
    c.imageSmoothingEnabled = false;
});

EM_JS(void, js_gfx_fill, (int rgba), {
    var c=Module._ctx;
    var r=(rgba)&0xff, g=(rgba>>8)&0xff, b=(rgba>>16)&0xff, a=((rgba>>24)&0xff)/255;
    c.fillStyle = "rgba("+r+","+g+","+b+","+a+")";
});

EM_JS(void, js_gfx_rect, (float x, float y, float w, float h, int rgba), {
    js_gfx_fill(rgba); Module._ctx.fillRect(x,y,w,h);
});

EM_JS(void, js_gfx_text, (const char *s, float x, float y, float sc, int rgba), {
    var c=Module._ctx; js_gfx_fill(rgba);
    var sz = Math.max(1, Math.floor(20*sc));
    c.font = sz+"px monospace, \"Courier New\", sans-serif";
    c.textBaseline="top";
    c.fillText(UTF8ToString(s), x, y);
});

/* Измерение ширины строки в пикселях канваса (для text_ink_width и центрирования). */
EM_JS(int, js_text_width, (const char *s, float sc), {
    var c=Module._ctx; if(!c||!s) return 0;
    var sz = Math.max(1, Math.floor(20*sc));
    c.font = sz+"px monospace, \"Courier New\", sans-serif";
    return Math.ceil(c.measureText(UTF8ToString(s)).width);
});

EM_JS(void, js_gfx_end, (), {
    Module._ctx.restore();
});

EM_JS(double, js_now, (), { return performance.now()/1000.0; });

EM_JS(int, js_poll_resize, (), {
    var r = Module.__cb4_resize ? 1 : 0;
    Module.__cb4_resize = 0;
    return r;
});

EM_JS(void, js_on_resize, (), {
    if (Module._cb4_or) return;
    Module._cb4_or = 1;
    var h = function(){ Module.__cb4_resize = 1; };
    window.addEventListener("resize", h);
    window.addEventListener("orientationchange", h);
    /* visualViewport точнее ловит появление/скрытие адресной строки на мобильных */
    if (window.visualViewport) window.visualViewport.addEventListener("resize", h);
});

/* ---- Полноэкранный режим по первому нажатию (один раз).
 * Вешаем один слушатель прямо в JS на document (capture), чтобы он сработал
 * на ЛЮБОЙ первый тап/клик, независимо от того, куда Emscripten повесил
 * свои callback'и и показан ли alert(). requestFullscreen обязательно
 * должен вызываться из user-gesture, поэтому прямо в обработчике.
 * Сюда же — resume Web Audio (браузеры блокируют autoplay до жеста). */
EM_JS(void, js_arm_fullscreen_on_first_tap, (), {
    if (Module._cb4_fs_armed) return;
    Module._cb4_fs_armed = 1;
    var done = 0;
    var go = function(e) {
        if (done) return;
        done = 1;
        document.removeEventListener("pointerdown", go, true);
        document.removeEventListener("touchstart", go, true);
        document.removeEventListener("mousedown", go, true);
        document.removeEventListener("keydown", go, true);
        try { if (Module._audioCtx && Module._audioCtx.state === "suspended") Module._audioCtx.resume(); } catch(ex){}
        var el = document.documentElement;
        var fn = el.requestFullscreen || el.webkitRequestFullscreen || el.mozRequestFullScreen || el.msRequestFullscreen;
        if (fn) { try { fn.call(el); } catch(ex){} }
    };
    document.addEventListener("pointerdown", go, true);
    document.addEventListener("touchstart", go, true);
    document.addEventListener("mousedown", go, true);
    document.addEventListener("keydown", go, true);
});

EM_JS(void, js_request_fullscreen, (), {
    var el = document.documentElement;
    var fn = el.requestFullscreen || el.webkitRequestFullscreen || el.mozRequestFullScreen || el.msRequestFullscreen;
    if (fn) { try { fn.call(el); } catch(e){} }
});

EM_JS(int, js_is_mobile, (), {
    return /Mobi|Android|iPhone|iPad|iPod/i.test(navigator.userAgent) ? 1 : 0;
});

#endif /* CB_JS_BRIDGE_INC */
