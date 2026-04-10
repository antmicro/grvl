// Copyright 2014-2026 Antmicro <antmicro.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <grvl/File.h>
#include <grvl/Font.h>
#include <grvl/grvl.h>

#include <cmath>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <grvl/Endian.h>

namespace grvl {

    struct FontFileHeader {
        char magic[8];
        uint32_t version;
        int32_t height;
        uint64_t entries;
        uint64_t kernings;
    };

    struct FontFileEntry {
        uint32_t bitmap;
        uint32_t unicode;
        int16_t width;
        int16_t height;
        int8_t xoff;
        int8_t yoff;
        int16_t advance;
    };

    struct FontFileKerning {
        uint32_t prev, next;
        int32_t horizontal;
    };

    // make sure that the structs are tightly packed in memory
    static_assert(sizeof(FontFileHeader) == 32);
    static_assert(sizeof(FontFileEntry) == 16);
    static_assert(sizeof(FontFileKerning) == 12);

    /*
     * Font
     */

    Font::~Font()
    {
        for (auto& [key, value] : glyphs) {
            free(value.bitmap);
        }

        glyphs.clear();
    }

    int16_t Font::GetFontHeight() const
    {
        return height;
    }

    uint16_t Font::GetCharWidth(uint32_t code)
    {
        return GetGlyph(code).advance;
    }

    uint16_t Font::GetWidth(const char* text)
    {

        int32_t width = 0;
        uint32_t prev_unicode = 0;

        while (*text) {

            Unicode unicode = ParseUnicodeCodepoint(text);
            text += unicode.length;

            Glyph glyph = GetGlyph(unicode.code);
            width += glyph.advance;

            if (prev_unicode) {
                width += GetKerning(prev_unicode, unicode.code);
            }

            if (unicode.code != ' ') {
                prev_unicode = unicode.code;
            }

        }

        return width;
    }

    Glyph Font::GetFallback()
    {
        Glyph glyph;
        glyph.bitmap = nullptr;
        glyph.width = 0;
        glyph.height = 0;
        glyph.xoff = 0;
        glyph.yoff = 0;
        glyph.advance = 10;

        return glyph;
    }

    /*
     * GrvlBakedFont
     */

    GrvlBakedFont::GrvlBakedFont(const char* path)
    {
        File file(path);

        if (!file.Exists()) {
            grvl::Log("[ERROR] Unable to load font file %s (no such file)", path);
            return;
        }

        auto buffer = file.Read();
        uint8_t* data = reinterpret_cast<uint8_t*>(buffer.data());
        uint8_t* start = data;

        if (buffer.size() < sizeof(FontFileHeader)) {
            grvl::Log("[ERROR] Font file %s too short", path);
            return;
        }

        FontFileHeader header;
        memcpy(&header, data, sizeof(header));
        data += sizeof(header);

        if (memcmp(header.magic, "grvlfnt\0", 8) != 0 || header.version != 0) {
            grvl::Log("[ERROR] Font file %s has invalid header", path);
            return;
        }

        this->height = BigEndian32(header.height);
        int64_t entry_count = BigEndian64(header.entries);
        int64_t kerning_count = BigEndian64(header.kernings);
        int32_t version = BigEndian32(header.version);

        if (version != 0) {
            grvl::Log("[ERROR] Baked font %s has unknown version %d!", path, version);
            return;
        }

        grvl::Log("[INFO] Loading baked font %s (height %dpx), %ld glyphs, %ld kernings", path, height, entry_count, kerning_count);

        for (int64_t i = 0; i < entry_count; i ++) {

            FontFileEntry entry;
            memcpy(&entry, data, sizeof(entry));
            data += sizeof(entry);

            uint32_t unicode = BigEndian32(entry.unicode);
            uint32_t bitmap = BigEndian32(entry.bitmap);

            Glyph glyph {};
            glyph.bitmap = nullptr;
            glyph.width = BigEndian16(entry.width);
            glyph.height = BigEndian16(entry.height);
            glyph.advance = BigEndian16(entry.advance);
            glyph.xoff = entry.xoff;
            glyph.yoff = entry.yoff;

            uint32_t bytes = glyph.bytes();

            if (bytes != 0) {
                glyph.bitmap = (uint8_t*) malloc(bytes);
                memcpy(glyph.bitmap, start + bitmap, bytes);
            }

            glyphs[unicode] = glyph;

        }

        for (int64_t i = 0; i < kerning_count; i ++) {

            FontFileKerning kerning {};
            memcpy(&kerning, data, sizeof(kerning));
            data += sizeof(kerning);

            uint64_t key = static_cast<uint64_t>(BigEndian32(kerning.prev)) << 32 | BigEndian32(kerning.next);
            kernings[key] = BigEndian32(kerning.horizontal);

        }

        grvl::Log("[INFO] Loaded font file %s (%d kerning entries, %d glyphs)", path, kerning_count, entry_count);

    }

