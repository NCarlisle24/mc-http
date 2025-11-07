#pragma once

#include <http/http.hpp>

#include <cctype>
#include <format>
#include <string>
#include <unordered_map>

#define HTTP_RESPONSE_BUFFER_SIZE 1024
#define HTTP_RESPONSE_NEWLINE "\r\n"

std::string static inline convertToLower(const std::string &str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
    return lower;
}

class HttpResponse {
    public:
        void setBody(const std::string &text);
        void setResponseHeader(const std::string &key, const std::string &value);
        void setRepresentationHeader(const std::string &key, const std::string &value);
        void setStatus(const int &code, const std::string &reason = "");
        void removeResponseHeader(const std::string &key);
        void removeRepresentationHeader(const std::string &key);
        std::string serialize() const; // generates the full response string
    
    private:
        std::string httpVersion = "1.1";
        int statusCode = 200;
        std::string reasonPhrase = "OK"; // reason for status code in human-readable format
        std::unordered_map<std::string, HttpHeader> responseHeaders;
        std::unordered_map<std::string, HttpHeader> representationHeaders;
        std::string body;

        // not implemented
        bool keepAlive;
        std::string timestamp;
};