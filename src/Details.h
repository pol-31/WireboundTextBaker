#ifndef WIREBOUNDTEXTBAKER__DETAILS_H_
#define WIREBOUNDTEXTBAKER__DETAILS_H_

#include <array>
#include <string_view>

namespace details {

inline constexpr std::string_view kBmpsInfoMetadata =
    "--- --- --- METADATA (first 20 rows will be skipped) --- --- ---\n"
    "8 columns, 12 rows, 96 chars max; name and font_path are necessary.\n"
    "name = no extension, no parent path; filename = name + extension;\n"
    "path = filename + parent path.\n"
    "If no resolution provided (or it's invalid) - used default value 512.\n"
    "___ Symbols to generate may be wrapped in \"\" and can contain indents;\n"
    "should be represented in hex format with 6 symbols (e.g. 0xFFFF) and\n"
    "separated by comma ','.\n"
    "Symbols ranges described with '-' (inclusive), e.g. 0x0020-0x007E.\n"
    "___ autogen('y') or glyph mapping data:\n"
    "- use 'y' to autogen symbols (everything else considered as \"no\");\n"
    "- if file is already generated, glyph mapping data store here.\n"
    "autogen symbols don't contain ASCII symbols (they always\n"
    "should be stored in separate bitmap). Width data represented as:\n"
    "float factor (uint32), int width min (uint32), int height (uint32),\n"
    "int full height (uint32) - including between-line space and\n"
    "std::vector<uint8_t> widths (uint8).\n"
    "___ localization path is optimal and used for symbols autogen\n"
    "  (it would be scanned for more frequent chars).\n"
    "name; font path (.ttf); resolution; symbols to generate; "
    "autogen('y') or glyph mapping data; localization path;";
inline constexpr int kBmpsMetadataLinesNum = 20;

inline constexpr int kWindowWidth = 2048;
inline constexpr int kWindowHeight = 2048;

inline constexpr int kPrerenderedFramebufferSize = 2048;

inline constexpr int kFontDefaultBitmapSize = 1024;

inline constexpr int kBitmapTotalChar = 96;
inline constexpr int kBitmapMaxResolution = 2048;
inline constexpr int kCharMaxResolution = 128;

/// column num should be power of two and kBitmapTotalChar in total
/// (alternative is 16 by 6);
/// texture size also power of two for GPU optimizations,
/// better mip-mapping, cache coherence, etc...
inline constexpr int kBitmapColumnNum = 8;
inline constexpr int kBitmapRowNum = 12;

/// small adjustment, used for rendering
/// useful for playing around with kBitmapRowNum & kBitmapColumnNum
inline consteval int GetRenderRowNum() {
  /// -1 just because it doesn't work without it... idk...
  return details::kBitmapRowNum + 1;
}

inline constexpr int kDefaultTextSize = 100;

/// factor can be changed, but this works well
inline constexpr float ConvertScale(int text_size) {
  return static_cast<float>(text_size)
         / (static_cast<float>(kDefaultTextSize) * 1000);
}

inline constexpr int kFontBufferSize = 1 << 25;

inline constexpr int kShaderErrorBufferSize = 1024;

inline constexpr int kAsciiFirstChar = 0x0020; // ' '
inline constexpr int kAsciiLastChar = 0x007E; // '~'

inline constexpr std::string_view kTexCoordsHeaderGuard =
    "WIREBOUND__ASSETS_TEXT_TEXTURE_DATA_";

inline constexpr std::string_view kBmpInfoHeaderFilename = "BitmapsInfo.h";
inline constexpr std::string_view kBmpInfoNamespace = "bitmaps";
inline constexpr std::string_view kBmpInfoHeaderGuard =
    "WIREBOUND__ASSETS_TEXT_BITMAPS_H_";

} // namespace details

#endif  // WIREBOUNDTEXTBAKER__DETAILS_H_