    Glyph GrvlBakedFont::GetGlyph(uint32_t unicode)
    {
        auto it = glyphs.find(unicode);

        if (it != glyphs.end()) {
            return it->second;
        }

        it = glyphs.find('?');

        if (it != glyphs.end()) {
            return it->second;
        }

        grvl::Log("[WARN] No glyph found for U+%04x", unicode);

        // if all failes return anything
        Glyph g = GetFallback();
        glyphs[unicode] = g;
        return g;
    }

    int8_t GrvlBakedFont::GetKerning(uint32_t prev, uint32_t next) const
    {
        uint64_t key = static_cast<uint64_t>(prev) << 32 | next;
        auto it = kernings.find(key);

        if (it != kernings.end()) {

            return it->second;
        }

        return 0;
    }

    /*
     * TrueTypeData
     */

    TrueTypeData::TrueTypeData(const char* path)
    {
        File file(path);

        if (!file.Exists()) {
            grvl::Log("[ERROR] No such font file %s", path);
            return;
        }

        m_buffer = file.Read();
        m_path = path;
        m_info = nullptr;

        auto* info = new ::stbtt_fontinfo();

        if (stbtt_InitFont(info, data(), 0) != 1) {
            grvl::Log("[ERROR] Unable to load font file %s", path);
            delete info;
            return;
        }

        m_info = info;
        grvl::Log("[INFO] Loaded font file %s (%ld bytes)", path, size());
    }

    TrueTypeData::~TrueTypeData()
    {
        if (m_info) {
            delete m_info;
        }

        m_info = nullptr;
    }

    const uint8_t* TrueTypeData::data() const
    {
        return reinterpret_cast<const uint8_t*>(m_buffer.data());
    }

    size_t TrueTypeData::size() const
    {
        return m_buffer.size();
    }

    const char* TrueTypeData::path() const
    {
        return m_path.c_str();
    }

    ::stbtt_fontinfo* TrueTypeData::info() const
    {
        return m_info;
    }

    /*
     * TrueTypeFont
     */

    TrueTypeFont::TrueTypeFont(const std::shared_ptr<const TrueTypeData>& data, int size)
    {
        ttf = data;
        int ascent, descent, gap;

        this->scale = stbtt_ScaleForPixelHeight(ttf->info(), size);
        stbtt_GetFontVMetrics(ttf->info(), &ascent, &descent, &gap);

        this->height = round((ascent + descent + gap) * scale);
    }

    Glyph TrueTypeFont::GetGlyph(uint32_t unicode)
    {
        auto it = glyphs.find(unicode);

        if (it != glyphs.end()) {
            return it->second;
        }

        // check if the codepoint is not defined
        if (stbtt_FindGlyphIndex(ttf->info(), unicode) == 0) {
            auto it = glyphs.find('?');

            if (it != glyphs.end()) {
                return it->second;
            }

            return GetFallback();
        }

        int x0, y0, x1, y1;
        int w, h, xo, yo;

        stbtt_GetCodepointBitmapBox(ttf->info(), unicode, scale, scale, &x0, &y0, &x1, &y1);

        float advance = GetCodepointAdvance(unicode, scale);
        uint8_t* bitmap = stbtt_GetCodepointBitmap(ttf->info(), scale, scale, unicode, &w, &h, &xo, &yo);

        Glyph glyph;
        glyph.bitmap = bitmap;
        glyph.width = w;
        glyph.height = h;
        glyph.xoff = xo;
        glyph.yoff = yo;
        glyph.advance = round(advance);

        // Add to cache
        glyphs[unicode] = glyph;

        return glyph;
    }

    int8_t TrueTypeFont::GetKerning(uint32_t leftCode, uint32_t rightCode) const
    {
        return round(scale * stbtt_GetCodepointKernAdvance(ttf->info(), leftCode, rightCode));
    }

    float TrueTypeFont::GetCodepointAdvance(int unicode, float scale) const
    {
        int advance, bearing;
        stbtt_GetCodepointHMetrics(ttf->info(), unicode, &advance, &bearing);

        return advance * scale;
    }

    int TrueTypeFont::Preload(uint32_t start, uint32_t end)
    {
        for (uint32_t codepoint = start; codepoint <= end; codepoint ++) {
            GetGlyph(codepoint);
        }

        return 0;
    }

