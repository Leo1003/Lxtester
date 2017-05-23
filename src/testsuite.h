#ifndef TESTSUITE_H
#define TESTSUITE_H
#include <map>
#include <string>
#include <vector>
#include "config.h"
#include "logger.h"
#include "global.h"
#include "runner.h"
#include "utils.h"

struct language
{
    std::string name, complier, executer, compargs, execargs,
        srcext;
    bool needComplie;
};

extern std::map<std::string, language> langs;
extern exec_opt execset;
extern exec_opt compset;

void loadLangs(std::string confpath);
void loadSandboxOption(std::string confpath);

#endif // TESTSUITE_H
