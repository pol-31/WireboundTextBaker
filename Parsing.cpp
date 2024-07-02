#include "Parsing.h"

#include <algorithm>
#include <cstring>
#include <execution>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <utf8.h>

bool user_respond_yes = false;
bool user_respond_no = false;

bool AskUser(std::string_view question) {
  bool answer = false;
  if (user_respond_yes) {
    std::cout << question << "\nStatus: -auto yes-" << std::endl;
    return true;
  } else if (user_respond_no) {
    std::cout << question << "\nStatus: -auto no-" << std::endl;
    return false;
  }
  std::cout << question << std::endl;
  char respond;
  std::cin >> respond;
  if (respond == 'y' || respond == 'Y') {
    answer = true;
  } else if (respond == 'a' || respond == 'A') {
    user_respond_yes = true;
    answer = true;
  } else if (respond == '0') {
    user_respond_no = true;
  }
  if (answer) {
    std::cout << "Status: YES" << std::endl;
  } else {
    std::cout << "Status: NO" << std::endl;
  }
  return answer;
}

void SkipMetadata(std::ifstream& file_stream) {
  std::string buffer;
  for (int i = 0; i < details::kBmpsMetadataLinesNum; ++i) {
    std::getline(file_stream, buffer, '\n');
  }
}

void ParseSymbols(std::ifstream& file_stream,
                  std::vector<int>& symbols) {
  std::string buffer;
  std::getline(file_stream, buffer, ';');
  if (buffer.empty()) {
    return;
  }
  std::erase(buffer, ' ');
  std::erase(buffer, '\n');
  std::erase(buffer, '\t');
  if (buffer[0] == '\"') {
    buffer = buffer.substr(1, buffer.length() - 2);
  }
  std::istringstream iss_symbols;
  iss_symbols.str(buffer);
  while(std::getline(iss_symbols, buffer, ',')) {
    if (buffer.length() == 6) {
      /// "0x0000" - 6 symbols
      symbols.push_back(std::stoi(buffer, nullptr, 16));
    } else if (buffer.length() == 13 && buffer[6] == '-') {
      /// "0x0000-0x0001" - 13 symbols
      for (int i = std::stoi(buffer.substr(0, 6), nullptr, 16);
           i <= std::stoi(buffer.substr(7), nullptr, 16); ++i) {
        symbols.push_back(i);
      }
    } else if (buffer.empty()) {
      continue; // ",,"
    } else {
      throw std::runtime_error("bitmap info file has incorrect data");
    }
    if (symbols.size() > details::kBitmapTotalChar) {
      std::cerr << std::format("too many symbols for bitmap - {} is max",
                               details::kBitmapTotalChar) << std::endl;
      break;
    }
  }
  /// important in future
  std::sort(std::execution::par_unseq, symbols.begin(), symbols.end());
  auto it = std::unique(symbols.begin(), symbols.end());
  symbols.erase(it, symbols.end());
}

GlyphMapping ParseGlyphMapping(std::string_view buffer) {
  std::istringstream iss_widths(buffer.data());
  std::string value;
  value.reserve(10);

  GlyphMapping glyph_mapping;
  std::getline(iss_widths, value, ',');
  glyph_mapping.factor = std::stof(value);
  std::getline(iss_widths, value, ',');
  glyph_mapping.min = std::stoi(value);
  std::getline(iss_widths, value, ',');
  glyph_mapping.height = std::stoi(value);
  std::getline(iss_widths, value, ',');
  glyph_mapping.full_height = std::stoi(value);

  /// parentheses are important (!)
  std::string width_data((std::istreambuf_iterator<char>(iss_widths)),
                         std::istreambuf_iterator<char>());
  std::erase(width_data, ' ');
  std::erase(width_data, '\n');
  if (width_data[0] == '\"') {
    width_data = width_data.substr(1, width_data.length() - 2);
  }
  iss_widths.str(std::move(width_data));
  while(std::getline(iss_widths, value, ',')) {
    if (value.empty()) {
      continue; // ",,"
    }
    glyph_mapping.widths.push_back(static_cast<uint8_t>(std::stoi(value)));
  }
  return glyph_mapping;
}

