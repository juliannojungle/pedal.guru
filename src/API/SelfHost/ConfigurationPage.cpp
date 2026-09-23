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

#include "ConfigurationPage.hpp"

#include <cstdio>

namespace PedalGuru {

static const char PAGE_TEMPLATE[] = R"HTML(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>pedal.guru</title><style>
body { font-family: sans-serif; margin: 0; padding: 1rem; background: #16191d; color: #e8e8e8; }
h1 { font-size: 1.2rem; margin: 0 0 1rem; }
label { display: block; margin: 0.8rem 0 0.2rem; font-size: 0.9rem; }
select, input { width: 100%; box-sizing: border-box; padding: 0.5rem; font-size: 1rem;
    border: 1px solid #444; border-radius: 4px; background: #21262c; color: #e8e8e8; }
button { margin-top: 1rem; width: 100%; padding: 0.6rem; font-size: 1rem; border: 0;
    border-radius: 4px; background: #2f7d32; color: #fff; }
button.secondary { background: #3a4149; }
div.network { display: flex; gap: 0.5rem; align-items: stretch; }
div.network select { flex: 1; }
button#scan { margin-top: 0; width: auto; flex: 0 0 auto; white-space: nowrap; }
p#status { min-height: 1.2rem; font-size: 0.9rem; color: #f0b429; }
</style></head>
<body>
<h1>pedal.guru</h1>
<form method="post" action="/settings/save" accept-charset="UTF-8">
<label for="ssid">Network</label>
<div class="network">
<select id="ssid" name="ssid" size="8"></select>
<button id="scan" class="secondary" type="button">Scan</button>
</div>
<label for="ssidTyped">Or type the network name</label>
<input id="ssidTyped" type="text" name="ssidTyped" autocomplete="off">
<label for="password">Password</label>
<input id="password" type="password" name="password" autocomplete="off">
<button type="submit">Save</button>
</form>
<p id="status"></p>
<script>
var scan = document.getElementById('scan');
var select = document.getElementById('ssid');
var status = document.getElementById('status');
var pollTimer = null;
var pollDeadline = 0;
function finishScan(message) {
    clearInterval(pollTimer);
    pollTimer = null;
    scan.disabled = false;
    status.textContent = message;
}
function renderNetworks(networks) {
    var chosen = select.value;
    select.innerHTML = '';
    if (!networks.length) {
        var empty = document.createElement('option');
        empty.disabled = true;
        empty.textContent = 'no network found';
        select.appendChild(empty);
        finishScan('No network found.');
        return;
    }
    for (var index = 0; index < networks.length; index++) {
        var network = networks[index];
        var option = document.createElement('option');
        option.value = network.ssid;
        option.textContent = network.ssid + ' (' + network.auth + ', ' + network.rssi + ' dBm)';
        select.appendChild(option);
    }
    select.value = chosen;
    finishScan('');
}
function fetchResults() {
    var request = new XMLHttpRequest();
    request.open('GET', '/network/scan/results', true);
    request.onreadystatechange = function () {
        if (request.readyState !== 4) {
            return;
        }
        if (request.status !== 200) {
            finishScan('Scan failed.');
            return;
        }
        var networks;
        try {
            networks = JSON.parse(request.responseText);
        } catch (error) {
            finishScan('Scan failed.');
            return;
        }
        renderNetworks(networks);
    };
    request.send();
}
function pollStatus() {
    if (Date.now() >= pollDeadline) {
        finishScan('Scan timed out.');
        return;
    }
    var request = new XMLHttpRequest();
    request.open('GET', '/network/scan/status', true);
    request.onreadystatechange = function () {
        if (request.readyState !== 4 || pollTimer === null) {
            return;
        }
        if (request.status !== 200) {
            return;
        }
        var state;
        try {
            state = JSON.parse(request.responseText);
        } catch (error) {
            return;
        }
        if (state.scanComplete) {
            clearInterval(pollTimer);
            pollTimer = null;
            fetchResults();
        }
    };
    request.send();
}
scan.addEventListener('click', function () {
    if (pollTimer !== null) {
        return;
    }
    scan.disabled = true;
    status.textContent = 'Scanning...';
    var request = new XMLHttpRequest();
    request.open('GET', '/network/scan/start', true);
    request.onreadystatechange = function () {
        if (request.readyState !== 4) {
            return;
        }
        if (request.status !== 202 && request.status !== 200) {
            finishScan('Scan failed to start.');
            return;
        }
        pollDeadline = Date.now() + 10000;
        pollTimer = setInterval(pollStatus, 1000);
    };
    request.send();
});
</script>
</body></html>
)HTML";

namespace {

const char *AuthModeLabel(WiFiAuthMode mode) {
    switch (mode) {
        case WIFI_AUTH_MODE_OPEN: return "open";
        case WIFI_AUTH_MODE_WEP: return "WEP";
        case WIFI_AUTH_MODE_WPA: return "WPA";
        case WIFI_AUTH_MODE_WPA2: return "WPA2";
        case WIFI_AUTH_MODE_WPA3: return "WPA3";
        default: return "unidentified";
    }
}

std::list<WiFiNetwork> PresentedNetworks(const std::list<WiFiNetwork> &networks) {
    std::list<WiFiNetwork> presented;

    for (const auto &network : networks) {
        if (network.Ssid[0] == '\0') {
            continue;
        }

        auto existing = presented.begin();

        while (existing != presented.end() && std::string(existing->Ssid) != std::string(network.Ssid)) {
            ++existing;
        }

        if (existing == presented.end()) {
            presented.push_back(network);
        } else if (network.Rssi > existing->Rssi) {
            *existing = network;
        }
    }

    return presented;
}

bool IsValidUtf8(const std::string &text) {
    std::size_t index = 0;

    while (index < text.size()) {
        unsigned char lead = static_cast<unsigned char>(text[index]);
        std::size_t continuationBytes;

        if (lead < 0x80) {
            continuationBytes = 0;
        } else if (lead >= 0xC2 && lead <= 0xDF) {
            continuationBytes = 1;
        } else if (lead >= 0xE0 && lead <= 0xEF) {
            continuationBytes = 2;
        } else if (lead >= 0xF0 && lead <= 0xF4) {
            continuationBytes = 3;
        } else {
            return false;
        }

        if (index + continuationBytes >= text.size() && continuationBytes > 0) {
            return false;
        }

        for (std::size_t offset = 1; offset <= continuationBytes; ++offset) {
            unsigned char continuation = static_cast<unsigned char>(text[index + offset]);

            if (continuation < 0x80 || continuation > 0xBF) {
                return false;
            }
        }

        index += continuationBytes + 1;
    }

    return true;
}

void AppendUnicodeEscape(std::string &escaped, unsigned char byte) {
    char digits[7];
    std::snprintf(digits, sizeof(digits), "\\u%04X", byte);
    escaped.append(digits);
}

std::string JsonEscape(const std::string &text) {
    bool escapeHighBytes = !IsValidUtf8(text);
    std::string escaped;
    escaped.reserve(text.size());

    for (char character : text) {
        unsigned char byte = static_cast<unsigned char>(character);

        switch (byte) {
            case '"': escaped.append("\\\""); continue;
            case '\\': escaped.append("\\\\"); continue;
            case '\b': escaped.append("\\b"); continue;
            case '\f': escaped.append("\\f"); continue;
            case '\n': escaped.append("\\n"); continue;
            case '\r': escaped.append("\\r"); continue;
            case '\t': escaped.append("\\t"); continue;
            default: break;
        }

        if (byte < 0x20 || (byte >= 0x80 && escapeHighBytes)) {
            AppendUnicodeEscape(escaped, byte);
            continue;
        }

        escaped.push_back(character);
    }

    return escaped;
}

}

std::string ConfigurationPage::RenderNetworkJson(const std::list<WiFiNetwork> &networks) const {
    std::string json("[");
    bool first = true;

    for (const auto &network : PresentedNetworks(networks)) {
        if (!first) {
            json.append(",");
        }

        first = false;
        json.append("{\"ssid\":\"").append(JsonEscape(network.Ssid))
            .append("\",\"auth\":\"").append(AuthModeLabel(network.AuthMode))
            .append("\",\"rssi\":").append(std::to_string(network.Rssi)).append("}");
    }

    return json.append("]");
}

const char *ConfigurationPage::Template() const {
    return PAGE_TEMPLATE;
}

std::size_t ConfigurationPage::TemplateLength() const {
    return sizeof(PAGE_TEMPLATE) - 1;
}

}
