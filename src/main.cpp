#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <format>

#include <stb_image.h>
#include <stb_truetype.h>

#include "Details.h"
#include "Parsing.h"
#include "Serializing.h"
#include "SetupWindow.h"
#include "Renderer.h"

/// NOTE 1:
/// in CLion there's bug related to incorrect error detecting:
/// https://youtrack.jetbrains.com/issue/CPP-38778/CLion-with-gcc-14.1-warns-on-stdformat

/// NOTE 2:
/// at the time of writing the code, <execution> was not supported,
/// but std::execution::par_unseq should be safe according to cppreference
/// (anyway we use only one thread and all components are "independent")

/// NOTE 3:
/// all images will be generated in .png format

/// Dictionary:
/// loc - localization
/// bmp - bitmap
/// name - no parent path, no extension
/// filename - no parent path
/// path - all: name, extension and parent path

/// symbols drawn from left upper corner
/// (left bearing & indent from the top included)
GlyphMapping RenderGlyphs(
    unsigned char *font_data, unsigned char *tex_data,
    int tex_width, int tex_height, const std::vector<int>& chars);

GlyphMapping CompressWidths(const GlyphMapping& glyph_mapping);

/// returns path to generated bitmap_info file
std::string GenerateBitmaps(std::string_view bmp_info_path,
                            std::string_view bmps_dir);

bool IsPhrasesValid(const PhrasesType& phrases);

void PrerenderPhrases(std::string_view bmps_info_path,
                      std::string_view localizations_dir,
                      std::string_view phrases_out_dir,
                      std::string_view hash_loc_name);

void PrintFailure();

void PrintHelp();

/** Possible cases:
 * 1) generate only bitmaps (2 args):
 *  - bitmap_info.csv (should exist)
 *  - dir for generated output
 * 2) generate only phrases (4 args):
 *  - bitmap_info.csv (should exist)
 *  - dir with localization files
 *  - dir for generated output
 *  - en localization name (no extension or parent path)
 *      (or just ascii) for hash generation
 * 3) all (5 args):
 *  - bitmap_info.csv (should exist)
 *  - dir for generated output (bitmaps)
 *  - dir with localization files
 *  - dir for generated output (prerendered phrases & hash)
 *  - en localization file name (no extension or parent path)
 *      (or just ascii) for hash generation
 *
 *  EXPLANATION:
 *  bitmap_info.csv contains all info for bitmap generation
 *  dir for bitmaps generation
 *    - if bitmap_info.csv already exist there: merge OR replace;
 *    - if bitmap already exists: replace / skip request to user
 *  dir for prerendered data generation
 *    - no replace request (always replace if already exists)
 *    - hash headers, prerendered phrases (.png and .h for coordinates
 *      and further including to main application)
 *  en localization file name
 *    - used for generating of the hash (gperf) (we generate hash only
 *      for english localization, and seatch always by English localization
 *      file, and only then render by line id (all loc files has the same
 *      phrases <line_num :: phrase> mapping)
 * */
int main(int argc, char* argv[]) {
  SetupWindow();
  stbi_set_flip_vertically_on_load(true);
  switch (argc) {
    case 2:
      if (std::string{argv[0]} == "-h") {
        PrintHelp();
      } else {
        PrintFailure();
      }
      break;
    case 3: /// only bitmaps generation
      if (!std::filesystem::exists(argv[1])) {
        PrintFailure();
        return 1;
      }
      std::filesystem::create_directories(argv[2]);
      GenerateBitmaps(argv[1], argv[2]);
      break;
    case 5: /// only phrases prerendering
      if (!std::filesystem::exists(argv[1])) {
        PrintFailure();
        return 1;
      }
      if (!std::filesystem::exists(argv[2])) {
        std::cout << "no localization files provided, skip" << std::endl;
        break;
      }
      std::filesystem::create_directories(argv[3]);
      PrerenderPhrases(argv[1], argv[2], argv[3], argv[4]);
      /// generate .h file for full embedding to Wirebound
      break;
    case 6: /// both bitmaps generation and phrases prerendering
      if (!std::filesystem::exists(argv[1])) {
        PrintFailure();
        return 1;
      }
      if (!std::filesystem::exists(argv[3])) {
        std::cout << "no localization files provided, skip" << std::endl;
        break;
      }
      std::filesystem::create_directories(argv[2]);
      std::filesystem::create_directories(argv[4]);
      {
        auto generated_bmps_info = GenerateBitmaps(argv[1], argv[2]);
        PrerenderPhrases(generated_bmps_info, argv[3], argv[4], argv[5]);
      }
      break;
    default:
      std::cerr
          << "incorrect program arguments, use \'-h\' for help"
          << std::endl;
      return 1;
  }
  return 0;
}

