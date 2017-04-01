#include "submission.h"
using namespace std;
using namespace boost::io;
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
    log(this->lang.complier, LVDE);
    log(this->lang.compargs);
    log(this->lang.executer);
    log(this->lang.execargs);
    
    opt.id = id % 100;
    opt.fsize = 65536;
    opt.time = 30;
    opt.mem = 131072;
    opt.processes = 1;
    opt.stack = 256;
    opt.metafile = "./meta/task" + to_string(id);
    opt.std_in = "stdin.txt";
}

language submission::getLang(string lang)
{
    log(lang, LVDE);
    language l = langs.at(lang);
    try
    {
        l.complier = formatCMD(l.complier);
        l.compargs = formatCMD(l.compargs);
        l.executer = formatCMD(l.executer);
        l.execargs = formatCMD(l.execargs);
    }
    catch (exception ex)
    {
        log("Error when parsing language format string", LVER);
    }
    return l;
}

std::string submission::formatCMD(std::string fmstr)
{
    string str = fmstr;
    try
    {
        format fm(fmstr);
        fm.exceptions(all_error_bits ^ (too_many_args_bit | too_few_args_bit));
        str = (fm % srcname % exename).str();
    }
    catch(exception ex)
    {
        log("Error when parsing language format string: ", LVER);
        log(fmstr);
        log(ex.what());
    }
    return str;
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

exec_opt submission::getOption() const
{
    return opt;
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
    
    exec_opt compile_opt;
    compile_opt.id = opt.id;
    compile_opt.mem = opt.mem;
    compile_opt.fsize = opt.fsize;
    compile_opt.metafile = opt.metafile;
    compile_opt.processes = 10;
    compile_opt.stack = 0;
    compile_opt.time = opt.time;
    
    int status = boxExec(lang.complier + " " + lang.compargs, compile_opt, false);
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

result::result() 
{ 
    type = TYPE_FAILED;
}

result::result (exec_opt option, meta metas)
{
    time = metas.time_wall;
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

