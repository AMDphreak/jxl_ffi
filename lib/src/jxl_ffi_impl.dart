import 'dart:ffi' as ffi;
import 'dart:io';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';


typedef _JxlBridgeDecodeNative = ffi.Int32 Function(
  ffi.Pointer<ffi.Uint8> inPtr,
  ffi.Size inLen,
  ffi.Pointer<ffi.Pointer<ffi.Uint8>> outRgba,
  ffi.Pointer<ffi.Uint32> w,
  ffi.Pointer<ffi.Uint32> h,
);
typedef _JxlBridgeDecodeDart = int Function(
  ffi.Pointer<ffi.Uint8> inPtr,
  int inLen,
  ffi.Pointer<ffi.Pointer<ffi.Uint8>> outRgba,
  ffi.Pointer<ffi.Uint32> w,
  ffi.Pointer<ffi.Uint32> h,
);

typedef _JxlBridgeEncodeLosslessNative = ffi.Int32 Function(
  ffi.Pointer<ffi.Uint8> rgba,
  ffi.Uint32 width,
  ffi.Uint32 height,
  ffi.Int32 hasAlpha,
  ffi.Pointer<ffi.Pointer<ffi.Uint8>> out,
  ffi.Pointer<ffi.Size> outLen,
);
typedef _JxlBridgeEncodeLosslessDart = int Function(
  ffi.Pointer<ffi.Uint8> rgba,
  int width,
  int height,
  int hasAlpha,
  ffi.Pointer<ffi.Pointer<ffi.Uint8>> out,
  ffi.Pointer<ffi.Size> outLen,
);

typedef _JxlBridgeEncodeLossyNative = ffi.Int32 Function(
  ffi.Pointer<ffi.Uint8> rgb,
  ffi.Uint32 width,
  ffi.Uint32 height,
  ffi.Float distance,
  ffi.Pointer<ffi.Pointer<ffi.Uint8>> out,
  ffi.Pointer<ffi.Size> outLen,
);
typedef _JxlBridgeEncodeLossyDart = int Function(
  ffi.Pointer<ffi.Uint8> rgb,
  int width,
  int height,
  double distance,
  ffi.Pointer<ffi.Pointer<ffi.Uint8>> out,
  ffi.Pointer<ffi.Size> outLen,
);

typedef _JxlBridgeFreeNative = ffi.Void Function(ffi.Pointer<ffi.Void> p);
typedef _JxlBridgeFreeDart = void Function(ffi.Pointer<ffi.Void> p);

class _NativeJxl {
  final ffi.DynamicLibrary dylib;
  late final _JxlBridgeDecodeDart _decode =
      dylib.lookupFunction<_JxlBridgeDecodeNative, _JxlBridgeDecodeDart>(
    'jxl_bridge_decode',
  );

  late final _JxlBridgeEncodeLosslessDart _encodeLossless =
      dylib.lookupFunction<_JxlBridgeEncodeLosslessNative,
          _JxlBridgeEncodeLosslessDart>(
    'jxl_bridge_encode_lossless',
  );

  late final _JxlBridgeEncodeLossyDart _encodeLossy =
      dylib.lookupFunction<_JxlBridgeEncodeLossyNative, _JxlBridgeEncodeLossyDart>(
    'jxl_bridge_encode_lossy',
  );

  late final _JxlBridgeFreeDart _free =
      dylib.lookupFunction<_JxlBridgeFreeNative, _JxlBridgeFreeDart>(
    'jxl_bridge_free',
  );

  _NativeJxl(this.dylib);
}

/// Native libjxl FFI wrapper.
abstract final class JxlFfi {
  static const String _libName = 'jxl_ffi';

  static final _NativeJxl? _native = _tryInit();

  static _NativeJxl? _tryInit() {
    try {
      if (Platform.isWindows) {
        return _NativeJxl(ffi.DynamicLibrary.open('$_libName.dll'));
      }
      if (Platform.isLinux) {
        return _NativeJxl(ffi.DynamicLibrary.open('lib$_libName.so'));
      }
    } catch (_) {
      // Ignore; isAvailable should remain false.
    }
    return null;
  }

  static bool get isAvailable => _native != null;

