#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <regex>
#include <vector>
#include <unistd.h>

enum loglevel
{
    LVDE,
    LVIN,
    LVWA,
    LVER,
    LVFA,
    LVNU
};

void log(std::string mes, loglevel lvpre = LVNU);
loglevel getLevel();
void setLevel(loglevel lv);
std::string getSelfPath();
std::string getConfDir();

bool isFile(std::string path);
bool isDir(std::string path);

int tryParse(std::string str, int def = 0);
long long tryParsell(std::string str, long long def = 0);

void parseVecstr(std::vector<std::string> vec, char *** output);

#endif // UTILS_H
