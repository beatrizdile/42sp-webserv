#include "Logger.hpp"

#include <cstdlib>

static std::map<std::string, LogLevel> createLogLevelMap();

const std::map<std::string, LogLevel> Logger::logLevelMap = createLogLevelMap();

Logger::Logger() : tag("DEFAULT"), logLevel(INFO) {
    const char* env_level = std::getenv("ROOT_LOG_LEVEL");
    logLevel = parseLogLevel(env_level ? env_level : "INFO");
}

Logger::Logger(const std::string& tag) : tag(tag) {
    const char* env_level = std::getenv("ROOT_LOG_LEVEL");
    logLevel = parseLogLevel(env_level ? env_level : "INFO");
}

Logger::Logger(const Logger& other) : tag(other.tag), logLevel(other.logLevel) {}

Logger& Logger::operator=(const Logger& other) {
    if (this != &other) {
        tag = other.tag;
        logLevel = other.logLevel;
    }
    return *this;
}

Logger::~Logger() {}

LogLevel Logger::parseLogLevel(const std::string& levelStr) {
    std::map<std::string, LogLevel>::const_iterator it = logLevelMap.find(levelStr);
    return it != logLevelMap.end() ? it->second : INFO;
}

std::string Logger::getCurrentTime() {
    std::time_t now = std::time(0);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::ostream& Logger::log(const std::string& levelStr, LogLevel level, const std::string& color) {
    if (level >= logLevel) {
        std::ostream& output = (level == ERROR) ? std::cerr : std::cout;
        output << color << "[" << getCurrentTime() << "][" << tag << "][" << levelStr << "] " << COLOR_RESET;
        return output;
    }
    return std::cout;
}

std::ostream& Logger::debug() {
    return log("DEBUG", DEBUG, COLOR_DEBUG);
}

std::ostream& Logger::info() {
    return log("INFO", INFO, COLOR_INFO);
}

std::ostream& Logger::warn() {
    return log("WARN", WARN, COLOR_WARN);
}

std::ostream& Logger::error() {
    return log("ERROR", ERROR, COLOR_ERROR);
}

void Logger::perror(const std::string& message) {
    std::string err = strerror(errno);
    std::cerr << COLOR_ERROR << "[" << getCurrentTime() << "][" << tag << "][ERROR] " << COLOR_RESET << message << ": " << err << std::endl;
}

std::map<std::string, LogLevel> createLogLevelMap() {
    std::map<std::string, LogLevel> m;
    m.insert(std::make_pair("DEBUG", DEBUG));
    m.insert(std::make_pair("INFO", INFO));
    m.insert(std::make_pair("WARN", WARN));
    m.insert(std::make_pair("ERROR", ERROR));
    m.insert(std::make_pair("NONE", NONE));
    return m;
}
