#include "runner.h"
#define PB push_back
using namespace std;

int boxInit(exec_opt option)
{
    vector<string> args;
    args.PB("./bin/isolate");
    args.PB("--init");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.id));
    
    char** argp = parseVecstr(args);

    pid_t pid;
    int status = advFork(argp, pid);
    
    delCStrings(argp);
    
    if(WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int boxExec(string cmd, exec_opt option, bool enableStdin)
{
    vector<string> args;
    args.PB("./bin/isolate"); //TODO:fix relative path
    args.PB("--run");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.id));
    double timeout = option.time / 1000.0;
    args.PB("--time=" + to_string(timeout));
    args.PB("--wall-time=" + to_string(timeout));
    args.PB("--mem=" + to_string(option.mem));
    args.PB("--processes=" + to_string(option.processes));
    args.PB("--stack=" + to_string(option.stack));
    args.PB("--fsize=" + to_string(option.fsize));
    if(enableStdin)
        args.PB("--stdin=" + option.std_in);
    args.PB("--stdout=stdout.log");
    args.PB("--stderr=stderr.log");
    args.PB("--meta=" + option.metafile);
    args.PB("--");
    
    //split cmd string into vector<string>
    cmd = trim(cmd);
    log("SandboxExec: CMD = \"" + cmd + "\"", LVDE);
    stringstream ss(cmd);
    string tmp;
    while(!ss.eof())
    {
        ss >> tmp;
        args.PB(tmp);
    }
    
    char** argp = parseVecstr(args);
    
    pid_t pid;
    int status = advFork(argp, pid);
    
    delCStrings(argp);

    if(WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int boxDel(exec_opt option)
{
    vector<string> args;
    args.PB("./bin/isolate");
    args.PB("--cleanup");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.id));

    char** argp = parseVecstr(args);

    pid_t pid;
    int status = advFork(argp, pid);
    
    delCStrings(argp);
    
    if(WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int advFork(char** argp, pid_t& pid, bool wait)
{
    if(getLevel() == LVDE)
    {
        stringstream ss;
        log("Advfork: Exec command:", LVDE);
        ss << "[";
        int i = 0;
        while(argp[i] != NULL)
        {
            ss << argp[i] << ", ";
            i++;
        }
        ss << "]" << endl;
        log(ss.str());
    }
    int status = 0;
    pid = fork();
    if(pid == 0)
    {
        //child process
        execvp(argp[0], argp);
        exit(127);
    }
    else if(pid > 0)
    {
        //main process
        if(wait)
        {
            waitpid(pid, &status, 0);
        }
    }
    else
    {
        //failed
        log("Advfork Failed.", LVER);
        log(strerror(errno));
        status = -1;
    }
    return status;
}

meta::meta() { }

meta::meta (string metafile)
{
    ifstream mf(metafile);
    map<string, string> m;
    string buff;
    while(getline(mf, buff))
    {
        stringstream ss(buff);
        string key, data;
        getline(ss, key, ':');
        getline(ss, data, ':');
        m[key] = data;
    }
    try
    {
        cg_mem = tryParsell(m["cg-mem"]);
        csw_forced = tryParsell(m["csw-forced"]);
        csw_voluntary = tryParsell(m["csw-voluntary"]);
        max_rss = tryParsell(m["max-rss"]);
        time = tryParsell(m["time"], -1);
        time_wall = tryParsell(m["time-wall"], -1);
        exitcode = tryParse(m["exitcode"]);
        exitsig = tryParse(m["exitsig"]);
        isKilled = tryParse(m["killed"]);
        message = m["message"];
        status = m["status"];
    }
    catch(exception ex)
    {
        cerr << ex.what() << endl;
        cerr << "Bad META file format" <<endl;
    }
}

