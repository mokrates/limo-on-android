#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(moNativeLimo)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)


# i found a precompiled libgmp
add_library(libgmp SHARED IMPORTED)
set_target_properties(libgmp PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/gmp/${CMAKE_ANDROID_ARCH_ABI}/libgmp.so)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Werror")

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -v -u ANativeActivity_onCreate")

  add_subdirectory (fonts)
  add_subdirectory (mogl22d)
  add_subdirectory (gc-8.0.6)   # libgc
  add_subdirectory (limo)

  add_library(native-activity SHARED main.cpp text.c)

target_include_directories(native-activity PRIVATE
  ${ANDROID_NDK}/sources/android/native_app_glue
  gc-8.0.6/include                # libgc
  gmp/${CMAKE_ANDROID_ARCH_ABI}   # libgmp
  limo)

  # add lib dependencies
  #GLESv1_CM
target_link_libraries(native-activity
  simplefonts
  mogl22d
  android
  native_app_glue
  EGL
  GLESv1_CM  # GLESv1_CM  #GLESv3
  log

  gc-lib   # libgc
  libgmp
  limo)
