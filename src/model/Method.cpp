#include "Method.hpp"

Method getMethodFromString(const std::string& method) {
    if (method == "GET") {
        return (GET);
    } else if (method == "POST") {
        return (POST);
    } else if (method == "DELETE") {
        return (DELETE);
    } else {
        return (INVALID);
    }
}

std::string getMethodString(Method method) {
    switch (method) {
        case GET:
            return ("GET");
        case POST:
            return ("POST");
        case DELETE:
            return ("DELETE");
        default:
            return ("INVALID");
    }
}
