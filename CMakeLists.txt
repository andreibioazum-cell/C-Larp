cmake_minimum_required(VERSION 3.16)
project(CubicBattle4 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT ANDROID)
    message(FATAL_ERROR
        "Cubic Battle 4 builds for Android only. Configure CMake with the Android "
        "NDK toolchain, for example: cmake -B build "
        "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake "
        "-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26")
endif()

set(CB_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CB_ANDROID_GLUE "${ANDROID_NDK}/sources/android/native_app_glue")

add_library(cb_game SHARED
    "${CB_SOURCE_ROOT}/game/game.c"
    "${CB_SOURCE_ROOT}/engine/runtime.c"
    "${CB_SOURCE_ROOT}/platform/android/android_main.c"
    "${CB_ANDROID_GLUE}/android_native_app_glue.c"
)

target_include_directories(cb_game PRIVATE
    "${CB_SOURCE_ROOT}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CB_ANDROID_GLUE}"
)
target_link_libraries(cb_game PRIVATE android log dl m vulkan)
