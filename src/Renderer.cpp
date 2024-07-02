#include "Renderer.h"

#include <algorithm>
#include <execution>
#include <fstream>
#include <format>
#include <cmath>

#include <utf8.h>

std::vector<Rectangle> Renderer::Process(
    const std::vector<BitmapInfo>& bmps_info,
    const PrerenderData& loc_info,
    std::string_view bmps_dir) {
  ResetLocale();
  try {
    SetAscii(bmps_info, loc_info, bmps_dir);
    SetLocale(bmps_info, loc_info, bmps_dir);
    CheckSetup();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return {};
  }
  glBindVertexArray(vao_);
  shader_.Bind();
  ResetFramebuffers();
  advance_.Reset();

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tex_char_);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, tex_phrase_temp_);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_phrases_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_phrase_temp_, 0);

  auto tex_coords = RenderMainTexture(
      loc_info.phrases, loc_info.scale);
  if (tex_ascii_mask_) {
    RenderMaskTexture(tex_coords, loc_info.phrases, loc_info.scale);
  }
  glBindVertexArray(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return tex_coords;
}

/// not const (changes internal OpenGL data)
[[nodiscard]] Renderer::TextureData Renderer::GetTextureData() {
  TextureData tex;
  glBindTexture(GL_TEXTURE_2D, tex_phrases_packed_);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex.width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex.height);
  tex.data.reserve(tex.width * tex.height * 4);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE,
                tex.data.data());
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

/// not const (changes internal OpenGL data)
[[nodiscard]] Renderer::TextureData Renderer::GetTextureMaskData() {
  TextureData tex;
  glBindTexture(GL_TEXTURE_2D, tex_phrases_mask_);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex.width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex.height);
  tex.data.reserve(tex.width * tex.height * 4);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE,
                tex.data.data());
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

void Renderer::Init() {
  loc_ttf_.userdata = nullptr;
  font_buffer_ = std::make_unique<unsigned char[]>(
      details::kFontBufferSize);
  char_buffer_ = std::make_unique<unsigned char[]>(
      details::kCharMaxResolution * details::kCharMaxResolution);

  InitTextures();
  InitFramebuffers();
  InitBuffers();

  shader_ = Shader("../shaders/Glyph.vert", "../shaders/Glyph.frag");
  shader_.Bind();
  shader_.SetUniform("tex_bitmap_ascii", 0);
  shader_.SetUniform("tex_bitmap_loc", 1);
  shader_.SetUniform("tex_ttf_glyph", 2);
  shader_.SetUniform("tex_temp", 3);
}

void Renderer::InitTextures() {
  GLuint texture[5];
  glGenTextures(5, texture);
  tex_char_ = texture[0];
  tex_phrase_temp_ = texture[1];
  tex_phrases_ = texture[2];
  tex_phrases_mask_ = texture[3];
  tex_phrases_packed_ = texture[4];

  int glyph_size = details::kPrerenderedFramebufferSize;
  int bitmap_size = details::kPrerenderedFramebufferSize;

  SetupGreyscaleTexture(tex_char_, glyph_size, glyph_size);
  SetupGreyscaleTexture(tex_phrase_temp_, bitmap_size, bitmap_size);
  SetupGreyscaleTexture(tex_phrases_, bitmap_size, bitmap_size);
  SetupGreyscaleTexture(tex_phrases_mask_, bitmap_size, bitmap_size);
  SetupGreyscaleTexture(tex_phrases_packed_, bitmap_size, bitmap_size);
}

void Renderer::InitFramebuffers() {
  GLuint fbos[2];
  glGenFramebuffers(2, fbos);
  fbo_phrases_ = fbos[0];
  fbo_phrases_packed_ = fbos[1];
  SetupFramebuffer(fbo_phrases_, tex_phrases_);
  SetupFramebuffer(fbo_phrases_packed_, tex_phrases_packed_);
}

void Renderer::InitBuffers() {
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  /// GL_TRIANGLE_STRIP: 2 for pos, 2 for coords * 4 vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4,
               nullptr, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        reinterpret_cast<void*>(8 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);
}

