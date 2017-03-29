#include "submission.h"
using namespace std;
using boost::format;

/*--------------------------
 * class submission
 * -------------------------*/

string submission::BOXDIR = "/tmp/box";
submission::submission() {}
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
    if(boxInit(opt) != 0) 
    {
        log("Unable to create box.", LVER);
        log("Box id: " + to_string(opt.id));
    }
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

int submission::compile()
{
    if (!lang.needComplie)
        return 0;
    
    ofstream code(BOXDIR + "/" + to_string(opt.id) + "/box/" + srcname);
    if(!code)
    {
        log("Failed to opened code file.", LVER);
        log("Box id: " + to_string(opt.id));
        return -1;
    }
    code << this->code;
    code.flush();
    code.close();
    
    int status = boxExec(lang.complier + " " + lang.compargs, opt, false);
    return status;
}

int submission::execute()
{
    ofstream infile(BOXDIR + "/" + to_string(opt.id) + "/box/" + opt.std_in);
    if(!infile)
    {
        log("Failed to opened stdin file.", LVER);
        log("Box id: " + to_string(opt.id));
        return -1;
    }
    infile << this->opt.std_in;
    infile.flush();
    infile.close();

    int status = boxExec(lang.executer + " " + lang.execargs, opt);
    return status;
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
        ifstream outf(submission::BOXDIR + "/" + to_string(option.id) + "/box/stdout.log");
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
        ifstream errf(submission::BOXDIR + "/" + to_string(option.id) + "/box/stderr.log");
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

