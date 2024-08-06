#pragma once

#include <string>
#include <vector>

void removeUnecessarySpaces(std::string &fileString);
bool verifySpaceBetweenBlocks(const std::string &fileString, size_t &start, size_t end);
bool verifyBracesBalance(const std::string &fileString, size_t startBracePos, size_t &end);
void split(const std::string &s, char delim, std::vector<std::string> &elems);
void trim(std::string &s);
std::string numberToString(long number);
