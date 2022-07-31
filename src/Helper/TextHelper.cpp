/*
    Open Cycle Computer (aka OpenCC) is an open-source software
    for cycle computers based on DIY hardware (primarily Raspberry Pi).
    Copyright (C) 2022, Julianno F. C. Silva (@juliannojungle)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/agpl-3.0.html>.
*/

#pragma once

#include <vector>
#include <string>

namespace OpenCC {

class TextHelper {
    public:
        static void Tokenize(std::string &source, char delimiter, char checksumChar, char (&target)[16][16]);
        static std::vector<std::string> Tokenize(std::string &text, char delimiter, char checksumChar);
        static bool contains(char* string, char* substring);
};

bool TextHelper::contains(char* string, char* substring) {
    if (strlen(string) < strlen(substring)) return false;

    for (int i = 0; i < strlen(substring); i++)
        if (string[i] != substring[i]) return false;

    return true;
}

void TextHelper::Tokenize(std::string &source, char delimiter, char checksumChar, char (&target)[16][16]) {
    int index, item = 0, itemIndex = 0;
    for (index = 0; index < source.size(); index++) {
        if (source[index] == delimiter) {
            target[item][itemIndex] = '\0';
            item++;
            itemIndex = 0;
        } else {
            if (source[index] == checksumChar) {
                target[item][itemIndex] = '\0';
                item++;
                itemIndex = 0;
            }

            target[item][itemIndex] = source[index];
            itemIndex++;
        }
    }

    target[item][itemIndex] = '\0';
}

std::vector<std::string> TextHelper::Tokenize(std::string &text, char delimiter, char checksumChar) {
    int i;
    std::string token = "";
    std::vector<std::string> tokens;

    for (i = 0; i < text.size(); i++) {
        if (text[i] == delimiter) {
            tokens.push_back(token);
            token = "";
        } else {
            if (text[i] == checksumChar) {
                tokens.push_back(token);
                token = "";
            }

            token += text[i];
        }
    }

    if (token != "") tokens.push_back(token);

    return tokens;
}

}