GlyphMapping RenderGlyphs(
    unsigned char *font_data, unsigned char *tex_data,
    int tex_width, int tex_height, const std::vector<int>& chars) {
  stbtt_fontinfo f;
  f.userdata = nullptr;
  int font_offset = stbtt_GetFontOffsetForIndex(font_data, 0);
  if (!stbtt_InitFont(&f, font_data, font_offset)) {
    throw std::runtime_error("font hasn't been initialized");
  }
  std::memset(tex_data, 0, tex_width*tex_height); // paint black

  int cell_width = tex_width / details::kBitmapColumnNum;
  int cell_height = tex_height / details::GetRenderRowNum();

  int pixel_height = std::min(cell_width, cell_height);
  float scale =
      stbtt_ScaleForPixelHeight(&f, static_cast<float>(pixel_height));
  int offset_x = 0;
  int offset_y = 0;
  GlyphMapping glyph_mapping;
  int ascent, descent, line_gap;
  stbtt_GetFontVMetrics(&f, &ascent, &descent, &line_gap);
  glyph_mapping.height = static_cast<int>(
      scale * static_cast<float>(ascent - descent));
  glyph_mapping.full_height = static_cast<int>(
      scale * static_cast<float>(ascent - descent + line_gap));

  for (auto ch : chars) {
    int advance, lsb, x0, y0, x1, y1;
    int g = stbtt_FindGlyphIndex(&f, ch);
    stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
    stbtt_GetGlyphBitmapBox(&f, g, scale,scale, &x0, &y0, &x1, &y1);

    if (offset_x + cell_width > tex_width) {
      offset_y += cell_height;
      offset_x = 0;
    }
    if (offset_y + cell_height > tex_height) {
      std::cerr << "bitmap texture is too small, skip the rest" << std::endl;
      break;
    }

    /// descent often is negative, while ascend is positive number
    /// (relative to baseline), so to get correct indent we want to convert:
    /// (e.g.) [1024;-200] -> [1024;0]
    auto y_adjust = static_cast<int>(static_cast<float>(descent) * scale);
    int indent_y = glyph_mapping.height + y0 + y_adjust;
    auto indent_x = static_cast<int>(
        std::round(scale * static_cast<float>(lsb)));
    if (g != 0) {
      int start_pixel = indent_x + offset_x
                        + (offset_y + indent_y) * tex_width;
      stbtt_MakeGlyphBitmap(
          &f, tex_data + start_pixel, cell_width, cell_height,
          tex_width, scale, scale, g);
    }
    offset_x += cell_width;
    auto glyph_width = static_cast<int>(scale * static_cast<float>(advance));
    if (glyph_width > std::pow(2, 8) - 1) {
      throw std::runtime_error(std::format(
          "glyph with code {} can't be packed to 8 bit", ch));
    }
    glyph_mapping.widths.push_back(glyph_width);
  }
  return glyph_mapping;
}

