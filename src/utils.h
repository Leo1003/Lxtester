#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <string>
#include <vector>

void warn(std::string mes, bool showlv = true);

int tryParse(std::string str, int def = 0);
long long tryParsell(std::string str, int def = 0);

void parseVecstr(std::vector<std::string> vec, char** output);

#endif // UTILS_H
