#include "Config.hpp"

#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <sstream>

std::string Config::SERVER_KEY = "server";

Config::Config() : logger(Logger("CONFIG")), rootAstNode(AstNode(Token("main", -1), false)), tokens(std::vector<Token>()), servers(std::vector<ServerConfig>()) {}

Config::Config(const Config &other) {
    *this = other;
}

Config &Config::operator=(const Config &other) {
    if (this != &other) {
        logger = other.logger;
        rootAstNode = other.rootAstNode;
        tokens = other.tokens;
        servers = other.servers;
    }
    return (*this);
}

Config::~Config() {}

void Config::loadConfig(std::string configFilePath) {
    tokenize(configFilePath);
    verifyBrackets();
    parseConfigToAst(&rootAstNode);
    parseServers();
}

void Config::tokenize(const std::string &configFilePath) {
    std::ifstream configFile(configFilePath.c_str());
    if (!configFile.is_open()) {
        logger.error() << "Error to open file: " << configFilePath << std::endl;
        throw std::runtime_error("Error to open file");
    }

    std::string line;
    size_t lineNum = 1;
    while (std::getline(configFile, line)) {
        if (line.empty()) {
            lineNum++;
            continue;
        }
        removeUnecessarySpaces(line);

        std::istringstream iss(line);
        std::string tokenValue;
        size_t pos = 0;
        while (pos != std::string::npos) {
            pos = line.find_first_of(";{}");
            std::string segment = (pos == std::string::npos) ? line : line.substr(0, pos);
            iss.str(segment);
            iss.clear();
            while (iss >> tokenValue) {
                tokens.push_back(Token(tokenValue, lineNum));
            }
            if (pos != std::string::npos) {
                tokens.push_back(Token(std::string(1, line[pos]), lineNum));
                line.erase(0, pos + 1);
            }
        }

        lineNum++;
    }

    configFile.close();
}

void Config::verifyBrackets() {
    int brackets = 0;
    std::string line;
    size_t lastOpen = 0;

    for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        if ((*it).getValue() == "{") {
            if (brackets == 0) lastOpen = (*it).getLine();
            ++brackets;
        } else if ((*it).getValue() == "}") {
            --brackets;
        }

        if (brackets < 0) {
            throw std::runtime_error("Close bracket without open before at line: " + numberToString((*it).getLine()));
        }
    }

    if (brackets != 0) {
        throw std::runtime_error("Unclosed bracket at line: " + numberToString(lastOpen));
    }
}

void Config::parseConfigToAst(AstNode *parentBlock) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].getValue() == "{") {
            AstNode *block = new AstNode(tokens[0], false);

            for (size_t j = 1; j < i; ++j) {
                block->addValue(tokens[j]);
            }

            parentBlock->addChild(block);
            tokens.erase(tokens.begin(), tokens.begin() + i + 1);
            parseConfigToAst(block);
            i = -1;
        } else if (tokens[i].getValue() == "}") {
            tokens.erase(tokens.begin());
            return;
        } else if (tokens[i].getValue() == ";") {
            AstNode *leaf = new AstNode(tokens[0], true);

            for (size_t j = 1; j < i; ++j) {
                leaf->addValue(tokens[j]);
            }

            parentBlock->addChild(leaf);
            tokens.erase(tokens.begin(), tokens.begin() + i + 1);
            i = -1;
        }
    }
}

void Config::parseServers() {
    std::vector<AstNode *> children = rootAstNode.getChildren();
    if (children.size() == 0) {
        throw std::runtime_error("No server block found in config file");
    }

    for (std::vector<AstNode *>::iterator it = children.begin(); it != children.end(); ++it) {
        if ((*it)->getKey().getValue() == Config::SERVER_KEY && !(*it)->getIsLeaf()) {
            ServerConfig serverConfig;
            serverConfig.parseServer(*(*it));
            servers.push_back(serverConfig);
        } else {
            throw std::runtime_error("Invalid block with name '" + (*it)->getKey().getValue() + "' in config file at line: " + numberToString((*it)->getKey().getLine()));
        }
    }
}

std::vector<ServerConfig> Config::getServers() const {
    return (servers);
}
