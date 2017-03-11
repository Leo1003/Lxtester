#ifndef RUNNER_H
#define RUNNER_H
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

struct result
{
    std::string std_out, std_err;
    int time, mem, exitcode, signal;
    bool isKilled;
};

struct exec_opt
{
    int id, time, mem, fsize, files, processes, stack;
    std::string std_in, metafile;
};

struct meta
{
    long long time, time_wall, max_rss, csw_voluntary, csw_forced, cg_mem;
    int exitsig, exitcode;
    bool isKilled;
    std::string status, message;
};

int boxInit(exec_opt option);
pid_t boxExec(std::string cmd, exec_opt option);
int boxDel(exec_opt option);

int parsemeta(std::string metafile, meta &metas);

#endif // RUNNER_H
