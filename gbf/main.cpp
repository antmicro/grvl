// Copyright 2026 Antmicro <antmicro.com>
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

#include <unordered_set>

#include <grvl.h>
#include <Font.h>
#include <File.h>

static void PrintfNewline(const char* text, va_list argList)
{
    vprintf(text, argList);
    printf("\n");
}

struct Args
{
    const char** argv;
    int index;
    int count;

    const char* Next()
    {
        return argv[index ++];
    }

    bool IfNext(const char* expected)
    {
        bool matched = strcmp(argv[index], expected) == 0;

        if (matched) {
            index ++;
        }

        return matched;
    }

    bool HasNext()
    {
        return index < count;
    }
};

struct Range
{
    uint32_t start;
    uint32_t end;

    constexpr Range(uint32_t start, uint32_t end) : start(start), end(end) {}
};

struct Config
{
    const char* ttf_path = nullptr;
    const char* gbf_path = "./out.gbf";
    int font_size = -1;
    bool help = false;
    bool invalid = false;

    std::vector<Range> ranges;
};

static uint32_t ParseUnicode(const std::string& str)
{
    const char* cstr = str.c_str();
    size_t base = 10;

    if (str.length() > 2) {
        if ((str[0] == '0' && str[1] == 'x') | (str[0] == 'U' && str[1] == '+')) {
            base = 16;
            cstr += 2;
        }
    }

    return stoi(cstr, nullptr, base);
}

static Range ParseRange(const std::string& str)
{
    if (str == "ascii") return {' ', '~'};
    if (str == "bmp") return {' ', 0xFFFF}; // Basic Multilingual Plane
    if (str == "smp") return {0x10000, 0x1FFFF}; // Supplementary Multilingual Plane
    if (str == "sip") return {0x20000, 0x27FFF}; // Supplementary Ideographic Plane
    if (str == "tip") return {0x30000, 0x33FFF}; // Tertiary Ideographic Plane
    if (str == "ssp") return {0xE0000, 0xE0FFF}; // Supplement­ary Special-purpose Plane
    if (str == "all") return {' ', 0xF0000}; // All non-private, printable Unicodes

    auto pos = str.find_first_of('-');

    // single codepoint
    if (pos == std::string::npos) {
        auto codepoint = ParseUnicode(str.c_str());
        return {codepoint, codepoint};
    }

    // range
    auto first = str.substr(0, pos);
    auto second = str.substr(pos + 1);

    return {ParseUnicode(first.c_str()), ParseUnicode(second.c_str())};
}

static void ParseRanges(std::vector<Range>& ranges, const char* str)
{
    int start = 0;
    std::string pattern = str;

    // Uses '<=' as we want to read the null byte too
    for (int i = 0; i <= pattern.length(); i ++) {
        char c = str[i];

        if (c == 0 || c == ',') {
            auto fragment = pattern.substr(start, i - start);
            Range range = ParseRange(fragment);
            start = i + 1;

            if (range.end - range.start >= 0) {
                ranges.push_back(range);
            }
        }
    }
}

static void LoadSourceFile(std::vector<Range>& ranges, const char* path)
{
    grvl::File file {path};
    auto bytes = file.Read();
    bytes.push_back(0);

    int length = bytes.size() - 1;
    const char* string = bytes.data();

    std::unordered_set<uint32_t> codepoints;
    grvl::Unicode unicode {};

    for (int i = 0; i < length; i += unicode.length) {
        unicode = grvl::ParseUnicodeCodepoint(string + i);
        codepoints.insert(unicode.code);
    }

    grvl::grvl::Log("[INFO] Source file contains %ld unique codepoints.", codepoints.size());

    for (uint32_t codepoint : codepoints) {
        ranges.emplace_back(codepoint, codepoint);
    }
}

static void ParseNext(Config& cfg, Args& args)
{
    while (args.HasNext()) {

        if (args.IfNext("--ttf") && args.HasNext()) {
            cfg.ttf_path = args.Next();
            continue;
        }

        if (args.IfNext("--size") && args.HasNext()) {
            cfg.font_size = atoi(args.Next());
            continue;
        }

        if (args.IfNext("--gbf") && args.HasNext()) {
            cfg.gbf_path = args.Next();
            continue;
        }

        if (args.IfNext("--help")) {
            cfg.help = true;
            continue;
        }

        if (args.IfNext("--range") && args.HasNext()) {
            ParseRanges(cfg.ranges, args.Next());
            continue;
        }

        if (args.IfNext("--source") && args.HasNext()) {
            LoadSourceFile(cfg.ranges, args.Next());
            continue;
        }

        grvl::grvl::Log("[ERROR] Invalid argument '%s', expected option.", args.Next());
        cfg.invalid = true;
        return;

    }
}

