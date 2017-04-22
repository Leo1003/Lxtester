#ifndef RUNNER_H
#define RUNNER_H
#include <errno.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include "global.h"
#include "logger.h"
#include "utils.h"

struct exec_opt
{
    long long time, mem;
    int id, fsize, processes, stack;
    std::string std_in, metafile;
};

struct meta
{
    long long time, time_wall, max_rss, csw_voluntary, csw_forced, cg_mem;
    int exitsig, exitcode;
    bool isKilled;
    std::string status, message;
    meta();
    meta(std::string metapath);
};

int boxInit(exec_opt option);
int boxExec(std::string cmd, exec_opt option, bool enableStdin = true);
int boxDel(exec_opt option);

int advFork(char** argp, pid_t& pid, bool wait = true);
#endif // RUNNER_H
