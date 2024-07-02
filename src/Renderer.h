#ifndef WIREBOUNDTEXTBAKER__RENDERER_H_
#define WIREBOUNDTEXTBAKER__RENDERER_H_

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <glad/glad.h>
#include <stb_truetype.h>

#include "Details.h"
#include "Parsing.h"
#include "Shader.h"
#include "Texture.h"

class Renderer {
 public:
  struct TextureData {
    std::vector<GLubyte> data;
    int width{0};
    int height{0};
  };

  Renderer() {
    Init();
  }

  ~Renderer() {
    DeInit();
  }

  /// render by symbol, pack to one texture
  /// (to pack more tightly we render all into one, then onto other
  /// (after first render we have all "rectangles", so then can repack better))
  std::vector<Rectangle> Process(
      const std::vector<BitmapInfo>& bmps_info,
      const PrerenderData& loc_info,
      std::string_view bmps_dir);

  [[nodiscard]] TextureData GetTextureData();
  [[nodiscard]] TextureData GetTextureMaskData();

 private:
  enum class TexType {
    kAscii = 0,
    kLangSpec = 1,
    kTtfGlyph = 2,
    kTemp = 3
  };

  /// useful to calculate them only once
  struct TtfFontData {
    float ttf_scale{0.0f};
    int glyph_height{0};
    int y_adjust{0};
  };

  /// advance on tex coordinates [-1;1] for x and [1;-1] for y
  class TexPosAdvance {
   public:
    TexPosAdvance() = default;
    TexPosAdvance(float x, float y) : x_(x), y_(y) {
    }

    void Reset() {
      x_ = -1.0f;
      y_ = 1.0f;
    }

    [[nodiscard]] float GetX() const {
      return x_;
    }
    [[nodiscard]] float GetY() const {
      return y_;
    }

    bool TrySetX(float x) {
      if (x > 1.0f) {
        return false;
      } else {
        x_ = x;
        return true;
      }
    }
    bool TrySetY(float y) {
      if (y < -1.0f) {
        return false;
      } else {
        y_ = y;
        return true;
      }
    }

   private:
    float x_{-1.0f};
    float y_{1.0f};
  };

  void Init();
  void InitTextures();
  void InitFramebuffers();
  void InitBuffers();

  void DeInit();

  void ResetLocale();
  void ResetFramebuffers();

  void CheckSetup();

  static void SetupGreyscaleTexture(
      GLuint tex_id, int tex_width, int tex_height);

  static void SetupFramebuffer(GLuint fbo_id, GLuint tex_id);

  /// it modifies framebuffer by attaching tex_id
  static void ClearFramebufferTexture(GLuint fb_id, GLuint tex_id);

  void SetAscii(
      const std::vector<BitmapInfo>& bmps_info,
      const PrerenderData& loc_info,
      std::string_view bmps_dir);

  void SetLocale(
      const std::vector<BitmapInfo>& bmps_info,
      const PrerenderData& loc_info,
      std::string_view bmps_dir);

  std::vector<Rectangle> RenderMainTexture(
      const PhrasesType& phrases,
      int text_size);

  void RenderMaskTexture(
      const std::vector<Rectangle>& tex_coords,
      const PhrasesType& phrases,
      int text_size);

  static std::vector<Rectangle> GetRepackOrder(
      const std::vector<Rectangle>& tex_coords);

  /// uses fbo_phrases_ as SOURCE and fbo_phrases_packed_ as DESTINATION
  std::vector<Rectangle> Repack(const std::vector<Rectangle>& positions) const;

  /// rerender from tex_phrase_temp_ to tex_phrases_
  /// (see tex_phrases_temp explanation)
  bool RerenderToMain(std::vector<Rectangle>& tex_coords,
                      bool already_repacked = false);

  /// in & out position data is normalized in [0;1]
  /// width, height is in its internal representation (not scaled)
  /// return bottom-right corner.
  /// Full height (including between-line space) skipped here,
  /// because this app aimed to keep it as much tightly packed as possible;
  /// BUT: it should be used at game runtime rendering
  TexPosAdvance RenderGlyph(
      int idx, int tex_size, int width, int height,
      float pos_left, float pos_top, float scale) const;

  TexPosAdvance RenderGlyphTtf(
      int code, int height, float pos_left, float pos_top, float scale);

  std::vector<Rectangle> ProcessImpl(
      const PhrasesType& phrases,
      int text_size);

  void ProcessMaskImpl(
      const std::vector<Rectangle>& tex_coords,
      const PhrasesType& phrases,
      int text_size);

  constexpr inline static bool IsAsciiBitmap(int code) {
    return (code <= details::kAsciiLastChar) &&
           (code >= details::kAsciiFirstChar);
  }

  /// bmp both for ascii and loc taken only from base bitmap (not mask),
  /// because they have the same characteristics (at least should have)

  const BitmapInfo* bmp_ascii_{nullptr};
  Texture tex_ascii_;
  Texture tex_ascii_mask_;

  /// tex_loc_ is for language-specific chars (extra, supplement)
  const BitmapInfo* bmp_loc_{nullptr};
  Texture tex_loc_;
  Texture tex_loc_mask_;
  std::string loc_ttf_cur_path_; // for loc_ttf_ reusing
  stbtt_fontinfo loc_ttf_{};

  /// useful data calculated once and shared across all glyphs
  TtfFontData loc_ttf_data_;

  /// we don't deallocate or invalidate loc_ttf_, created
  /// by stb_true_type.h, but want to know was it provided
  /// (we don't "invalidate" previously loaded font for possible reusing)
  bool loc_ttf_provided_{false};
  std::unique_ptr<unsigned char[]> font_buffer_;
  std::unique_ptr<unsigned char[]> char_buffer_;

  /// if glyph absent in bitmaps, stb_truetype.h render glyph from .ttf here,
  /// then we rerender it to phrases texture
  GLuint tex_char_{0};

  /// firstly we render phrase to this texture and then trying to pack it
  /// as more tightly (closer to top) on tex_phrases_.
  /// (so we always render -> tex_phrase_temp -> then -> tex_phrases_;
  /// this way we reduce repacking calls)
  GLuint tex_phrase_temp_{0};

  GLuint tex_phrases_{0};
  GLuint tex_phrases_packed_{0};
  GLuint tex_phrases_mask_{0}; // processed after base, already repacked

  GLuint fbo_phrases_{0};
  GLuint fbo_phrases_packed_{0};

  GLuint vao_{0};
  GLuint vbo_{0};

  Shader shader_;

  TexPosAdvance advance_;
};

#endif  // WIREBOUNDTEXTBAKER__RENDERER_H_
