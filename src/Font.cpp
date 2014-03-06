// Copyright 2014-2024 Antmicro <antmicro.com>
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

// NOLINTBEGIN(readability-magic-numbers)

#include "Font.h"
#include "File.h"
#include "grvl.h"

namespace grvl {
    static inline uint16_t SwapBytes(uint16_t data)
    {
        return (data >> 8) | (data << 8);
    }

    static inline uint32_t SwapUint32(uint32_t val)
    {
        val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
        return (val << 16) | (val >> 16);
    }

    // Fixme add freeing memory
    Font::~Font()
    {
        grvl::Callbacks()->free(WideCharData);
        grvl::Callbacks()->free(WideCharMap);
    }

    Font::Font(const uint16_t* FontData)
    {
        Init(FontData);
    }

    Font::Font(const char* fpath)
    {
        File file(fpath);
        int fSize = file.GetSize();
        if(fSize == -1) {
            grvl::Log("[ERROR] Font: Could not load font from file %s", fpath);
        } else {
            uint8_t* buffer = (uint8_t*)grvl::Callbacks()->malloc(fSize);
            file.ReadToBuffer(buffer);

            Init((uint16_t*)buffer);
        }
    }

    void Font::Init(const uint16_t* FontData)
    {
        CharacterEntries = (SwapBytes(FontData[0]));
        CharHeight = (SwapBytes(FontData[1]));
        FontHeight = (SwapBytes(FontData[2]));
        BasicLength = (SwapBytes(FontData[3]));

        uint16_t CharMapOffset = ((uint32_t)SwapBytes(FontData[4]));
        uint32_t CharWidthsOffset = ((uint32_t)SwapUint32(FontData[5] + (FontData[6] << 16)));
        uint32_t KerningMapOffset = ((uint32_t)SwapUint32(FontData[7] + (FontData[8] << 16)));

        KerningEntries = SwapBytes(FontData[9]);

        CharMap = (map_entry*)((uintptr_t)FontData + CharMapOffset);
        Widths = (uint8_t*)((uintptr_t)FontData + CharWidthsOffset);
        KerningMap = (uint16_t*)((uintptr_t)FontData + KerningMapOffset);

        current_offset = 0;
        CharData = (uint8_t*)((uintptr_t)KerningMap + (uint32_t)(2 * CharacterEntries));
        WideCharMap = (map_entry*)grvl::Callbacks()->malloc(CharacterEntries * sizeof(map_entry));
        for(int i = 0; i < CharacterEntries; ++i) {
            WideCharMap[i].code = CharMap[i].code;
            WideCharMap[i].offset = current_offset;
            current_offset += Widths[i] * CharHeight * 4;
        }
        WideCharData = (uint32_t*)grvl::Callbacks()->malloc(current_offset);
        for(int c = 0; c < CharacterEntries; ++c) {
            uint32_t* offset = (uint32_t*)((uintptr_t)WideCharData + (uintptr_t)WideCharMap[c].offset);
            uint8_t* orig_offset = (uint8_t*)((uintptr_t)CharData + (uintptr_t)SwapUint32(CharMap[c].offset));
            for(int i = 0; i < CharHeight * Widths[c]; ++i) {
                offset[i] = (uint32_t)(orig_offset[i] << 24); // | (color & 0xffffff);
            }
        }

        KerningData = (kerning_entry*)((uintptr_t)Widths + (uint32_t)(CharacterEntries));

        SpaceLength = FontHeight * 0.4 - BasicLength;

        // Filling map
        for(int i = 0; i < CharacterEntries; i++) {
            CodeMap[SwapBytes(CharMap[i].code)] = i;
        }
    }

    Font& Font::operator=(const Font& Obj)
    {
        if(this != &Obj) {
            CodeMap = Obj.CodeMap;
            current_offset = Obj.current_offset;
            WideCharMap = (map_entry*)grvl::Callbacks()->malloc(CharacterEntries * sizeof(map_entry));
            WideCharData = (uint32_t*)grvl::Callbacks()->malloc(current_offset);
            for(int i = 0; i < CharacterEntries; ++i) {
                WideCharMap[i].code = Obj.WideCharMap[i].code;
                WideCharMap[i].offset = Obj.WideCharMap[i].offset;
            }
            memcpy(Obj.WideCharData, WideCharData, static_cast<size_t>(current_offset) * 4);
            CharacterEntries = Obj.CharacterEntries;
            CharHeight = Obj.CharHeight;
            FontHeight = Obj.FontHeight;
            BasicLength = Obj.BasicLength;
            CharMap = Obj.CharMap;
            CharData = Obj.CharData;
            Widths = Obj.Widths;
            KerningMap = Obj.KerningMap;
            KerningData = Obj.KerningData;
            KerningEntries = Obj.KerningEntries;
        }
        return *this;
    }

    uint16_t Font::GetHeight() const
    {
        return CharHeight;
    }

