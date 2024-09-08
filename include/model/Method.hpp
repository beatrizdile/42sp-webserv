#pragma once

#include <string>

enum Method {
    GET,
    POST,
    DELETE,
    INVALID,
};

Method getMethodFromString(const std::string& method);
std::string getMethodString(Method method);
