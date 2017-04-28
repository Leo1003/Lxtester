#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <iomanip>
#include <string>
#include <sys/stat.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include "global.h"
#include "logger.h"

std::string getSelfPath();
std::string getConfDir();

bool isExec(std::string path);
bool isFile(std::string path);
bool isDir(std::string path);

int tryParse(std::string str, int def = 0);
long long tryParsell(std::string str, long long def = 0);

std::string trim(const std::string& str);

char** parseVecstr(std::vector<std::string> vec);
void delCStrings(char** cstrings);

#endif // UTILS_H