    uint16_t Font::GetKerningEntries() const
    {
        return KerningEntries;
    }

    uint32_t* Font::GetCharData(uint32_t code) const
    {
        auto searchCode = CodeMap.find(code);
        if(searchCode == CodeMap.end()) searchCode = CodeMap.find('_');
        if(searchCode == CodeMap.end()) return 0;

        auto* data = CharData + SwapUint32(CharMap[searchCode->second].offset);
        return (uint32_t*) data;
    }

    uint16_t Font::GetCharWidth(const uint32_t code) const
    {
        Map::const_iterator searchCode = CodeMap.find(code);
        if(searchCode == CodeMap.end()) { // If Looking for '_'
            searchCode = CodeMap.find(0x5f);
            if(searchCode == CodeMap.end()) {
                return BasicLength * 3; // Not available
            }
        }

        return Widths[searchCode->second];
    }

    // based on MIT-licensed Haiku-OS, taken from: UdfString.cpp
    Font::unicode_character Font::GetUnicodeCharacter(const char* in)
    {
        unicode_character uchar;
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

    int8_t Font::GetKerning(uint32_t leftCode, uint32_t rightCode) const
    {

        Map::const_iterator searchLeftCode = CodeMap.find(leftCode);
        if(searchLeftCode == CodeMap.end()) {
            return BasicLength;
        }

        Map::const_iterator searchRightCode = CodeMap.find(rightCode);
        if(searchRightCode == CodeMap.end()) {
            return BasicLength;
        }

        uint16_t leftIndex = searchLeftCode->second;
        uint16_t rightIndex = searchRightCode->second;

        uint16_t kerningOffset = 0;
        int8_t KerningValue = BasicLength;

        if(KerningEntries == 0) {
            return KerningValue;
        }
        if(KerningMap)
            kerningOffset = SwapBytes(KerningMap[leftIndex]);

        if(kerningOffset == 0xffff) {
            return KerningValue;
        }

        kerning_entry* CharKerning = (kerning_entry*)((unsigned long)KerningData + (uint32_t)kerningOffset);

        for(int i = 0; CharKerning[i].left == leftIndex; i++) {
            if(CharKerning[i].right == rightIndex) {
                KerningValue += CharKerning[i].space;
                break;
            }
        }
        return KerningValue;
    }

    uint16_t Font::GetWidth(const char* Text) const
    {

        int32_t size = 0;

        uint32_t previousCharCode = 0; // Save char code placed before space
        while(*Text != 0) {
            uint32_t charCode = 0;
            uint32_t nextCharCode = 0;
            int length = 1;
            Font::unicode_character uchar;
            Font::unicode_character next_uchar;

            /* Display one character on LCD */
            if(*Text != 0x20) { // Not a space
                if((unsigned char)(*Text) > 0x7F) { // Unicode
                    uchar = Font::GetUnicodeCharacter(Text);
                    length = uchar.length;
                    charCode = uchar.code;
                    if((*(Text + length)) != 0) {
                        next_uchar = Font::GetUnicodeCharacter(Text + length);
                        nextCharCode = next_uchar.code;
                    }
                } else { // UTF8
                    charCode = (unsigned char)(*Text);
                    if((*(Text + length)) != 0) { //NOLINT
                        if((*(Text + length)) <= 0x7f) { //NOLINT
                            nextCharCode = (unsigned char) (*(Text + length));
                        } else { // Next character is a unicode character
                            next_uchar = Font::GetUnicodeCharacter(Text + length);
                            nextCharCode = next_uchar.code;
                        }
                    }
                }

                size += GetCharWidth(charCode);
                if(nextCharCode) {
                    size += GetKerning(charCode, nextCharCode);
                }
            } else { // space
                charCode = (unsigned char) (*Text);

                if((*(Text + length)) != 0) {
                    if((*(Text + length)) <= 0x7f) {
                        nextCharCode = (unsigned char)(*(Text + length));
                    } else { // Next character is a unicode character
                        next_uchar = Font::GetUnicodeCharacter(Text + length);
                        nextCharCode = next_uchar.code;
                    }
                }

                size += GetSpaceLength();
                if(nextCharCode && previousCharCode) {
                    size += GetKerning(previousCharCode, nextCharCode) - GetBasicLength();
                }
            }

            Text += length;

            if(nextCharCode == 0x20) { // Save char code placed before space
                previousCharCode = charCode;
            }
        }

        return size;
    }

    uint16_t Font::GetCharEntries() const
    {
        return CharacterEntries;
    }

    uint16_t Font::GetFontHeight() const
    {
        return FontHeight;
    }

    uint16_t Font::GetBasicLength() const
    {
        return BasicLength;
    }

    int16_t Font::GetSpaceLength() const
    {
        return SpaceLength;
    }

} /* namespace grvl */
// NOLINTEND(readability-magic-numbers)
