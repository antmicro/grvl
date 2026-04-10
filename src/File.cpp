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
#include <grvl/grvl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include <sys/stat.h>
#include <string>

namespace grvl {
    static constexpr auto bufferSize = 128;

    bool File::noFS = false;
    std::unordered_map<std::string, std::pair<unsigned char*, uint64_t>>* File::files;

    void File::NoFilesystem(std::unordered_map<std::string, std::pair<unsigned char*, uint64_t>>* _files) {
        noFS = true;
        files = _files;
    }

    File::File(const char* path)
        : path(path)
    {
        if(noFS) {
            readOnly = true;
            storage = DICTIONARY;
        }

        else if(EndsWith(path, ".gz")) {
            readOnly = true;
            storage = GZIPPED;
        }

        else {
            readOnly = false;
            storage = NORMAL;
        }
    }

    bool File::HasExtension(const char* ext) const
    {
    	const size_t min = strlen(ext);
        const size_t len = strlen(path);
        return len >= min && strcmp(ext, &path[len - min]) == 0;
    }

    bool File::Exists() const
    {
        if(storage == DICTIONARY) {
            std::string name = GetName();
            return files->find(name) != files->end();
        }

        if (storage == NORMAL || storage == GZIPPED) {
            FILE* file = fopen(path, "rb");

            if(file == nullptr) {
                return false;
            }

            fclose(file);
            return true;
        }

        return false;
    }

    std::string File::GetName() const
    {
        std::string path_str(path);
        return path_str.substr(path_str.rfind("/") + 1);
    }

    int32_t File::GetSize() const
    {
        if(storage == DICTIONARY) {
            std::string name = GetName();
            auto it = files->find(name);

            if(it == files->end()) {
                return 0;
            }

            return it->second.second;
        }

        if (storage == NORMAL) {
            struct stat file_stat;
            if(stat(path, &file_stat) == -1) {
                return 0;
            }

            return file_stat.st_size;
        }

        if(storage == GZIPPED) {
            // TODO: is it possible to check the size without decompression ?
            gzFile file = gzopen(path, "r");
            if(file == nullptr) {
                return 0;
            }
            uint32_t offset = 0;
            int read_bytes;
            char buffer[bufferSize];
            while((read_bytes = gzread(file, buffer, bufferSize)) > 0) {
                offset += read_bytes;
            }

            gzclose(file);
            return offset;
        }

        return 0;
    }

    int32_t File::ReadToBuffer(uint8_t* buffer, int32_t size) const
    {
        if(storage == DICTIONARY) {
            std::string name = GetName();
            const auto it = files->find(name);

            if (it == files->end()) {
                grvl::Log("[ERROR] No such filesystem entry: %s", name.c_str());
                return -1;
            }

            const auto [file_buffer, file_size] = it->second;
            const int32_t bytes = std::min(size, static_cast<int32_t>(file_size));

            memcpy(buffer, file_buffer, bytes);
            return bytes;
        }

        if(storage == GZIPPED) {
            size_t read_bytes;

            gzFile file = gzopen(path, "r");
            if(file == nullptr) {
                grvl::Log("No such file: %s", path);
                return -1;
            }

            while((size > 0) && ((read_bytes = gzread(file, buffer, size)) > 0)) {
                buffer += read_bytes;
                size -= read_bytes;
            }

            gzclose(file);
            return read_bytes;
        }

        if (storage == NORMAL) {
            uint32_t offset = 0;
            size_t read_bytes;

            FILE* file = fopen(path, "rb");
            if(file == nullptr) {
                grvl::Log("[ERROR] No such file: %s", path);
                return -1;
            }

            while((size > 0) && ((read_bytes = fread(buffer, 1, size, file)) > 0)) {
                buffer += read_bytes;
                size -= read_bytes;
            }

            fclose(file);
            return read_bytes;
        }

        return -1;
    }

    int32_t File::WriteFromBuffer(const uint8_t* buffer, int32_t size)
    {
        if(readOnly) {
            return 0;
        }

        if (storage == NORMAL) {
            FILE* file = fopen(path, "wb");
            if(file == nullptr) {
                grvl::Log("[ERROR] No such file: %s", path);
                return -1;
            }

            uint32_t offset = 0;
            size_t wrote_bytes;

            do {
                wrote_bytes = fwrite(buffer + offset, 1, size, file);
                offset += wrote_bytes;
                size -= wrote_bytes;
            } while(size > 0);

            fclose(file);
            return wrote_bytes;
        }

        return -1;
    }

    std::vector<char> File::Read() const
    {
        std::vector<char> buffer(GetSize());

        uint8_t* data = reinterpret_cast<uint8_t*>(buffer.data());
        ReadToBuffer(data, buffer.size());

        return buffer;
    }

    bool File::Write(const std::vector<char>& buffer)
    {
        return WriteFromBuffer(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size()) == buffer.size();
    }

} /* namespace grvl */