void Renderer::DeInit() {
  GLuint textures[5] = {tex_char_, tex_phrase_temp_, tex_phrases_,
                        tex_phrases_mask_, tex_phrases_packed_};
  GLuint fbos[2] = {fbo_phrases_, fbo_phrases_packed_};
  glDeleteTextures(5, textures);
  glDeleteFramebuffers(2, fbos);
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}

void Renderer::ResetLocale() {
  bmp_ascii_ = nullptr;
  tex_ascii_ = Texture();
  tex_ascii_mask_ = Texture();
  bmp_loc_ = nullptr;
  tex_loc_ = Texture();
  tex_loc_mask_ = Texture();
}

/// not const (changes internal OpenGL data)
void Renderer::ResetFramebuffers() {
  /// ORDER: mask, then default (so default is last binded)
  ClearFramebufferTexture(fbo_phrases_, tex_phrase_temp_);
  ClearFramebufferTexture(fbo_phrases_, tex_phrases_mask_);
  ClearFramebufferTexture(fbo_phrases_, tex_phrases_);
  ClearFramebufferTexture(fbo_phrases_packed_, tex_phrases_packed_);
}

void Renderer::CheckSetup() {
  auto size = tex_ascii_.GetSize();
  if (tex_ascii_mask_ && size != tex_ascii_mask_.GetSize()) {
    throw std::runtime_error("ascii base and mask bitmaps sizes differ");
  }
  if (tex_loc_ && size != tex_loc_.GetSize()) {
    throw std::runtime_error("ascii base and loc bitmaps sizes differ");
  }
  if (tex_loc_mask_ && size != tex_loc_mask_.GetSize()) {
    throw std::runtime_error("ascii base and loc mask bitmaps sizes differ");
  }
}

