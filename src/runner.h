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

struct result
{
    std::string std_out, std_err;
    int time, mem, exitcode, signal;
    bool isKilled;
    result();
    result(exec_opt option, meta metas);
};

int boxInit(exec_opt option);
pid_t boxExec(std::string cmd, exec_opt option);
int boxDel(exec_opt option);

int advFork(char** argp, pid_t& pid, bool wait = true);
#endif // RUNNER_H
