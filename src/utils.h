#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <regex>
#include <vector>
#include <unistd.h>

//Define log level prefix
#define LVFA "[FATAL]"
#define LVER "[ERROR]"
#define LVWA "[WARN] "
#define LVIN "[INFO] "
#define LVDE "[DEBUG]"
#define LVNU "       "

void log(std::string mes, std::string lvpre = LVNU);
std::string getSelfPath();
std::string getWorkDir();

int tryParse(std::string str, int def = 0);
long long tryParsell(std::string str, long long def = 0);

void parseVecstr(std::vector<std::string> vec, char *** output);

#endif // UTILS_H
