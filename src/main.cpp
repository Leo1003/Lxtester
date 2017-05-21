#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include "config.h"
#include "daemon.h"
#include "global.h"
#include "logger.h"
#include "utils.h"
using namespace std;

config mainconf;
bool argdm = false, arglv = false;

void ConfigLoader();
void usage(bool wa = false);

const char optstring[] = "Ddl:hKrRs";
const struct option longopts[] = {
    {"daemon",      no_argument,        NULL,   'd'},
    {"kill",        no_argument,        NULL,   'K'},
    {"no-daemon",   no_argument,        NULL,   'D'},
    {"loglevel",    required_argument,  NULL,   'l'},
    {"help",        no_argument,        NULL,   'h'},
    {"refresh",      no_argument,        NULL,   'r'},
    {"restart",     no_argument,        NULL,   'R'},
    {"stop",        no_argument,        NULL,   's'},
    {NULL,          0,                  NULL,   0}
};
enum Command
{
    DAE_DEFAULT = 0,
    DAE_START,
    DAE_STOP,
    DAE_RELOAD,
    DAE_REFRESH,
    DAE_RESTART,
    DAE_KILL
};

int main(int argc,char* argv[])
{
    /*** Parse Arguments ***/
    logger lg("Command");
    loglevel loglv;
    int opt;
    Command ty = DAE_DEFAULT;
    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch (opt)
        {
            case 'd':
                if (ty) usage(true);
                ty = DAE_START;
                argdm = true;
                DaemonMode = true;
                break;
            case 'D':
                if (ty) usage(true);
                ty = DAE_START;
                argdm = true;
                DaemonMode = false;
                break;
            case 'r':
                if (ty) usage(true);
                ty = DAE_REFRESH;
                break;
            case 'R':
                if (ty) usage(true);
                ty = DAE_RESTART;
                break;
            case 's':
                if (ty) usage(true);
                ty = DAE_STOP;
                break;
            case 'K':
                if (ty) usage(true);
                ty = DAE_KILL;
                break;
            case 'l':
                loglv = (loglevel)tryParse(optarg, -1);
                if (loglv == LVUNDEF)
                    loglv = tryParseLevel(optarg);
                if (loglv == LVUNDEF)
                    usage(true);
                else
                {
                    arglv = true;
                    logger::setGlobalLevel(loglv);
                }
                break;
            case '?':
            case ':':
                usage(true);
                break;
            case 'h':
            default:
                usage();
        }
    }

    if (geteuid())
    {
        lg.log("Please run lxtester as root!", LVFA);
        exit(2);
    }

    ConfigLoader();
    DetectDaemon();

    if ((ty == DAE_DEFAULT || ty == DAE_START))
    {
        if (daerunning)
        {
            lg.log("There is another process running.", LVER);
            exit(1);
        }
    }
    else
    {
        if (!daerunning)
        {
            lg.log("There is no process running.", LVER);
            exit(1);
        }
    }

    switch (ty)
    {
        case DAE_START:
        case DAE_DEFAULT:
            if (DaemonMode)
                daemon(1, 0);
            enterDaemon();
            break;
        case DAE_STOP:
            kill(daepid, SIGTERM);
            lg.log("Stopping...", LVIN);
            while (DetectDaemon())
            {
                sleep(1);
            }
            lg.log("Stopped.", LVIN);
            break;
        case DAE_RELOAD:
            kill(daepid, SIGHUP);
        case DAE_REFRESH:
            kill(daepid, SIGUSR1);
            break;
        case DAE_RESTART:
            kill(daepid, SIGTERM);
            while (DetectDaemon())
            {
                sleep(1);
            }
            if (DaemonMode)
                daemon(1, 0);
            enterDaemon();
            break;
        case DAE_KILL:
            kill(-daepid, SIGKILL);
            lg.log("Killed.", LVIN);
            break;
    }
}

void usage(bool wa)
{
    printf("Usage: lxtester [option]\n");
    printf("\n");
    printf("Options:\n");
    printf("\t--daemon, -d \t\tRun and daemonize the program.\n");
    printf("\t--no-daemon, -D \tRun but not daemonize the program.\n");
    printf("\t--stop, -s \t\tStop the background daemon.\n");
    printf("\t--refresh, -r \t\tTry to reconnect to server instantly.\n");
    printf("\t--restart, -R \t\tRestart the background daemon.\n");
    printf("\t--kill, -K \t\tKill the background daemon directly without waiting it to shutdown.\n");
    printf("\t--loglevel, -l \t\tSet the log output's level.\n");
    printf("\t--help, -h \t\tShow this help.\n");
    if (wa) exit(3);
    else exit(0);
}

void ConfigLoader()
{
    logger lg("ConfigLoader");
    string confpath = getConfDir() + "/lxtester.conf";
    if (!isFile(confpath))
    {
        lg.log("Can't open config file.", LVFA);
        lg.log(strerror(errno));
        exit(1);
    }
    try
    {
        mainconf = config(confpath);
    }
    catch(exception ex)
    {
        lg.log("Failed to load main config.", LVFA);
        lg.log(ex.what());
        exit(1);
    }
    if (!argdm)
        mainconf.trygetBool(DaemonMode, "DaemonMode");
    mainconf.trygetString(PIDFile, "PIDFile");
    mainconf.trygetString(LOCKFile, "LockFile");
    mainconf.trygetString(BoxDir, "SandboxDirectory");
    mainconf.trygetInt(MaxWorker, "MaxWorker");
    mainconf.trygetString(WorkingDir, "WorkingDirectory");
    if (!isDir(WorkingDir))
        mainlg.log("WorkingDir may not be a directory!" ,LVWA);
    mainconf.trygetString(LogFile, "LogFile");
    mainconf.trygetString(LangFile, "LanguageFile");
    mainconf.trygetString(OptionFile, "OptionFile");
    mainconf.trygetString(IsoBinFile, "IsolatePath");
    mainconf.trygetString(ServerAddr, "ServerAddress");
    if (mainconf.isExist("ServerPort"))
        ServerPort = mainconf.getInt("ServerPort");
    mainconf.trygetString(ServerToken, "ServerToken");
    if (!arglv && mainconf.isExist("DebugLevel"))
        logger::setGlobalLevel((loglevel)mainconf.getInt("DebugLevel"));
}

