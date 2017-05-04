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
#include "global.h"
#include "logger.h"
#include "server_socket.h"
#include "submission.h"
#include "utils.h"
using namespace std;

config mainconf;

ServerSocket *s;
//Record running submission
map<pid_t, submission> pidmap;
bool argdm = false, arglv = false, sigchild = false;

int maind();
void reconnect();
void signal_handler(int);
void child_handler();
pid_t testWorkFlow(submission& sub);
logger mainlg("MainProc");

void ConfigLoader()
{
    logger lg("ConfigLoader");
    string confpath = getConfDir() + "/lxtester.conf";
    if(!isFile(confpath))
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
    if(!argdm && mainconf.isExist("DaemonMode"))
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
    if(mainconf.isExist("IsolatePath"))
        IsoBinFile = mainconf.getString("IsolatePath");
    if(mainconf.isExist("ServerAddress"))
        ServerAddr = mainconf.getString("ServerAddress");
    if(mainconf.isExist("ServerPort"))
        ServerPort = mainconf.getInt("ServerPort");
    if(mainconf.isExist("ServerToken"))
        ServerToken = mainconf.getString("ServerToken");
    if(!arglv && mainconf.isExist("DebugLevel"))
        logger::setGlobalLevel((loglevel)mainconf.getInt("DebugLevel"));
}

