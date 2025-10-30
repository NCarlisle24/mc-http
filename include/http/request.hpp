#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <string>

void inline readHttpHeader(const char* const &header, char* const &key, char* const &value) {
    if (header[0] == '\0' || header[0] == ':') {
        return;
    }

    int i = 0;
    while (header[i] != ':') {
        key[i] = header[i];
        i++;
    }
    key[i] = '\0';
    i += 2;

    int j = 0;
    while (header[i] != '\0') {
        value[j] = header[i];
        
        i++;
        j++;
    }
    value[j] = '\0';
}

typedef struct {
    std::string key;
    std::string value;
} HttpHeader;

class HttpRequest {
    public:
        std::string method; // GET, POST, etc.
        std::string path; // url
        std::string httpVersion;

        std::unordered_map<std::string, HttpHeader> headers; // keys are case-insensitive

        std::string body; // relvant for POST and PUT
        size_t bodyLength;

        std::unordered_map<std::string, std::string> queryParams; // ".../?key=value" stuff

        std::string clientIp;
        int clientPort;

        // these are more or less optional
        std::string host;
        std::string contentType;
        size_t contentLength;
        int keepAlive;

        HttpRequest(const std::string &requestString);
        // ~HttpRequest();
        void print();
};