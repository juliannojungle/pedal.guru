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

#include "Server.hpp"

#include <cstddef>

#include "DataManager.hpp"
#include "FormBody.hpp"

extern "C" {

void ServeRoot(const HttpRequest *request, HttpResponse *response, void *context) {
    static_cast<PedalGuru::Server*>(context)->OnRoot(request, response);
}

void ServeScan(const HttpRequest *request, HttpResponse *response, void *context) {
    static_cast<PedalGuru::Server*>(context)->OnScan(request, response);
}

void ServeSave(const HttpRequest *request, HttpResponse *response, void *context) {
    static_cast<PedalGuru::Server*>(context)->OnSave(request, response);
}

}

namespace PedalGuru {

static const uint16_t SERVER_PORT {80};
static const uint16_t SCAN_MAX_NETWORKS {20};
static const std::size_t SSID_MAX_LENGTH {WIFI_SSID_MAX_LENGTH};
static const std::size_t PASSWORD_MIN_LENGTH {8};
static const std::size_t PASSWORD_MAX_LENGTH {63};

namespace {

/* An SSID absent from the scan carries no reported mode, so it is not open. */
bool IsOpenNetwork(const std::list<WiFiNetwork> &networks, const std::string &ssid) {
    for (const auto &network : networks) {
        if (ssid == network.Ssid) {
            return network.AuthMode == WIFI_AUTH_MODE_OPEN;
        }
    }

    return false;
}

bool IsBlank(const std::string &text) {
    for (char character : text) {
        if (character != ' ' && character != '\t') {
            return false;
        }
    }

    return true;
}

}

bool Server::Start(const std::list<WiFiNetwork> &networks) {
    networks_ = networks;
    credentialsStored_ = false;
    credentialStoreFailed_ = false;

    if (!HttpServerStart(SERVER_PORT)) {
        return false;
    }

    return HttpServerRegisterEndpoint(HTTP_METHOD_GET, "/", ServeRoot, this)
        && HttpServerRegisterEndpoint(HTTP_METHOD_GET, "/network/scan", ServeScan, this)
        && HttpServerRegisterEndpoint(HTTP_METHOD_POST, "/settings/save", ServeSave, this);
}

void Server::Stop() {
    HttpServerStop();
}

bool Server::Poll(unsigned int timeoutMilliseconds) {
    return HttpServerPoll(timeoutMilliseconds);
}

bool Server::CredentialsStored() const {
    return credentialsStored_;
}

bool Server::CredentialStoreFailed() const {
    return credentialStoreFailed_;
}

void Server::OnRoot(const HttpRequest *request, HttpResponse *response) {
    (void)request;
    response->ContentType = "text/html";

    if (networks_.empty()) {
        response->Body = page_.Template();
        response->BodyLength = page_.TemplateLength();
        return;
    }

    responseBody_ = page_.Render(networks_);
    response->Body = responseBody_.data();
    response->BodyLength = responseBody_.size();
}

void Server::OnScan(const HttpRequest *request, HttpResponse *response) {
    (void)request;
    WiFiNetwork scanned[SCAN_MAX_NETWORKS];
    uint16_t found {0};

    if (!WiFiScan(scanned, SCAN_MAX_NETWORKS, &found)) {
        Respond(response, 503, "text/plain", "Scan failed.");
        return;
    }

    networks_.assign(scanned, scanned + found);
    Respond(response, 200, "application/json", page_.RenderNetworkJson(networks_));
}

void Server::OnSave(const HttpRequest *request, HttpResponse *response) {
    FormBody body(request->Body == nullptr
        ? std::string()
        : std::string(request->Body, request->BodyLength));
    std::string typed = body.Value("ssidTyped");
    std::string ssid = typed.empty() ? body.Value("ssid") : typed;
    std::string password = body.Value("password");

    if (ssid.empty() || IsBlank(ssid) || ssid.size() > SSID_MAX_LENGTH) {
        Respond(response, 400, "text/plain", "The network name was rejected.");
        return;
    }

    if (!IsOpenNetwork(networks_, ssid)
        && (password.size() < PASSWORD_MIN_LENGTH || password.size() > PASSWORD_MAX_LENGTH)) {
        Respond(response, 400, "text/plain", "The password was rejected.");
        return;
    }

    if (!DataManager::GetInstance()->WriteCredentials(ssid, password)) {
        credentialStoreFailed_ = true;
        Respond(response, 500, "text/plain", "The credentials were not stored.");
        return;
    }

    credentialsStored_ = true;
    Respond(response, 200, "text/html",
        "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">"
        "<title>pedal.guru</title></head><body>"
        "<h1>pedal.guru</h1><p>The device has been configured and is restarting.</p>"
        "</body></html>");
}

void Server::Respond(HttpResponse *response, uint16_t statusCode, const char *contentType,
        const std::string &body) {
    responseBody_ = body;
    response->StatusCode = statusCode;
    response->ContentType = contentType;
    response->Body = responseBody_.data();
    response->BodyLength = responseBody_.size();
}

}
