#ifndef RUNNER_H
#define RUNNER_H
#include <errno.h>
#include <fcntl.h>
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
#include <sys/stat.h>
#include "global.h"
#include "logger.h"
#include "utils.h"

struct exec_opt
{
    exec_opt();
    exec_opt(int id);
    int getId() const;
    void copySettings(exec_opt &dest) const;
    int fsize, mem, processes, stack, time;
    std::string std_in, metafile;
    int registerbox();
    void releasebox();
    void static setMax(int value);
private:
    int id;
    bool hasID;
    static vector<bool> boxslist;
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

int boxInit(const exec_opt& option);
int boxExec(std::string cmd, const exec_opt& option, bool enableStdin = true);
int boxDel(const exec_opt& option);

int advFork(char** argp, pid_t& pid);
#endif // RUNNER_H
