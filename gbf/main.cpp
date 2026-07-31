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
#include <string>

#include <grvl/grvl.h>
#include <grvl/Font.h>
#include <grvl/File.h>

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

    return std::stoi(cstr, nullptr, base);
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

    grvl::Log(grvl::INFO, "Source file contains %ld unique codepoints.", codepoints.size());

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

        grvl::Log(grvl::ERROR, "Invalid argument '%s', expected option.", args.Next());
        cfg.invalid = true;
        return;

    }

    // check required arguments
    if (cfg.ttf_path == nullptr) cfg.invalid = true;
    if (cfg.font_size == -1) cfg.invalid = true;
}

int main(int argc, const char* argv[])
{
    grvl::gui_callbacks_t callbacks {};
    grvl::grvl::Init(&callbacks);

    Config cfg;
    Args args {argv, 1, argc};
    ParseNext(cfg, args);

    if (cfg.help) {
        printf("Usage: gbf [OPTION]...\n");
        printf("Create GBF files from TTFs\n");

        printf("\nRequired options:\n");
        printf("  --ttf <path>     : Source TTF file path\n");
        printf("  --size <size>    : Size of the baked font\n");

        printf("\nOther options:\n");
        printf("  --help           : Print this help page and exit\n");
        printf("  --gbf <path>     : Output path, by default './out.gbf' is used\n");
        printf("  --range <ranges> : Range (or ranges) of Unicodes to bake\n");
        printf("  --source <file>  : Use a file as reference for Unicodes to bake\n");

        printf("\nRange format:\n");
        printf("  A comma separated list of ranges, each range can be\n");
        printf("  expressed as one of the following expressions:\n\n");

        printf("  <int>-<int>      : Inclusive range, <int> can be a decimal, hex (0x), or Unicode (U+)\n");
        printf("  <int>            : Include one specific codepoint\n");
        printf("  ascii            : Same as '0x20-0x78'\n");
        printf("  bmp              : Include the Unicode Basic Multilingual Plane\n");
        printf("  smp              : Include the Unicode Supplementary Multilingual Plane\n");
        printf("  sip              : Include the Unicode Supplementary Ideographic Plane\n");
        printf("  tip              : Include the Unicode Tertiary Ideographic Plane\n");
        printf("  ssp              : Include the Unicode Supplementary Special-purpose Plane\n");
        printf("  all              : Include all Unicode characters\n");

        printf("\nExamples:\n");
        printf("  gbf --ttf ./my_font.ttf --size 18\n");
        printf("  gbf --ttf ./my_font.ttf --size 11 --range ascii,0x100-0x200 --gbf ./my_font.gbf\n");
        printf("  gbf --ttf ./my_font.ttf --size 11 --source ./translation.txt --gbf ./my_font.gbf\n");
        return 0;
    }

    if (cfg.invalid) {
        grvl::Log(grvl::INFO, "Usage: gbf [OPTION]...");
        grvl::Log(grvl::INFO, "Use '--help' for a list of options.");
        return 1;
    }

    if (cfg.ranges.empty()) {
        grvl::Log(grvl::INFO, "Explicit range not given, using Basic Multilingual Plane.");
        cfg.ranges.push_back({' ', 0xFFFF});
    }

    auto data = std::make_shared<grvl::TrueTypeData>(cfg.ttf_path);
    grvl::TrueTypeFont ttf {data, cfg.font_size};

    grvl::Log(grvl::INFO, "Rasterizing...");

    for (const Range& range : cfg.ranges) {
        ttf.Preload(range.start, range.end);
    }

    grvl::Log(grvl::INFO, "Exporting...");
    ttf.Save(cfg.gbf_path);

    FILE* file = fopen(cfg.gbf_path, "rb");
    fseek(file, 0L, SEEK_END);
    size_t bytes = ftell(file);
    fclose(file);

    grvl::Log(grvl::INFO, "Done! Baked font saved to %s (%ld bytes)", cfg.gbf_path, bytes);

    return 0;

}
