#include "jxl_ffi.h"

#include <jxl/decode.h>
#include <jxl/encode.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/types.h>

#include <stdlib.h>
#include <stdbool.h>

static void* jxl_create_parallel_runner(void) {
  return JxlThreadParallelRunnerCreate(NULL,
                                        JxlThreadParallelRunnerDefaultNumWorkerThreads());
}

static void jxl_destroy_parallel_runner(void* runner) {
  if (runner == NULL) return;
  JxlThreadParallelRunnerDestroy(runner);
}

uint32_t jxl_bridge_version(void) { return JxlDecoderVersion(); }

int jxl_bridge_decode(const uint8_t* in, size_t in_len, uint8_t** out_rgba,
                       uint32_t* w, uint32_t* h) {
  if (in == NULL || out_rgba == NULL || w == NULL || h == NULL) return -1;
  *out_rgba = NULL;
  *w = 0;
  *h = 0;

  JxlDecoder* dec = JxlDecoderCreate(NULL);
  if (dec == NULL) return -2;

  void* runner = jxl_create_parallel_runner();
  if (runner != NULL) {
    JxlDecoderSetParallelRunner(dec, JxlThreadParallelRunner, runner);
  }

  // Ask for the basic image info and the fully decoded frame.
  if (JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE) !=
      JXL_DEC_SUCCESS) {
    jxl_destroy_parallel_runner(runner);
    JxlDecoderDestroy(dec);
    return -3;
  }

  if (JxlDecoderSetInput(dec, in, in_len) != JXL_DEC_SUCCESS) {
    jxl_destroy_parallel_runner(runner);
    JxlDecoderDestroy(dec);
    return -4;
  }

  JxlBasicInfo basic_info;
  JxlDecoderStatus status = JXL_DEC_NEED_MORE_INPUT;

  // We allocate the output buffer when the decoder asks for it, so we can
  // be sure the buffer size matches the exact output format.
  JxlPixelFormat out_format;
  out_format.num_channels = 4;  // RGBA
  out_format.data_type = JXL_TYPE_UINT8;
  out_format.endianness = JXL_NATIVE_ENDIAN;
  out_format.align = 0;

  size_t out_buf_size = 0;
  uint8_t* out_buf = NULL;
  while ((status = JxlDecoderProcessInput(dec)) != JXL_DEC_SUCCESS) {
    if (status == JXL_DEC_ERROR) {
      jxl_destroy_parallel_runner(runner);
      JxlDecoderDestroy(dec);
      free(out_buf);
      return -5;
    }

    if (status == JXL_DEC_BASIC_INFO) {
      if (JxlDecoderGetBasicInfo(dec, &basic_info) != JXL_DEC_SUCCESS) {
        jxl_destroy_parallel_runner(runner);
        JxlDecoderDestroy(dec);
        free(out_buf);
        return -6;
      }
      *w = basic_info.xsize;
      *h = basic_info.ysize;
    } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
      // Determine the exact buffer size for the selected pixel format.
      if (JxlDecoderImageOutBufferSize(dec, &out_format, &out_buf_size) !=
          JXL_DEC_SUCCESS) {
        jxl_destroy_parallel_runner(runner);
        JxlDecoderDestroy(dec);
        free(out_buf);
        return -7;
      }
      out_buf = (uint8_t*)malloc(out_buf_size);
      if (out_buf == NULL) {
        jxl_destroy_parallel_runner(runner);
        JxlDecoderDestroy(dec);
        return -8;
      }

      if (JxlDecoderSetImageOutBuffer(dec, &out_format, out_buf,
                                       out_buf_size) != JXL_DEC_SUCCESS) {
        jxl_destroy_parallel_runner(runner);
        JxlDecoderDestroy(dec);
        free(out_buf);
        return -9;
      }
    }

    // Most non-streaming use cases either never return NEED_MORE_INPUT
    // or will require more bytes. In this app we always pass complete
    // buffers, so treat it as an error.
    if (status == JXL_DEC_NEED_MORE_INPUT) {
      jxl_destroy_parallel_runner(runner);
      JxlDecoderDestroy(dec);
      free(out_buf);
      return -10;
    }
  }

  jxl_destroy_parallel_runner(runner);
  JxlDecoderDestroy(dec);

  // If the decoder never requested an output buffer, we can't produce RGBA.
  if (out_buf == NULL) return -11;

  *out_rgba = out_buf;
  return 0;
}

