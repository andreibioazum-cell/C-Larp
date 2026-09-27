# C-Larp Obby

Небольшое мобильное 3D-обби в стиле Roblox. Игрок вводит ник при запуске,
нажимает **PLAY** и проходит короткий курс с платформами, прыжком и камерой,
которая следует за персонажем.

## Архитектура

- игровой цикл и игровая логика написаны вручную на **C**;
- вывод кадра идёт только через **Vulkan 1.0** и Android NDK;
- ввод ника получает обычный Java `EditText` в
  `game/java/com/clarp/ObbyActivity.java`, а native-часть читает его через JNI;
- генерации исходников и DimScript больше нет;
- модель игрока можно положить в `game/assets/models/player/player.gltf`.
  Рядом можно хранить `.bin` и текстуры. Пока загрузчик GLTF не подключён,
  поэтому на сцене используется статичный C-плейсхолдер без анимаций.

Папка `game/assets/models/player/` уже создана и содержит инструкцию для модели.

## Сборка APK

Нужны Android SDK, NDK r25c, Python только для копирования assets и Java 17.
Основной CI-путь:

```sh
python3 stage_assets.py game/assets staging/assets
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26
cmake --build build
```

Для публикации APK используется workflow `.github/workflows/main.yml`; он
собирает `arm64-v8a`, `armeabi-v7a` и `x86_64`, компилирует Java activity и
упаковывает библиотеку `libobby_game.so`.

## Управление

- ник: латинские буквы, цифры и `_`, от 1 до 16 символов;
- левая часть экрана — движение влево/вправо;
- правая кнопка — прыжок;
- Android Back закрывает клавиатуру, а затем возвращает к вводу ника.
