#ifndef TESTSUITE_H
#define TESTSUITE_H
#include <map>
#include <string>
#include <vector>
#include "config.h"
#include "utils.h"

struct language
{
    std::string name, complier, executer, compargs, execargs;
    bool needComplie;
};

extern std::map<std::string, language> langs;
extern std::map<pid_t, int> pidmap;

void loadLangs(std::string confpath);

#endif // TESTSUITE_H