int jxl_bridge_encode_lossless(const uint8_t* rgba, uint32_t width,
                               uint32_t height, int has_alpha,
                               uint8_t** out, size_t* out_len) {
  if (rgba == NULL || out == NULL || out_len == NULL) return -1;
  *out = NULL;
  *out_len = 0;

  JxlEncoder* enc = JxlEncoderCreate(NULL);
  if (enc == NULL) return -2;

  void* runner = jxl_create_parallel_runner();
  if (runner != NULL) {
    JxlEncoderSetParallelRunner(enc, JxlThreadParallelRunner, runner);
  }

  JxlBasicInfo basic_info;
  JxlEncoderInitBasicInfo(&basic_info);
  basic_info.xsize = width;
  basic_info.ysize = height;
  basic_info.bits_per_sample = 8;
  basic_info.num_color_channels = 3;
  basic_info.num_extra_channels = has_alpha ? 1 : 0;
  basic_info.alpha_bits = has_alpha ? 8 : 0;
  basic_info.alpha_exponent_bits = 0;
  basic_info.alpha_premultiplied = 0;
  basic_info.uses_original_profile = 0;

  if (JxlEncoderSetBasicInfo(enc, &basic_info) != JXL_ENC_SUCCESS) {
    jxl_destroy_parallel_runner(runner);
    JxlEncoderDestroy(enc);
    return -3;
  }

  if (has_alpha) {
    // The encoder expects alpha to be added as part of the image frame
    // if it is passed through the input pixel buffer.
  }

  JxlEncoderFrameSettings* frame_settings =
      JxlEncoderFrameSettingsCreate(enc, NULL);
  if (frame_settings == NULL) {
    jxl_destroy_parallel_runner(runner);
    JxlEncoderDestroy(enc);
    return -4;
  }

  if (JxlEncoderSetFrameLossless(frame_settings, JXL_TRUE) != JXL_ENC_SUCCESS) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -5;
  }

  JxlFrameHeader frame_header;
  JxlEncoderInitFrameHeader(&frame_header);
  // For still images a default header is fine (duration = 0).
  if (JxlEncoderSetFrameHeader(frame_settings, &frame_header) !=
      JXL_ENC_SUCCESS) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -6;
  }

  // Input pixel buffer format.
  JxlPixelFormat pixel_format;
  pixel_format.data_type = JXL_TYPE_UINT8;
  pixel_format.endianness = JXL_NATIVE_ENDIAN;
  pixel_format.align = 0;

  const size_t rgba_bytes = (size_t)width * (size_t)height * 4;
  const size_t rgb_bytes = (size_t)width * (size_t)height * 3;

  const uint8_t* frame_pixels = rgba;
  size_t input_size = rgba_bytes;

  uint8_t* rgb_tmp = NULL;
  if (!has_alpha) {
    // Strip alpha channel since the C API expects a consistent input
    // channel count.
    rgb_tmp = (uint8_t*)malloc(rgb_bytes);
    if (rgb_tmp == NULL) {
      JxlEncoderDestroy(enc);
      jxl_destroy_parallel_runner(runner);
      return -7;
    }
    for (size_t i = 0; i < (size_t)width * (size_t)height; i++) {
      rgb_tmp[i * 3 + 0] = rgba[i * 4 + 0];
      rgb_tmp[i * 3 + 1] = rgba[i * 4 + 1];
      rgb_tmp[i * 3 + 2] = rgba[i * 4 + 2];
    }
    frame_pixels = rgb_tmp;
    input_size = rgb_bytes;
    pixel_format.num_channels = 3;
  } else {
    pixel_format.num_channels = 4;
  }

  if (JxlEncoderAddImageFrame(frame_settings, &pixel_format, frame_pixels,
                              input_size) != JXL_ENC_SUCCESS) {
    free(rgb_tmp);
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -7;
  }

  free(rgb_tmp);

  JxlEncoderCloseInput(enc);

  // Output: grow a malloc'ed buffer until encoding succeeds.
  size_t cap = 1024 * 64;
  uint8_t* out_buf = (uint8_t*)malloc(cap);
  if (out_buf == NULL) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -8;
  }

  uint8_t* next_out = out_buf;
  size_t avail_out = cap;
  JxlEncoderStatus status = JXL_ENC_NEED_MORE_OUTPUT;
  while (true) {
    status = JxlEncoderProcessOutput(enc, &next_out, &avail_out);
    if (status == JXL_ENC_SUCCESS) break;
    if (status == JXL_ENC_NEED_MORE_OUTPUT) {
      size_t used = (size_t)(next_out - out_buf);
      size_t new_cap = cap * 2;
      uint8_t* resized = (uint8_t*)realloc(out_buf, new_cap);
      if (resized == NULL) {
        free(out_buf);
        JxlEncoderDestroy(enc);
        jxl_destroy_parallel_runner(runner);
        return -9;
      }
      out_buf = resized;
      cap = new_cap;
      next_out = out_buf + used;
      avail_out = cap - used;
      continue;
    }
    // Any other error.
    free(out_buf);
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -10;
  }

  size_t used = (size_t)(next_out - out_buf);
  *out = out_buf;
  *out_len = used;

  JxlEncoderDestroy(enc);
  jxl_destroy_parallel_runner(runner);
  return 0;
}

