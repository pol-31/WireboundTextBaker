#ifdef WIREBOUND__ASSETS_TEXT_BITMAPS_H__H_
#define WIREBOUND__ASSETS_TEXT_BITMAPS_H__H_

#include <array>

#include <string_view>

namespace bitmaps {

inline constexpr int kBitmapTotalChar = 96;
inline constexpr int kBitmapRowNum = 12;
inline constexpr int kBitmapColumnNum = 8;

/// widths packed by 4 bits; how to convert:
/// float comp_width;
/// if (char_idx & 1) {
/// 	comp_width = static_cast<float>((widths[idx >> 1]) >> 4);
/// } else {
/// 	comp_width = static_cast<float>((widths[idx >> 1]) & 0x0F);
/// }
/// return static_cast<int>(comp_width * factor) + min;

inline constexpr std::string_view kbmp_ascii_headerName = "bmp_ascii_header";
inline constexpr std::string_view kbmp_ascii_headerFontPath = "/home/pavlo/Desktop/Wirebound_header.ttf";
inline constexpr int kbmp_ascii_headerSize = 1024;
inline constexpr std::array<int, 95> kbmp_ascii_headerSymbols = {
	32, 33, 34, 35, 36, 37, 38, 39, 
	40, 41, 42, 43, 44, 45, 46, 47, 
	48, 49, 50, 51, 52, 53, 54, 55, 
	56, 57, 58, 59, 60, 61, 62, 63, 
	64, 65, 66, 67, 68, 69, 70, 71, 
	72, 73, 74, 75, 76, 77, 78, 79, 
	80, 81, 82, 83, 84, 85, 86, 87, 
	88, 89, 90, 91, 92, 93, 94, 95, 
	96, 97, 98, 99, 100, 101, 102, 103, 
	104, 105, 106, 107, 108, 109, 110, 111, 
	112, 113, 114, 115, 116, 117, 118, 119, 
	120, 121, 122, 123, 124, 125, 126, 
};
inline constexpr int kbmp_ascii_headerWidthMin = 16;
inline constexpr int kbmp_ascii_headerHeight = 78;
inline constexpr int kbmp_ascii_headerFullHeight = 85;
inline constexpr float kbmp_ascii_headerWidthFactor = 4.13333;
inline constexpr std::array<std::uint8_t, 48> kbmp_ascii_headerWidths = {
	18, 17, 17, 17, 17, 17, 17, 17, 
	17, 17, 17, 17, 17, 17, 17, 17, 
	113, 119, 71, 116, 55, 115, 247, 119, 
	119, 119, 121, 117, 119, 23, 17, 17, 
	33, 34, 34, 34, 2, 34, 32, 34, 
	34, 34, 18, 34, 34, 18, 17, 33, 
};

} // namespace bitmaps

#endif  // WIREBOUND__ASSETS_TEXT_BITMAPS_H__H_
