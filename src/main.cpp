#include "Config.hpp"
#include "Logger.hpp"
#include "WebServer.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return (1);
    }

    Logger logger("Webserv");
    try {
        Config config;
        config.loadConfig(argv[1]);
        WebServer webServer(config);
        webServer.setupServers();
    } catch (std::exception &e) {
        logger.error() << "Error: " << e.what() << std::endl;
        return (1);
    }
}
