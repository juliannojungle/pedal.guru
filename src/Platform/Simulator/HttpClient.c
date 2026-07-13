/*
    Pedal.guru is an open-source software
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

#include "HttpClient.h"
#include "FileSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP_BUFFER_SIZE 4096
#define USER_AGENT "PedalGuru/0.alpha (Linux; Simulator)"

/* --------------------------------------------------------------------------
 * Internal: strip the scheme from the URL and return whether it's HTTPS.
 * -------------------------------------------------------------------------- */
static bool ParseScheme(const char **cursor) {
    if (strncmp(*cursor, "https://", 8) == 0) {
        *cursor += 8;
        return true;
    }
    if (strncmp(*cursor, "http://", 7) == 0) {
        *cursor += 7;
    }
    return false;
}

/* --------------------------------------------------------------------------
 * Internal: extract the host portion from the cursor (up to ':' or '/').
 * -------------------------------------------------------------------------- */
static void ParseHost(const char *cursor, const char *pathStart,
                      const char *portStart, char *host, size_t hostLen) {
    size_t hLen;
    if (portStart && (!pathStart || portStart < pathStart)) {
        hLen = (size_t)(portStart - cursor);
    } else {
        hLen = pathStart ? (size_t)(pathStart - cursor) : strlen(cursor);
    }
    if (hLen >= hostLen) hLen = hostLen - 1;
    strncpy(host, cursor, hLen);
    host[hLen] = '\0';
}

/* --------------------------------------------------------------------------
 * Internal: extract the port from the cursor, or use the default for scheme.
 * -------------------------------------------------------------------------- */
static void ParsePort(const char *portStart, const char *pathStart,
                      bool https, char *port, size_t portLen) {
    if (portStart && (!pathStart || portStart < pathStart)) {
        portStart++; /* skip ':' */
        const char *portEnd = pathStart ? pathStart : portStart + strlen(portStart);
        size_t pLen = (size_t)(portEnd - portStart);
        if (pLen >= portLen) pLen = portLen - 1;
        strncpy(port, portStart, pLen);
        port[pLen] = '\0';
    } else {
        strncpy(port, https ? "443" : "80", portLen - 1);
        port[portLen - 1] = '\0';
    }
}

/* --------------------------------------------------------------------------
 * Internal: extract the path portion from the URL (everything from first '/').
 * -------------------------------------------------------------------------- */
static void ParsePath(const char *pathStart, char *path, size_t pathLen) {
    if (pathStart) {
        strncpy(path, pathStart, pathLen - 1);
        path[pathLen - 1] = '\0';
    } else {
        strncpy(path, "/", pathLen - 1);
        path[pathLen - 1] = '\0';
    }
}

/* --------------------------------------------------------------------------
 * Internal: parse URL into host, port, and path components.
 * Returns true if the URL uses HTTPS.
 * -------------------------------------------------------------------------- */
static bool ParseUrl(const char *url, char *host, size_t hostLen,
                     char *port, size_t portLen,
                     char *path, size_t pathLen) {
    const char *cursor = url;
    bool https = ParseScheme(&cursor);

    const char *pathStart = strchr(cursor, '/');
    const char *portStart = strchr(cursor, ':');

    ParseHost(cursor, pathStart, portStart, host, hostLen);
    ParsePort(portStart, pathStart, https, port, portLen);
    ParsePath(pathStart, path, pathLen);

    return https;
}

/* --------------------------------------------------------------------------
 * Internal: open a TCP connection to host:port.
 * Returns the socket fd, or -1 on failure.
 * -------------------------------------------------------------------------- */
static int ConnectSocket(const char *host, const char *port) {
    struct addrinfo hints, *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        printf("HttpClient: getaddrinfo failed for %s:%s\n", host, port);
        return -1;
    }

    int sockfd = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd < 0) continue;
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);

    if (sockfd < 0) {
        printf("HttpClient: could not connect to %s:%s\n", host, port);
    }
    return sockfd;
}

/* --------------------------------------------------------------------------
 * Internal: build the HTTP GET request string.
 * -------------------------------------------------------------------------- */
static int BuildRequest(char *buffer, size_t bufferSize,
                        const char *host, const char *path) {
    return snprintf(buffer, bufferSize,
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "User-Agent: %s\r\n"
        "Accept: */*\r\n"
        "\r\n",
        path, host, USER_AGENT);
}

/* --------------------------------------------------------------------------
 * Internal: skip HTTP response headers, returning a pointer to the start of
 * the body within the buffer, and the number of body bytes already read.
 * -------------------------------------------------------------------------- */
static const char *SkipHeaders(const char *buffer, int totalRead, int *bodyBytes) {
    const char *headerEnd = strstr(buffer, "\r\n\r\n");
    if (!headerEnd) {
        *bodyBytes = 0;
        return NULL;
    }
    headerEnd += 4; /* skip past \r\n\r\n */
    *bodyBytes = totalRead - (int)(headerEnd - buffer);
    return headerEnd;
}