bool DetectDaemon()
{
    logger lg("DetectDaemon");
    int lfp=open(LOCKFile.c_str(), O_RDWR, 0640);
	if (lfp<0) return 0;
    if (lockf(lfp,F_TEST,0) < 0)
    {
        daerunning = true;
        ifstream pidf(PIDFile);
        if(!pidf)
        {
            lg.log("Fail to read pid file.", LVFA);
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

const char optstring[] = "Ddl:hKrRs";

const struct option longopts[] = {
    {"daemon",      no_argument,        NULL,'d'},
    {"kill",        no_argument,        NULL,'K'},
    {"no-daemon",   no_argument,        NULL,'D'},
    {"loglevel",    required_argument,  NULL,'l'},
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
    DAE_RESTART,
    DAE_KILL
};

void usage(bool wa = false)
{
    printf("Usage: lxtester [option]\n");
    printf("\n");
    printf("Options:\n");
    printf("\t--daemon, -d \t\tRun and daemonize the program.\n");
    printf("\t--no-daemon, -D \tRun but not daemonize the program.\n");
    printf("\t--stop, -s \t\tStop the background daemon.\n");
    printf("\t--restart, -R \t\tRestart the background daemon.\n");
    printf("\t--kill, -K \t\tKill the background daemon directly without waiting it to shutdown.\n");
    printf("\t--loglevel, -l \t\tSet the log output's level.\n");
    printf("\t--help, -h \t\tShow this help.\n");
    if(wa) exit(3);
    else exit(0);
}

int main(int argc,char* argv[])
{
    /*** Parse Arguments ***/
    logger lg("Command");
    loglevel loglv;
    int opt;
    OptType ty = DAE_DEFAULT;
    while((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'd':
                if(ty)usage(true);
                ty = DAE_START;
                argdm = true;
                DaemonMode = true;
                break;
            case 'D':
                if(ty)usage(true);
                ty = DAE_START;
                argdm = true;
                DaemonMode = false;
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
            case 'K':
                if(ty)usage(true);
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

    if(geteuid())
    {
        lg.log("Please run lxtester as root!", LVFA);
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
                lg.log("There is another process running.", LVER);
                return 1;
            }
            if(DaemonMode)
                daemon(1,0);
            return maind();
        case DAE_STOP:
            if(!daerunning)
            {
                lg.log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 15);
            lg.log("Stopping...", LVIN);
            while(DetectDaemon())
            {
                sleep(1);
            }
            lg.log("Stopped.", LVIN);
            break;
        case DAE_RELOAD:
            if(!daerunning)
            {
                lg.log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 1);
            break;
        case DAE_RESTART:
            if(!daerunning)
            {
                lg.log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 15);
            while(DetectDaemon())
            {
                sleep(1);
            }
            daemon(1,0);
            return maind();
        case DAE_KILL:
            if(!daerunning)
            {
                lg.log("There is no process running.", LVER);
                return 1;
            }
            kill(daepid, 9);
            lg.log("Killed.", LVIN);
            break;
    }
}

int maind()
{
    umask(0022);
    if(chdir(WorkingDir.c_str()) == -1)
    {
        mainlg.log("Failed to chdir", LVFA);
        mainlg.log(strerror(errno));
        exit(1);
    }
    if(DaemonMode)
    {
        for(int i = getdtablesize();i >= 0;--i)
            close(i);
        bool failed = false;
        if(open("/dev/null", O_RDWR) == -1)/* open stdin */
            failed = true;
        if(open(LogFile.c_str(), O_RDWR|O_CREAT|O_APPEND, 0644) == -1)/* stdout */
            failed = true;
        if(dup2(1, 2) == -1)/* stderr */
            failed = true;
        if(failed)
        {
            mainlg.log("Failed to redirect IO.", LVFA);
            mainlg.log(strerror(errno));
            exit(1);
        }
    }
    mainlg.log("Chdir to:" + string(getcwd(NULL, 0)), LVDE);
    int lfp = open(LOCKFile.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0640);
	if (lfp < 0) return 1;
	if (lockf(lfp, F_TLOCK, 0) < 0)
    {
        mainlg.log("Can't lock \"" + LOCKFile + "\"", LVFA);
        mainlg.log("Maybe another process is running.");
        return 2;
    }
    ofstream pidf;
    pidf.open(PIDFile);
    if(!pidf){
        mainlg.log("Fail to write pid file:", LVFA);
        return 2;
    }
    pidf << getpid() << endl;
    pidf.close();
    if(!isExec(IsoBinFile))
    {
        mainlg.log("Fail to find isolate binary: " + IsoBinFile, LVFA);
        exit(1);
    }
    if(!isDir("./meta"))
    {
        if(mkdir("./meta", 0755) == -1)
        {
            mainlg.log("Failed to mkdir: meta", LVFA);
            mainlg.log(strerror(errno));
            exit(1);
        }
        mainlg.log("Created directory: meta", LVIN);
    }
    if (DaemonMode) {
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
    }
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
    signal(SIGCHLD, signal_handler);
    
    try
    {
        loadLangs(LangFile);
    }
    catch(exception ex)
    {
        mainlg.log("Failed to load languages config.", LVFA);
        mainlg.log(ex.what());
        mainlg.log("Abort!", LVFA);
        exit(1);
    }
    mainlg.log("Daemon PID: " + to_string(getpid()), LVIN);
    mainlg.log("Daemon Started", LVIN);
	s = new ServerSocket(ServerAddr, ServerPort, ServerToken);
    s->connect();
    while(!stopping)
    {
        if (sigchild)
            child_handler();
        switch(s->getStatus())
        {
            case NotConnected:
            case Failed:
            case Disconnected:
                reconnect();
                continue;
            case Errored:
                mainlg.log("Socket Errored, ", LVFA);
                exit(1);
            case Connected:
                break;
        }
        submission sub;
        if(pidmap.size() < 10 && s->getSubmission(sub))
        {
            mainlg.log("Received submission, ID : " + to_string(sub.getId()), LVIN);
            testWorkFlow(sub);
            mainlg.log("Sent to workflow!", LVD2);
        }
        else
            sleep(1);
    }
    s->disconnect();
    mainlg.log("Daemon stopped.", LVIN);
    return 0;
}

void reconnect()
{
    mainlg.log("Failed to connect to server.", LVER);
    mainlg.log("Retry in 30 seconds...", LVIN);
    int time = 30;
    while(time--)
    {
        if(stopping)
        {
            return;
        }
        sleep(1);
    }
    mainlg.log("Reconnecting...", LVIN);
    s->connect();
}

void signal_handler(int sig)
{
    switch(sig) {
        case SIGINT:
            if(!DaemonMode)
            {
                mainlg.log("Received signal 2", LVIN);
                mainlg.log("Stopping lxtester server...", LVIN);
                stopping = 1;
            }
            break;
        case SIGTERM:
            mainlg.log("Stopping lxtester server...", LVIN);
            stopping = 1;
            break;
        case SIGCHLD:
            sigchild = true;
            break;
    }
    return;
}

void child_handler()
{
    int chldsta;
    pid_t chldpid;
    while(chldpid = waitpid(-1, &chldsta, WNOHANG), chldpid > 0)
    {
        mainlg.log("Child process terminated, PID: " + to_string(chldpid), LVD2);
        try
        {
            submission sub = move(pidmap.at(chldpid));
            logger lg("Worker" + to_string(sub.getOption().getId()));
            if(WIFEXITED(chldsta))
            {
                try
                {
                    RESULT_TYPE resty = (RESULT_TYPE)WEXITSTATUS(chldsta);
                    meta mf;
                    result res;
                    switch(resty)
                    {
                        case TYPE_COMPILATION:
                        case TYPE_EXECUTION:
                            mf = meta(sub.getOption().metafile);
                            res = result(sub.getOption().getId(), mf);
                            res.type = resty;
                            break;
                    }
                    sub.setResult(res);
                }
                catch(ifstream::failure ex)
                {
                    lg.log("Failed to load meta file: " + sub.getOption().metafile, LVER);
                    result res;
                    sub.setResult(res);
                }
            }
            else if(WIFSIGNALED(chldsta))
            {
                result res;
                sub.setResult(res);
                lg.log("Return PID: " + to_string(chldpid) + " has failed.", LVER);
                lg.log("Signal: " + string(strsignal(WTERMSIG(chldsta))));
            }
            //remove sandbox
            //sighandler_t rawsig = signal(SIGCHLD, SIG_DFL);
            if(boxDel(sub.getOption()))
            {
                lg.log("Unable to remove box.", LVER);
                lg.log("Box id: " + to_string(sub.getOption().getId()));
            }
            //signal(SIGCHLD, rawsig);
            lg.log("Box id: " + to_string(sub.getOption().getId()) + " removed.", LVDE);
            //sendResult
            lg.log("Sending result.", LVDE);
            s->sendResult(sub);
            lg.log("Successfully sent result.", LVDE);
            pidmap.erase(pidmap.find(chldpid));
            if(unlink(sub.getOption().metafile.c_str()))
            {
                mainlg.log("Unable to delete meta file.", LVWA);
                mainlg.log(strerror(errno));
            }
        }
        catch(out_of_range ex)
        {
            mainlg.log("An unknown child process returned, PID: " + to_string(chldpid), LVER);
            continue;
        }
    }
    sigchild = false;
}

pid_t testWorkFlow(submission& sub)
{
    pid_t pid = fork();
    if(pid > 0)
        pidmap[pid] = move(sub);
    else if(pid == 0)
    {
        //set child environment
        mainlg = logger("Worker" + to_string(sub.getOption().getId()));
        signal(SIGCHLD, SIG_DFL);
        signal(SIGHUP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        //create sandbox
        if(boxInit(sub.getOption())) 
        {
            mainlg.log("Unable to create box.", LVER);
            mainlg.log("Box id: " + to_string(sub.getOption().getId()));
            exit(2);
        }
        mainlg.log("Box id: " + to_string(sub.getOption().getId()) + " created.", LVDE);

        try
        {
            if(sub.compile())
            {
                mainlg.log("Compile Failed.", LVWA);
                exit(1);
            }
            else
                sub.execute();
        }
        catch(exception ex)
        {
            mainlg.log("Failed when execute submission.", LVER);
            mainlg.log(ex.what());
            exit(2);
        }
        exit(0);
    }
    else
    {
        mainlg.log("Unable to fork.", LVFA);
        mainlg.log(strerror(errno));
    }
    return pid;
}

