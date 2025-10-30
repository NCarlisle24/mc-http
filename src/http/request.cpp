#include <http/request.hpp>

HttpRequest::HttpRequest(const std::string &requestString) {
    std::istringstream inputStream(requestString);
    std::string line;

    // method, request type, and version
    std::getline(inputStream, line);

    char tempMethod[16];
    char tempPath[256];
    char tempHttpVersion[4];

    sscanf(line.c_str(), "%s %s HTTP/%s", tempMethod, tempPath, tempHttpVersion);
    this->method = tempMethod;
    this->path = tempPath;
    this->httpVersion = tempHttpVersion;

    // headers
    constexpr size_t HEADER_KEY_BUFFER_LENGTH = 64;
    constexpr size_t HEADER_VALUE_BUFFER_LENGTH = 256;

    char headerKey[HEADER_KEY_BUFFER_LENGTH];
    char headerHashKey[HEADER_KEY_BUFFER_LENGTH];
    char headerValue[HEADER_VALUE_BUFFER_LENGTH];
    while (std::getline(inputStream, line)) {
        // remove trailing newlines
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());

        if (line[0] == '\0') continue;

        headerKey[HEADER_KEY_BUFFER_LENGTH - 1] = '\0';
        headerValue[HEADER_VALUE_BUFFER_LENGTH - 1] = '\0';

        readHttpHeader(line.c_str(), headerKey, headerValue);

        // convert the key to lowercase for hashing
        strncpy(headerHashKey, headerKey, HEADER_KEY_BUFFER_LENGTH);
        const size_t keyLength = strlen(headerHashKey);
        for (size_t i = 0; i < keyLength; i++) {
            headerHashKey[i] = tolower(headerHashKey[i]);
        }

        if (headerKey[HEADER_KEY_BUFFER_LENGTH - 1] != '\0' || headerValue[HEADER_VALUE_BUFFER_LENGTH - 1] != '\0') {
            std::cerr << "[HttpRequest::HttpRequest] Warning: HTTP request line too long. Skipping '"
                      << line << "'." << std::endl;
        } else if (headerKey[0] == '\0' || headerValue[0] == '\0') {
            std::cerr << "[HttpRequest::HttpRequest] Warning: Couldn't read HTTP request line. Skipping '"
                      << line << "'." << std::endl;
        } else {
            this->headers.insert({headerHashKey, {headerKey, headerValue}});
        }
    }

    // query parameters

    strtok(tempPath, "?");
    char* queryKey = strtok(NULL, "=");
    char* queryValue = strtok(NULL, "&");
    while (queryKey != NULL) {
        if (queryValue == NULL) {
            this->queryParameters.insert({queryKey, ""});
        } else {
            this->queryParameters.insert({queryKey, queryValue});
        }

        queryKey = strtok(NULL, "=");
        queryValue = strtok(NULL, "&");
    }
}

void HttpRequest::print() {
    std::cerr << this->method << " " << this->path << " HTTP/" << this->httpVersion << std::endl;

    // TODO: make these print in the original order
    for (const auto &header : this->headers) {
        std::cerr << header.second.key << ": " << header.second.value << std::endl;
    }
}

void HttpRequest::printQueryParameters() {
    for (const auto &parameter : this->queryParameters) {
        std::cerr << parameter.first << " = '" << parameter.second << "'" << std::endl;
    }
}