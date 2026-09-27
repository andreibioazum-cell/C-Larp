cmake_minimum_required(VERSION 3.16)
project(CLarpObby C)
set(CMAKE_C_STANDARD 99)

# The game is deliberately a handwritten C translation unit. There is no
# generated source and no scripting language in the Android build.
set(GAME_C "${CMAKE_CURRENT_SOURCE_DIR}/game/game.c")

if(NOT ANDROID)
    message(FATAL_ERROR
        "C-Larp Obby builds for Android only. Run CMake through the NDK toolchain, "
        "for example: cmake -B build "
        "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake "
        "-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26.")
endif()

add_library(obby_game SHARED
    "${GAME_C}"
    runtime.c
    main.c
    "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
target_include_directories(obby_game PRIVATE
    . game "${ANDROID_NDK}/sources/android/native_app_glue")

# Vulkan is the sole graphics backend. There is no OpenGL or software fallback.
target_link_libraries(obby_game PRIVATE android log dl m vulkan)
