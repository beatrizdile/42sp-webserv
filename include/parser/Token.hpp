#pragma once

#include <string>

class Token {
   public:
    Token();
    Token(std::string value, size_t line);
    Token(const Token &other);
    Token &operator=(const Token &other);
    ~Token();

    std::string getValue() const;
    size_t getLine() const;
  
  private:
    std::string value;
    size_t line;
};
