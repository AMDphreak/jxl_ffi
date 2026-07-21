# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

# Project-level configuration.
set(PROJECT_NAME "jxl_ffi")
project(${PROJECT_NAME} LANGUAGES CXX)

# libjxl only publishes linux-x86_64 static prebuilts; aarch64 uses a stub .so.
if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(JXL_FFI_BUILD_STUB TRUE)
endif()

if (NOT JXL_FFI_BUILD_STUB)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FetchLibJxl.cmake")
  fetch_libjxl()

  set(JXL_FFI_INCLUDE "${LIBJXL_ROOT}/include")
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
  set(JXL_FFI_LIBS "")
  foreach(_c IN LISTS _lib_candidates)
    if (EXISTS "${_c}")
      list(APPEND JXL_FFI_LIBS "${_c}")
    endif()
  endforeach()

  if (JXL_FFI_LIBS STREQUAL "")
    message(FATAL_ERROR "No libjxl static archives found under ${_LIBJXL_LIB_DIR}")
  endif()
else()
  message(STATUS "jxl_ffi: building stub (no libjxl prebuilt for ${CMAKE_SYSTEM_PROCESSOR})")
endif()

# Invoke the build for native code shared with the other target platforms.
# This can be changed to accommodate different builds.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../src" "${CMAKE_CURRENT_BINARY_DIR}/shared")

# List of absolute paths to libraries that should be bundled with the plugin.
# This list could contain prebuilt libraries, or libraries created by an
# external build triggered from this build file.
set(jxl_ffi_bundled_libraries
  # Defined in ../src/CMakeLists.txt.
  # This can be changed to accommodate different builds.
  $<TARGET_FILE:jxl_ffi>
  PARENT_SCOPE
)