std::vector<BitmapInfo> ParseBmpsData(
    std::string_view bmps_dir,
    std::string_view bmp_info_path,
    bool safe_replace) {
  std::ifstream bmp_info_file(bmp_info_path.data());
  if (!bmp_info_file.is_open()) {
    throw std::runtime_error("unable to open bitmap info file");
  }
  SkipMetadata(bmp_info_file);

  std::vector<BitmapInfo> bmps_info;
  std::string bmp_path, buffer;
  int row_number = 0;

  while (std::getline(bmp_info_file, buffer, ';')) {
    BitmapInfo bmp_info;
    if (buffer.empty()) {
      throw std::runtime_error(
          std::format("bitmap name can't be empty (row {})", row_number));
    }
    bmp_info.name = buffer;
    bmp_path = std::format("{}/{}.png", bmps_dir, bmp_info.name);

    std::getline(bmp_info_file, bmp_info.font_path, ';');
    if (bmp_info.font_path.empty()) {
      throw std::runtime_error(std::format(
          "incorrect font path (row \"\n{}\")\"", row_number));
    }

    std::getline(bmp_info_file, buffer, ';');
    /// by default bmp_info.size = kFontDefaultBitmapSize
    if (!buffer.empty()) {
      bmp_info.size = std::stoi(buffer);
    }

    ParseSymbols(bmp_info_file, bmp_info.chars);

    std::getline(bmp_info_file, buffer, ';');
    bmp_info.autogen_symbols = buffer == "y";
    if (!bmp_info.autogen_symbols) {
      try {
        bmp_info.glyph_mapping = ParseGlyphMapping(buffer);
      } catch (...) {
        bmp_info.autogen_symbols = false;
      }
    }

    std::getline(bmp_info_file, bmp_info.loc_path, ';');
    if (bmp_info.loc_path.empty() && bmp_info.autogen_symbols) {
      throw std::runtime_error(std::format(
          "symbol autogen is not possible without provided path to "
          "localization file (row {})", row_number));
    }

    if (!safe_replace ||
        !std::filesystem::exists(bmp_path) ||
        AskUser(std::format(
            "{} already exist. Do you want to replace it?"
            "\ny(yes), n(no), a(yes for all), 0(no for all))",
            bmp_info.name))) {
      bmps_info.push_back(std::move(bmp_info));
    }
    /// read to the end of line
    std::getline(bmp_info_file, buffer, '\n');
    ++row_number;
  }
  return bmps_info;
}

std::vector<std::string> ParseOldBmpsData(
    const std::vector<BitmapInfo>& bmps_info,
    std::string_view bmp_info_path) {
  std::ifstream bmp_info_file(bmp_info_path.data());
  if (!bmp_info_file.is_open()) {
    throw std::runtime_error("unable to open bitmap info file");
  }
  SkipMetadata(bmp_info_file);

  std::vector<std::string> old_entries;
  std::string entry;
  entry.reserve(512);
  std::string buffer;
  buffer.reserve(512);
  bool skip_entry = false;
  /// because we already parsed it at ParseBmpsData() all data is valid and
  /// there's no need to check it
  while (std::getline(bmp_info_file, buffer, ';')) {
    for (const auto& bmp_info : bmps_info) {
      /// if bitmap name is still present in bitmaps_info, that means user has
      /// chosen "replace" with main "merge" option; otherwise name
      /// wouldn't be added to bitmaps_info (see ParseBmpsData());
      /// so it will be replaced and we don't need it
      if (bmp_info.name == buffer) {
        skip_entry = true;
        break;
      }
    }
    if (skip_entry) {
      /// skip other entry fields
      for (int i = 0; i < 5; ++i) {
        std::getline(bmp_info_file, buffer, ';');
      }
      std::getline(bmp_info_file, buffer, '\n');
      skip_entry = false;
      continue;
    }
    entry += buffer; // name
    entry += ';';
    // font path, size, symbols, autogen_symbols / glyph_mapping, loc path
    for (int i = 0; i < 5; ++i) {
      std::getline(bmp_info_file, buffer, ';');
      entry += buffer;
      entry += ';';
    }
    std::getline(bmp_info_file, buffer, '\n');
    entry += buffer;
    entry += '\n';
    old_entries.push_back(entry);
    entry.clear();
  }
  /// removing last '\n'
  if (!old_entries.empty()) {
    std::string& last_entry = old_entries.back();
    last_entry = last_entry.substr(0, last_entry.size() - 1);
  }
  return old_entries;
}

