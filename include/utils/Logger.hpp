#pragma once

#include <string.h>

#include <cerrno>
#include <ctime>
#include <iostream>
#include <map>
#include <string>

#define COLOR_DEBUG "\033[36m"
#define COLOR_INFO "\033[32m"
#define COLOR_WARN "\033[33m"
#define COLOR_ERROR "\033[31m"
#define COLOR_RESET "\033[0m"

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE,
};

class Logger {
   public:
    Logger();
    Logger(const std::string& tag);
    Logger(const Logger& other);
    Logger& operator=(const Logger& other);
    ~Logger();

    std::ostream& debug();
    std::ostream& info();
    std::ostream& warn();
    std::ostream& error();
    void perror(const std::string& message);

   private:
    static const std::map<std::string, LogLevel> logLevelMap;

    std::string tag;
    LogLevel logLevel;

    LogLevel parseLogLevel(const std::string& levelStr);
    std::string getCurrentTime();
    std::ostream& log(const std::string& levelStr, LogLevel level, const std::string& color);
};
