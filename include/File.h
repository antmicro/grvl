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

#ifndef GRVL_FILE_H_
#define GRVL_FILE_H_

#include <stdint.h>
#include <map>
#include <string>

namespace grvl {

    /// Represents a file stored on a file system.
    ///
    /// This class allows to get basic information
    /// of a file and load it into the memory.
    class File {
    public:
        File(const char* path);

        /**
         * @param _files <filename, <data, length>>
        */
        static void NoFilesystem(std::map<std::string, std::pair<unsigned char*, uint64_t>>* files);
        static bool noFS;

        /// @return File size in bytes.
        int32_t GetSize();
        bool ReadToBuffer(uint8_t*& buffer);
        bool WriteFromBuffer(uint8_t*& buffer, uint32_t size);
        bool HasExtension(const char* ext);

    private:
        const char* path;
        bool readOnly;
        bool gzipped;

        static std::map<std::string, std::pair<unsigned char*, uint64_t>>* files;
    };

} /* namespace grvl */

#endif /* GRVL_FILE_H */
