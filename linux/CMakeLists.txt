# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

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

# Link static archives (libjxl + dependencies).
set(_LIBJXL_LIB_DIR "${LIBJXL_ROOT}/lib")
set(_lib_candidates
  "${_LIBJXL_LIB_DIR}/libjxl.a"
  "${_LIBJXL_LIB_DIR}/libjxl_cms.a"
  "${_LIBJXL_LIB_DIR}/libjxl_threads.a"
  "${_LIBJXL_LIB_DIR}/hwy.a"
  "${_LIBJXL_LIB_DIR}/libhwy.a"
  "${_LIBJXL_LIB_DIR}/libbrotlicommon.a"
  "${_LIBJXL_LIB_DIR}/libbrotlidec.a"
  "${_LIBJXL_LIB_DIR}/libbrotlienc.a"
)
set(_libs_to_link "")
foreach(_c IN LISTS _lib_candidates)
  if (EXISTS "${_c}")
    list(APPEND _libs_to_link "${_c}")
  endif()
endforeach()

if (_libs_to_link STREQUAL "")
  message(FATAL_ERROR "No libjxl static archives found under ${_LIBJXL_LIB_DIR}")
endif()

target_link_libraries(jxl_ffi PRIVATE ${_libs_to_link})

# List of absolute paths to libraries that should be bundled with the plugin.
# This list could contain prebuilt libraries, or libraries created by an
# external build triggered from this build file.
set(jxl_ffi_bundled_libraries
  # Defined in ../src/CMakeLists.txt.
  # This can be changed to accommodate different builds.
  $<TARGET_FILE:jxl_ffi>
  PARENT_SCOPE
)
