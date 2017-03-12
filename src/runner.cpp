#include "runner.h"
#define PB push_back
using namespace std;

int boxInit(exec_opt option)
{
    vector<string> args;
    args.PB("./isolate");
    args.PB("--init");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.id));

    char** argp;
    parseVecstr(args, &argp);

    pid_t pid;
    int status = advFork(argp, pid);
    
    if(WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

pid_t boxExec(string cmd, exec_opt option)
{
    vector<string> args;
    args.PB("./isolate"); //TODO:fix relative path
    args.PB("--run");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.id));
    double timeout = option.time / 1000.0;
    args.PB("--time=" + to_string(timeout));
    args.PB("--wall-time=" + to_string(timeout));
    args.PB("--mem=" + to_string(option.mem));
    args.PB("--process=" + to_string(option.processes));
    args.PB("--stack=" + to_string(option.stack));
    args.PB("--fsize=" + to_string(option.fsize));
    args.PB("--file-limit=" + to_string(option.files));
    args.PB("--stdin=" + option.std_in);
    args.PB("--stdout=stdout.log");
    args.PB("--stderr=stderr.log");
    args.PB("--stdin=" + option.metafile);
    args.PB("--");
    
    //split cmd string into vector<string>
    stringstream ss(cmd);
    string tmp;
    while(ss)
    {
        ss >> tmp;
        args.PB(tmp);
    }
    
    char** argp;
    parseVecstr(args, &argp);
    
    pid_t pid;
    advFork(argp, pid, false);

    return pid;
}

int boxDel(exec_opt option)
{
    vector<string> args;
    args.PB("./isolate");
    args.PB("--cleanup");
    args.PB("--cg");
    args.PB("--box-id=" + to_string(option.id));

    char** argp;
    parseVecstr(args, &argp);

    pid_t pid;
    int status = advFork(argp, pid);
    
    if(WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 255;
}

int advFork(char** argp, pid_t& pid, bool wait)
{
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
        status = -1;
    }
    return status;
}

int parsemeta(string metafile, meta &metas)
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
        metas.cg_mem = tryParsell(m["cg-mem"]);
        metas.csw_forced = tryParsell(m["csw-forced"]);
        metas.csw_voluntary = tryParsell(m["csw-voluntary"]);
        metas.max_rss = tryParsell(m["max-rss"]);
        metas.time = tryParsell(m["time"], -1);
        metas.time_wall = tryParsell(m["time-wall"], -1);
        metas.exitcode = tryParse(m["exitcode"]);
        metas.exitsig = tryParse(m["exitsig"]);
        metas.isKilled = tryParse(m["killed"]);
        metas.message = m["message"];
        metas.status = m["status"];
    }
    catch(exception ex)
    {
        cerr << ex.what() << endl;
        cerr << "Bad META file format" <<endl;
        return 2;
    }
    return 0;
}
