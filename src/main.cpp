#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/types.h>
#include "daemon.h"

using namespace std;

bool createdae = true;
bool daerunning;
int daepid;

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
            cerr<<"Fail to read pid file."<<endl;
            exit(1);
        }
        pidf>>daepid;
        pidf.close();
    }
    else
        daerunning = false;
    close(lfp);
    return daerunning;
}

inline int CreateDaemonSec()
{
    if(createdae)setsid();
    ofstream pidf;
    pidf.open("/tmp/lxtester.pid");
    if(!pidf){
        cerr<<"Fail to open pid file:"<<endl;
        return 2;
    }
    pid_t p = fork();
    switch(p)
    {
        case -1:
            cerr<<"Fork Error."<<endl;
            return -1;
        case 0:
            return maind();
            break;
        default:
            pidf<<p<<endl;
            cout<<"Daemon Process ID:"<<p<<endl;
            break;
    }
    pidf.close();
    return 0;
}
int CreateDaemon()
{
    int status;
        pid_t p = fork();
        switch(p)
        {
            case -1:
                cerr<<"Fork Error."<<endl;
                return -1;
            case 0:
                CreateDaemonSec();
                break;
            default:
                waitpid(p,&status,0);
                if(WIFSIGNALED(status))
                {
                    cerr<<"Creating daemon failed"<<endl;
                    return status;
                }
                break;
        }
        return 0;
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
                createdae = true;
                break;
            case 'D':
                if(ty)usage(true);
                ty = DAE_START;
                createdae = false;
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
        cerr<<"Please run lxtester as root!"<<endl;
        return 2;
    }

    DetectDaemon();

    switch(ty)
    {
        case DAE_START:
        case DAE_DEFAULT:
            if(daerunning)
            {
                cerr<<"There is another daemon running."<<endl;
                return 1;
            }
            if(createdae)
                return CreateDaemon();
            else
                return maind();
        case DAE_STOP:
            if(!daerunning)
            {
                cerr<<"There is no daemon running."<<endl;
                return 1;
            }
            kill(daepid, 15);
            break;
        case DAE_RELOAD:
            if(!daerunning)
            {
                cerr<<"There is no daemon running."<<endl;
                return 1;
            }
            kill(daepid, 1);
            break;
        case DAE_RESTART:
            if(!daerunning)
            {
                cerr<<"There is no daemon running."<<endl;
                return 1;
            }
            kill(daepid, 15);
            while(DetectDaemon())
            {
                sleep(1);
            }
            return CreateDaemon();
            break;
    }
}
