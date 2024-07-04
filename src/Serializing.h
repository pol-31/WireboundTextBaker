#ifndef WIREBOUNDTEXTBAKER__SERIALIZING_H_
#define WIREBOUNDTEXTBAKER__SERIALIZING_H_

#include <string>
#include <string_view>
#include <vector>

#include "Common.h"
#include "Renderer.h"

void WriteBitmapsInfo(std::string_view path,
                      const std::vector<BitmapInfo>& bmps_info,
                      std::string_view old_entries);

void WriteBitmapsInfo(std::string_view path,
                      const std::vector<BitmapInfo>& bmps_info);

void GenBmtInfoHeader(const std::vector<BitmapInfo>& bmp_info,
                      std::string_view out_path);

std::string SerializePhrasesTexCoords(
    const std::vector<Rectangle>& tex_coords, std::string_view name);

void StoreBitmapTexture(std::string_view bmps_dir,
                        const BitmapInfo& bmp_info,
                        unsigned char data[]);

void StorePhrasesTex(Renderer::TextureData& texture_data,
                     std::string_view name,
                     std::string_view phrases_out_dir);

void StorePhrasesTexMask(Renderer::TextureData& texture_data,
                         std::string_view name,
                         std::string_view phrases_out_dir);

std::vector<std::string> CompressUtf8ToAsciiString(
    const PhrasesType& utf8_phrases);

/// there should be English localization, so only ASCII symbols
void GenHash(const PhrasesType& utf8_phrases,
             std::string_view out_path);

void StoreTexCoords(std::string_view phrases_out_dir,
                    const std::vector<std::string>& tex_coords_string);

#endif  // WIREBOUNDTEXTBAKER__SERIALIZING_H_