PhrasesType ParsePhrases(std::ifstream& file_stream, int lines_to_read) {
  std::string phrases_str, buffer;
  int phrases_read;
  std::vector<std::string> phrases_buffer;
  phrases_buffer.reserve(64);
  int counter = lines_to_read;
  PhrasesType phrases;
  while (true) {
    while (--counter >= 0) {
      std::getline(file_stream, buffer, '\n');
      phrases_buffer.push_back(buffer);
    }
    for (const auto& str : phrases_buffer) {
      phrases_str += str;
      phrases_str += '\n';
    }
    phrases_read = 0;
    for (auto it = phrases_str.begin(), end = phrases_str.end(); it != end;) {
      utf8::utfchar32_t ch = utf8::next(it, end);
      if (ch == U'\n') {
        ++phrases_read;
      }
    }
    // case phrases_read >= lines_to_read is impossible
    if (phrases_read == lines_to_read) {
      break;
    } else {
      counter = lines_to_read - phrases_read;
      phrases_str.clear();
    }
  }
  /// convert all phrases into utf8 phrases
  std::vector<utf8::utfchar32_t> utf8_phrase;
  for (auto it = phrases_str.begin(), end = phrases_str.end(); it != end;) {
    utf8::utfchar32_t ch = utf8::next(it, end);
    if (ch != U'\n') {
      utf8_phrase.push_back(ch);
    } else {
      phrases.push_back(std::move(utf8_phrase));
      utf8_phrase = std::vector<utf8::utfchar32_t>();
    }
  }
  return phrases;
}

std::vector<PrerenderData> ParseLocFile(std::string_view loc_path) {
  std::ifstream file(loc_path.data());
  std::string buffer;
  buffer.reserve(256);
  /// skip first row "METADATA"
  std::getline(file, buffer, '\n');

  /// line contains all offsets: prerender1, ..., prerenderN, main text start
  /// example: "10;20;40;" means first prerender starts at 10 and ends at 19,
  /// while text part starts at 40.
  std::getline(file, buffer, '\n');
  std::istringstream iss_line;
  iss_line.str(buffer);

  /// last section means text start, so we use it only to compute
  /// length of a section's that comes before last;
  /// 3 sections max: prerender header/text + all program text
  std::vector<int> sections_landmarks;
  sections_landmarks.reserve(3);
  while (std::getline(iss_line, buffer, ';')) {
    sections_landmarks.push_back(std::stoi(buffer));
  }
  if (sections_landmarks.empty()) {
    return {};
  }

  int lines_read = 2; // already read

  /// skip before (probably, other metadata)
  while(++lines_read < sections_landmarks[0]) {
    std::getline(file, buffer, '\n');
  }

  std::vector<PrerenderData> all_prerender_data;
  /// if phrases_sections.size(), there's no prerender sections, but
  /// only main text section, so we skip it).
  for (std::size_t i = 0; i < sections_landmarks.size() - 1; ++i) {
    PrerenderData instance;
    std::getline(file, buffer, '\n');
    iss_line.clear();
    iss_line.str(buffer);

    std::getline(iss_line, instance.name, ';');
    std::getline(iss_line, instance.name_bmp_ascii, ';');
    std::getline(iss_line, instance.name_bmp_ascii_mask, ';');
    std::getline(iss_line, instance.name_bmp_loc, ';');
    std::getline(iss_line, instance.name_bmp_loc_mask, ';');
    std::getline(iss_line, instance.path_ttf_loc, ';');
    std::getline(iss_line, buffer, ';');
    if (!buffer.empty()) {
      instance.scale = std::stoi(buffer);
    }

    ++lines_read;

    if (instance.name.empty()) {
      throw std::runtime_error(std::format(
          "no name provided ({} at row {})", loc_path, lines_read));
    } else if (instance.name_bmp_ascii.empty()) {
      throw std::runtime_error(std::format(
          "no ascii bmp provided ({} at row {})", loc_path, lines_read));
    }

    int lines_to_read = sections_landmarks[i+1] - lines_read;
    /// we can't just std::getline('\n'), because it's utf8
    instance.phrases = ParsePhrases(file, lines_to_read);

    lines_read = sections_landmarks[i+1] - 1;
    all_prerender_data.push_back(std::move(instance));
  }
  return all_prerender_data;
}

