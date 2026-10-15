#!/usr/bin/env bash
# Компиляция в WebAssembly: два файла-результата — web/ и phone/
# Каждый получает .html + .js + .wasm (отдельные сборки, как просили).
#
# Использование:
#   source ./emsdk_env.sh      # активировать Emscripten SDK (если не в PATH)
#   ./build_web.sh
set -e
cd "$(dirname "$0")"
SRC="$PWD"
EMFLAGS="-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 \
-s EXPORTED_RUNTIME_METHODS=['UTF8ToString','HEAP8','ccall','cwrap'] \
-s INVOKE_RUN=1 -s DISABLE_EXCEPTION_CATCHING=0"

# --- Web-сборка (desktop-браузер, Canvas 2D, JS-клавиатура, WASM) ---
emcc "$SRC/src/platform/web/web_main.c" \
  -I"$SRC/src" -I"$SRC" \
  $EMFLAGS \
  --shell-file "$SRC/build/web/shell.html" \
  -o "$SRC/build/web/cb4_web.html" -O2

# --- Phone-сборка (PWA, fullscreen, тот же WASM-код, другой shell) ---
emcc "$SRC/src/platform/web/web_main.c" \
  -I"$SRC/src" -I"$SRC" \
  $EMFLAGS \
  --shell-file "$SRC/build/phone/shell.html" \
  -o "$SRC/build/phone/cb4_phone.html" -O2

echo "ГОТОВО:"
echo "  build/web/cb4_web.{html,wasm,js}     <-- браузер на ПК"
echo "  build/phone/cb4_phone.{html,wasm,js} <-- телефоны (PWA, фулскрин)"
echo "  build/phone/manifest.json            <-- для установки как приложение"
