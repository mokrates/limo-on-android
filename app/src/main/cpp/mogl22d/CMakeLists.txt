cmake_minimum_required(VERSION 3.4.1)

# project (mogl22d)

# add_custom_command(OUTPUT "vertex_glsl.h" "fragment_glsl.h"
#   COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/gen_sources.sh ${CMAKE_CURRENT_SOURCE_DIR})

add_library (mogl22d mogl22d.c)

# adding dependencies

target_link_libraries (mogl22d LINK_PUBLIC
  EGL
  GLESv3
)

target_include_directories (mogl22d SYSTEM PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue ../gc-8.0.6/include)
