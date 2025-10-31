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
        // implemented
        std::string method; // GET, POST, etc.
        std::string path; // url
        std::string rawPath; // url with query parameters
        std::string httpVersion;

        // not implemented
        std::string body; // relevant for POST and PUT
        std::string clientIp;
        int clientPort;

        // might not implement
        std::string host;
        std::string contentType;
        size_t contentLength;
        int keepAlive;

        // public methods
        HttpRequest(const std::string &requestString);
        // ~HttpRequest();
        void print() const;
        void printQueryParameters() const;
        std::unordered_map<std::string, std::string> getHeaders() const;
        std::unordered_map<std::string, std::string> getQueryParameters() const;
    
    private:
        std::unordered_map<std::string, HttpHeader> headers; // keys are case-insensitive
        std::unordered_map<std::string, std::string> queryParameters; // keys are case-sensitive
};