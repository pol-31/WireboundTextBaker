#include "Serializing.h"

#include <iomanip>
#include <iostream>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>

#include <cstdlib> // for gperf sys-call

#include <stb_image_write.h>

#include "Details.h"

void WriteBitmapsInfo(std::string_view path,
                      const std::vector<BitmapInfo>& bmps_info,
                      std::string_view old_entries) {
  std::ofstream out_file(path.data());
  if (!out_file) {
    throw std::runtime_error("Failed to generate bitmaps info file");
  }
  out_file.write(details::kBmpsInfoMetadata.data(),
                 details::kBmpsInfoMetadata.size());
  for (const auto& bmp_info : bmps_info) {
    std::string buffer;
    buffer.reserve(1024);
    std::ostringstream oss_text(std::move(buffer));
    oss_text
        << '\n' << bmp_info.name << ';'
        << bmp_info.font_path << ';'
        << bmp_info.size << ";"
        << "\"\n";
    int chars_added = -1;
    for (auto code : bmp_info.chars) {
      if (++chars_added == details::kBitmapColumnNum) {
        oss_text << '\n';
        chars_added = 0;
      }
      oss_text << std::format("0x{:04X},", code);
    }
    oss_text << "\n\";";

    /// .glyph_mapping and .autogen_symbols stored in the same field;
    /// after generation we write .glyph_mapping
    oss_text
        << bmp_info.glyph_mapping.factor << ','
        << bmp_info.glyph_mapping.min << ','
        << bmp_info.glyph_mapping.height << ","
        << bmp_info.glyph_mapping.full_height << ","
        << "\"\n";
    int widths_added = -1;
    for (auto width : bmp_info.glyph_mapping.widths) {
      if (++widths_added == details::kBitmapColumnNum) {
        oss_text << '\n';
        widths_added = 0;
      }
      oss_text << static_cast<int>(width) << ',';
    }
    oss_text << "\n\";" << bmp_info.loc_path << ';';
    out_file << oss_text.str();
  }
  if (!old_entries.empty()) {
    out_file.write("\n", 1);
    out_file.write(
        old_entries.data(), static_cast<std::streamsize>(old_entries.size()));
  }
}

void WriteBitmapsInfo(std::string_view path,
                      const std::vector<BitmapInfo>& bmps_info) {
  return WriteBitmapsInfo(path, bmps_info, {});
}

