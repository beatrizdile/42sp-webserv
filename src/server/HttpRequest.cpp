#include "HttpRequest.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "utils.h"

const std::string HttpRequest::HEADER_HOST_KEY = "host";
const std::string HttpRequest::URI_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVXWYZabcdefghijklmnopqrstuvxwyz0123456789-_.~/?:@&=+$,#";
const std::string HttpRequest::HEADER_VALUE_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-!@#$%^&*()_+|~=`{}[];:'\",.<>/? \t\r\n";
const std::string HttpRequest::HEADER_KEY_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-";
const std::string HttpRequest::HTTP_VERSION = "HTTP/1.1";
const std::string HttpRequest::CONTENT_LENTH_HEADER_KEY = "content-length";
const std::string HttpRequest::HEADER_ETAG_KEY = "if-none-match";
const std::string HttpRequest::HEADER_CONTENT_TYPE_KEY = "content-type";

HttpRequest::HttpRequest() : logger("HTTP_REQUEST"), rawData(""), method(INVALID), uri(""), version(""), headers(), body(""), contentLength(0), complete(false) {}

HttpRequest::HttpRequest(const HttpRequest &copy) {
    *this = copy;
}

HttpRequest::~HttpRequest() {}

HttpRequest &HttpRequest::operator=(const HttpRequest &assign) {
    if (this != &assign) {
        logger = assign.logger;
        rawData = assign.rawData;
        method = assign.method;
        uri = assign.uri;
        version = assign.version;
        headers = assign.headers;
        body = assign.body;
        contentLength = assign.contentLength;
        complete = assign.complete;
    }
    return (*this);
}

void HttpRequest::clear() {
    method = INVALID;
    uri.clear();
    version.clear();
    headers.clear();
    body.clear();
    contentLength = 0;
    complete = false;
}

bool HttpRequest::digestRequest(const std::string &data) {
    rawData += data;

    try {
        if (version.empty() && uri.empty() && method == INVALID) {
            parseFristLine();
        }

        size_t pos = rawData.find("\r\n\r\n");
        if (pos != std::string::npos) {
            parseHeaders(pos);

            if (headers.find(HEADER_HOST_KEY) == headers.end()) {
                throw std::runtime_error("Host header not found");
            }

            if (contentLength == 0) {
                complete = true;
            }
        }

        if (contentLength > 0 && rawData.size() >= contentLength) {
            body = rawData.substr(0, contentLength);
            rawData = rawData.substr(contentLength);
            complete = true;
        }

        return (true);
    } catch (std::exception &e) {
        logger.error() << "Error: " << e.what() << std::endl;
        return (false);
    }
}

void HttpRequest::parseFristLine() {
    size_t pos = rawData.find("\r\n");
    if (pos == std::string::npos) {
        return;
    }

    std::string line = rawData.substr(0, pos);
    rawData = rawData.substr(pos + 2);

    std::istringstream ss(line);
    std::string stringMethod;
    ss >> stringMethod;
    ss >> uri;
    ss >> version;

    if (stringMethod.empty()) {
        throw std::runtime_error("Not found method");
    }
    if (uri.empty()) {
        throw std::runtime_error("Not found URI");
    }
    if (version.empty()) {
        throw std::runtime_error("Not found version");
    }

    std::string rest;
    ss >> rest;
    if (!rest.empty()) {
        throw std::runtime_error("Extra parameters in first line");
    }

    method = getMethodFromString(stringMethod);
    if (method == INVALID) {
        throw std::runtime_error("Invalid method found '" + stringMethod + "'");
    }

    if (uri.find_first_not_of(URI_CHARACTERS) != std::string::npos) {
        throw std::runtime_error("Invalid caracter in URI");
    }

    if (version != HTTP_VERSION) {
        throw std::runtime_error("Invalid HTTP version '" + version + "'");
    }
}

void HttpRequest::parseHeaders(size_t endPos) {
    std::string data = rawData.substr(0, endPos + 2);
    rawData = rawData.substr(endPos + 4);

    size_t pos;
    while ((pos = data.find("\r\n")) != std::string::npos) {
        std::string line = data.substr(0, pos);
        data = data.substr(pos + 2);

        if ((pos = line.find(':')) == std::string::npos) {
            throw std::runtime_error("Invalid header '" + line + "'");
        }

        std::string key = line.substr(0, pos);
        if (key.find_first_not_of(HEADER_KEY_CHARACTERS) != std::string::npos) {
            throw std::runtime_error("Invalid header key '" + key + "'");
        }
        lowercase(key);

        std::string value = line.substr(pos + 1);
        trim(value);
        if (value.find_first_not_of(HEADER_VALUE_CHARACTERS) != std::string::npos) {
            throw std::runtime_error("Invalid header value '" + value + "'");
        }

        headers[key] = value;
    }

    if (headers.find(CONTENT_LENTH_HEADER_KEY) != headers.end()) {
        char *end;
        long size = std::strtol(headers[CONTENT_LENTH_HEADER_KEY].c_str(), &end, 10);
        // check se Content-Length é maior que body size because yeah
        if (*end != '\0' || size < 0) {
            throw std::runtime_error("Invalid Content-Length '" + headers[CONTENT_LENTH_HEADER_KEY] + "'");
        }
        contentLength = size;
    }
}

Method HttpRequest::getMethod() const {
    return (method);
}

const std::string &HttpRequest::getUri() const {
    return (uri);
}

const std::string &HttpRequest::getVersion() const {
    return (version);
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const {
    return (headers);
}

const std::string &HttpRequest::getBody() const {
    return (body);
}

bool HttpRequest::isComplete() const {
    return (complete);
}

std::string HttpRequest::getEtag() const {
    if (headers.find(HEADER_ETAG_KEY) != headers.end()) {
        return headers.at(HEADER_ETAG_KEY);
    }
    return "";
}
