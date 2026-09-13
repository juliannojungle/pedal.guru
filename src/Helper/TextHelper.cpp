/*
    Pedal.guru is an open-source software
    for cycle computers based on DIY hardware (MCUs like RP2040 and ESP32-S3).
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

#include "TextHelper.hpp"

namespace PedalGuru {

bool TextHelper::Contains(char* string, char* substring) {
    std::size_t substringLength = std::strlen(substring);
    if (std::strlen(string) < substringLength) return false;

    for (std::size_t i = 0; i < substringLength; i++)
        if (string[i] != substring[i]) return false;

    return true;
}

void TextHelper::Tokenize(std::string &source, char delimiter, char checksumChar, char (&target)[16][16]) {
    int item = 0, itemIndex = 0;
    for (std::size_t index = 0; index < source.size(); index++) {
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
    std::string token = "";
    std::vector<std::string> tokens;

    for (std::size_t i = 0; i < text.size(); i++) {
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