void GenBmtInfoHeader(const std::vector<BitmapInfo>& bmp_info,
                      std::string_view out_path) {
  std::string buffer;
  buffer.reserve(1024);
  std::ostringstream oss_text(std::move(buffer));

  std::string header_guard;
  header_guard.reserve(details::kBmpInfoHeaderGuard.size() + 3);
  header_guard += details::kBmpInfoHeaderGuard;
  header_guard += "_H_";
  oss_text
      << "#ifndef " << header_guard
      <<  "\n#define " << header_guard
      << "\n\n#include <array>"
      << "\n#include <cinttypes>"
      << "\n#include <string_view>"
      <<  "\n\nnamespace " <<  details::kBmpInfoNamespace << " {"
      << "\n\ninline constexpr int kBitmapTotalChar = " <<
      details::kBitmapTotalChar << ";"
      <<  "\ninline constexpr int kBitmapRowNum = " <<
      details::kBitmapRowNum << ";"
      << "\ninline constexpr int kBitmapColumnNum = " <<
      details::kBitmapColumnNum <<  ";";

  oss_text <<
      "\n\n/// widths packed by 4 bits; how to convert:\n"
      "/// float comp_width;\n"
      "/// if (char_idx & 1) {\n"
      "///   comp_width = static_cast<float>((widths[idx >> 1]) >> 4);\n"
      "/// } else {\n"
      "///   comp_width = static_cast<float>((widths[idx >> 1]) & 0x0F);\n"
      "/// }\n"
      "/// return static_cast<int>(comp_width * factor) + min;";

  oss_text <<
      "\n\nstruct BitmapInfo {\n"
      "  std::string_view name;\n"
      "  std::string_view ttf_path;\n"
      "  std::array<int, 96> chars;\n"
      "  int width_min;\n"
      "  int height;\n"
      "  int full_height;\n"
      "  float width_factor;\n"
      "  std::array<std::uint8_t, 48> widths;\n"
      "};";

  for (const auto& bmp : bmp_info) {
    oss_text
        << "\n\ninline constexpr BitmapInfo k_" << bmp.name << " = {\n"
        << "  \"" << bmp.name << "\",\n"
        << "  \"" << bmp.font_path << "\",\n"
        << "  {\n    ";

    int chars_added = -1;
    for (auto code : bmp.chars) {
      if (++chars_added == details::kBitmapColumnNum) {
        oss_text << "\n    ";
        chars_added = 0;
      }
      oss_text << code << ", ";
    }
    oss_text
        << "\n  },\n  " << bmp.glyph_mapping.min
        << ", " << bmp.glyph_mapping.height
        << ", "  << bmp.glyph_mapping.full_height
        << ", " << bmp.glyph_mapping.factor << ",\n  {\n    ";

    int widths_added = -1;
    for (auto width : bmp.glyph_mapping.widths) {
      if (++widths_added == details::kBitmapColumnNum) {
        oss_text << "\n    ";
        widths_added = 0;
      }
      oss_text << static_cast<int>(width) << ", ";
    }
    oss_text << "\n  }\n};";
  }
  oss_text
      << "\n\n} // namespace " <<  details::kBmpInfoNamespace
      <<  "\n\n#endif  // " <<  header_guard <<  "\n";
  std::ofstream out_file(out_path.data());
  out_file << oss_text.str();
}

inline void PrintFloat(std::ostringstream& ss, float value) {
  ss << value;
  if (static_cast<float>(static_cast<int>(value)) == value) {
    ss << ".0";
  }
  ss << 'f';
}

std::string FormatFloat(float value) {
  if (static_cast<float>(static_cast<int>(value)) == value) {
    return std::format("{:.1f}f", value);
  } else {
    return std::format("{:.4f}f", value);
  }
}

std::string SerializePhrasesTexCoords(
    const std::vector<Rectangle>& tex_coords, std::string_view name) {
  std::string buffer;
  buffer.reserve(1024);
  std::ostringstream oss_text(std::move(buffer));
  /// order according to GL_TRIANGLE_STRIP (4 points in 2d -> 8 floats)
  oss_text << "\ninline constexpr std::array<float, " << tex_coords.size() * 8
           << "> kTexCoordsVbo_" << name << " = {" << std::setprecision(4);
  for (const auto& coords : tex_coords) {
    auto norm_coords = coords.PosToTexCoords();
    oss_text
        << "\n  "
        << FormatFloat(norm_coords.right) << ','
        << FormatFloat(norm_coords.bottom) << ','
        << FormatFloat(norm_coords.right) << ','
        << FormatFloat(norm_coords.top) << ','
        << FormatFloat(norm_coords.left) << ','
        << FormatFloat(norm_coords.bottom) << ','
        << FormatFloat(norm_coords.left) << ','
        << FormatFloat(norm_coords.top) << ',';
  }
  oss_text << "\n};";
  return oss_text.str();
}

void StoreBitmapTexture(std::string_view bmps_dir,
                        const BitmapInfo& bmp_info,
                        unsigned char data[]) {
  /// we've already resolved "already exists" conflicts at ParseBmpsData()
  stbi_write_png(
      std::format("{}/{}.png", bmps_dir, bmp_info.name).c_str(),
      bmp_info.size, bmp_info.size, 1, data, 0);
}

void StorePhrasesTex(Renderer::TextureData& texture_data,
                     std::string_view name,
                     std::string_view phrases_out_dir) {
  stbi_flip_vertically_on_write(true);
  auto out_path = std::format(
      "{}/tex_{}.png", phrases_out_dir, name);
  stbi_write_png(out_path.data(), texture_data.width,
                 texture_data.height, 1, texture_data.data.data(), 0);
  stbi_flip_vertically_on_write(false);
}

