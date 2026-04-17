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

#ifndef GRVL_FILE_H_
#define GRVL_FILE_H_

#include <stdint.h>
#include <unordered_map>
#include <string>
#include <vector>

namespace grvl {

    /// Represents a file stored on a file system.
    ///
    /// This class allows to get basic information
    /// of a file and load it into the memory.
    class File {
    public:
        File(const char* path);

        using MapEntry = std::pair<unsigned char*, uint64_t>;

        /// @param _files <filename, <data, length>>
        static void NoFilesystem(std::unordered_map<std::string, MapEntry>* files);
        static bool noFS;

        /// Check if this file exists
        bool Exists() const;

        /// Get the filename of this file
        std::string GetName() const;

        /// @return File size in bytes. If the file is missing 0 is returned.
        int32_t GetSize() const;

        /// Read file contains to buffer, up to the given size.
        /// @param buffer Buffer to be written to, must be at least 'size' bytes long.
        /// @param size Number of bytes to read.
        /// @return Number of bytes read, if this is less than was requested the file was shorter. -1 is returned if the file is missing.
        int32_t ReadToBuffer(uint8_t* buffer, int32_t size) const;

        /// Write buffer to file, up to the given size.
        /// @param buffer Buffer to be written, must be at least 'size' bytes long.
        /// @param size Number of bytes to write.
        /// @return Number of bytes written, if this is less than was requested an error occured, -1 is returned if the file is missing.
        int32_t WriteFromBuffer(const uint8_t* buffer, int32_t size);

        /// Check if the filename ends with a specific extension.
        bool HasExtension(const char* ext) const;

        /// Read file into a buffer
        std::vector<char> Read() const;

        /// Write the given buffer to the file
        bool Write(const std::vector<char>& data);

    private:
        enum Storage : uint8_t {
            NORMAL,
            GZIPPED,
            DICTIONARY,
        };

        const char* path;
        bool readOnly;
        Storage storage;

        static std::unordered_map<std::string, MapEntry>* files;
    };

} /* namespace grvl */

#endif /* GRVL_FILE_H */
