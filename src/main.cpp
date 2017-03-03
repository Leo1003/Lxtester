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
            if(daemonmode)
                daemon(1,0);
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
            daemon(1,0);
            return maind();
    }
}

bool stopping = 0;
mode_t newfile = S_IREAD | S_IWRITE | S_IRGRP | S_IROTH;

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
        cerr<<"Can't lock \"/tmp/lxtester .lock\""<<endl;
        cerr<<"Maybe another process is running."<<endl;
        return 0;
    }
    ofstream pidf;
    pidf.open("/tmp/lxtester.pid");
    if(!pidf){
        cerr<<"Fail to open pid file:"<<endl;
        return 2;
    }
    pidf<<getpid()<<endl;
    pidf.close();

    signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGCHLD,child_handler);
	signal(SIGHUP,signal_handler);
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
	cout<<"Daemon Started"<<endl;
	//JudgeSocket js("host", 1234, "token");
    while(!stopping)
    {
        /*
        Submission sub = js.checkJobs();
        if(sub.id != -1)
        {
            switch(sub.job)
            {
            case J_Verdict:
                break;
            case J_Test:

                break;
            default:
                continue;
            }
        }
        */
        sleep(1);

    }
    cout<<"Server stopped."<<endl;
    return 0;
}

void child_handler(int sig)
{
    while(true)
    {
        int chldsta;
        pid_t chpid = waitpid(-1, &chldsta, WNOHANG);
        if(chpid == 0) return;

    }
}


void signal_handler(int sig)
{
    switch(sig) {
        case SIGHUP:
            cout<<"Reloading lxtester server..."<<endl;
            cout<<"Server reloaded."<<endl;
            break;
        case SIGINT:
            if(!daemonmode)
            {
                cout<<"Stopping lxtester server..."<<endl;
                stopping = 1;
            }
            break;
        case SIGTERM:
            cout<<"Stopping lxtester server..."<<endl;
            stopping = 1;
            break;
    }
    return;
}
