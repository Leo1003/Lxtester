#include "submission.h"
using namespace std;

submission::submission(int id, string lang)
{
    this->id = id;
    this->lang = lang; //TODO:directly change string into langusge struct
    opt.id = id;
    opt.fsize = 65536;
    opt.time = 30;
    opt.mem = 131072;
    opt.processes = 1;
    opt.stack = 256;
    opt.metafile = "./meta/task" + to_string(id);
    opt.std_in = "stdin.txt";
}

string submission::getCode()
{
    return code;
}

void submission::setCode(string code)
{
    this->code = code;
}

int submission::getId()
{
    return id;
}

pid_t submission::getPID()
{
    return pid;
}

result submission::getResult()
{
    return res;
}

void submission::setResult(result res)
{
    this->res = res;
}

pid_t submission::compile()
{
    language l = langs[lang];
    //TODO:parse formatted string to real arg
    if (!l.needComplie)
        return 0;
    if (!created)
    {
        boxInit(opt);
        created = true;
    }
    return boxExec(l.complier + " " + l.compargs, opt);
}

pid_t submission::execute()
{
    language l = langs[lang];
    //TODO:parse formatted string to real arg
    if (!created)
    {
        boxInit(opt);
        created = true;
    }
    return boxExec(l.executer + " " + l.execargs, opt);
}
