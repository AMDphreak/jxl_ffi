# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(jxl_ffi_library VERSION 0.0.1 LANGUAGES C)

if (JXL_FFI_BUILD_STUB)
  add_library(jxl_ffi SHARED
    "jxl_bridge_stub.c"
  )
else()
  add_library(jxl_ffi SHARED
    "jxl_bridge.c"
  )
endif()

set_target_properties(jxl_ffi PROPERTIES
  PUBLIC_HEADER jxl_bridge.h
  OUTPUT_NAME "jxl_ffi"
)

target_compile_definitions(jxl_ffi PUBLIC DART_SHARED_LIB)

# Official libjxl Windows prebuilts are static archives. Without these
# defines, jxl_export.h uses __declspec(dllimport) and the linker looks
# for __imp_Jxl* symbols that do not exist in the .lib files.
if (WIN32 AND NOT JXL_FFI_BUILD_STUB)
  # libjxl static archives are C++; a C-only link skips the STL and leaves
  # unresolved __std_min_element_* / __std_max_element_* symbols.
  enable_language(CXX)
  set_target_properties(jxl_ffi PROPERTIES LINKER_LANGUAGE CXX)
  target_compile_definitions(jxl_ffi PRIVATE
    JXL_STATIC_DEFINE
    JXL_CMS_STATIC_DEFINE
    JXL_THREADS_STATIC_DEFINE
  )
endif()

if (JXL_FFI_INCLUDE)
  target_include_directories(jxl_ffi PRIVATE "${JXL_FFI_INCLUDE}")
endif()
if (JXL_FFI_LIBS)
  target_link_libraries(jxl_ffi PRIVATE ${JXL_FFI_LIBS})
endif()

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(jxl_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
