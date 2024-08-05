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
    for (size_t i = 0; i < children.size(); i++) {
        delete children[i];
    }
    children.clear();
}

void AstNode::addChild(AstNode *child) {
    children.push_back(child);
}

void AstNode::addValue(Token value) {
    values.push_back(value);
}

Token AstNode::getKey() const {
    return (key);
}

bool AstNode::getIsLeaf() const {
    return (isLeaf);
}

std::vector<Token> AstNode::getValues() const {
    return (values);
}

std::vector<AstNode *> AstNode::getChildren() const {
    return (children);
}