void Renderer::SetupFramebuffer(GLuint fbo_id, GLuint tex_id) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_id, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw std::runtime_error("Framebuffer is not complete!");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupGreyscaleTexture(
    GLuint tex_id, int tex_width, int tex_height) {
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height,
               0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::ClearFramebufferTexture(GLuint fb_id, GLuint tex_id) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_id, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetAscii(
    const std::vector<BitmapInfo>& bmps_info,
    const PrerenderData& loc_info,
    std::string_view bmps_dir) {
  for (const auto& bmp_info : bmps_info) {
    if (loc_info.name_bmp_ascii == bmp_info.name) {
      bmp_ascii_ = &bmp_info;
      break;
    }
  }
  if (!bmp_ascii_) {
    throw std::runtime_error("no ascii bmp provided");
  }

  std::string bmp_ascii_path;
  if (!loc_info.name_bmp_ascii.empty()) {
    bmp_ascii_path = std::format(
        "{}/{}.png", bmps_dir, loc_info.name_bmp_ascii);
  }
  tex_ascii_ = Texture(bmp_ascii_path);
  std::string bmp_ascii_mask_path;
  if (!loc_info.name_bmp_ascii_mask.empty()) {
    bmp_ascii_mask_path = std::format(
        "{}/{}.png", bmps_dir, loc_info.name_bmp_ascii_mask);
  }
  tex_ascii_mask_ = Texture(bmp_ascii_mask_path);
}

void Renderer::SetLocale(
    const std::vector<BitmapInfo>& bmps_info,
    const PrerenderData& loc_info,
    std::string_view bmps_dir) {
  for (const auto& bmp_info : bmps_info) {
    if (loc_info.name_bmp_loc == bmp_info.name) {
      bmp_loc_ = &bmp_info;
      break;
    }
  }

  std::string bmp_loc_path;
  if (!loc_info.name_bmp_loc.empty()) {
    bmp_loc_path = std::format(
        "{}/{}.png", bmps_dir, loc_info.name_bmp_loc);
  }
  tex_loc_ = Texture(bmp_loc_path);
  std::string bmp_loc_mask_path;
  if (!loc_info.name_bmp_loc_mask.empty()) {
    bmp_loc_mask_path = std::format(
        "{}/{}.png", bmps_dir, loc_info.name_bmp_loc_mask);
  }
  tex_loc_mask_ = Texture(bmp_loc_mask_path);

  /// ttf file
  loc_ttf_provided_ = !loc_info.path_ttf_loc.empty();
  if (!loc_ttf_provided_) {
    return;
  }
  if (loc_ttf_cur_path_ == loc_info.path_ttf_loc) {
    return; // already set, so we can reuse from memory
  }
  loc_ttf_cur_path_ = loc_info.path_ttf_loc;
  std::ifstream font_file(loc_info.path_ttf_loc.data(), std::ios::binary);
  if (!font_file) {
    throw std::runtime_error(std::format(
        "failed to open font file: {}", loc_info.path_ttf_loc));
  }
  auto font_buffer_ptr = font_buffer_.get();
  font_file.read(reinterpret_cast<char*>(font_buffer_ptr),
                 details::kFontBufferSize);
  if (font_file.gcount() == 0 && font_file.fail()) {
    throw std::runtime_error(std::format(
        "failed to read font file: {}", loc_info.path_ttf_loc));
  }
  if (!stbtt_InitFont(&loc_ttf_, font_buffer_ptr,
                      stbtt_GetFontOffsetForIndex(font_buffer_.get(), 0))) {
    throw std::runtime_error("font initialization error");
  }

  int cell_size = details::kCharMaxResolution;
  loc_ttf_data_.ttf_scale = stbtt_ScaleForPixelHeight(
      &loc_ttf_, static_cast<float>(cell_size));
  int ascent, descent, line_gap;
  stbtt_GetFontVMetrics(&loc_ttf_, &ascent, &descent, &line_gap);
  loc_ttf_data_.glyph_height = static_cast<int>(
      loc_ttf_data_.ttf_scale * static_cast<float>(ascent - descent));
  loc_ttf_data_.y_adjust = static_cast<int>(static_cast<float>(descent)
                                            * loc_ttf_data_.ttf_scale);
}

std::vector<Rectangle> Renderer::RenderMainTexture(
    const PhrasesType& phrases, int text_size) {
  glActiveTexture(GL_TEXTURE0);
  tex_ascii_.Bind();
  glActiveTexture(GL_TEXTURE1);
  tex_loc_.Bind();
  std::vector<Rectangle> tex_coords;
  tex_coords = ProcessImpl(phrases, text_size);
  return tex_coords;
}

void Renderer::RenderMaskTexture(
    const std::vector<Rectangle>& tex_coords,
    const PhrasesType& phrases,
    int text_size) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_phrases_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_phrases_mask_, 0);
  glActiveTexture(GL_TEXTURE0);
  tex_ascii_mask_.Bind();
  glActiveTexture(GL_TEXTURE1);
  tex_loc_mask_.Bind();
  ProcessMaskImpl(tex_coords, phrases, text_size);
}

