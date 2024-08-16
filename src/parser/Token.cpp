#include "Token.hpp"

Token::Token() : value(""), line(0) {}

Token::Token(std::string value, size_t line) : value(value), line(line) {}

Token::Token(const Token &other) {
    *this = other;
}

Token &Token::operator=(const Token &other) {
    if (this != &other) {
        value = other.value;
        line = other.line;
    }

    return (*this);
}

Token::~Token() {}

const std::string &Token::getValue() const {
    return (value);
}

size_t Token::getLine() const {
    return (line);
}
