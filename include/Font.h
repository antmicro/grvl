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

#ifndef GRVL_Font_H_
#define GRVL_Font_H_

#include "stl.h"
#include <stdint.h>

namespace grvl {

    /// Represents font loaded into memory.
    class Font {
    public:
        struct unicode_character {
            uint32_t code;
            int32_t length;
        };

        struct kerning_entry {
            uint8_t left;
            uint8_t right;
            int8_t space;
        };

        struct __attribute__((packed)) map_entry {
            uint16_t code;
            uint32_t offset;
        };

        Font()
            : CharacterEntries(0)
            , CharHeight(0)
            , FontHeight(0)
            , BasicLength(0)
            , CharMap(0)
            , CharData(0)
            , Widths(0)
            , KerningMap(0)
            , KerningData(0)
            , KerningEntries(0)
            , SpaceLength(0)
        {
        }

        Font(const uint16_t* FontData);
        Font(const char* fpath);
        Font(const Font& Obj)
            : CharacterEntries(Obj.CharacterEntries)
            , CharHeight(Obj.CharHeight)
            , FontHeight(Obj.FontHeight)
            , BasicLength(Obj.BasicLength)
            , CharMap(Obj.CharMap)
            , WideCharData(Obj.WideCharData)
            , WideCharMap(Obj.WideCharMap)
            , current_offset(Obj.current_offset)
            , CharData(Obj.CharData)
            , Widths(Obj.Widths)
            , KerningMap(Obj.KerningMap)
            , KerningData(Obj.KerningData)
            , KerningEntries(Obj.KerningEntries)
            , SpaceLength(Obj.SpaceLength)
        {
            CodeMap = Obj.CodeMap;
        }

        virtual ~Font();

        Font& operator=(const Font& Obj);

        uint16_t GetCharEntries() const;
        uint16_t GetFontHeight() const;
        uint16_t GetHeight() const;
        uint16_t GetBasicLength() const;
        uint16_t GetKerningEntries() const;

        uint32_t* GetCharData(uint32_t code) const;
        uint32_t* GetWideCharData(uint32_t code) const;
        uint16_t GetCharWidth(uint32_t code) const;
        int8_t GetKerning(uint32_t leftCode, uint32_t rightCode) const;

        uint16_t GetWidth(const char* text) const;
        int16_t GetSpaceLength() const;

        static unicode_character GetUnicodeCharacter(const char* in);

    protected:
        typedef map<uint16_t, uint16_t> Map;
        Map CodeMap;
        uint16_t CharacterEntries; // Number of characters
        uint16_t CharHeight; // Actual character height
        uint16_t FontHeight; // Desired font height
        uint16_t BasicLength; // Standard space
        map_entry* CharMap; // Char map location
        map_entry* WideCharMap; // WideChar map location
        uint8_t* CharData; // Char data location
        uint32_t* WideCharData; // WideChar data location
        uint8_t* Widths; // Widths map location
        uint16_t* KerningMap; // Kerning Map location (lookup table)
        kerning_entry* KerningData; // Kerning Data location
        uint16_t KerningEntries; // Number of kerning entries
        int16_t SpaceLength;
        uint32_t current_offset;

        void Init(const uint16_t* data);
    };

} /* namespace grvl */

#endif /* GRVL_Font_H_ */
