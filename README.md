# jxl_ffi

Flutter FFI plugin wrapping **libjxl 0.12.0** static prebuilts for JPEG XL encode/decode on Windows and Linux.

## Quick start

```yaml
dependencies:
  jxl_ffi: ^0.1.0
```

```dart
import 'package:jxl_ffi/jxl_ffi.dart';

if (JxlFfi.isAvailable) {
  final decoded = JxlFfi.decodeRgba(jxlBytes);
  final lossless = JxlFfi.encodeLossless(rgbaBytes, width, height);
  final lossy = JxlFfi.encodeLossy(rgbBytes, width, height, 1.0);
}
```

See [README.adoc](README.adoc) for full documentation (build requirements, API, platforms).

## Requirements

- **Windows:** 7-Zip (`7z.exe`) to extract the official libjxl `.7z` prebuilt
- **Linux:** `tar --lzma` to extract the official `.tar.lz` prebuilt

## License

BSD-3-Clause — see [LICENSE](LICENSE).

## Contact

Ryan Johnson — [@amdphreak](https://twitter.com/amdphreak)

Project: [https://github.com/AMDphreak/jxl_ffi](https://github.com/AMDphreak/jxl_ffi)
