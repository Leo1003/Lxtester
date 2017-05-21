#include "daemon.h"
using namespace std;

map<pid_t, submission> pidmap;
ServerSocket *s;
bool sigchild = false;
logger mainlg("MainProc");

void enterDaemon()
{
    umask(0022);
    if (chdir(WorkingDir.c_str()) == -1)
    {
        mainlg.log("Failed to chdir", LVFA);
        mainlg.log(strerror(errno));
        exit(1);
    }
    if (DaemonMode)
    {
        for (int i = getdtablesize(); i >= 0; --i)
            close(i);
        if (open("/dev/null", O_RDONLY) == -1 || /* open stdin */
            open("/dev/null", O_WRONLY) == -1 || /* stdout */
            open(LogFile.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0644) == -1) /* stderr */
        {
            mainlg.log("Failed to redirect I/O.", LVFA);
            mainlg.log(strerror(errno));
            exit(1);
        }
    } else {
        if (close(0) == -1 ||
            open("/dev/null", O_RDONLY) == -1 || /* open stdin */
            close(1) == -1 ||
            open("/dev/null", O_WRONLY) == -1) /* stdout */
        {
            mainlg.log("Failed to redirect I/O.", LVFA);
            mainlg.log(strerror(errno));
            exit(1);
        }
    }
    mainlg.log("Daemon PID: " + to_string(getpid()), LVIN);
    mainlg.log("Chdir to: " + string(getcwd(NULL, 0)), LVDE);
    int lfp = open(LOCKFile.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0640);
	if (lfp == -1)
    {
        mainlg.log("Failed to open lock file.", LVFA);
        exit(2);
    }
	if (lockf(lfp, F_TLOCK, 0) < 0)
    {
        mainlg.log("Can't lock: " + LOCKFile , LVFA);
        mainlg.log("Maybe another process is running.");
        exit(2);
    }
    ofstream pidf;
    pidf.open(PIDFile);
    if (!pidf)
    {
        mainlg.log("Fail to write pid file.", LVFA);
        exit(2);
    }
    pidf << getpid() << endl;
    pidf.close();
    if (!isExec(IsoBinFile))
    {
        mainlg.log("Fail to find isolate binary: " + IsoBinFile, LVFA);
        exit(1);
    }
    if (!isDir("./meta"))
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
        signal(SIGHUP, signal_handler);
    }
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGCHLD, signal_handler);

    try
    {
        loadLangs(LangFile);
        loadSandboxOption(OptionFile);
    }
    catch (exception ex)
    {
        mainlg.log("Failed to load config.", LVFA);
        mainlg.log(ex.what());
        mainlg.log("Abort!", LVFA);
        exit(1);
    }
    maind();
}

void maind()
{
    mainlg.log("Daemon Started", LVIN);
	s = new ServerSocket(ServerAddr, ServerPort, ServerToken);
    s->connect();
    while (!stopping)
    {
        if (sigchild)
            child_handler();
        switch (s->getStatus())
        {
            case NotConnected:
            case Failed:
            case Disconnected:
                reconnect();
                continue;
            case Errored:
                mainlg.log("Receive error from server.", LVFA);
                exit(1);
            case Connected:
                reset = false;
                break;
        }
        Job j;
        if (j = s->getJob(), pidmap.size() < 10 && j.type != None)
        {
            if (j.type == Submission)
            {
                mainlg.log("Received submission, ID : " + to_string(j.submissionid), LVIN);
                j.sub.initBoxid();
                if (testWorkFlow(j.sub) != -1)
                    mainlg.log("Sent to workflow!", LVD2);
                else
                {
                    result res("Workflow failed!");
                    j.sub.setResult(res);
                    s->sendResult(j.sub);
                }
            }
            else if (j.type == Cancel)
            {
                mainlg.log("Received cancel request, ID : " + to_string(j.submissionid), LVIN);
                bool killed = false;
                for (auto const &ele : pidmap)
                {
                    if (ele.second.getId() == j.submissionid)
                    {
                        if (!kill(ele.first, SIGTERM))
                            killed = true;
                        break;
                    }
                }
                if (!killed)
                    mainlg.log("Cancel not found, ID : " + to_string(j.submissionid), LVIN);
            }
        }
        else
            sleep(1);
    }
    if (s->getStatus() == Connected)
        s->suspend();
    int left = 0;
    while (!stopping2 && pidmap.size()) {
        if (left != pidmap.size()) {
            left = pidmap.size();
            mainlg.log("Waiting for Workers to exit: (" + to_string(left) + "/10)", LVIN);
        }
        child_handler();
        sleep(1);
    }
    if (s->getStatus() == Connected) {
        while (!stopping2 && s->countJob()) {
            Job j = s->getJob();
            if (j.type == Submission) {
                result res("Lxtester stopped");
                j.sub.setResult(res);
                s->sendResult(j.sub);
            }
        }
    }
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    s->disconnect();
    kill(0, SIGTERM);
    mainlg.log("Daemon stopped.", LVIN);
    exit(0);
}

