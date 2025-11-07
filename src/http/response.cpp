#include <http/response.hpp>

void HttpResponse::setResponseHeader(const std::string &key, const std::string &value) {
    this->responseHeaders[convertToLower(key)] = (HttpHeader){key, value};
}

void HttpResponse::removeResponseHeader(const std::string &key) {
    this->responseHeaders.erase(convertToLower(key));
}

void HttpResponse::setRepresentationHeader(const std::string &key, const std::string &value) {
    this->representationHeaders[convertToLower(key)] = (HttpHeader){key, value};
}

void HttpResponse::removeRepresentationHeader(const std::string &key) {
    this->representationHeaders.erase(convertToLower(key));
}

void HttpResponse::setStatus(const int &code, const std::string& reason) {
    this->statusCode = code;
    this->reasonPhrase = reason;
}

void HttpResponse::setBody(const std::string &text) {
    this->body = text;
}

std::string HttpResponse::serialize() const {
    std::string result;
    result.reserve(HTTP_RESPONSE_BUFFER_SIZE);
    
    // HTTP header
    result += "HTTP/";
    result += this->httpVersion;
    result += " ";
    result += std::to_string(this->statusCode);
    result += " ";
    result += this->reasonPhrase;
    result += HTTP_RESPONSE_NEWLINE;

    // Response headers
    for (const auto &responseHeader : this->responseHeaders) {
        result += responseHeader.second.key;
        result += ": ";
        result += responseHeader.second.value;
        result += HTTP_RESPONSE_NEWLINE;
    }

    // Representation headers
    for (const auto &representationHeader : this->representationHeaders) {
        result += representationHeader.second.key;
        result += ": ";
        result += representationHeader.second.value;
        result += HTTP_RESPONSE_NEWLINE;
    }

    result += HTTP_RESPONSE_NEWLINE;
    result += body;

    return result;
}