void StorePhrasesTexMask(Renderer::TextureData& texture_data,
                         std::string_view name,
                         std::string_view phrases_out_dir) {
  stbi_flip_vertically_on_write(true);
  auto out_path = std::format(
      "{}/tex_mask_{}.png", phrases_out_dir, name);
  stbi_write_png(out_path.c_str(), texture_data.width,
                 texture_data.height, 1, texture_data.data.data(), 0);
  stbi_flip_vertically_on_write(false);
}

std::vector<std::string> CompressUtf8ToAsciiString(
    const PhrasesType& utf8_phrases) {
  std::vector<std::string> ascii_phrases;
  std::string buffer;
  buffer.reserve(128);
  for (const auto& phrase : utf8_phrases) {
    buffer.clear();
    for (auto ch : phrase) {
      int code = static_cast<int>(ch);
      if (code < 128) {
        buffer.push_back(static_cast<char>(code));
      } else {
        throw std::runtime_error(
            std::format("char with code {} doesn't belong "
                        "to ASCII symbols", code));
      }
    }
    ascii_phrases.push_back(buffer);
  }
  return ascii_phrases;
}

void GenHash(const PhrasesType& utf8_phrases,
             std::string_view out_path) {
  std::vector<std::string> ascii_phrases;
  try {
    ascii_phrases= CompressUtf8ToAsciiString(utf8_phrases);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return;
  }
  std::size_t total_length = 0;
  for (const auto& phrase : ascii_phrases) {
    total_length += phrase.size();
  }

  /// $ used as a delimiter, so to allow user to provide ','/'.'
  auto total_phrases =  ascii_phrases.size();
  if (total_phrases > details::kMaxPhrasesAllowed) {
    throw std::runtime_error(
        std::format("too much phrases({}). Only {} is allowed",
        total_phrases, details::kMaxPhrasesAllowed));
  }
  total_length += 3 * ascii_phrases.size(); // for '$','/n' and line_num (<999)
  std::string all_text;
  all_text.reserve(total_length + details::kGpeftHashDeclaration.size());

  /// struct declaration
  all_text += details::kGpeftHashDeclaration;

  for (int i = 0; i < ascii_phrases.size(); ++i) {
    all_text += std::format("{}${}\n", ascii_phrases[i], i);
  }

  auto temp_path = std::format(
      "{}_temp.cpp", out_path);
  std::ofstream temp_file(temp_path);
  temp_file.write(
      all_text.data(), static_cast<std::streamsize>(all_text.size()));
  temp_file.close();

  auto command = std::format(
      "gperf {} {} > {}.h", details::kGperfFlags, temp_path, out_path);
  int err_code = system(command.c_str());
  if (err_code != 0) {
    std::cerr << "gperf command failed with code: " << err_code << std::endl;
  }
  std::filesystem::remove(temp_path);
}

void StoreTexCoords(std::string_view phrases_out_dir,
                    const std::vector<std::string>& tex_coords_string) {
  auto out_path = std::format("{}/PhrasesTexCoords.h", phrases_out_dir);
  std::ofstream out_file(out_path.data());
  if (!out_file) {
    throw std::runtime_error("Failed to generate file with texture coords");
  }
  std::string buffer;
  buffer.reserve(1024);
  std::stringstream oss_text(std::move(buffer));

  std::string header_guard;
  header_guard.reserve(details::kTexCoordsHeaderGuard.size() + 8);
  header_guard += details::kTexCoordsHeaderGuard;
  oss_text
      << "#ifndef " << header_guard
      << "\n#define " << header_guard
      << "\n\n#include <array>"
      << "\n\nnamespace " << details::kBmpInfoNamespace << " {\n";

  for (const auto& entry : tex_coords_string) {
    oss_text << entry << '\n';
  }

  oss_text
      << "\n} // namespace " << details::kBmpInfoNamespace
      << "\n\n#endif  // " << header_guard << "\n";

  out_file << oss_text.str();
}

