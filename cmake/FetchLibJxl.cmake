cmake_minimum_required(VERSION 3.10)

include_guard(GLOBAL)

# Provides LIBJXL_ROOT in the parent scope.
#
# - Windows: downloads `jxl-x64-windows-static.7z` and extracts it.
# - Linux: downloads `jxl-linux-x86_64-static.tar.lz` and extracts it.
#
# It also supports an optional manual cache under:
#   native/prebuilt/<platform>/...
function(fetch_libjxl)
  set(JXL_VERSION "0.12.0")
  set(LIBJXL_BASE_URL
      "https://github.com/libjxl/libjxl/releases/download/v${JXL_VERSION}")

  set(_prebuilt_root "${CMAKE_CURRENT_LIST_DIR}/../native/prebuilt")
  set(_bin_root "${CMAKE_BINARY_DIR}/libjxl")

  if (WIN32)
    set(_archive_name "jxl-x64-windows-static.7z")
    set(_archive_url "${LIBJXL_BASE_URL}/${_archive_name}")
    set(_extract_subdir "x64-windows-static")

    set(_prebuilt_lib "${_prebuilt_root}/windows-x64/lib/jxl.lib")
    if (EXISTS "${_prebuilt_lib}")
      set(LIBJXL_ROOT "${_prebuilt_root}/windows-x64")
      set(LIBJXL_ROOT "${LIBJXL_ROOT}" PARENT_SCOPE)
      return()
    endif()

    set(_download_dir "${_bin_root}/download")
    file(MAKE_DIRECTORY "${_download_dir}")
    set(_archive_path "${_download_dir}/${_archive_name}")

    if (NOT EXISTS "${_archive_path}")
      message(STATUS "Downloading ${_archive_name}...")
      file(
        DOWNLOAD "${_archive_url}" "${_archive_path}"
        SHOW_PROGRESS
        STATUS _dl_status
      )
      list(GET _dl_status 0 _dl_code)
      if (NOT _dl_code EQUAL 0)
        message(FATAL_ERROR "Failed to download libjxl prebuilts: ${_dl_status}")
      endif()
    endif()

    find_program(_7Z_EXE NAMES "7z.exe" PATHS "C:/Program Files/7-Zip" NO_DEFAULT_PATH)
    if (NOT _7Z_EXE)
      find_program(_7Z_EXE NAMES "7z" )
    endif()
    if (NOT _7Z_EXE)
      message(FATAL_ERROR "7-Zip (7z.exe) not found. Install 7-Zip or ensure it is on PATH.")
    endif()

    set(_extract_dir "${_bin_root}/${_extract_subdir}")
    file(MAKE_DIRECTORY "${_extract_dir}")

    message(STATUS "Extracting ${_archive_name}...")
    execute_process(
      COMMAND "${_7Z_EXE}" x "${_archive_path}" "-o${_extract_dir}" "-y"
      RESULT_VARIABLE _extract_rv
      OUTPUT_VARIABLE _extract_out
      ERROR_VARIABLE _extract_err
    )
    if (NOT _extract_rv EQUAL 0)
      message(FATAL_ERROR "Failed to extract libjxl prebuilts. ${_extract_err}")
    endif()

    # Some archives extract with a nested folder, so check both.
    set(_cand1 "${_extract_dir}")
    set(_cand2 "${_extract_dir}/${_extract_subdir}")

    if (EXISTS "${_cand1}/include" AND EXISTS "${_cand1}/lib/jxl.lib")
      set(LIBJXL_ROOT "${_cand1}")
    elseif (EXISTS "${_cand2}/include" AND EXISTS "${_cand2}/lib/jxl.lib")
      set(LIBJXL_ROOT "${_cand2}")
    else()
      message(FATAL_ERROR "Could not locate extracted libjxl root under ${_extract_dir}")
    endif()

  else()
    # Assume x86_64 Linux desktop.
    set(_archive_name "jxl-linux-x86_64-static.tar.lz")
    set(_archive_url "${LIBJXL_BASE_URL}/${_archive_name}")
    set(_extract_subdir "linux-x86_64-static")

    set(_prebuilt_lib "${_prebuilt_root}/linux-x86_64-static/lib/libjxl.a")
    if (EXISTS "${_prebuilt_lib}")
      set(LIBJXL_ROOT "${_prebuilt_root}/linux-x86_64-static")
      set(LIBJXL_ROOT "${LIBJXL_ROOT}" PARENT_SCOPE)
      return()
    endif()

    set(_download_dir "${_bin_root}/download")
    file(MAKE_DIRECTORY "${_download_dir}")
    set(_archive_path "${_download_dir}/${_archive_name}")

    if (NOT EXISTS "${_archive_path}")
      message(STATUS "Downloading ${_archive_name}...")
      file(
        DOWNLOAD "${_archive_url}" "${_archive_path}"
        SHOW_PROGRESS
        STATUS _dl_status
      )
      list(GET _dl_status 0 _dl_code)
      if (NOT _dl_code EQUAL 0)
        message(FATAL_ERROR "Failed to download libjxl prebuilts: ${_dl_status}")
      endif()
    endif()

    set(_extract_dir "${_bin_root}/${_extract_subdir}")
    file(MAKE_DIRECTORY "${_extract_dir}")

    message(STATUS "Extracting ${_archive_name}...")
    execute_process(
      COMMAND tar --lzma -xf "${_archive_path}" -C "${_extract_dir}"
      RESULT_VARIABLE _extract_rv
      OUTPUT_VARIABLE _extract_out
      ERROR_VARIABLE _extract_err
    )
    if (NOT _extract_rv EQUAL 0)
      message(FATAL_ERROR "Failed to extract libjxl prebuilts. ${_extract_err}")
    endif()

    set(_cand1 "${_extract_dir}")
    set(_cand2 "${_extract_dir}/${_extract_subdir}")

    if (EXISTS "${_cand1}/include" AND EXISTS "${_cand1}/lib/libjxl.a")
      set(LIBJXL_ROOT "${_cand1}")
    elseif (EXISTS "${_cand2}/include" AND EXISTS "${_cand2}/lib/libjxl.a")
      set(LIBJXL_ROOT "${_cand2}")
    else()
      message(FATAL_ERROR "Could not locate extracted libjxl root under ${_extract_dir}")
    endif()
  endif()

  set(LIBJXL_ROOT "${LIBJXL_ROOT}" PARENT_SCOPE)
endfunction()

