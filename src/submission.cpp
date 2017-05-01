#include "submission.h"
using namespace std;
using namespace boost::io;
using boost::format;

/*--------------------------
 * class submission
 * -------------------------*/

submission::submission() : opt(-1) {}
submission::submission(int id, string lang, string exe, string src)
{
    this->id = id;
    this->exename = exe;
    this->srcname = src;
    this->lang = getLang(lang);
    
    opt.fsize = 65536;
    opt.time = 30;
    opt.mem = 131072;
    opt.processes = 1;
    opt.stack = 256;
    opt.metafile = "./meta/task" + to_string(opt.getId());
    opt.std_in = "stdin.txt";
}

language submission::getLang(string lang)
{
    mainlg.log(lang, LVD2);
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
        mainlg.log("Error when parsing language format string", LVER);
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
        mainlg.log("Error when parsing language format string: ", LVER);
        mainlg.log(fmstr);
        mainlg.log(ex.what());
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

exec_opt& submission::getOption()
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
    string path = BoxDir + "/" + to_string(opt.getId()) + "/box/" + srcname;
    ofstream code(path);
    mainlg.log("Source file: " + path, LVD2);
    if(!code)
    {
        mainlg.log("Failed to opened code file.", LVER);
        mainlg.log("Box id: " + to_string(opt.getId()));
        throw ifstream::failure(strerror(errno));
    }
    code << this->code;
    code.flush();
    code.close();
    
    if (!lang.needComplie)
        return 0;
    
    exec_opt compile_opt(opt.getId());
    compile_opt.mem = 262144;
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
    string path = BoxDir + "/" + to_string(opt.getId()) + "/box/" + opt.std_in;
    ofstream infile(path);
    mainlg.log("Stdin file: " + path, LVD2);
    if(!infile)
    {
        mainlg.log("Failed to opened stdin file.", LVER);
        mainlg.log("Box id: " + to_string(opt.getId()));
        throw ifstream::failure(strerror(errno));
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
    time = 0;
    mem = 0;
    exitcode = 0;
    signal = 0;
    isKilled = false;
    std_out = "";
    std_err = "";
    type = TYPE_FAILED;
}

result::result (int boxid, meta metas)
{
    time = metas.time_wall;
    mem = metas.max_rss;
    exitcode = metas.exitcode;
    signal = metas.exitsig;
    isKilled = metas.isKilled;
    try
    {
        ifstream outf(BoxDir + "/" + to_string(boxid) + "/box/stdout.log");
        if(outf.fail())
            throw ifstream::failure(strerror(errno));
        string s;
        while(getline(outf, s))
            std_out += s + "\n";
        outf.close();
    }
    catch(exception ex)
    {
        mainlg.log("Fail to open output", LVER);
        mainlg.log(ex.what());
    }
    try
    {
        ifstream errf(BoxDir + "/" + to_string(boxid) + "/box/stderr.log");
        if(errf.fail())
            throw ifstream::failure(strerror(errno));
        string s;
        while(getline(errf, s))
            std_err += s + "\n";
        errf.close();
    }
    catch(exception ex)
    {
        mainlg.log("Fail to open stderr", LVER);
        mainlg.log(ex.what());
    }
}

