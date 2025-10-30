#include <http/request.hpp>

HttpRequest::HttpRequest(const std::string &requestString) {
    std::istringstream inputStream(requestString);
    std::string line;

    // method, request type, and version
    std::getline(inputStream, line);

    char tempMethod[16];
    char tempHttpVersion[4];

    sscanf(line.c_str(), "%s / HTTP/%s", tempMethod, tempHttpVersion);
    this->method = tempMethod;
    this->httpVersion = tempHttpVersion;

    // headers
    constexpr size_t KEY_BUFFER_LENGTH = 64;
    constexpr size_t VALUE_BUFFER_LENGTH = 256;

    char key[KEY_BUFFER_LENGTH];
    char value[VALUE_BUFFER_LENGTH];
    while (std::getline(inputStream, line)) {
        // remove trailing newlines
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());

        key[KEY_BUFFER_LENGTH - 1] = '\0';
        value[VALUE_BUFFER_LENGTH - 1] = '\0';

        readHttpHeader(line.c_str(), key, value);

        if (key[KEY_BUFFER_LENGTH - 1] != '\0' || value[VALUE_BUFFER_LENGTH - 1] != '\0') {
            std::cerr << "[HttpRequest::HttpRequest] Warning: HTTP request line too long. Skipping '"
                      << line << "'." << std::endl;
        } else if (key[0] == '\0' || value[0] == '\0') {
            std::cerr << "[HttpRequest::HttpRequest] Warning: Couldn't read HTTP request line. Skipping '"
                      << line << "'." << std::endl;
        } else {
            this->headers.insert({key, value});
        }
    }
}

void HttpRequest::print() {
    std::cerr << this->method << " / HTTP/" << this->httpVersion << std::endl;

    for (const auto &header : this->headers) {
        std::cerr << header.first << ": " << header.second << std::endl;
    }
}