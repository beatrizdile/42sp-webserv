#include "utils.h"

#include <sstream>

void removeUnecessarySpaces(std::string &fileString) {
    size_t pos = 0;

    while ((pos = fileString.find_first_of("\t\n\r\f\v", pos)) != std::string::npos) {
        fileString.replace(pos, 1, " ");
    }

    pos = 0;
    while ((pos = fileString.find("  ", pos)) != std::string::npos) {
        fileString.replace(pos, 2, " ");
    }

    trim(fileString);
}

bool verifySpaceBetweenBlocks(const std::string &fileString, size_t &start, size_t end) {
    while (start < fileString.size() && start < end) {
        if (fileString[start] != ' ' && fileString[start] != '\t') {
            return (false);
        }
        ++start;
    }

    return (true);
}

bool verifyBracesBalance(const std::string &fileString, size_t startBracePos, size_t &end) {
    size_t openBraces = 1;
    end = startBracePos + 1;

    while (openBraces > 0 && end < fileString.size()) {
        if (fileString[end] == '{') {
            ++openBraces;
        } else if (fileString[end] == '}') {
            --openBraces;
        }
        ++end;
    }

    return (openBraces == 0);
}

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

void trim(std::string &s) {
    size_t start = 0;
    size_t end = s.size();

    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) {
        ++start;
    }

    while (end > 0 && (s[end - 1] == ' ' || s[end - 1] == '\t')) {
        --end;
    }

    s = s.substr(start, end - start);
}

std::string numberToString(long number) {
    std::ostringstream oss;
    oss << number;
    return (oss.str());
}
