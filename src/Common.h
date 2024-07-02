#ifndef WIREBOUNDTEXTBAKER__COMMON_H_
#define WIREBOUNDTEXTBAKER__COMMON_H_

#include <array>
#include <cinttypes>
#include <string>
#include <string_view>
#include <vector>

#include <utf8.h>

#include "Details.h"

using PhrasesType = std::vector<std::vector<utf8::utfchar32_t>>;

/// for each char - 4 bits.
/// to convert it to actual width you need: width * factor + min.
/// in file stored as (uint32)factor, (uint32)min, (uint32)height,
/// (uint32)full_height, widths in field "('y') or generated widths"
struct GlyphMapping {
  std::vector<std::uint8_t> widths;
  float factor{1};
  int min{0};
  int height{0}; // including between-row distance
  int full_height{0}; // including between-row distance

  /// 4 bits for each width
  [[nodiscard]] inline int GetWidthByIdx(int char_idx) const {
    float comp_width;
    if (char_idx & 1) {
      comp_width = static_cast<float>((widths[char_idx >> 1]) >> 4);
    } else {
      comp_width = static_cast<float>((widths[char_idx >> 1]) & 0x0F);
    }
    return static_cast<int>(comp_width * factor) + min;
  }
};

struct BitmapInfo {
  std::string name;
  std::string font_path;
  int size{details::kFontDefaultBitmapSize};
  std::vector<int> chars;
  bool autogen_symbols{false};
  GlyphMapping glyph_mapping;
  std::string loc_path;
};

struct Rectangle {
  float left{0};
  float right{0};
  float top{0};
  float bottom{0};

  [[nodiscard]] float Width() const {
    return right - left;
  }
  [[nodiscard]] float Height() const {
    return top - bottom;
  }

  /// according to OpenGL coordinates usage:
  /// position (pos) coords means [-1;1]
  /// texture (tex) coords means [0;1]
  [[nodiscard]] inline constexpr Rectangle PosToTexCoords() const {
    return {
      (left + 1.0f) / 2.0f,
      (right + 1.0f) / 2.0f,
      (top + 1.0f) / 2.0f,
      (bottom + 1.0f) / 2.0f
    };
  }
  [[nodiscard]]inline constexpr Rectangle TexToPosCoords() const {
    return {
        left * 2.0f - 1.0f,
        right * 2.0f - 1.0f,
        top * 2.0f - 1.0f,
        bottom * 2.0f - 1.0f
    };
  }
};

struct IdxRectangle {
  Rectangle rect;
  int idx;
};

struct PrerenderData {
  std::string name;
  std::string name_bmp_ascii;
  std::string name_bmp_ascii_mask;  // optional
  std::string name_bmp_loc;         // optional
  std::string name_bmp_loc_mask;    // optional
  std::string path_ttf_loc;         // optional
  int scale = details::kDefaultTextSize;
  PhrasesType phrases;
};

/// converts into rectangle (two triangles) for GL_TRIANGLE_STRIP
/// you can just put it to a buffer with glBufferSubData.
inline constexpr std::array<float, 16> ConvertToTriangleStrip(
    float pos_left, float pos_right, float pos_top, float pos_bottom,
    float tex_left, float tex_right, float tex_top, float tex_bottom) {
  return {
      pos_right, pos_bottom,
      pos_right, pos_top,
      pos_left, pos_bottom,
      pos_left, pos_top,
      tex_right, tex_bottom,
      tex_right, tex_top,
      tex_left, tex_bottom,
      tex_left, tex_top,
  };
}

/// each corner's pos and tex coord
inline constexpr std::size_t kTriangleStripSize = 16 * sizeof(float);

#endif  // WIREBOUNDTEXTBAKER__COMMON_H_
