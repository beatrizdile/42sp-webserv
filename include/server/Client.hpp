#pragma once

#include <poll.h>

#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "Server.hpp"

class Client {
   public:
    static const size_t BUFFER_SIZE;

    Client();
    ~Client();
    Client(int fd);
    Client(const Client& other);
    Client& operator=(const Client& other);

    int getFd() const;
    bool isFdValid(int fd) const;
    int processSendedData(int fdAffected, const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd);
    int sendResponse(int clientSocket);
    void closeAll() const;

   private:
    int fd;
    int pipeIn;
    int pipeOut;
    HttpRequest request;
    HttpResponse response;
    std::string responseStr;
    Logger logger;

    int readCgiResponse(const std::string& bytesReded);
    std::string createCgiProcess(const t_config& config, std::string& execPath, std::string& scriptPath, std::vector<pollfd>& fdsToAdd);
    void matchUriAndResponseClient(const std::vector<Server>& servers, std::vector<pollfd>& fdsToAdd);
    std::string processRequest(const t_config& config, const std::string& uri, const std::map<std::string, std::string>& headers, std::vector<pollfd>& fdsToAdd);
    std::string processGetRequest(const t_config& config, const std::string& path, const std::string& uri);
    std::string processPostRequest(const t_config& config, const std::string& path, const std::string& uri, const std::map<std::string, std::string>& headers);
    std::string processDeleteRequest(const t_config& config, const std::string& path);
    std::vector<Server>::const_iterator findServer(const std::vector<Server>& servers, const std::string& host) const;
};
