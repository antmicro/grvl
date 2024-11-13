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

#include "ParsingUtils.h"

namespace grvl {

    string ParsingUtils::GetFunctionName(const string& CallbackString)
    {
        return CallbackString.substr(0, CallbackString.find_first_of(" (", 0));
    }

    Event::ArgVector ParsingUtils::GetArguments(const string& CallbackString)
    {
        Event::ArgVector Vec;
        size_t start_pos = CallbackString.find_first_of("(", 0); //NOLINT

        unsigned int currentChar = start_pos + 1;
        string argument;

        enum mode {
            Regular,
            Text
        } currentMode
            = Regular;

        while(currentChar < CallbackString.size()) {
            switch(CallbackString[currentChar]) {
                case ' ':
                    if(currentMode == Regular) {
                        break;
                    } else if(currentMode == Text) {
                        argument.push_back(CallbackString[currentChar]);
                    }
                    break;
                case '\\': // add special character
                    if(currentChar + 1 < CallbackString.size() && currentMode == Text) {
                        currentChar++;
                        switch(CallbackString[currentChar]) {
                            case '\'':
                                argument.push_back('\'');
                                break;
                            case 'r': // CR
                                argument.push_back('\r');
                                break;
                            case 'n': // NL
                                argument.push_back('\n');
                                break;
                            case 'b': // BS
                                argument.push_back('\b');
                                break;
                            case 't': // TAB
                                argument.push_back('\t');
                                break;
                        }
                    }
                    break;
                case '\'':
                    if(currentMode == Regular) {
                        currentMode = Text;
                    } else if(currentMode == Text) {
                        currentMode = Regular;
                    }
                    break;
                case ')':
                    if(currentMode == Regular) {
                        Vec.push_back(argument);
                        currentChar = CallbackString.size(); // Finish parsing
                    } else {
                        argument.push_back(CallbackString[currentChar]);
                    }
                    break;
                case ',':
                    if(currentMode == Regular) {
                        Vec.push_back(argument);
                        argument.clear();
                    } else if(currentMode == Text) {
                        argument.push_back(CallbackString[currentChar]);
                    }
                    break;
                default:
                    argument.push_back(CallbackString[currentChar]);
                    break;
            }
            currentChar++;
        }
        return Vec;
    }

} /* namespace grvl */
