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

#include "PageProvisioning.hpp"

#include "Canvas.h"
#include "Color.hpp"
#include "DataManager.hpp"
#include "Time.hpp"

namespace PedalGuru {

static const char ACCESS_POINT_SSID[] = "pedal.guru";
static const char CONFIGURATION_URL[] = "http://" WIFI_ACCESS_POINT_ADDRESS ":3333/";
static const unsigned int SERVER_POLL_MILLISECONDS = 100;
static const unsigned int CONFIRMATION_HOLD_MILLISECONDS = 5000;
static const uint16_t SCAN_MAX_NETWORKS = 32;

void PageProvisioning::PreDrawPageContents() {
    if (!WiFiInitialize()) {
        state_ = ProvisioningState::UNAVAILABLE;
        return;
    }

    WiFiNetwork scanned[SCAN_MAX_NETWORKS];
    uint16_t foundNetworks {0};

    if (WiFiScan(scanned, SCAN_MAX_NETWORKS, &foundNetworks)) {
        for (uint16_t index = 0; index < foundNetworks; index++) {
            networks_.push_back(scanned[index]);
        }
    }

    if (!WiFiAccessPointStart(ACCESS_POINT_SSID, nullptr)) {
        state_ = ProvisioningState::UNAVAILABLE;
        return;
    }

    if (!server_.Start(networks_)) {
        state_ = ProvisioningState::UNAVAILABLE;
    }
}

void PageProvisioning::DrawPageContents() {
    DrawScreen();

    if (state_ == ProvisioningState::UNAVAILABLE) return;

    server_.Poll(SERVER_POLL_MILLISECONDS);

    if (server_.CredentialStoreFailed()) {
        state_ = ProvisioningState::STORE_FAILED;
    } else if (server_.CredentialsStored() && state_ != ProvisioningState::CONFIGURED) {
        state_ = ProvisioningState::CONFIGURED;
        confirmationStart_ = Time::TicksMs();
    }

    // Unsigned subtraction, so the hold survives the tick counter wrapping.
    if (state_ == ProvisioningState::CONFIGURED
        && (Time::TicksMs() - confirmationStart_) >= CONFIRMATION_HOLD_MILLISECONDS) {
        DataManager::GetInstance()->SetRestartRequested();
        drawer_.RequestClose();
    }
}

void PageProvisioning::DrawScreen() {
    screenTexture_.DrawCircle({120, 120}, 120, COLOR_WHITE, 1, true);

    switch (state_) {
        case ProvisioningState::SERVING:
            screenTexture_.DrawCurvedText(
                std::string("Connect to ") + ACCESS_POINT_SSID + " wi-fi then scan the QR code",
                { 120, 120},
                105,
                125,
                TEXT_ORIENTATION_INWARDS,
                16,
                COLOR_BLUE,
                COLOR_WHITE);
            screenTexture_.DrawQRCode(CONFIGURATION_URL, {55, 55});
            break;
        case ProvisioningState::CONFIGURED:
            screenTexture_.DrawText("Network saved", {55, 100}, 16, COLOR_DARK_GREEN, COLOR_WHITE);
            screenTexture_.DrawText("Restarting", {65, 125}, 16, COLOR_BLACK, COLOR_WHITE);
            break;
        case ProvisioningState::STORE_FAILED:
            screenTexture_.DrawText("Could not save", {50, 100}, 16, COLOR_RED, COLOR_WHITE);
            screenTexture_.DrawText("Check the card", {50, 125}, 16, COLOR_BLACK, COLOR_WHITE);
            break;
        case ProvisioningState::UNAVAILABLE:
            screenTexture_.DrawText("Cannot be", {70, 100}, 16, COLOR_RED, COLOR_WHITE);
            screenTexture_.DrawText("configured", {65, 125}, 16, COLOR_RED, COLOR_WHITE);
            break;
    }

    window_.DrawTexture(screenTexture_);
}

void PageProvisioning::PostDrawPageContents() {
    server_.Stop();
    WiFiAccessPointStop();
    WiFiDeinitialize();
    screenTexture_.Release();
}

}
