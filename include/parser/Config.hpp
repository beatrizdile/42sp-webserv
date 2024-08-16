#pragma once

#include <vector>

#include "AstNode.hpp"
#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "Token.hpp"
#include "utils.h"

class Config {
   public:
    static const std::string SERVER_KEY;

    Config();
    Config(const Config &other);
    Config &operator=(const Config &other);
    ~Config();

    void loadConfig(std::string configFilePath);

    const std::vector<ServerConfig> &getServers() const;

   private:
    Logger logger;
    AstNode rootAstNode;
    std::vector<Token> tokens;
    std::vector<ServerConfig> servers;

    void tokenize(const std::string &fileString);
    void verifyBrackets();
    void parseConfigToAst(AstNode *parentBlock);
    void parseServers();
};