int main(int argc, const char* argv[])
{

    grvl::gui_callbacks_t callbacks {};
    callbacks.gui_printf = PrintfNewline;
    grvl::grvl::Init(&callbacks);

    Config cfg;
    Args args {argv, 1, argc};
    ParseNext(cfg, args);

    if (cfg.invalid) {
        grvl::grvl::Log("[INFO] Usage: gbf [OPTION]...");
        grvl::grvl::Log("[INFO] Use '--help' for a list of options.");
        return 1;
    }

    if (cfg.help) {
        grvl::grvl::Log("Usage: gbf [OPTION]...");
        grvl::grvl::Log("Create GBF files from TTFs");

        grvl::grvl::Log("\nRequired options:");
        grvl::grvl::Log("  --ttf <path>     : Source TTF file path");
        grvl::grvl::Log("  --size <size>    : Size of the baked font");

        grvl::grvl::Log("\nOther options:");
        grvl::grvl::Log("  --help           : Print this help page and exit");
        grvl::grvl::Log("  --gbf <path>     : Output path, by default './out.gbf' is used");
        grvl::grvl::Log("  --range <ranges> : Range (or ranges) of Unicodes to bake");
        grvl::grvl::Log("  --source <file>  : Use a file as reference for Unicodes to bake");

        grvl::grvl::Log("\nRange format:");
        grvl::grvl::Log("  A comma separated list of ranges, each range can be");
        grvl::grvl::Log("  expressed as one of the following expressions:\n");

        grvl::grvl::Log("  <int>-<int>      : Inclusive range, <int> can be a decimal, hex (0x), or Unicode (U+)");
        grvl::grvl::Log("  <int>            : Include one specific codepoint");
        grvl::grvl::Log("  ascii            : Same as '0x20-0x78'");
        grvl::grvl::Log("  bmp              : Include the Unicode Basic Multilingual Plane");
        grvl::grvl::Log("  smp              : Include the Unicode Supplementary Multilingual Plane");
        grvl::grvl::Log("  sip              : Include the Unicode Supplementary Ideographic Plane");
        grvl::grvl::Log("  tip              : Include the Unicode Tertiary Ideographic Plane");
        grvl::grvl::Log("  ssp              : Include the Unicode Supplement­ary Special-purpose Plane");
        grvl::grvl::Log("  all              : Include the all Unicode characters");

        grvl::grvl::Log("\nExamples:");
        grvl::grvl::Log("  gbf --ttf ./my_font.ttf --size 18");
        grvl::grvl::Log("  gbf --ttf ./my_font.ttf --size 11 --range ascii,0x100-0x200 --gbf ./my_font.gbf");
        grvl::grvl::Log("  gbf --ttf ./my_font.ttf --size 11 --source ./translation.txt --gbf ./my_font.gbf");
        return 0;
    }

    if (cfg.ranges.empty()) {
        grvl::grvl::Log("[INFO] Explicit range not given, using Basic Multilingual Plane.");
        cfg.ranges.push_back({' ', 0xFFFF});
    }

    auto data = std::make_shared<grvl::TrueTypeData>(cfg.ttf_path);
    grvl::TrueTypeFont ttf {data, cfg.font_size};

    grvl::grvl::Log("[INFO] Rasterizing...");

    for (const Range& range : cfg.ranges) {
        ttf.Preload(range.start, range.end);
    }

    grvl::grvl::Log("[INFO] Exporting...");
    ttf.Save(cfg.gbf_path);

    FILE* file = fopen(cfg.gbf_path, "rb");
    fseek(file, 0L, SEEK_END);
    size_t bytes = ftell(file);
    fclose(file);

    grvl::grvl::Log("[INFO] Done! Baked font saved to %s (%ld bytes)", cfg.gbf_path, bytes);

    return 0;

}