GlyphMapping CompressWidths(const GlyphMapping& glyph_mapping) {
  std::size_t original_size = glyph_mapping.widths.size();
  if (original_size == 0) {
    std::cerr << "no widths provided for compressing" << std::endl;
    return glyph_mapping;
  }
  auto minmax = std::minmax_element(
      glyph_mapping.widths.begin(), glyph_mapping.widths.end());
  /// iterators are always valid (we've already bailed out in case of size <=1)
  int min_width = *minmax.first;
  int max_width = *minmax.second;
  auto factor = static_cast<float>(max_width - min_width) / 15.0f;

  std::vector<std::uint8_t> normalized_widths; // now [0;15]
  /// if is not even we add 1
  normalized_widths.reserve(original_size + (glyph_mapping.widths.size() & 1));
  for (auto w : glyph_mapping.widths) {
    auto scaled_width = static_cast<std::uint8_t>(
        static_cast<float>(w - min_width) / factor);
    if (scaled_width > (std::pow(2, 4) - 1)) {
      throw std::runtime_error("can't pack width to uint8_t");
    }
    normalized_widths.push_back(scaled_width);
  }
  /// if initially amount of chars is even number, we add dummy, which don't
  /// affect calculation and in rendering will point to black square (no glyph)
  if (original_size & 1) {
    normalized_widths.push_back(normalized_widths[0]);
  }

  std::vector<std::uint8_t> packed4bit_widths;
  packed4bit_widths.reserve((normalized_widths.size() >> 1) + 1);
  /// 1 - 0 - 3 - 2 - 5 - 4 - 7 - 6 ...
  for (size_t i = 0; i < normalized_widths.size(); i += 2) {
    std::uint8_t packed_values = 0;
    packed_values |= static_cast<std::uint8_t>(
        normalized_widths[i] & 0x0F);
    packed_values |= static_cast<std::uint8_t>(
        (normalized_widths[i + 1] & 0x0F) << 4);
    packed4bit_widths.push_back(packed_values);
  }
  return {std::move(packed4bit_widths), factor,
          min_width, glyph_mapping.height, glyph_mapping.full_height};
}

std::string GenerateBitmaps(std::string_view bmp_info_path,
                            std::string_view bmps_dir) {
  bool merge_files = false;
  std::filesystem::path bmp_info_out(bmps_dir);
  bmp_info_out /= std::filesystem::path{bmp_info_path}.filename();
  if (std::filesystem::exists(bmp_info_out)) {
    std::cout << bmp_info_out << " already exists.\n"
              << "Do you want to replace(r), merge(m) or skip?\n" << std::endl;
    char respond;
    std::cin >> respond;
    /// for 'r' we simply rewrite with std::ofstream creation by path
    if (respond == 'm') {
      merge_files = true;
    } else if (respond != 'r') {
      std::cout << "Skip bitmap generation for " << bmp_info_out << std::endl;
      return {};
    }
  }

  auto bmp_info_out_str = bmp_info_out.string();
  std::cout << "...Bitmaps info parsing" << std::endl;
  auto bmps_info = ParseBmpsData(bmps_dir, bmp_info_path, true);
  std::cout << "->Bitmaps info parsing is completed" << std::endl;

  AutogenChars(bmps_info);
  auto font_buffer = std::make_unique<unsigned char[]>(
      details::kFontBufferSize);
  auto tex_buffer = std::make_unique<unsigned char[]>(
      details::kBitmapMaxResolution * details::kBitmapMaxResolution);

  std::cout << "...Glyph rendering" << std::endl;
  for (auto& bmp_info : bmps_info) {
    ReadFontFile(bmp_info.font_path, font_buffer.get());
    auto glyph_mapping = RenderGlyphs(
        font_buffer.get(), tex_buffer.get(),
        bmp_info.size, bmp_info.size, bmp_info.chars);
    if (glyph_mapping.widths.empty()) {
      std::cerr << "no chars to render for "
                << bmp_info.name << std::endl;
      continue;
    }
    StoreBitmapTexture(bmps_dir, bmp_info, tex_buffer.get());
    bmp_info.glyph_mapping = CompressWidths(glyph_mapping);
  }
  std::cout << "->Glyph rendering is completed" << std::endl;


  std::cout << "...Textures creation" << std::endl;
  if (!merge_files) {
    WriteBitmapsInfo(bmp_info_out_str, bmps_info);
  } else {
    std::cout << "Merging..." << std::endl;
    auto old_entries = ParseOldBmpsData(bmps_info, bmp_info_out.string());
    std::size_t total_chars = 0;
    for (const auto& entry : old_entries) {
      total_chars += entry.size();
    }
    std::string old_entries_together;
    old_entries_together.reserve(total_chars);
    for (const auto& entry : old_entries) {
      old_entries_together += entry;
    }
    WriteBitmapsInfo(bmp_info_out_str, bmps_info,
                     old_entries_together);
  }
  std::cout << "->Textures creation is completed" << std::endl;

  /// generate .h file for full embedding to Wirebound
  std::cout << "Generating the header file..." << std::endl;
  std::filesystem::path header_path(bmps_dir);
  header_path /= details::kBmpInfoHeaderFilename;
  GenBmtInfoHeader(bmps_info, header_path.string());

  std::cout << "->Generation of bitmaps is completed" << std::endl;
  return bmp_info_out_str;
}

