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

#pragma once

#include <list>
#include <string>

#include "ConfigurationPage.hpp"

extern "C" {
#include "HttpServer.h"

void ServeDefault(const HttpRequest *request, HttpResponse *response, void *context);
void ServeRoot(const HttpRequest *request, HttpResponse *response, void *context);
void ServeSave(const HttpRequest *request, HttpResponse *response, void *context);
void ServeWifiScanStart(const HttpRequest *request, HttpResponse *response, void *context);
void ServeWifiScanGetStatus(const HttpRequest *request, HttpResponse *response, void *context);
void ServeWifiScanGetResults(const HttpRequest *request, HttpResponse *response, void *context);
}

namespace PedalGuru {

class Server {
    private:
        ConfigurationPage page_;
        std::list<WiFiNetwork> networks_;
        std::string responseBody_;
        bool credentialsStored_ {false};
        bool credentialStoreFailed_ {false};
        void OnDefault(const HttpRequest *request, HttpResponse *response);
        void OnRoot(const HttpRequest *request, HttpResponse *response);
        void OnWifiScanStart(const HttpRequest *request, HttpResponse *response);
        void OnWifiScanGetStatus(const HttpRequest *request, HttpResponse *response);
        void OnWifiScanGetResults(const HttpRequest *request, HttpResponse *response);
        void OnSave(const HttpRequest *request, HttpResponse *response);
        void Respond(HttpResponse *response, uint16_t statusCode, const char *contentType,
            const std::string &body);
    public:
        bool Start();
        void Stop();
        bool Poll(unsigned int timeoutMilliseconds);
        bool CredentialsStored() const;
        bool CredentialStoreFailed() const;
        friend void ::ServeDefault(const HttpRequest *request, HttpResponse *response, void *context);
        friend void ::ServeRoot(const HttpRequest *request, HttpResponse *response, void *context);
        friend void ::ServeWifiScanStart(const HttpRequest *request, HttpResponse *response, void *context);
        friend void ::ServeWifiScanGetStatus(const HttpRequest *request, HttpResponse *response, void *context);
        friend void ::ServeWifiScanGetResults(const HttpRequest *request, HttpResponse *response, void *context);
        friend void ::ServeSave(const HttpRequest *request, HttpResponse *response, void *context);
};

}
