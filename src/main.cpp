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
#include "server_socket.h"
#include "utils.h"

using namespace std;

bool daemonmode = true;
bool daerunning;
int daepid;

int maind();
void child_handler(int);
void signal_handler(int);


bool DetectDaemon()
{
    int lfp=open("/tmp/lxtester.lock",O_RDWR,0640);
	if (lfp<0) return 0;
    if (lockf(lfp,F_TEST,0) < 0)
    {
        daerunning = true;
        ifstream pidf;
        pidf.open("/tmp/lxtester.pid", ios::in);
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
    OptType ty = DAE_DEFAULT;
    while((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'd':
                if(ty)usage(true);
                ty = DAE_START;
                daemonmode = true;
                break;
            case 'D':
                if(ty)usage(true);
                ty = DAE_START;
                daemonmode = false;
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
            if(daemonmode)
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
    if(daemonmode)
    {
        for(int i = getdtablesize();i >= 0;--i)
            close(i);
        open("/dev/null",O_RDWR);/* open stdin */
        open("/tmp/lxtester.out.log", O_RDWR|O_CREAT|O_APPEND, newfile);/* stdout */
        open("/tmp/lxtester.err.log", O_RDWR|O_CREAT|O_APPEND, newfile);/* stderr */
    }
    int lfp=open("/tmp/lxtester.lock",O_RDWR|O_CREAT|O_TRUNC,0640);
	if (lfp<0) return 1;
	if (lockf(lfp,F_TLOCK,0) < 0)
    {
        log("Can't lock \"/tmp/lxtester .lock\"", LVFA);
        log("Maybe another process is running.");
        return 0;
    }
    ofstream pidf;
    pidf.open("/tmp/lxtester.pid");
    if(!pidf){
        log("Fail to write pid file:", LVFA);
        return 2;
    }
    pidf<<getpid()<<endl;
    pidf.close();

    signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler);
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
    
    string wdir = getWorkDir();
    if(wdir == "")
    {
        log("Can't find config file: \"lxtester.conf\" ", LVFA);
        return 2;
    }
    chdir(wdir.c_str());
    log("Chdir to:" + wdir, LVDE);
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
            if(!daemonmode)
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
