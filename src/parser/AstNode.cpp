#include "AstNode.hpp"

#include <iostream>

AstNode::AstNode() : key(Token()), isLeaf(false), values(std::vector<Token>()), children(std::vector<AstNode *>()) {}

AstNode::AstNode(Token key, bool isLeaf) : key(key), isLeaf(isLeaf), values(std::vector<Token>()), children(std::vector<AstNode *>()) {}

AstNode::AstNode(const AstNode &other) {
    *this = other;
}

AstNode &AstNode::operator=(const AstNode &other) {
    if (this != &other) {
        key = other.key;
        values = other.values;
        isLeaf = other.isLeaf;
        children = other.children;
    }
    return (*this);
}

AstNode::~AstNode() {
    for (std::vector<AstNode *>::iterator it = children.begin(); it != children.end(); ++it) {
        delete (*it);
    }
    children.clear();
}

void AstNode::addChild(AstNode *child) {
    children.push_back(child);
}

void AstNode::addValue(Token value) {
    values.push_back(value);
}

const Token &AstNode::getKey() const {
    return (key);
}

bool AstNode::getIsLeaf() const {
    return (isLeaf);
}

const std::vector<Token> &AstNode::getValues() const {
    return (values);
}

const std::vector<AstNode *> &AstNode::getChildren() const {
    return (children);
}
