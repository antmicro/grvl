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

#include "File.h"
#include "grvl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include <sys/stat.h>
#include <string>

namespace grvl {
    static constexpr auto bufferSize = 128;

    inline static bool EndsWith(const char* name, const char* ext)
    {
        return strlen(name) >= strlen(ext) && !strcmp(name + strlen(name) - strlen(ext), ext);
    }

    bool File::noFS = false;
    std::map<std::string, std::pair<unsigned char*, uint64_t>>* File::files;

    void File::NoFilesystem(std::map<std::string, std::pair<unsigned char*, uint64_t>>* _files) {
        noFS = true;
        files = _files;
    }

    File::File(const char* path)
        : path(path)
    {
        if(EndsWith(path, ".gz")) {
            readOnly = true;
            gzipped = true;
        } else {
            readOnly = false;
            gzipped = false;
        }
    }

    bool File::HasExtension(const char* ext)
    {
        size_t len = strlen(path);
        return !(len < 3 || strcmp(ext, &path[len - 3]) != 0);
    }

    int32_t File::GetSize()
    {
        if (!noFS) {
            if(gzipped) {
                // TODO: is it possible to check the size without decompression ?
                gzFile file = gzopen(path, "r");
                if(file == NULL) {
                    return -1;
                }
                uint32_t offset = 0;
                int read_bytes;
                char buffer[bufferSize];
                while((read_bytes = gzread(file, buffer, bufferSize)) > 0) {
                    offset += read_bytes;
                }
                if(!gzeof(file)) {
                    gzclose(file);
                    return -1;
                }
                gzclose(file);
                return offset;
            }
            struct stat file_stat;
            if(stat(path, &file_stat) == -1) {
                return -1;
            }

            return file_stat.st_size;
        }
        std::string delimeter = "/";
        std::string name = std::string(path).substr(std::string(path).rfind(delimeter)+1, std::string(path).length());

        if(files->count(name) == 0) {
            grvl::Log("No such file: %s", name.c_str());
            return -1;
        }

        return files->at(name).second;
    }

    bool File::ReadToBuffer(uint8_t*& buffer)
    {
        if (!noFS) {
            if(gzipped) {
                uint32_t offset = 0;
                size_t read_bytes;

                gzFile file = gzopen(path, "r");
                if(file == NULL) {
                    return false;
                }

                while((read_bytes = gzread(file, buffer + offset, bufferSize)) > 0) {
                    offset += read_bytes;
                }

                if(!gzeof(file)) {
                    gzclose(file);
                    return false;
                }
                gzclose(file);
            } else {
                uint32_t offset = 0;
                size_t read_bytes;

                FILE* file = fopen(path, "rb");
                if(file == NULL) {
                    return false;
                }

                while((read_bytes = fread(buffer + offset, 1, bufferSize, file)) > 0) {
                    offset += read_bytes;
                }

                if(!feof(file) || ferror(file)) {
                    fclose(file);
                    return false;
                }
                fclose(file);
            }
        } else {
            std::string delimeter = "/";
            std::string name = std::string(path).substr(std::string(path).rfind(delimeter)+1, std::string(path).length());

            if(files->count(name) == 0) {
                grvl::Log("No such file: %s", name.c_str());
                return -1;
            }

            buffer = files->at(name).first;
        }
        return true;
    }

    bool File::WriteFromBuffer(uint8_t*& buffer, uint32_t size)
    {
        if(readOnly) {
            return false;
        }
        FILE* file = fopen(path, "wb");
        if(file == NULL) {
            return false;
        }

        uint32_t offset = 0;
        size_t wrote_bytes;

        do {
            wrote_bytes = fwrite(buffer + offset, 1, size, file);
            offset += wrote_bytes;
            size -= wrote_bytes;
        } while(size > 0);

        if(!feof(file) || ferror(file)) {
            fclose(file);
            return false;
        }
        fclose(file);

        return true;
    }

} /* namespace grvl */