std::vector<Rectangle> Renderer::GetRepackOrder(
    const std::vector<Rectangle>& tex_coords) {
  if (tex_coords.size() <= 1) {
    return tex_coords;
  }
  std::vector<IdxRectangle> tex_coords_sorted;
  for (int i = 0; i < tex_coords.size(); ++i) {
    tex_coords_sorted.push_back({tex_coords[i], i});
  }

  /// non-ascending order
  std::sort(std::execution::par_unseq,
            tex_coords_sorted.begin(), tex_coords_sorted.end(),
            [](const IdxRectangle& a, const IdxRectangle& b) {
              return a.rect.Width() > b.rect.Width();
            });
  std::vector<Rectangle> repacked_tex_coords;
  repacked_tex_coords.reserve(tex_coords.size());

  /// all rectangles have the same height

  /// always more than 1 element (bail out at the beginning)
  float cur_row_length;
  std::vector<int> idx_order;
  idx_order.reserve(tex_coords.size());
  while (!tex_coords_sorted.empty()) {
    std::vector<IdxRectangle> tex_coords_left;
    tex_coords_left.reserve(tex_coords.size());
    cur_row_length = 0; // new line
    for (auto it = tex_coords_sorted.begin();
         it != tex_coords_sorted.end(); ++it) {
      /// bail out if there's no way to append current row
      if (cur_row_length + tex_coords_sorted.back().rect.Width() > 2.0f) {
        tex_coords_left.insert(tex_coords_left.end(), it,
                               tex_coords_sorted.end());
        break;
      }
      float phrase_width = it->rect.Width();
      if (phrase_width > 2.0f) {
        throw std::runtime_error("width > 2.0f, need to normalize");
      }
      if (cur_row_length + phrase_width <= 2.0f) {
        cur_row_length += phrase_width;
        repacked_tex_coords.push_back(it->rect);
        idx_order.push_back(it->idx);
      } else {
        tex_coords_left.push_back(*it);
      }
    }
    tex_coords_sorted = std::move(tex_coords_left);
  }

  /// reassign updated tex coords
  float height = repacked_tex_coords[0].Height(); // arbitrary
  int row_idx = 0;
  float current_row_length = 0;
  float width;
  for (auto& rect : repacked_tex_coords) {
    width = rect.Width();
    if (current_row_length + width <= 2.0f) {
      rect.left = current_row_length - 1; // [0;2] -> [-1;1]
      current_row_length += width;
    } else {
      ++row_idx;
      rect.left = -1.0f;
      current_row_length = width;
    }
    rect.top = 1.0f - height * static_cast<float>(row_idx);
    rect.bottom = 1.0f - height * (static_cast<float>(row_idx) + 1.0f);
    rect.right = current_row_length - 1; // [0;2] -> [-1;1]
  }

  /// reorder to the same order as in the beginning
  std::vector<Rectangle> repacked_reordered_tex_coords(tex_coords.size());
  for (std::size_t i = 0; i < tex_coords.size(); ++i) {
    repacked_reordered_tex_coords[idx_order[i]] = repacked_tex_coords[i];
  }
  return repacked_reordered_tex_coords;
}

std::vector<Rectangle> Renderer::Repack(
    const std::vector<Rectangle>& positions) const {
  auto positions_new = GetRepackOrder(positions);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_phrases_);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_phrases_, 0);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_phrases_packed_);

  int tex_w_int, tex_h_int;
  GLuint tex_id;
  glGetFramebufferAttachmentParameteriv(
      GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex_w_int);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_h_int);
  auto tex1_w = static_cast<float>(tex_w_int);
  auto tex1_h = static_cast<float>(tex_h_int);

  glGetFramebufferAttachmentParameteriv(
      GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex_w_int);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_h_int);
  auto tex2_w = static_cast<float>(tex_w_int);
  auto tex2_h = static_cast<float>(tex_h_int);

  if (positions.size() != positions_new.size()) {
    throw std::runtime_error("tex_pos.size() != tex_pos_repacked.size()");
  }

  /// [-1;1] -> [0;1] -> pixels coords
  /// x0 y0 are inclusive; x1, y1 are exclusive (added some adjustment)
  for (int i = 0; i < positions.size(); ++i) {
    auto rect_old = positions[i].PosToTexCoords();
    auto rect_new = positions_new[i].PosToTexCoords();
    int src_x0 = static_cast<int>(rect_old.left * tex1_w + 1);
    int src_y0 = static_cast<int>(rect_old.bottom * tex1_h + 2);
    int src_x1 = static_cast<int>(rect_old.right * tex1_w);
    int src_y1 = static_cast<int>(rect_old.top * tex1_h);
    int dst_x0 = static_cast<int>(rect_new.left * tex2_w + 1);
    int dst_y0 = static_cast<int>(rect_new.bottom * tex2_h + 2);
    int dst_x1 = static_cast<int>(rect_new.right * tex2_w);
    int dst_y1 = static_cast<int>(rect_new.top * tex2_h);
    glBlitFramebuffer(src_x0, src_y0, src_x1, src_y1,
                      dst_x0, dst_y0, dst_x1, dst_y1,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return positions_new;
}

bool Renderer::RerenderToMain(std::vector<Rectangle>& tex_coords,
                              bool already_repacked) {
  if (tex_coords.empty()) {
    throw std::runtime_error("TextTraits::RerenderToMain() empty tex_coords");
  }
  auto& prev = tex_coords.back();
  float width = prev.Width();
  float height = prev.Height();
  Rectangle rect_new;
  if (tex_coords.size() == 1) {
    rect_new = prev;
    rect_new.right = std::min(rect_new.left + width, 1.0f);
  } else if (const auto& before_prev = tex_coords[tex_coords.size() - 2];
             before_prev.right + width < 1.0f) {
    rect_new.left = before_prev.right;
    rect_new.right = std::min(rect_new.left + width, 1.0f);
    rect_new.top = before_prev.top;
    rect_new.bottom = before_prev.bottom;
  } else {
    advance_.TrySetX(-1.0f);
    if (!advance_.TrySetY(advance_.GetY() - height)) {
      if (!already_repacked) {
        prev.right = std::min(1.0f, prev.right);
        tex_coords = Repack(tex_coords);
        return RerenderToMain(tex_coords, true);
      } else {
        tex_coords.pop_back();
        return false;
      }
    }
    rect_new.left = advance_.GetX();
    rect_new.top = advance_.GetY();
    rect_new.right = std::min(rect_new.left + width, 1.0f);
    rect_new.bottom = rect_new.top - height;
  }
  Rectangle rect_old_coords = prev.PosToTexCoords();
  auto render_data = ConvertToTriangleStrip(
      rect_new.left, rect_new.right,
      rect_new.top, rect_new.bottom,
      rect_old_coords.left, rect_old_coords.right,
      rect_old_coords.top, rect_old_coords.bottom);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, kTriangleStripSize, render_data.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  shader_.SetUniform("tex_type", // GL_TEXTURE3 tex_phrase_temp_
                     static_cast<int>(TexType::kTemp));
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_phrases_, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  /// set the same texture as it was before
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex_phrase_temp_, 0);
  prev = rect_new;
  return true;
}

