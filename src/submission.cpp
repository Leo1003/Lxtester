#include "submission.h"
using namespace std;
using boost::format;

/*--------------------------
 * class submission
 * -------------------------*/

submission::submission(int id, string lang) : submission(id, lang, "exe", "src") {}
submission::submission(int id, string lang, string exe, string src)
{
    this->id = id;
    this->exename = exe;
    this->srcname = src;
    this->lang = getLang(lang);
    
    opt.id = id;
    opt.fsize = 65536;
    opt.time = 30;
    opt.mem = 131072;
    opt.processes = 1;
    opt.stack = 256;
    opt.metafile = "./meta/task" + to_string(id);
    opt.std_in = "stdin.txt";
    //create sandbox
    boxInit(opt);
}

submission::~submission()
{
    //remove sandbox
    boxDel(opt);
}

language submission::getLang(string lang)
{
    language l = langs[lang];
    try
    {
        l.complier = (format(l.complier) % srcname % exename).str();
        l.compargs = (format(l.compargs) % srcname % exename).str();
        l.executer = (format(l.executer) % srcname % exename).str();
        l.execargs = (format(l.execargs) % srcname % exename).str();
    }
    catch (exception ex)
    {
        log("Error when parsing language format string", LVER);
    }
    return l;
}

string submission::getCode() const
{
    return code;
}

void submission::setCode(string code)
{
    this->code = code;
}

string submission::getStdin() const
{
    return stdin;
}

void submission::setStdin(string data)
{
    this->stdin = data;
}

int submission::getId() const
{
    return id;
}

pid_t submission::getPID() const
{
    return pid;
}

result submission::getResult() const
{
    return res;
}

void submission::setResult(result res)
{
    this->res = res;
}

pid_t submission::compile()
{
    if (!lang.needComplie)
        return 0;
    if (!created)
    {
        boxInit(opt);
        created = true;
    }
    pid = boxExec(lang.complier + " " + lang.compargs, opt);
    return pid;
}

pid_t submission::execute()
{
    if (!created)
    {
        boxInit(opt);
        created = true;
    }
    pid = boxExec(lang.executer + " " + lang.execargs, opt);
    return pid;
}

/*--------------------------
 * struct result
 * -------------------------*/

result::result() { }

result::result (exec_opt option, meta metas)
{
    time = metas.time;
    mem = metas.max_rss;
    exitcode = metas.exitcode;
    signal = metas.exitsig;
    isKilled = metas.isKilled;
    try
    {
        ifstream outf("/tmp/box/" + to_string(option.id) + "/box/stdout.log");
        string s;
        while(getline(outf, s))
        {
            std_out += s + "\n";
        }
        outf.close();
    }
    catch(exception ex)
    {
        log("Fail to open output", LVER);
        log(ex.what());
    }
    try
    {
        ifstream errf("/tmp/box/" + to_string(option.id) + "/box/stderr.log");
        string s;
        while(getline(errf, s))
        {
            std_err += s + "\n";
        }
        errf.close();
    }
    catch(exception ex)
    {
        log("Fail to open stderr", LVER);
        log(ex.what());
    }
}

