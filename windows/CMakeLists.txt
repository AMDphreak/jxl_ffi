# The Flutter tooling requires that developers have a version of Visual Studio
# installed that includes CMake 3.14 or later. You should not increase this
# version, as doing so will cause the plugin to fail to compile for some
# customers of the plugin.
cmake_minimum_required(VERSION 3.14)

# Project-level configuration.
set(PROJECT_NAME "jxl_ffi")
project(${PROJECT_NAME} LANGUAGES CXX)

# Official libjxl Windows static prebuilts are x64-only; arm64 uses a stub DLL.
if (CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64" OR CMAKE_VS_PLATFORM_NAME MATCHES "ARM64")
  set(JXL_FFI_BUILD_STUB TRUE)
endif()

if (NOT JXL_FFI_BUILD_STUB)
  # Prefer a locally built tree (scripts/build_libjxl_prebuilt_windows.ps1) so
  # the MSVC STL ABI matches the runner; otherwise FetchLibJxl downloads the
  # official jxl-x64-windows-static.7z.
  include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FetchLibJxl.cmake")
  fetch_libjxl()

  set(JXL_FFI_INCLUDE "${LIBJXL_ROOT}/include")
  set(_LIBJXL_LIB_DIR "${LIBJXL_ROOT}/lib")
  set(JXL_FFI_LIBS
    "${_LIBJXL_LIB_DIR}/jxl.lib"
    "${_LIBJXL_LIB_DIR}/jxl_cms.lib"
    "${_LIBJXL_LIB_DIR}/jxl_threads.lib"
    "${_LIBJXL_LIB_DIR}/hwy.lib"
    "${_LIBJXL_LIB_DIR}/brotlicommon.lib"
    "${_LIBJXL_LIB_DIR}/brotlidec.lib"
    "${_LIBJXL_LIB_DIR}/brotlienc.lib"
  )
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