[[nodiscard]] Renderer::TexPosAdvance Renderer::RenderGlyph(
    int idx, int tex_size, int width, int height,
    float pos_left, float pos_top, float scale) const {
  static_assert(details::kBitmapColumnNum == 8);
  static_assert(details::kBitmapRowNum == 12);
  int row = idx >> 3; // idx / num_columns for 8
  int column = idx & 7; // idx % num_columns for 8

  // dev note: possible constexpr precalculate
  int cell_width = tex_size / details::kBitmapColumnNum;
  int cell_height = tex_size / details::GetRenderRowNum();

  auto tex_left = static_cast<float>(column * cell_width);
  auto tex_top = static_cast<float>(tex_size - row * cell_height);
  float tex_right = tex_left + static_cast<float>(width);
  float tex_bottom = tex_top - static_cast<float>(height);

  // dev note: possible constexpr precalculate OR
  // cast<int>, bitshift if size is pow of 2
  /// normalizing to [0;1]
  tex_left /= static_cast<float>(tex_size);
  tex_top /= static_cast<float>(tex_size);
  tex_right /= static_cast<float>(tex_size);
  tex_bottom /= static_cast<float>(tex_size);

  /// positions already at [-1;+1]
  auto pos_right = pos_left + static_cast<float>(width) * scale;
  auto pos_bottom = pos_top - static_cast<float>(height) * scale;

  auto render_data = ConvertToTriangleStrip(
      pos_left, pos_right, pos_top, pos_bottom,
      tex_left, tex_right, tex_top, tex_bottom);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, kTriangleStripSize, render_data.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  return {pos_right, pos_bottom};
}