void reconnect()
{
    mainlg.log("Failed to connect to server.", LVER);
    mainlg.log("Retry in 30 seconds...", LVIN);
    int time = 30;
    while (time-- && !reset)
    {
        if (stopping)
            return;
        sleep(1);
    }
    mainlg.log("Reconnecting...", LVIN);
    s->connect();
    reset = false;
}

void signal_handler(int sig)
{
    switch (sig)
    {
        case SIGUSR1:
            reset = true;
        case SIGINT:
            if (DaemonMode)
                break;
            mainlg.log("Received signal 2", LVIN);
        case SIGTERM:
            mainlg.log("Stopping lxtester server...", LVIN);
            if (!stopping) stopping = true;
            else stopping2 = true;
            break;
        case SIGCHLD:
            sigchild = true;
            break;
    }
}

void child_handler()
{
    int chldsta;
    pid_t chldpid;
    while (chldpid = waitpid(-1, &chldsta, WNOHANG), chldpid > 0)
    {
        mainlg.log("Child process terminated, PID: " + to_string(chldpid), LVDE);
        try
        {
            submission sub = pidmap.at(chldpid);
            logger lg("Worker" + to_string(sub.getOption().getId()));
            if (WIFEXITED(chldsta))
            {
                try
                {
                    RESULT_TYPE resty = (RESULT_TYPE)WEXITSTATUS(chldsta);
                    meta mf;
                    result res("Fatal error occurred when execute submission");
                    switch (resty)
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
                catch (ifstream::failure ex)
                {
                    lg.log("Failed to load meta file: " + sub.getOption().metafile, LVER);
                    result res("Failed to load meta file");
                    sub.setResult(res);
                }
            }
            else if (WIFSIGNALED(chldsta))
            {
                string sigstr(strsignal(WTERMSIG(chldsta)));
                result res("Workflow had been killed by: " + sigstr);
                sub.setResult(res);
                lg.log("Return PID: " + to_string(chldpid) + " has failed.", LVER);
                lg.log("Signal: " + sigstr);
            }
            //remove sandbox
            sub.clean();
            //sendResult
            lg.log("Sending result.", LVD2);
            s->sendResult(sub);
            lg.log("Successfully sent result, ID: " + to_string(sub.getId()), LVIN);
            pidmap.erase(pidmap.find(chldpid));
        }
        catch (out_of_range ex)
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
    if (pid > 0)
        pidmap[pid] = sub;
    else if (pid == 0)
    {
        //set child environment
        mainlg = logger("Worker" + to_string(sub.getOption().getId()));
        signal(SIGCHLD, SIG_DFL);
        signal(SIGHUP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        try
        {
            //create sandbox

            if (sub.setup())
            {
                mainlg.log("Setup Failed, ID: " + to_string(sub.getId()), LVWA);
                exit(2);
            }
            if (sub.compile())
            {
                mainlg.log("Compile Failed, ID: " + to_string(sub.getId()), LVWA);
                exit(1);
            }
            else
                sub.execute();
            mainlg.log("Submission Completed, ID: " + to_string(sub.getId()), LVIN);
        }
        catch (exception ex)
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
        return -1;
    }
    return pid;
}

bool DetectDaemon()
{
    logger lg("DetectDaemon");
    int lfp = open(LOCKFile.c_str(), O_RDWR, 0640);
	if (lfp < 0) return 0;
    if (lockf(lfp, F_TEST, 0) < 0)
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