// std::ranges::all_of(phrases)
bool IsPhrasesValid(const PhrasesType& phrases) {
  for (const auto& phrase : phrases) {
    if (phrase.empty()) {
      return false;
    }
  }
  return true;
}

void PrerenderPhrases(std::string_view bmps_info_path,
                      std::string_view localizations_dir,
                      std::string_view phrases_out_dir,
                      std::string_view hash_loc_name) {
  /// first three rows of bmp_info_path are: ascii header, text, mask
  /// (mask is used only for text font)

  auto bmps_dir_fs = std::filesystem::path(bmps_info_path).parent_path();
  auto bmps_dir = bmps_dir_fs.string();
  auto bmps_info = ParseBmpsData(bmps_dir, bmps_info_path, false);

  Renderer renderer;
  std::vector<std::string> tex_coords_strings;

  for(const auto& entry
       : std::filesystem::directory_iterator(localizations_dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    auto prerendered_data = ParseLocFile(entry.path().string());
    bool is_en_loc = (entry.path().stem() == hash_loc_name);
    for (auto& phrases_info : prerendered_data) {
      /// there's no sense in them; moreover they will be harmful for
      /// hash generation and phrases prerendering
      if (!IsPhrasesValid(phrases_info.phrases)) {
        std::cerr
            << "localization file " << entry << " contains empty lines"
            << " at prerendering section " << phrases_info.name
            << "\nRemove them (skipped for now)" << std::endl;
        continue;
      }

      std::cout << "--- --- --- ---\nProcessing: "
                << phrases_info.name << std::endl;

      /// perfect hash function generation (if gperf command is available)
      if (is_en_loc) {
        std::cout << "Hash generation..." << std::endl;
        auto out_path = std::format(
            "{}/{}", phrases_out_dir, phrases_info.name);
        GenHash(phrases_info.phrases, out_path);
        std::cout << "->Hash generating is completed" << std::endl;
      }

      std::cout << "Phrases prerendering..." << std::endl;
      auto tex_coords = renderer.Process(
          bmps_info, phrases_info, bmps_dir);
      if (tex_coords.empty()) {
        std::cout << "->Phrases prerendering skipped" << std::endl;
        continue;
      }
      tex_coords_strings.push_back(SerializePhrasesTexCoords(
          tex_coords, phrases_info.name));
      std::cout << "->Phrases prerendering is completed" << std::endl;

      std::cout << "Storing data to files..." << std::endl;
      auto texture_data = renderer.GetTextureData();
      StorePhrasesTex(texture_data, phrases_info.name, phrases_out_dir);

      bool has_mask = !phrases_info.name_bmp_ascii_mask.empty();
      if (has_mask) {
        texture_data = renderer.GetTextureMaskData();
        StorePhrasesTexMask(texture_data, phrases_info.name, phrases_out_dir);
      }
      std::cout << "->Storing data to files completed" << std::endl;
      std::cout << "->Processing for " << phrases_info.name
                << " is completed" << std::endl;
    }
  }
  std::cout << "Storing texture coords to files..." << std::endl;
  StoreTexCoords(phrases_out_dir, tex_coords_strings);
  std::cout << "->Storing texture coords is completed" << std::endl;
}

void PrintFailure() {
  std::cerr << "incorrect program arguments, use \'-h\' for help" << std::endl;
}

void PrintHelp() {
  std::cout
      << "\n--- --- --- ---"
         "1) generate only bitmaps (2 args):\n"
         "  - bitmap_info.csv (should exist)\n"
         "  - dir for generated output\n"
         "2) generate only phrases (4 args):\n"
         "  - bitmap_info.csv (should exist)\n"
         "  - dir with localization files\n"
         "  - dir for generated output\n"
         "  - en localization name (no extension or parent path)\n"
         "      (or just ascii) for hash generation\n"
         "3) all (5 args):\n"
         "  - bitmap_info.csv (should exist)\n"
         "  - dir for generated output (bitmaps)\n"
         "  - dir with localization files\n"
         "  - dir for generated output (prerendered phrases & hash)\n"
         "  - en localization file name (no extension or parent path)\n"
         "      (or just ascii) for hash generation\n"
         "--- --- --- ---"
      << std::endl;
}
