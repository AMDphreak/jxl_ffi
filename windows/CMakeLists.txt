# The Flutter tooling requires that developers have a version of Visual Studio
# installed that includes CMake 3.14 or later. You should not increase this
# version, as doing so will cause the plugin to fail to compile for some
# customers of the plugin.
cmake_minimum_required(VERSION 3.14)

# Project-level configuration.
set(PROJECT_NAME "jxl_ffi")
project(${PROJECT_NAME} LANGUAGES CXX)

# Invoke the build for native code shared with the other target platforms.
# This can be changed to accommodate different builds.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../src" "${CMAKE_CURRENT_BINARY_DIR}/shared")

# Pull in the official libjxl prebuilt static archives.
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FetchLibJxl.cmake")
fetch_libjxl()

# Header path.
target_include_directories(jxl_ffi PRIVATE "${LIBJXL_ROOT}/include")

# Link static archives.
set(_LIBJXL_LIB_DIR "${LIBJXL_ROOT}/lib")
target_link_libraries(jxl_ffi PRIVATE
  "${_LIBJXL_LIB_DIR}/jxl.lib"
  "${_LIBJXL_LIB_DIR}/jxl_cms.lib"
  "${_LIBJXL_LIB_DIR}/jxl_threads.lib"
  "${_LIBJXL_LIB_DIR}/hwy.lib"
  "${_LIBJXL_LIB_DIR}/brotlicommon.lib"
  "${_LIBJXL_LIB_DIR}/brotlidec.lib"
  "${_LIBJXL_LIB_DIR}/brotlienc.lib"
)

# List of absolute paths to libraries that should be bundled with the plugin.
# This list could contain prebuilt libraries, or libraries created by an
# external build triggered from this build file.
set(jxl_ffi_bundled_libraries
  # Defined in ../src/CMakeLists.txt.
  # This can be changed to accommodate different builds.
  $<TARGET_FILE:jxl_ffi>
  PARENT_SCOPE
)
