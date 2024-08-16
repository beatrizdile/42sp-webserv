#pragma once

#include <vector>

#include "Token.hpp"

class AstNode {
   public:
    AstNode();
    AstNode(Token key, bool isLeaf);
    AstNode(const AstNode &other);
    AstNode &operator=(const AstNode &other);
    ~AstNode();

    void addChild(AstNode *child);
    void addValue(Token value);

    const Token &getKey() const;
    bool getIsLeaf() const;
    const std::vector<Token> &getValues() const;
    const std::vector<AstNode *> &getChildren() const;

   private:
    Token key;
    bool isLeaf;
    std::vector<Token> values;
    std::vector<AstNode *> children;
};