Renderer::TexPosAdvance Renderer::RenderGlyphTtf(
    int code, int height, float pos_left, float pos_top, float scale) {
  /// background to black
  std::memset(char_buffer_.get(), 0, details::kCharMaxResolution
                                         * details::kCharMaxResolution);
  int cell_size = details::kCharMaxResolution;
  int g = stbtt_FindGlyphIndex(&loc_ttf_, code);
  if (g == 0) {
    /// common case: \t (code 9)
    std::cerr << "font doesn't contain glyph with idx " << code << std::endl;
  }

  int advance, lsb, x0, y0, x1, y1;
  stbtt_GetGlyphHMetrics(&loc_ttf_, g, &advance, &lsb);
  stbtt_GetGlyphBitmapBox(&loc_ttf_, g, loc_ttf_data_.ttf_scale,
                          loc_ttf_data_.ttf_scale, &x0, &y0, &x1, &y1);

  /// descent often is negative, while ascend is positive number
  /// (relative to baseline), so to get correct indent we want to convert:
  /// (e.g.) [1024;-200] -> [1024;0]
  int indent_y = loc_ttf_data_.glyph_height + y0 + loc_ttf_data_.y_adjust;
  auto indent_x = static_cast<int>(
      std::round(loc_ttf_data_.ttf_scale * static_cast<float>(lsb)));
  stbtt_MakeGlyphBitmap(
      &loc_ttf_, char_buffer_.get() + indent_x + indent_y * cell_size,
      cell_size - indent_x, cell_size - indent_y,
      cell_size, loc_ttf_data_.ttf_scale, loc_ttf_data_.ttf_scale, g);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, tex_char_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, details::kCharMaxResolution,
               details::kCharMaxResolution, 0, GL_RED, GL_UNSIGNED_BYTE,
               char_buffer_.get());

  auto glyph_width = static_cast<int>(loc_ttf_data_.ttf_scale
                                      * static_cast<float>(advance));
  auto pos_bottom = pos_top - static_cast<float>(height) * scale;

  /// we don't know widths (comparing to bitmaps), but do know height,
  /// so can calculate it to find correct width
  float ascept_ratio = static_cast<float>(loc_ttf_data_.glyph_height)
                       / static_cast<float>(glyph_width);
  auto pos_right =
      pos_left + (static_cast<float>(height) / ascept_ratio) * scale;

  auto tex_top = 0.0f;
  auto tex_bottom = 1.0f;
  auto tex_right = 1.0f;
  auto tex_left = 0.0f;

  auto render_data = ConvertToTriangleStrip(
      pos_left, pos_right, pos_top, pos_bottom,
      tex_left, tex_right, tex_top, tex_bottom);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, kTriangleStripSize, render_data.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  return {pos_right, pos_bottom};
}

std::vector<Rectangle> Renderer::ProcessImpl(
    const PhrasesType& phrases, int text_size) {
  std::vector<Rectangle> tex_coords;
  int code, char_idx, char_width, char_height, tex_size;
  TexPosAdvance char_advance;
  float scale = details::ConvertScale(text_size);
  for (int i = 0; i < phrases.size(); ++i) {
    advance_.TrySetX(-1.0f);
    for (auto ch : phrases[i]) {
      code = static_cast<int>(ch);
      /// sett Y-coord only at RerenderToMain()
      if (IsAsciiBitmap(code)) {
        shader_.SetUniform("tex_type",
                           static_cast<int>(TexType::kAscii));
        char_idx = code - details::kAsciiFirstChar;
        char_width = bmp_ascii_->glyph_mapping.GetWidthByIdx(char_idx);
        char_height = bmp_ascii_->glyph_mapping.height;
        tex_size = bmp_ascii_->size;
        char_advance = RenderGlyph(
            char_idx, tex_size, char_width, char_height,
            advance_.GetX(), 1.0f, scale);
        if (!advance_.TrySetX(char_advance.GetX())) {
          std::cerr << "overflow on X for phrase #" << i << std::endl;
          break;
        }
        continue;
      }
      if (bmp_loc_) {
        auto found = std::lower_bound(
             bmp_loc_->chars.begin(),
             bmp_loc_->chars.end(), code);
        if (found != bmp_loc_->chars.end() &&
            *found == code) {
          shader_.SetUniform("tex_type",
                             static_cast<int>(TexType::kLangSpec));
          char_idx = static_cast<int>(
              std::distance(bmp_loc_->chars.begin(), found));
          char_width = bmp_loc_->glyph_mapping.GetWidthByIdx(char_idx);
          char_height = bmp_loc_->glyph_mapping.height;
          tex_size = bmp_loc_->size;
          char_advance = RenderGlyph(
              char_idx, tex_size, char_width, char_height,
              advance_.GetX(), 1.0f, scale);
          if (!advance_.TrySetX(char_advance.GetX())) {
            std::cerr << "overflow on X for phrase #" << i << std::endl;
            break;
          }
          continue;
        }
      }
      if (loc_ttf_provided_) {
        shader_.SetUniform("tex_type",
                           static_cast<int>(TexType::kTtfGlyph));
        /// all metrics across all bitmaps for the same loc file are the same
        char_advance = RenderGlyphTtf(code, bmp_ascii_->glyph_mapping.height,
                                      advance_.GetX(), 1.0f, scale);
        if (!advance_.TrySetX(char_advance.GetX())) {
          std::cerr << "overflow on X for phrase #" << i << std::endl;
          break;
        }
        continue;
      }
      std::cerr << "char code " << code
                << " doesn't have glyph for render" << std::endl;
    }
    tex_coords.push_back({-1.0f, char_advance.GetX(),
                          1.0f, char_advance.GetY()});
    if (!RerenderToMain(tex_coords)) {
      std::cerr << "overflow on Y for phrase #" << i << std::endl;
      break;
    }
  }
  return Repack(tex_coords);
}

