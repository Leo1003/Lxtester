#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "config.h"
#include "server_socket.h"
#include "submission.h"
#include "utils.h"
using namespace std;

bool daerunning;
int daepid;

/*--------------------
 * config variables
 * -------------------*/
config mainconf;
bool DaemonMode = true;
string PIDFile = "/tmp/lxtester.pid";
string LOCKFile = "/tmp/lxtester.lock";
string BoxDir = "/tmp/box";
string WorkingDir = ".";
string LogFile = "/tmp/lxtester.log";
string LangFile = "languages.conf";
string ServerAddr;
short ServerPort = 80;
string ServerToken;

int maind();
void signal_handler(int);
pid_t testWorkFlow(submission& sub);

void ConfigLoader()
{
    string confpath = getConfDir() + "/lxtester.conf";
    if(!isFile(confpath))
    {
        log("Can't open config file.", LVFA);
        log(strerror(errno));
        exit(1);
    }
    config mainconf(confpath);
    if(mainconf.isExist("DaemonMode"))
        DaemonMode = mainconf.getBool("DaemonMode");
    if(mainconf.isExist("PIDFile"))
        PIDFile = mainconf.getString("PIDFile");
    if(mainconf.isExist("LockFile"))
        LOCKFile = mainconf.getString("LockFile");
    if(mainconf.isExist("SandboxDirectory"))
        BoxDir = mainconf.getString("SandboxDirectory");
    if(mainconf.isExist("WorkingDirectory") && isDir(mainconf.getString("WorkingDirectory")))
        WorkingDir = mainconf.getString("WorkingDirectory");
    if(mainconf.isExist("LogFile"))
        LogFile = mainconf.getString("LogFile");
    if(mainconf.isExist("LanguageFile"))
        LangFile = mainconf.getString("LanguageFile");
    if(mainconf.isExist("ServerAddress"))
        ServerAddr = mainconf.getString("ServerAddress");
    if(mainconf.isExist("ServerPort"))
        ServerPort = mainconf.getInt("ServerPort");
    if(mainconf.isExist("ServerToken"))
        ServerToken = mainconf.getString("ServerToken");
    if(mainconf.isExist("DebugLevel"))
        setLevel((loglevel)mainconf.getInt("DebugLevel"));
}

bool DetectDaemon()
{
    int lfp=open(LOCKFile.c_str(), O_RDWR, 0640);
	if (lfp<0) return 0;
    if (lockf(lfp,F_TEST,0) < 0)
    {
        daerunning = true;
        ifstream pidf(PIDFile);
        if(!pidf)
        {
            log("Fail to read pid file.", LVFA);
            exit(1);
        }
        pidf >> daepid;
        pidf.close();
    }
    else
        daerunning = false;
    close(lfp);
    return daerunning;
}

const char optstring[] = "DdhrRs";

const struct option longopts[] = {
    {"daemon",      no_argument,        NULL,'d'},
    {"no-daemon",   no_argument,        NULL,'D'},
    {"help",        no_argument,        NULL,'h'},
    {"reload",      no_argument,        NULL,'r'},
    {"restart",     no_argument,        NULL,'R'},
    {"stop",        no_argument,        NULL,'s'},
    {NULL,          0,                  NULL,0}
};

enum OptType
{
    DAE_DEFAULT = 0,
    DAE_START,
    DAE_STOP,
    DAE_RELOAD,
    DAE_RESTART
};

void usage(bool wa = false)
{
    //add usage
    if(wa)exit(255);
    else exit(0);
}

int main(int argc,char* argv[])
{
    /*** Parse Arguments ***/
    int opt;
    bool argdm = DaemonMode;
    OptType ty = DAE_DEFAULT;
    while((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'd':
                if(ty)usage(true);
                ty = DAE_START;
                argdm = true;
                break;
            case 'D':
                if(ty)usage(true);
                ty = DAE_START;
                argdm = false;
                break;
            case 'r':
                if(ty)usage(true);
                ty = DAE_RELOAD;
                break;
            case 'R':
                if(ty)usage(true);
                ty = DAE_RESTART;
                break;
            case 's':
                if(ty)usage(true);
                ty = DAE_STOP;
                break;
            case '?':
                usage(true);
            case 'h':
            default:
                usage();
        }
    }

    if(geteuid())
    {
        log("Please run lxtester as root!", LVFA);
        return 2;
    }

    ConfigLoader();
    DaemonMode = argdm; //prevent daemonmode config override argument
    
    DetectDaemon();

    switch(ty)
    {
        case DAE_START:
        case DAE_DEFAULT:
            if(daerunning)
            {
                log("There is another process running.", LVER);
                return 1;
            }
            if(DaemonMode)
                daemon(1,0);
            return maind();
        case DAE_STOP:
            if(!daerunning)
            {
                log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 15);
            break;
        case DAE_RELOAD:
            if(!daerunning)
            {
                log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 1);
            break;
        case DAE_RESTART:
            if(!daerunning)
            {
                log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 15);
            while(DetectDaemon())
            {
                sleep(1);
            }
            daemon(1,0);
            return maind();
    }
}

bool stopping = 0;
mode_t newfile = 0644;

int maind()
{
    umask(0022);
    
    chdir(WorkingDir.c_str());
    if(DaemonMode)
    {
        for(int i = getdtablesize();i >= 0;--i)
            close(i);
        open("/dev/null",O_RDWR);/* open stdin */
        open(LogFile.c_str(), O_RDWR|O_CREAT|O_APPEND, newfile);/* stdout */
        dup2(1, 2);/* stderr */
    }
    log("Chdir to:" + string(getcwd(NULL, 0)), LVDE);
    int lfp = open(LOCKFile.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0640);
	if (lfp < 0) return 1;
	if (lockf(lfp, F_TLOCK, 0) < 0)
    {
        log("Can't lock \"" + LOCKFile + "\"", LVFA);
        log("Maybe another process is running.");
        return 2;
    }
    ofstream pidf;
    pidf.open(PIDFile);
    if(!pidf){
        log("Fail to write pid file:", LVFA);
        return 2;
    }
    pidf << getpid() << endl;
    pidf.close();

    signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler);
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
    
    loadLangs(LangFile);
    log("Server Started", LVIN);
	//ServerSocket s(); //TODO:Add config
    while(!stopping)
    {
        
        sleep(1);
    }
    log("Server stopped.", LVIN);
    return 0;
}

void signal_handler(int sig)
{
    switch(sig) {
        case SIGHUP:
            log("Reloading lxtester server...", LVIN);
            log("Server reloaded.", LVIN);
            break;
        case SIGINT:
            if(!DaemonMode)
            {
                log("Stopping lxtester server...", LVIN);
                stopping = 1;
            }
            break;
        case SIGTERM:
            log("Stopping lxtester server...", LVIN);
            stopping = 1;
            break;
    }
    return;
}

pid_t testWorkFlow(submission& sub)
{
    
}

