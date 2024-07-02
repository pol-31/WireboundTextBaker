#ifndef WIREBOUNDTEXTBAKER__PARSING_H_
#define WIREBOUNDTEXTBAKER__PARSING_H_

#include <cinttypes>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "Common.h"

extern bool user_respond_yes;
extern bool user_respond_no;

bool AskUser(std::string_view question);

void SkipMetadata(std::ifstream& file_stream);

void ParseSymbols(std::ifstream& file_stream,
                  std::vector<int>& symbols);

GlyphMapping ParseGlyphMapping(std::string_view buffer);

std::vector<BitmapInfo> ParseBmpsData(
    std::string_view bmps_dir,
    std::string_view bmp_info_path,
    bool safe_replace);

std::vector<std::string> ParseOldBmpsData(
    const std::vector<BitmapInfo>& bmps_info,
    std::string_view bmp_info_path);

PhrasesType ParsePhrases(std::ifstream& file_stream, int lines_to_read);

/// it collects data for phrases prerendering
/// (loc file's main text section are ignored).
std::vector<PrerenderData> ParseLocFile(std::string_view loc_path);

std::vector<int> GetCharsFrequency(std::string_view loc_path);

void AppendUniqueChars(std::vector<int>& chars,
                       const std::vector<int>& chars_freq,
                       const std::vector<int>& chars_freq_sorted);

/// based on most frequently used chars from scanned localization files
void AutogenChars(std::vector<BitmapInfo>& bmps_info);

void ReadFontFile(std::string_view path, unsigned char data_buffer[]);

#endif  // WIREBOUNDTEXTBAKER__PARSING_H_
