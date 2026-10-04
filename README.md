# Cubic Battle 4

Минимальная Android-игра, полностью написанная на C.

## Что есть в игре

1. На стартовом экране находится фиолетовая кнопка **«ИГРАТЬ»**.
2. После нажатия открывается арена с бесшовным фоном `grass.png`.
3. В центре стоит кубик из `ordinary.png`.
4. Красная кнопка **«УДАР»** включает короткую анимацию: кубик меняется на
   `ordinary_punch.png`, делает выпад и показывает эффект попадания.
5. Кнопка **«НАЗАД»** и системная кнопка Android возвращают в меню.

Интерфейс рисуется тем же нативным Vulkan-рендерером: скруглённые кнопки,
Comic Relief с кириллицей, PNG-спрайты и масштабирование под экран телефона.

## Структура

```text
game/game.c                  вся игровая логика и интерфейс на C
game/assets/                 трава, два спрайта кубика, иконка и шрифт
main.c                       NativeActivity, ввод и главный цикл
runtime.c / runtime.h        общий нативный API и диагностика
graphics.c / native/graphics Vulkan-рендерер
native/ttf_font/             растеризация шрифта
stage_assets.py              подготовка ассетов для aapt
```

В проекте нет генерации игрового C-кода, скриптового компилятора, Java-слоя и
сетевой подсистемы. APK запускает стандартную `android.app.NativeActivity`, которая
загружает `libcb_game.so`.

## Проверка игровой логики

Хост-тест компилирует настоящий `game/game.c` и проверяет переход из меню на
арену, загрузку травы и спрайтов, удар и возврат назад:

```sh
python3 tools/test_game.py
```

Проверка ограничения размера исходников:

```sh
python3 tools/check_game_file_size.py
```

## Сборка APK

Проще всего запустить GitHub Actions workflow **Build Cubic Battle 4 (Pure C)**.
Он собирает `arm64-v8a`, `armeabi-v7a` и `x86_64`, упаковывает и подписывает APK.
Для релизной подписи используются secrets `CB4_P12_BASE64`,
`CB4_P12_PASSWORD` и, при необходимости, `CB4_KEY_ALIAS`/`CB4_KEY_PASSWORD`.
Если secrets не настроены, workflow создаёт временный debug-ключ, чтобы тестовая
сборка всё равно завершилась и APK появился в артефактах.

Локальная конфигурация через Android NDK:

```sh
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26
cmake --build build
```

Минимальная версия — Android 8 (API 26), целевая — Android 15 (API 35).
Устройство должно поддерживать Vulkan 1.0.