    int TrueTypeFont::Save(const char* path)
    {

        struct Character {
            uint32_t unicode;
            uint32_t index;
        };

        FILE* file = fopen(path, "wb");

        if(file == nullptr) {
            return -1;
        }

        std::vector<FontFileEntry> file_entries;
        std::vector<Character> characters;
        std::vector<FontFileKerning> file_kernings;

        file_entries.reserve(glyphs.size());
        characters.reserve(glyphs.size());

        uint32_t offset = 0;
        int current = 1;

        for (auto& [unicode, glyph] : glyphs) {

            FontFileEntry entry;
            entry.bitmap = offset; // this is temporary, and will be updated later
            entry.unicode = BigEndian32(unicode);
            entry.width = BigEndian16(glyph.width);
            entry.height = BigEndian16(glyph.height);
            entry.advance = BigEndian16(glyph.advance);
            entry.xoff = glyph.xoff;
            entry.yoff = glyph.yoff;

            file_entries.push_back(entry);
            offset += glyph.bytes();

            Character chr;
            chr.unicode = unicode;
            chr.index = stbtt_FindGlyphIndex(ttf->info(), unicode);

            if (glyph.bytes() != 0) {
                characters.push_back(chr);
            }
        }

        grvl::Log("[INFO] Exported %d glyphs", glyphs.size());
        grvl::Log("[INFO] Generating kerning table...", glyphs.size());

        for (Character prev : characters) {
            for (Character next : characters) {

                int points = stbtt_GetGlyphKernAdvance(ttf->info(), prev.index, next.index);
                int advance = round(scale * points);

                if (advance == 0) {
                    continue;
                }

                FontFileKerning entry {};
                entry.prev = BigEndian32(prev.unicode);
                entry.next = BigEndian32(next.unicode);
                entry.horizontal = BigEndian32(advance);
                file_kernings.push_back(entry);

            }

            printf("\r[INFO] Scanning %d/%d glyphs, found %d kernings", current, characters.size(), file_kernings.size());
            current ++;
        }

        printf("\n");

        FontFileHeader header {};
        memcpy(header.magic, "grvlfnt", 8);
        header.version = BigEndian32(0UL); // future-proofing
        header.height = BigEndian32(height);
        header.entries = BigEndian64(file_entries.size());
        header.kernings = BigEndian64(file_kernings.size());
        fwrite(&header, 1, sizeof(FontFileHeader), file);

        uint32_t bitmap_area_start = sizeof(FontFileHeader) + file_entries.size() * sizeof(FontFileEntry) + file_kernings.size() * sizeof(FontFileKerning);

        // update offsets to point into the bitmap area
        for (FontFileEntry& entry : file_entries) {
            entry.bitmap = BigEndian32(entry.bitmap + bitmap_area_start);
        }

        fwrite(file_entries.data(), file_entries.size(), sizeof(FontFileEntry), file);
        fwrite(file_kernings.data(), file_kernings.size(), sizeof(FontFileKerning), file);

        // bitmaps
        for (auto& [unicode, glyph] : glyphs) {
            fwrite(glyph.bitmap, 1, glyph.bytes(), file);
        }

        fclose(file);

        return 0;
    }

    /*
     * ParseUnicodeCodepoint
     */

    Unicode ParseUnicodeCodepoint(const char* in)
    {
        Unicode uchar;
        if(!in) {
            uchar.code = 0;
            uchar.length = 0;
            return uchar;
        }

        uint8_t* bytes = (uint8_t*)in;
        if(!bytes) {
            uchar.code = 0;
            uchar.length = 0;
            return uchar;
        }

        int32_t length;
        uint8_t mask = 0x1f;

        switch(bytes[0] & 0xf0) {
            case 0xc0:
            case 0xd0:
                length = 2;
                break;
            case 0xe0:
                length = 3;
                break;
            case 0xf0:
                mask = 0x0f;
                length = 4;
                break;
            default:
                // valid 1-byte character
                // and invalid characters
                (in)++;
                uchar.length = 1;
                uchar.code = (uint32_t)bytes[0];
                return uchar;
        }
        uint32_t c = bytes[0] & mask;
        int32_t i = 1;
        for(; i < length && (bytes[i] & 0x80) > 0; i++) {
            c = (c << 6) | (bytes[i] & 0x3f);
        }
        if(i < length) {
            // invalid character
            uchar.length = 1;
            uchar.code = (uint32_t)bytes[0];
            return uchar;
        }
        uchar.length = length;
        uchar.code = (uint32_t)c;
        return uchar;
    }

} /* namespace grvl */
// NOLINTEND(readability-magic-numbers)
