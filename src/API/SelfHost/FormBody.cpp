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

#include "FormBody.hpp"

namespace PedalGuru {

namespace {

bool HexDigitValue(char digit, unsigned char &value) {
    if (digit >= '0' && digit <= '9') {
        value = static_cast<unsigned char>(digit - '0');
    } else if (digit >= 'a' && digit <= 'f') {
        value = static_cast<unsigned char>(digit - 'a' + 10);
    } else if (digit >= 'A' && digit <= 'F') {
        value = static_cast<unsigned char>(digit - 'A' + 10);
    } else {
        return false;
    }

    return true;
}

}

std::string FormBody::PercentDecode(const std::string &text) {
    std::string decoded;
    decoded.reserve(text.size());

    for (std::size_t index = 0; index < text.size(); ++index) {
        char current = text[index];

        if (current == '+') {
            decoded.push_back(' ');
            continue;
        }

        unsigned char high, low;

        if (current == '%' && index + 2 < text.size()
                && HexDigitValue(text[index + 1], high)
                && HexDigitValue(text[index + 2], low)) {
            decoded.push_back(static_cast<char>((high << 4) | low));
            index += 2;
            continue;
        }

        decoded.push_back(current);
    }

    return decoded;
}

FormBody::FormBody(const std::string &body) {
    std::size_t pairStart = 0;

    while (pairStart <= body.size()) {
        std::size_t pairEnd = body.find('&', pairStart);

        if (pairEnd == std::string::npos) {
            pairEnd = body.size();
        }

        std::string pair = body.substr(pairStart, pairEnd - pairStart);
        pairStart = pairEnd + 1;

        if (pair.empty()) {
            continue;
        }

        std::size_t separator = pair.find('=');
        std::string name = pair.substr(0, separator);
        std::string value = (separator == std::string::npos)
            ? std::string()
            : pair.substr(separator + 1);

        fields_[PercentDecode(name)] = PercentDecode(value);
    }
}

bool FormBody::Has(const std::string &name) const {
    return fields_.find(name) != fields_.end();
}

std::string FormBody::Value(const std::string &name) const {
    auto field = fields_.find(name);
    return (field == fields_.end()) ? std::string() : field->second;
}

}
