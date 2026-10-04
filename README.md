# Cubic Battle 4

Полная версия Cubic Battle 4 для Android. Вся игровая логика перенесена на C:
меню, бой, бот, классы, способности, прогресс, квесты, достижения, чат и онлайн.
Исходники игры больше не генерируются и не требуют отдельного языка или
компилятора.

## Возможности

- одиночный бой с ботом;
- онлайн-комнаты до четырёх игроков;
- обычный кубик, Азум, Дед Мороз и ebuC;
- удары, рывок, снежинка, турели и вселенная;
- уровни классов и скин зомби-Азума;
- кубки, леденцы, награды и локальное сохранение;
- квесты, достижения, промокоды и таблица лидеров;
- чат, стикеры и сообщения над бойцами;
- травяная и зимняя арены, снег и игровые события;
- русский и английский интерфейс.

## Устройство исходников

```text
game/game.c                     единая точка сборки игровой логики
game/modules/core/              состояние, интерфейс и главный цикл
game/modules/ui/                меню, переводы, прогресс, чат и квесты
game/modules/combat/            бой, бот, способности и онлайн
game/modules/fx/                погода и эффекты движения
game/state/                     начальные значения игрового состояния
game/prototypes/                внутренние объявления подсистем
native/graphics/                Vulkan-рендерер
native/net/                     сеть и сохранения
native/runtime/                 базовый рантайм и клавиатура
native/sound/                   загрузка WAV и вывод звука
```

Каждая подсистема лежит в отдельном C-фрагменте и подключается в
`game/game.c`. Функции имеют обычные предметные имена без префиксов
транспилятора. Повторная инициализация состояния вынесена в одно место, поэтому
перезапуск игры не дублирует выделение памяти.

В `game/java/com/cb4/GameActivity.java` нет игровой логики. Это небольшой
системный мост Android для `EditText`, необходимый экранной клавиатуре в полях
логина, промокода и чата. Всё поведение игры остаётся в C.

Комментарии в собственных нативных исходниках оставлены только там, где они
нужны, и написаны по-русски. Лицензионные комментарии сторонней библиотеки
`third_party/stb_image.h` сохранены без изменений.

## Проверки

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic \
  -I. -Itools/host_test/stub -fsyntax-only game/game.c
python3 tools/check_game_file_size.py
python3 tools/test_resources.py
python3 tools/test_game_state.py
python3 tools/test_settings_storage.py
python3 tools/test_cloud_progress.py
python3 tools/test_promo.py
```

## Сборка APK

Workflow **Build Cubic Battle 4 (Pure C)** собирает библиотеки для
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
