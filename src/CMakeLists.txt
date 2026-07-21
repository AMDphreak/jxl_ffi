# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(jxl_ffi_library VERSION 0.0.1 LANGUAGES C)

add_library(jxl_ffi SHARED
  "jxl_bridge.c"
)

set_target_properties(jxl_ffi PROPERTIES
  PUBLIC_HEADER jxl_bridge.h
  OUTPUT_NAME "jxl_ffi"
)

target_compile_definitions(jxl_ffi PUBLIC DART_SHARED_LIB)

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(jxl_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
