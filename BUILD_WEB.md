# Сборка Web/WASM (две сборки: web и телефоны)

## Что сделано
- **WASM**: код компилируется `emcc` в WebAssembly (флаги `-s WASM=1`).
- **Два файла-билда**:
  - `build/web/cb4_web.{html,js,wasm}` — для десктоп-браузеров
  - `build/phone/cb4_phone.{html,js,wasm}` + `manifest.json` — для телефонов (PWA, полный экран, устанавливается как приложение)
- **Графика**: Canvas 2D (самый простой вариант, намного короче, чем WebGPU или Vulkan в вебе; все примитивы рисуются через EM_JS-вызовы).
- **Клавиатура**: Java InputMethod заменён на JavaScript `<input>` на странице (чистый JS-аналог).
- **Всё от Java убрано**: нет JNI, нет классов `com.cb4.*`, нет `AndroidManifest`-зависимостей — web-сборка использует только браузерные API.
- **Начальная табличка**: при старте срабатывает `alert("это та табличка в начале игры ^_^")`.

## Как собрать
```bash
# Устанавливаем Emscripten SDK (один раз):
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest && source ./emsdk_env.sh && cd ..

# В корне проекта:
./build_web.sh
```

После запуска скрипта появятся:
- `build/web/cb4_web.html` — открыть в браузере на ПК
- `build/phone/cb4_phone.html` — открыть на телефоне/PWA

Android-сборка (через NDK + Gradle) продолжает работать как раньше: CMakeLists.txt теперь поддерживает и `emcmake` (Web), и обычный NDK (Android .so).
