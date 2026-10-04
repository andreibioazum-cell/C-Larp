cmake_minimum_required(VERSION 3.16)
project(CubicBattle4 C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Cubic Battle 4 is a native Android game. game/game.c is authored C source;
# there is no code generator or scripting compiler in the build.
if(NOT ANDROID)
    message(FATAL_ERROR
        "Cubic Battle 4 builds for Android only. Configure CMake with the Android "
        "NDK toolchain, for example: cmake -B build "
        "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake "
        "-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26")
endif()

add_library(cb_game SHARED
    game/game.c
    runtime.c
    main.c
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(cb_game PRIVATE
    .
    ${ANDROID_NDK}/sources/android/native_app_glue
)
target_link_libraries(cb_game PRIVATE android log dl m vulkan)