std::vector<int> GetCharsFrequency(std::string_view loc_path) {
  std::ifstream loc_file(loc_path.data());
  if (!loc_file.is_open()) {
    throw std::runtime_error(std::format(
        "can\'t open localization file: {}", loc_path));
  }

  std::string buffer;
  buffer.reserve(256);
  /// skip first row "METADATA"
  std::getline(loc_file, buffer, '\n');

  /// line contains all offsets: prerender1, ..., prerenderN, main text start
  /// example: "10;20;40;" means first prerender starts at 10 and ends at 19,
  /// while text part starts at 40.
  std::getline(loc_file, buffer, '\n');

  auto last_section_pos = buffer.rfind(';');
  if (last_section_pos == std::string::npos || last_section_pos == 0) {
    throw std::runtime_error(std::format(
        "invalid localization file structure: {}", loc_path));
  }
  auto before_last_section_pos = buffer.rfind(';', last_section_pos - 1);
  if (before_last_section_pos == std::string::npos) {
    throw std::runtime_error(std::format(
        "invalid localization file structure: {}", loc_path));
  }

  /// skip metadata
  auto lines_before_text = std::stoi(buffer.substr(
      before_last_section_pos + 1,
      last_section_pos - before_last_section_pos - 1));
  lines_before_text -= 2; // we've already read 2 lines
  while (--lines_before_text > 0) {
    std::getline(loc_file, buffer, '\n');
  }

  /// <int, int> == <code, encountered>
  /// "int" default initialized with 0, so we can just increment by key
  std::map<int, int> char_freq_map;
  while (std::getline(loc_file, buffer, '\n')) {
    for (auto it = buffer.begin(); it != buffer.end();) {
      auto code = static_cast<int>(utf8::next(it, buffer.end()));
      ++char_freq_map[code];
    }
  }

  std::vector<std::pair<int, int>> char_freq_vec(
      char_freq_map.begin(), char_freq_map.end());
  std::sort(std::execution::par_unseq,
            char_freq_vec.begin(), char_freq_vec.end(),
            [](const auto& a, const auto& b) {
              return b.second < a.second;
            });

  std::vector<int> most_freq_chars(char_freq_vec.size());
  auto it = std::transform(char_freq_vec.begin(), char_freq_vec.end(),
                           most_freq_chars.begin(),
                           [](const auto& pair) {
                             return pair.first;
                           });

  /// we don't store ASCII printable symbols here - they are always
  /// stored in separated, designed for ascii bitmaps
  std::erase_if(most_freq_chars, [](int code) {
    return code <= 0x007E; // '~" (0x007E)
  });
  return most_freq_chars;
}

void AppendUniqueChars(std::vector<int>& chars,
                       const std::vector<int>& chars_freq,
                       const std::vector<int>& chars_freq_sorted) {
  int chars_to_add = details::kBitmapTotalChar
                     - static_cast<int>(chars.size());
  if (chars_to_add <= 0) {
    return;
  }

  /// we we need chars_freq_sorted to keep ordering of original chars_freq,
  /// where is non-ascending based on usage frequency
  std::vector<int> already_included(chars.size());
  auto it = std::set_intersection(
      std::execution::par_unseq,
      chars_freq_sorted.begin(), chars_freq_sorted.end(),
      chars.begin(), chars.end(),
      already_included.begin());
  already_included.resize(it - already_included.begin());

  auto chars_old_size = static_cast<
      std::vector<int>::difference_type>(chars.size());
  chars.resize(chars.size() + chars_to_add);
  auto it_new_start = chars.begin() + chars_old_size;

  auto it_new_end = std::copy_if(
      std::execution::par_unseq,
      chars_freq.begin(), chars_freq.end(),
      it_new_start, [&](int ch) {
        return (--chars_to_add >= 0) &&
               !std::binary_search(already_included.begin(),
                                   already_included.end(), ch);
      });
  chars.resize(it_new_end - it_new_start);
  std::sort(std::execution::par_unseq, chars.begin(), chars.end());
}

void AutogenChars(std::vector<BitmapInfo>& bmps_info) {
  std::unordered_set<std::string> loc_paths;
  std::unordered_multimap<std::string, BitmapInfo*> bmps_by_loc;
  for (auto& bmp_info : bmps_info) {
    if (bmp_info.autogen_symbols) {
      loc_paths.insert(bmp_info.loc_path);
      bmps_by_loc.insert({bmp_info.loc_path, &bmp_info});
    }
  }
  for (const auto& loc_path : loc_paths) {
    auto chars_freq = GetCharsFrequency(loc_path);

    /// bitmap_info symbols already sorted at ParseBmpsData()
    /// make temp copy to sort and find not-unique components
    auto chars_freq_sorted = chars_freq;
    std::sort(std::execution::par_unseq,
              chars_freq_sorted.begin(), chars_freq_sorted.end());

    for (auto it = bmps_by_loc.begin(bmps_by_loc.bucket(loc_path));
         it != bmps_by_loc.end(bmps_by_loc.bucket(loc_path)); ++it) {
      AppendUniqueChars((it->second)->chars, chars_freq,
                        chars_freq_sorted);
    }
  }
}

void ReadFontFile(std::string_view path,
                  unsigned char data_buffer[]) {
  std::ifstream font_file(path.data(), std::ios::binary);
  if (!font_file) {
    throw std::runtime_error(std::format("Failed to open file: {}", path));
  }
  font_file.read(reinterpret_cast<char*>(data_buffer),
                 details::kFontBufferSize);
  if (font_file.gcount() == 0 && font_file.fail()) {
    throw std::runtime_error(std::format("Failed to read file: {}", path));
  }
}