  static ({Uint8List rgba, int width, int height})? decodeRgba(
    Uint8List jxl,
  ) {
    final native = _native;
    if (native == null) return null;

    final int inLen = jxl.lengthInBytes;
    final ffi.Pointer<ffi.Uint8> inPtr = malloc<ffi.Uint8>(inLen);
    inPtr.asTypedList(inLen).setAll(0, jxl);

    final outRgbaPtr = calloc<ffi.Pointer<ffi.Uint8>>();
    final wPtr = calloc<ffi.Uint32>();
    final hPtr = calloc<ffi.Uint32>();

    final int status = native._decode(
      inPtr,
      inLen,
      outRgbaPtr,
      wPtr,
      hPtr,
    );

    // Native returns 0 on success; negative on failure.
    if (status != 0 || outRgbaPtr.value == ffi.Pointer<ffi.Uint8>.fromAddress(0)) {
      malloc.free(inPtr);
      malloc.free(outRgbaPtr);
      malloc.free(wPtr);
      malloc.free(hPtr);
      return null;
    }

    final int width = wPtr.value;
    final int height = hPtr.value;
    final int rgbaLen = width * height * 4;

    final Uint8List rgba = outRgbaPtr.value.asTypedList(rgbaLen);
    final result = (
      rgba: Uint8List.fromList(rgba),
      width: width,
      height: height,
    );

    native._free(outRgbaPtr.value.cast<ffi.Void>());

    malloc.free(inPtr);
    malloc.free(outRgbaPtr);
    malloc.free(wPtr);
    malloc.free(hPtr);

    return result;
  }

  static Uint8List? encodeLossless(
    Uint8List rgba,
    int width,
    int height, {
    bool hasAlpha = true,
  }) {
    final native = _native;
    if (native == null) return null;

    final int rgbaLen = rgba.lengthInBytes;
    final ffi.Pointer<ffi.Uint8> rgbaPtr = malloc<ffi.Uint8>(rgbaLen);
    rgbaPtr.asTypedList(rgbaLen).setAll(0, rgba);

    final outPtr = calloc<ffi.Pointer<ffi.Uint8>>();
    final outLenPtr = calloc<ffi.Size>();

    final int status = native._encodeLossless(
      rgbaPtr,
      width,
      height,
      hasAlpha ? 1 : 0,
      outPtr,
      outLenPtr,
    );

    if (status != 0 || outPtr.value == ffi.Pointer<ffi.Uint8>.fromAddress(0)) {
      malloc.free(rgbaPtr);
      malloc.free(outPtr);
      malloc.free(outLenPtr);
      return null;
    }

    final int outLen = outLenPtr.value.toInt();
    final Uint8List outBytes = outPtr.value.asTypedList(outLen);
    final result = Uint8List.fromList(outBytes);

    native._free(outPtr.value.cast<ffi.Void>());

    malloc.free(rgbaPtr);
    malloc.free(outPtr);
    malloc.free(outLenPtr);
    return result;
  }

  static Uint8List? encodeLossy(
    Uint8List rgb,
    int width,
    int height,
    double distance,
  ) {
    final native = _native;
    if (native == null) return null;

    final int rgbLen = rgb.lengthInBytes;
    final ffi.Pointer<ffi.Uint8> rgbPtr = malloc<ffi.Uint8>(rgbLen);
    rgbPtr.asTypedList(rgbLen).setAll(0, rgb);

    final outPtr = calloc<ffi.Pointer<ffi.Uint8>>();
    final outLenPtr = calloc<ffi.Size>();

    final int status = native._encodeLossy(
      rgbPtr,
      width,
      height,
      distance,
      outPtr,
      outLenPtr,
    );

    if (status != 0 || outPtr.value == ffi.Pointer<ffi.Uint8>.fromAddress(0)) {
      malloc.free(rgbPtr);
      malloc.free(outPtr);
      malloc.free(outLenPtr);
      return null;
    }

    final int outLen = outLenPtr.value.toInt();
    final Uint8List outBytes = outPtr.value.asTypedList(outLen);
    final result = Uint8List.fromList(outBytes);

    native._free(outPtr.value.cast<ffi.Void>());

    malloc.free(rgbPtr);
    malloc.free(outPtr);
    malloc.free(outLenPtr);
    return result;
  }
}

