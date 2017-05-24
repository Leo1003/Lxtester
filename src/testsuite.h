#ifndef TESTSUITE_H
#define TESTSUITE_H
#include <map>
#include <string>
#include <vector>
#include "config.h"
#include "logger.h"
#include "utils.h"

struct language
{
    std::string name, complier, executer, compargs, execargs,
        srcext;
    std::vector<std::string> dirrules, env;
    bool needCompile;
};

extern std::map<std::string, language> langs;

void loadLangs(std::string confpath);
void loadSandboxOption(std::string confpath);

#endif // TESTSUITE_H
