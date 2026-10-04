# Cubic Battle 4

Полная версия Cubic Battle 4 для Android. Игровая логика написана на C без
скриптового языка, генератора исходников и build-time codegen. В проекте
сохранены меню, одиночный бой, бот, классы, способности, прогресс, квесты,
достижения, промокоды, чат и сетевые комнаты.

## Возможности

- одиночный бой с ботом и онлайн-комнаты до четырёх игроков;
- обычный кубик, Азум, Дед Мороз, ebuC и Астра;
- удары, рывок, снежинка, турели, вселенная и захват Астры;
- уровни классов, скин зомби-Азума и скин эльфа-ebuC;
- кубки, леденцы, награды и локальные сохранения;
- квесты, достижения, промокоды и таблица лидеров;
- чат, стикеры и сообщения над бойцами;
- постоянный травяной фон, 35-секундные композиции лобби и отдельная музыка для зимы и противостояния Астры с Азумом;
- русский и английский интерфейс.

## Структура проекта

Исходный код, ресурсы, платформенная упаковка и инструменты находятся в разных
каталогах:

```text
src/
  game/                         игровая логика
    core/                       главный цикл и общие правила
    ui/                         меню, локализация, чат и прогресс
    combat/                     бой, бот, классы и сетевой бой
    fx/                         погода и визуальные эффекты
    state/                      создание и начальные значения состояния
    include/prototypes/         внутренние объявления подсистем
    game.c                      единая точка сборки игровой логики
    game.h                      публичные игровые hooks
  engine/                       независимые подсистемы движка
    graphics/                   Vulkan и геометрия
    network/                    Firebase, комнаты и сохранения
    runtime/                    память, строки и защищённые вызовы
    audio/                      WAV-декодер и микшер
    font/                       растеризация TTF
  platform/android/             нативная точка входа и Android-мосты
assets/
  textures/                     игровые PNG-текстуры
  fonts/                        шрифты и лицензии
  audio/                        музыка и звуки
  shaders/                      исходники GLSL
platform/android/
  AndroidManifest.xml           описание Android-приложения
  java/                         системный keyboard bridge
  res/                          иконки Android
third_party/                    сторонние библиотеки
tools/                          проверки и служебные программы
```

Игровые подсистемы собираются как единый C translation unit через
`src/game/game.c`. Это сохраняет внутренние функции закрытыми, но код остаётся
разделённым по предметным модулям. Публичный интерфейс игры находится в
`src/game/game.h`. Правила зависимостей подробно описаны в
[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

`platform/android/java/com/cb4/GameActivity.java` содержит системный мост к
`EditText` и Android-диалогу об альфа-версии. Игровой логики в Java нет.

Комментарии в собственных исходниках оставлены только там, где они нужны, и
написаны по-русски. Лицензионные комментарии в
`third_party/stb_image.h` сохранены без изменений.

## Проверки

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic \
  -Isrc -I. -Itools/host_test/stub \
  -fsyntax-only src/game/game.c
python3 tools/check_game_file_size.py
python3 tools/test_resources.py
python3 tools/test_game_state.py
python3 tools/test_settings_storage.py
python3 tools/test_cloud_progress.py
python3 tools/test_promo.py
```

Проверки контролируют размер файлов, структуру каталогов, состояние игры,
захват Астры в одиночном, сетевом и бот-режимах, обычный и новогодний фон,
игровые ресурсы, Firebase rules, настройки, облачный прогресс и промокоды.

## Ресурсы

Исходные ресурсы хранятся в тематических каталогах внутри `assets/`.
`stage_assets.py` создаёт APK-дерево, помещает текстуры в корень Android assets,
шрифты в `fonts/`, звук в `sounds/` и компилирует Java-мост в `classes.dex`:

```sh
python3 stage_assets.py assets staging/assets
```

Шейдеры Vulkan уже сохранены в виде SPIR-V. После изменения GLSL их можно
пересобрать командой:

```sh
python3 tools/shaders/build_shaders.py
```

Для неё требуется `glslangValidator`.

## Сборка APK

Workflow **Build Cubic Battle 4 (Pure C)** собирает `libcb_game.so` для
`arm64-v8a`, `armeabi-v7a` и `x86_64`, упаковывает APK и проверяет подпись.
При наличии используются секреты релизного ключа `CB4_P12_BASE64`,
`CB4_P12_PASSWORD`, `CB4_KEY_ALIAS` и `CB4_KEY_PASSWORD`. Без них создаётся
временный debug-ключ.

Локальная сборка через Android NDK:

```sh
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26
cmake --build build
```

Минимальная версия — Android 8, целевая — Android 15. Требуется Vulkan 1.0.