/* --------------------------------------------------------------------------
 * Internal: initialize OpenSSL and establish a TLS session over the socket.
 * Returns the SSL pointer on success, NULL on failure.
 * Caller must free ssl and context on success via CleanupSSL.
 * -------------------------------------------------------------------------- */
static SSL *EstablishTLS(int sockfd, const char *host, SSL_CTX **outCtx) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX *context = SSL_CTX_new(TLS_client_method());
    if (!context) {
        printf("HttpClient: SSL_CTX_new failed\n");
        return NULL;
    }

    SSL *ssl = SSL_new(context);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);

    if (SSL_connect(ssl) <= 0) {
        printf("HttpClient: SSL_connect failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        SSL_CTX_free(context);
        return NULL;
    }

    *outCtx = context;
    return ssl;
}

/* --------------------------------------------------------------------------
 * Internal: send data through SSL or plain socket.
 * -------------------------------------------------------------------------- */
static void SendRequest(int sockfd, SSL *ssl, const char *data, int len) {
    if (ssl) {
        SSL_write(ssl, data, len);
    } else {
        send(sockfd, data, (size_t)len, 0);
    }
}

/* --------------------------------------------------------------------------
 * Internal: receive data through SSL or plain socket.
 * Returns bytes read, or <= 0 on EOF/error.
 * -------------------------------------------------------------------------- */
static int ReceiveData(int sockfd, SSL *ssl, char *buffer, size_t bufferSize) {
    if (ssl) {
        return SSL_read(ssl, buffer, (int)bufferSize);
    }
    return (int)recv(sockfd, buffer, bufferSize, 0);
}

/* --------------------------------------------------------------------------
 * Internal: read the full response, skipping headers, and write body to file.
 * Returns true on success.
 * -------------------------------------------------------------------------- */
static bool ReadResponseToFile(int sockfd, SSL *ssl, const char *filePath) {
    char buffer[HTTP_BUFFER_SIZE];
    char headerBuffer[HTTP_BUFFER_SIZE * 2];
    int headerBufferLen = 0;
    int bytesRead;
    bool headersSkipped = false;
    FIL file;
    bool fileOpened = false;

    while ((bytesRead = ReceiveData(sockfd, ssl, buffer, sizeof(buffer))) > 0) {
        if (!headersSkipped) {
            /* Accumulate into headerBuffer until we find \r\n\r\n */
            int space = (int)sizeof(headerBuffer) - headerBufferLen;
            int toCopy = bytesRead < space ? bytesRead : space;
            memcpy(headerBuffer + headerBufferLen, buffer, (size_t)toCopy);
            headerBufferLen += toCopy;

            int bodyBytes = 0;
            const char *body = SkipHeaders(headerBuffer, headerBufferLen, &bodyBytes);
            if (!body) continue;

            headersSkipped = true;

            if (!OpenFile(&file, filePath)) {
                printf("HttpClient: could not open file %s\n", filePath);
                return false;
            }
            fileOpened = true;

            if (bodyBytes > 0) {
                WriteFile(&file, (void *)body, (unsigned int)bodyBytes);
            }
        } else {
            WriteFile(&file, buffer, (unsigned int)bytesRead);
        }
    }

    if (fileOpened) {
        CloseFile(&file);
    }
    return fileOpened;
}

/* --------------------------------------------------------------------------
 * Internal: shutdown and free SSL resources.
 * -------------------------------------------------------------------------- */
static void CleanupSSL(SSL *ssl, SSL_CTX *context) {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (context) {
        SSL_CTX_free(context);
    }
}

/* --------------------------------------------------------------------------
 * Public: download a file from URL and save to filePath using FileSystem API.
 * -------------------------------------------------------------------------- */
bool HttpClient_DownloadFile(const char *url, const char *filePath) {
    char host[256];
    char port[8];
    char path[1024];

    bool https = ParseUrl(url, host, sizeof(host), port, sizeof(port), path, sizeof(path));

    CreatePathDirectories(filePath);

    int sockfd = ConnectSocket(host, port);
    if (sockfd < 0) return false;

    SSL_CTX *context = NULL;
    SSL *ssl = NULL;

    if (https) {
        ssl = EstablishTLS(sockfd, host, &context);
        if (!ssl) {
            close(sockfd);
            return false;
        }
    }

    char request[2048];
    int reqLen = BuildRequest(request, sizeof(request), host, path);
    SendRequest(sockfd, ssl, request, reqLen);

    bool success = ReadResponseToFile(sockfd, ssl, filePath);

    CleanupSSL(ssl, context);
    close(sockfd);

    return success;
}
