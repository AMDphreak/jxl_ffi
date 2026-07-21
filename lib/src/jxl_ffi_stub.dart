import 'dart:typed_data';

/// Web / unsupported-platform stub for [JxlFfi].
///
/// This keeps the app compiling when `dart:ffi` is unavailable.
abstract final class JxlFfi {
  static bool get isAvailable => false;

  static ({Uint8List rgba, int width, int height})? decodeRgba(
    Uint8List jxl,
  ) =>
      null;

  static Uint8List? encodeLossless(
    Uint8List rgba,
    int width,
    int height, {
    bool hasAlpha = true,
  }) =>
      null;

  static Uint8List? encodeLossy(
    Uint8List rgb,
    int width,
    int height,
    double distance,
  ) =>
      null;
}

