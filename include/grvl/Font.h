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

#ifndef GRVL_Font_H_
#define GRVL_Font_H_

#include <grvl/stl.h>

#include <stdint.h>
#include <memory>

struct stbtt_fontinfo;

namespace grvl {

    /// Represents one draw-ready Unicode codepoint
    struct Glyph {
        uint8_t* bitmap;
        int16_t width, height;

        int8_t xoff; // offset to left glyph edge
        int8_t yoff; // offset to bottom glyph edge
        int16_t advance; // offset to the next character

        /// Get the number of bytes used by bitmap
        constexpr uint32_t bytes() const {
            return width * height;
        }
    };

    /// Represents one Unicode codepoint
    struct Unicode {
        uint32_t code;
        int32_t length;
    };

    /// Represents font loaded into memory.
    class Font {
    public:

        Font() = default;
        Font(const Font& other) = delete;
        Font(Font&& other) = default;

        virtual ~Font();

        /// Get the prepared glyph of a specific Unicode.
        virtual Glyph GetGlyph(uint32_t unicode) = 0;

        /// Get the aditional offset that should be added between two Unicodes.
        virtual int8_t GetKerning(uint32_t leftCode, uint32_t rightCode) const = 0;

        /// Get height of the font, in pixels.
        int16_t GetFontHeight() const;

        /// Calculate the length of the text, in pixels.
        uint16_t GetWidth(const char* text);

        /// Get width, in pixels, of the given Unicode.
        uint16_t GetCharWidth(uint32_t code);

        /// Return some fallback glyph for when the requested one is missing
        Glyph GetFallback();

    protected:

        std::unordered_map<uint32_t, Glyph> glyphs;
        int16_t height = 0;

    };

    /// Wraps a TTF file data.
    ///
    /// Wraps TTF file data held in memory and allows reusing it for multiple TrueTypeFonts
    /// of possibly different sizes, saving memory in the process.
    class TrueTypeData {
    public:
        TrueTypeData(const char* path);
        ~TrueTypeData();

        /// Pointer to the start of the font data
        const uint8_t* data() const;

        /// Size of font data in bytes
        size_t size() const;

        /// Get c-string path
        const char* path() const;

        /// Get STB FreeType font handle
        ::stbtt_fontinfo* info() const;

    private:
        std::vector<char> m_buffer;
        std::string m_path;
        ::stbtt_fontinfo* m_info;

    };

    /// Class for loading of .TTF fonts.
    ///
    /// This class will holds the whole file in memory but only load (and rasterize) specific
    /// codepoints once they are requested, as such expect memory usage to rise with each newly
    /// encountered character. As new glyphs need to be rasterized when first used expect a small
    /// speed penalty for each new (not previously used) character.
    /// This font can be resized, and the file overhead shared between multiple instances.
    class TrueTypeFont : public Font {
    public:

        /// The given TrueTypeData must be valid for the entire lifetime of TrueTypeFont
        TrueTypeFont(const std::shared_ptr<const TrueTypeData>& data, int size);

        /// Preload an inclusive range of codepoints
        int Preload(uint32_t start, uint32_t end);

        /// Export the font to a .GBF file, see GrvlBakedFont
        int Save(const char* path);

        Glyph GetGlyph(uint32_t unicode) override;
        int8_t GetKerning(uint32_t leftCode, uint32_t rightCode) const override;

    private:

        float GetCodepointAdvance(int unicode, float scale) const;

        std::shared_ptr<const TrueTypeData> ttf;
        float scale;

    };

    /// Class for loading .GBF fonts.
    ///
    /// This class doesn't hold the file in memory but will copy all the data from it
    /// to memory on load (expect a ~linear relation between font file size and memory usage),
    /// the memory usage will stay constant after that initial object creation.
    /// This font can't be resized.
    class GrvlBakedFont : public Font {
    public:

        GrvlBakedFont(const char* path);

        Glyph GetGlyph(uint32_t unicode) override;
        int8_t GetKerning(uint32_t leftCode, uint32_t rightCode) const override;

    private:

        std::unordered_map<uint64_t, int8_t> kernings;

    };

    /// Query information about Unicode codepoint from a UTF-8 pointer
    Unicode ParseUnicodeCodepoint(const char* in);

} /* namespace grvl */

#endif /* GRVL_Font_H_ */
