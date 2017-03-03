#include "daemon.h"
using namespace std;

bool stopping = 0;
bool daemonmode = 1;
mode_t newfile = S_IREAD | S_IWRITE | S_IRGRP | S_IROTH;

int maind()
{
    sleep(1);
    if(getppid() != 1)
        daemonmode = 0;
    umask(0022);
    if(daemonmode)
    {
        for(int i = getdtablesize();i >= 0;--i)
            close(i);
        open("/dev/null",O_RDWR);/* open stdin */
        open("tmp/lxctester/lxctester.out.log", O_RDWR|O_CREAT|O_APPEND, newfile);/* stdout */
        open("tmp/lxctester/lxctester.err.log", O_RDWR|O_CREAT|O_APPEND, newfile);/* stderr */
    }
    int lfp=open("/tmp/lxctester.lock",O_RDWR|O_CREAT,0640);
	if (lfp<0) return 1;
	if (lockf(lfp,F_TLOCK,0) < 0)
    {
        cerr<<"Can't lock \"/tmp/lxctester .lock\""<<endl;
        cerr<<"Maybe another daemon process is running."<<endl;
        return 0;
    }
    signal(SIGTSTP,SIG_IGN); // ignore tty signals
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGCHLD,child_handler);
	signal(SIGHUP,signal_handler);
	signal(SIGINT,signal_handler);
	signal(SIGTERM,signal_handler);
	cout<<"Daemon Started"<<endl;
	JudgeSocket js("host", 1234, "token");
    while(!stopping)
    {
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
            cout<<"Reloading lxctester server..."<<endl;
            cout<<"Server reloaded."<<endl;
            break;
        case SIGINT:
            if(!daemonmode)
            {
                cout<<"Stopping lxctester server..."<<endl;
                stopping = 1;
            }
            break;
        case SIGTERM:
            cout<<"Stopping lxctester server..."<<endl;
            stopping = 1;
            break;
    }
    return;
}
