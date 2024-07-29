#include "utils.h"

#include <sstream>

void removeUnecessarySpaces(std::string &fileString) {
    size_t pos = 0;

    while ((pos = fileString.find("\t", pos)) != std::string::npos) {
        fileString.replace(pos, 1, " ");
    }

    pos = 0;
    while ((pos = fileString.find("  ", pos)) != std::string::npos) {
        fileString.replace(pos, 2, " ");
    }

    if (fileString[0] == ' ') {
        fileString.erase(0, 1);
    }

    if (fileString[fileString.size() - 1] == ' ') {
        fileString.erase(fileString.size() - 1, 1);
    }
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
