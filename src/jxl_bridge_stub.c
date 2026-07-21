#include "jxl_ffi.h"

#include <stdlib.h>

// libjxl ships no Linux aarch64 static prebuilts; build a stub .so so the app
// links and falls back to koni_jxl at runtime (JxlFfi.decode/encode return null).

uint32_t jxl_bridge_version(void) { return 0; }

int jxl_bridge_decode(const uint8_t* in, size_t in_len, uint8_t** out_rgba,
                      uint32_t* w, uint32_t* h) {
  (void)in;
  (void)in_len;
  if (out_rgba != NULL) {
    *out_rgba = NULL;
  }
  if (w != NULL) {
    *w = 0;
  }
  if (h != NULL) {
    *h = 0;
  }
  return -1;
}

int jxl_bridge_encode_lossless(const uint8_t* rgba, uint32_t width,
                               uint32_t height, int has_alpha, uint8_t** out,
                               size_t* out_len) {
  (void)rgba;
  (void)width;
  (void)height;
  (void)has_alpha;
  if (out != NULL) {
    *out = NULL;
  }
  if (out_len != NULL) {
    *out_len = 0;
  }
  return -1;
}

int jxl_bridge_encode_lossy(const uint8_t* rgb, uint32_t width, uint32_t height,
                            float distance, uint8_t** out, size_t* out_len) {
  (void)rgb;
  (void)width;
  (void)height;
  (void)distance;
  if (out != NULL) {
    *out = NULL;
  }
  if (out_len != NULL) {
    *out_len = 0;
  }
  return -1;
}

void jxl_bridge_free(void* ptr) { free(ptr); }
