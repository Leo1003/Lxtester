#include "runner.h"
#include "global.h"
#define PB push_back
#define BUF_SIZE 1024
using namespace std;

int boxInit(const exec_opt& option) {
    vector<string> args;
    args.PB(IsoBinFile);
    args.PB("--init");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.getId()));

    char** argp = parseVecstr(args);
    pid_t pid;
    int status = advFork(argp, pid);
    delCStrings(argp);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int boxExec(string cmd, const exec_opt& option, const language& lang, bool enableStdin) {
    vector<string> args;
    args.PB(IsoBinFile);
    args.PB("--run");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.getId()));
    args.PB("--time=" + to_string(option.time));
    args.PB("--wall-time=" + to_string(option.time));
    args.PB("--mem=" + to_string(option.mem));
    args.PB("--processes=" + to_string(option.processes));
    args.PB("--stack=" + to_string(option.stack));
    args.PB("--fsize=" + to_string(option.fsize));
    if (enableStdin)
        args.PB("--stdin=" + option.std_in);
    args.PB("--stdout=stdout.log");
    args.PB("--stderr=stderr.log");
    args.PB("--meta=" + option.metafile);
    args.PB("--env=PATH=/usr/local/sbin:/usr/local/bin:/usr/bin:/usr/lib/jvm/default/bin");
    args.PB("--env=HOME=/box");
    for (auto env : lang.env) {
        args.PB("--env=" + env);
    }
    for (auto rule : lang.dirrules) {
        args.PB("--dir=" + rule);
    }
    args.PB("--");

    //split cmd string into vector<string>
    regex reg_args(R"-("([^"]+)"|([^ ]+))-");
    string s = trim(cmd);
    mainlg.log("SandboxExec: CMD = \"" + cmd + "\"", LVD2);
    smatch sm;
    while (regex_search(s, sm, reg_args)) {
        mainlg.log("Matched: " + string(sm[0]), LVD2);
        args.PB(sm[1] == "" ? sm[2] : sm[1]);
        s = sm.suffix().str();
    }

    char** argp = parseVecstr(args);
    pid_t pid;
    int status = advFork(argp, pid);
    delCStrings(argp);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int boxDel(const exec_opt& option) {
    vector<string> args;
    args.PB(IsoBinFile);
    args.PB("--cleanup");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.getId()));

    char** argp = parseVecstr(args);
    pid_t pid;
    int status = advFork(argp, pid);
    delCStrings(argp);

    if(WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int advFork(char** argp, pid_t& pid) {
    if (logger::getGlobalLevel() <= LVDE) {
        stringstream ss;
        mainlg.log("Advfork: Exec command:", LVDE);
        ss << "[";
        int i = 0;
        while (argp[i] != NULL) {
            ss << argp[i++];
            if (argp[i] != NULL)
                ss << ", ";
        }
        ss << "]" << endl;
        mainlg.log(ss.str());
    }

    int status = 0;
    bool p = false;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        mainlg.log("Advfork: Failed to create pipe", LVER);
        mainlg.log(strerror(errno));
    } else
        p = true;

    pid = fork();
    if (pid == 0) {
        //child process
        for (int i = getdtablesize(); i >= 0; --i)
            if (i != pipefd[1])
                close(i);
        open("/dev/null", O_RDONLY);
        if (p) {
            close(pipefd[0]);
            dup2(pipefd[1], 1);
            dup2(pipefd[1], 2);
            close(pipefd[1]);
        }
        execvp(argp[0], argp);
        mainlg.log("Failed to exec.", LVFA);
        exit(127);
    } else if (pid > 0) {
        nowrunning = pid;
        mainlg.log("Child PID: " + to_string(pid), LVDE);
        //main process
        if (p) {
            close(pipefd[1]);
            logger lg("Exec");
            char buf[BUF_SIZE];
            size_t c;
            string s;
            while (c = read(pipefd[0], buf, sizeof(buf)), c > 0) {
                if(c < BUF_SIZE) buf[c] = '\0';
                s += buf;
            }
            lg.log(s, LVDE);
            close(pipefd[0]);
        }
        if (waitpid(pid, &status, 0) == -1) {
            mainlg.log("Advfork: Failed to wait child process", LVER);
            mainlg.log(strerror(errno));
        }
        nowrunning = 0;
    } else {
        //failed
        mainlg.log("Advfork Failed.", LVER);
        mainlg.log(strerror(errno));
        status = -1;
    }
    return status;
}

vector<bool> exec_opt::boxslist;
exec_opt::exec_opt() {
    id = -1;
    hasID = false;
    time = 1;
    mem = 1024;
    fsize = 1024;
    processes = 1;
    stack = 1024;
}

exec_opt::exec_opt(int id): exec_opt() {
    this->id = id;
    hasID = true;
}

int exec_opt::getId() const {
    return id;
}

void exec_opt::copySettings(exec_opt &dest) const {
    dest.time = this->time;
    dest.mem = this->mem;
    dest.fsize = this->fsize;
    dest.processes = this->processes;
    dest.stack = this->stack;
}

void exec_opt::setMax(int value) {
    boxslist.resize(value);
}

int exec_opt::registerbox() {
    if (!hasID)
        for (int i = 0; i < MaxWorker; i++) {
            if (!boxslist[i]) {
                boxslist[i] = true;
                this->id = i;
                hasID = true;
                mainlg.log("RegisterBoxID: " + to_string(i), LVD2);
                break;
            }
        }
    return this->id;
}

void exec_opt::releasebox() {
    if (hasID) {
        mainlg.log("ReleaseBoxID: " + to_string(id), LVD2);
        boxslist[id] = false;
        id = -1;
        hasID = false;
    }
}

meta::meta() {
    cg_mem = 0;
    csw_forced = 0;
    csw_voluntary = 0;
    max_rss = 0;
    time = -1;
    time_wall = -1;
    exitcode = 0;
    exitsig = 0;
    isKilled = 0;
    message = "";
    status = "";
}

meta::meta (string metafile): meta() {
    ifstream mf(metafile);
    if (mf.fail())
        throw ifstream::failure(strerror(errno));
    map<string, string> m;
    string buff;
    while (getline(mf, buff)) {
        stringstream ss(buff);
        string key, data;
        getline(ss, key, ':');
        getline(ss, data, ':');
        m[key] = data;
    }
    try {
        cg_mem = tryParsell(m["cg-mem"]);
        csw_forced = tryParsell(m["csw-forced"]);
        csw_voluntary = tryParsell(m["csw-voluntary"]);
        max_rss = tryParsell(m["max-rss"]);
        time = tryParsed(m["time"], -1) * 1000;
        time_wall = tryParsed(m["time-wall"], -1) * 1000;
        exitcode = tryParse(m["exitcode"]);
        exitsig = tryParse(m["exitsig"]);
        isKilled = tryParse(m["killed"]);
        message = m["message"];
        status = m["status"];
    } catch (exception ex) {
        cerr << ex.what() << endl;
        cerr << "Bad META file format" <<endl;
    }
}