int jxl_bridge_encode_lossy(const uint8_t* rgb, uint32_t width, uint32_t height,
                            float distance, uint8_t** out, size_t* out_len) {
  if (rgb == NULL || out == NULL || out_len == NULL) return -1;
  *out = NULL;
  *out_len = 0;

  JxlEncoder* enc = JxlEncoderCreate(NULL);
  if (enc == NULL) return -2;

  void* runner = jxl_create_parallel_runner();
  if (runner != NULL) {
    JxlEncoderSetParallelRunner(enc, JxlThreadParallelRunner, runner);
  }

  JxlBasicInfo basic_info;
  JxlEncoderInitBasicInfo(&basic_info);
  basic_info.xsize = width;
  basic_info.ysize = height;
  basic_info.bits_per_sample = 8;
  basic_info.num_color_channels = 3;
  basic_info.num_extra_channels = 0;
  basic_info.uses_original_profile = 0;

  if (JxlEncoderSetBasicInfo(enc, &basic_info) != JXL_ENC_SUCCESS) {
    jxl_destroy_parallel_runner(runner);
    JxlEncoderDestroy(enc);
    return -3;
  }

  JxlEncoderFrameSettings* frame_settings =
      JxlEncoderFrameSettingsCreate(enc, NULL);
  if (frame_settings == NULL) {
    jxl_destroy_parallel_runner(runner);
    JxlEncoderDestroy(enc);
    return -4;
  }

  if (JxlEncoderSetFrameLossless(frame_settings, JXL_FALSE) != JXL_ENC_SUCCESS) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -5;
  }

  if (JxlEncoderSetFrameDistance(frame_settings, distance) != JXL_ENC_SUCCESS) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -6;
  }

  JxlFrameHeader frame_header;
  JxlEncoderInitFrameHeader(&frame_header);
  if (JxlEncoderSetFrameHeader(frame_settings, &frame_header) !=
      JXL_ENC_SUCCESS) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -7;
  }

  JxlPixelFormat pixel_format;
  pixel_format.num_channels = 3;  // RGB
  pixel_format.data_type = JXL_TYPE_UINT8;
  pixel_format.endianness = JXL_NATIVE_ENDIAN;
  pixel_format.align = 0;

  const size_t input_size = (size_t)width * (size_t)height * 3;
  if (JxlEncoderAddImageFrame(frame_settings, &pixel_format, rgb,
                                input_size) != JXL_ENC_SUCCESS) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -8;
  }

  JxlEncoderCloseInput(enc);

  size_t cap = 1024 * 64;
  uint8_t* out_buf = (uint8_t*)malloc(cap);
  if (out_buf == NULL) {
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -9;
  }

  uint8_t* next_out = out_buf;
  size_t avail_out = cap;
  JxlEncoderStatus status = JXL_ENC_NEED_MORE_OUTPUT;
  while (true) {
    status = JxlEncoderProcessOutput(enc, &next_out, &avail_out);
    if (status == JXL_ENC_SUCCESS) break;
    if (status == JXL_ENC_NEED_MORE_OUTPUT) {
      size_t used = (size_t)(next_out - out_buf);
      size_t new_cap = cap * 2;
      uint8_t* resized = (uint8_t*)realloc(out_buf, new_cap);
      if (resized == NULL) {
        free(out_buf);
        JxlEncoderDestroy(enc);
        jxl_destroy_parallel_runner(runner);
        return -10;
      }
      out_buf = resized;
      cap = new_cap;
      next_out = out_buf + used;
      avail_out = cap - used;
      continue;
    }
    free(out_buf);
    JxlEncoderDestroy(enc);
    jxl_destroy_parallel_runner(runner);
    return -11;
  }

  size_t used = (size_t)(next_out - out_buf);
  *out = out_buf;
  *out_len = used;

  JxlEncoderDestroy(enc);
  jxl_destroy_parallel_runner(runner);
  return 0;
}

void jxl_bridge_free(void* ptr) {
  free(ptr);
}
