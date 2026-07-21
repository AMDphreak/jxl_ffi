#ifndef JXL_FFI_H
#define JXL_FFI_H

#include <stddef.h>
#include <stdint.h>

#if _WIN32
#define JXL_FFI_EXPORT __declspec(dllexport)
#else
#define JXL_FFI_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

JXL_FFI_EXPORT uint32_t jxl_bridge_version(void);

// Decode: output is always RGBA8 (interleaved).
JXL_FFI_EXPORT int jxl_bridge_decode(
    const uint8_t* in, size_t in_len, uint8_t** out_rgba, uint32_t* w,
    uint32_t* h);

// Encode lossless: input is RGBA8.
JXL_FFI_EXPORT int jxl_bridge_encode_lossless(
    const uint8_t* rgba, uint32_t width, uint32_t height, int has_alpha,
    uint8_t** out, size_t* out_len);

// Encode lossy: input is RGB (no alpha).
JXL_FFI_EXPORT int jxl_bridge_encode_lossy(
    const uint8_t* rgb, uint32_t width, uint32_t height, float distance,
    uint8_t** out, size_t* out_len);

JXL_FFI_EXPORT void jxl_bridge_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* JXL_FFI_H */