/// no need to duplicate error messages: main font has the same metrics and
/// is something was wrong, user already got info
void Renderer::ProcessMaskImpl(
    const std::vector<Rectangle>& tex_coords,
    const PhrasesType& phrases,
    int text_size) {
  int code, char_idx, char_width, char_height, tex_size;
  TexPosAdvance char_advance;
  float scale = details::ConvertScale(text_size);
  for (int i = 0; i < phrases.size(); ++i) {
    advance_.TrySetX(tex_coords[i].left);
    advance_.TrySetY(tex_coords[i].top);
    const auto& phrase = phrases[i];
    for (auto ch : phrase) {
      code = static_cast<int>(ch);
      /// sett Y-coord only at RerenderToMain()
      if (IsAsciiBitmap(code)) {
        shader_.SetUniform("tex_type",
                           static_cast<int>(TexType::kAscii));
        char_idx = code - details::kAsciiFirstChar;
        char_width = bmp_ascii_->glyph_mapping.GetWidthByIdx(char_idx);
        char_height = bmp_ascii_->glyph_mapping.height;
        tex_size = bmp_ascii_->size;
        char_advance = RenderGlyph(
            char_idx, tex_size, char_width, char_height,
            advance_.GetX(), advance_.GetY(), scale);
        if (!advance_.TrySetX(char_advance.GetX())) {
          break;
        }
        continue;
      }
      if (bmp_loc_) {
        auto found = std::lower_bound(
             bmp_loc_->chars.begin(),
             bmp_loc_->chars.end(), char_idx);
        if (found != bmp_loc_->chars.end() &&
            *found == code) {
          shader_.SetUniform("tex_type",
                             static_cast<int>(TexType::kLangSpec));
          char_idx = static_cast<int>(found - bmp_loc_->chars.begin());
          char_width = bmp_loc_->glyph_mapping.GetWidthByIdx(char_idx);
          char_height = bmp_loc_->glyph_mapping.height;
          tex_size = bmp_loc_->size;
          char_advance = RenderGlyph(
              char_idx, tex_size, char_width, char_height,
              advance_.GetX(), advance_.GetY(), scale);
          if (!advance_.TrySetX(char_advance.GetX())) {
            break;
          }
          continue;
        }
      }
      if (loc_ttf_provided_) {
        shader_.SetUniform("tex_type",
                           static_cast<int>(TexType::kTtfGlyph));
        /// all metrics across all bitmaps for the same loc file are the same
        char_advance = RenderGlyphTtf(
            code, bmp_ascii_->glyph_mapping.height,
            advance_.GetX(), advance_.GetY(), scale);
        if (!advance_.TrySetX(char_advance.GetX())) {
          break;
        }
        continue;
      }
      std::cerr << "glyph " << code
                << " doesn't have glyph for render" << std::endl;
    }
  }
}
