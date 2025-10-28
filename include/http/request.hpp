#pragma once

#include <types.h>
#include <unordered_map>

class HttpRequest {
    public:
        char* method; // GET, POST, etc.
        char* path; // url
        char* httpVersion;

        std::unordered_map<char*, char*> headers; // keys are case-insensitive

        char* body; // relvant for POST and PUT
        size_t bodyLength;

        std::unordered_map<char*, char*> queryParams; // ".../?key=value" stuff

        char* clientIp;
        int clientPort;

        // these are more or less optional
        char* host;
        char* contentType;
        size_t contentLength;
        int keepAlive;

        HttpRequest(const char* const &httpText);
        ~HttpRequest();
};