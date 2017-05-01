#include "runner.h"
#define PB push_back
using namespace std;

int boxInit(exec_opt& option)
{
    vector<string> args;
    args.PB(IsoBinFile);
    args.PB("--init");
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

int boxExec(string cmd, exec_opt& option, bool enableStdin)
{
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
    if(enableStdin)
        args.PB("--stdin=" + option.std_in);
    args.PB("--stdout=stdout.log");
    args.PB("--stderr=stderr.log");
    args.PB("--meta=" + option.metafile);
    args.PB("--full-env");
    args.PB("--");
    
    //split cmd string into vector<string>
    cmd = trim(cmd);
    mainlg.log("SandboxExec: CMD = \"" + cmd + "\"", LVD2);
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

int boxDel(exec_opt& option)
{
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

int advFork(char** argp, pid_t& pid, bool wait)
{
    if(logger::getGlobalLevel() <= LVDE)
    {
        stringstream ss;
        mainlg.log("Advfork: Exec command:", LVDE);
        ss << "[";
        int i = 0;
        while(argp[i] != NULL)
        {
            ss << argp[i++];
            if(argp[i] != NULL) ss << ", ";
        }
        ss << "]" << endl;
        mainlg.log(ss.str());
    }

    int status = 0;
    pid = fork();
    if(pid == 0)
    {
        //child process
        execvp(argp[0], argp);
        mainlg.log("Failed to exec.", LVFA);
        exit(127);
    }
    else if(pid > 0)
    {
        mainlg.log("Child PID: " + to_string(pid));
        //main process
        if(wait)
            if(waitpid(pid, &status, 0) == -1)
            {
                mainlg.log("Advfork: Failed to wait child process", LVER);
                mainlg.log(strerror(errno));
            }
    }
    else
    {
        //failed
        mainlg.log("Advfork Failed.", LVER);
        mainlg.log(strerror(errno));
        status = -1;
    }
    
    return status;
}

bitset<100> exec_opt::boxslist;
exec_opt::exec_opt()
{
    id = registbox();
    registedID = true;
}

exec_opt::exec_opt(int id)
{
    this->id = id;
    registedID = false;
}

exec_opt::exec_opt(exec_opt && old)
{
    this->id = old.id;
    this->registedID = old.registedID;
    fsize = old.fsize;
    time = old.time;
    mem = old.mem;
    processes = old.processes;
    stack = old.stack;
    metafile = old.metafile;
    std_in = old.std_in;
    
    old.id = -1;
    old.registedID = false;
}

exec_opt & exec_opt::operator=(exec_opt && old)
{
    if (this != &old)
    {
        this->id = old.id;
        this->registedID = old.registedID;
        fsize = old.fsize;
        time = old.time;
        mem = old.mem;
        processes = old.processes;
        stack = old.stack;
        metafile = old.metafile;
        std_in = old.std_in;
        
        old.id = -1;
        old.registedID = false;
    }
    return *this;
}

exec_opt::~exec_opt()
{
    if(registedID)
        boxslist.reset(id);
}

int exec_opt::getId()
{
    return id;
}

int exec_opt::registbox()
{
    for(int i = 0; i < 100; i++)
    {
        if(!boxslist.test(i))
        {
            boxslist.set(i);
            mainlg.log("RegistID: " + to_string(i), LVDE);
            return i;
        }
    }
    return -1;
}

meta::meta() 
{ 
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

meta::meta (string metafile)
{
    ifstream mf(metafile);
    if(mf.fail())
        throw ifstream::failure(strerror(errno));
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
        time = tryParsed(m["time"], -1) * 1000;
        time_wall = tryParsed(m["time-wall"], -1) * 1000;
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

