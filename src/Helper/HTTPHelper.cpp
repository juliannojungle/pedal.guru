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

#include <stdio.h>
#include <string>
#include "FileHelper.cpp"

extern "C" {
    #include <curl/curl.h>
}

namespace OpenCC {

class HTTPHelper {
    public:
        static void DownloadFile(std::string url, std::string filePath);
};

size_t WriteData(void *elements, size_t size, size_t count, FILE *file) {
    size_t elementsWritten = fwrite(elements, size, count, file);
    return elementsWritten;
}

void HTTPHelper::DownloadFile(std::string url, std::string filePath) {
    FileHelper::CreatePathDirectories(filePath.c_str());

    CURL *curl = curl_easy_init();
    FILE *file;
    CURLcode result;
    struct curl_slist *list = NULL;

    if (curl) {
        file = fopen(filePath.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        list = curl_slist_append(list, "User-Agent: OpenCC/0.alpha (Raspberry Pi OS; armv6)");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        // TODO: check result: if failed retry 5 times. If still failing, delete empty file.
        result = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(file);
    }
}

}