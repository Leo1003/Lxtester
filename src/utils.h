#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <iomanip>
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
void setLogMainProc();
loglevel getLevel();
void setLevel(loglevel lv);
std::string getSelfPath();
std::string getConfDir();

bool isFile(std::string path);
bool isDir(std::string path);

int tryParse(std::string str, int def = 0);
long long tryParsell(std::string str, long long def = 0);

std::string trim(const std::string& str);

char** parseVecstr(std::vector<std::string> vec);
void delCStrings(char** cstrings);

#endif // UTILS_H
