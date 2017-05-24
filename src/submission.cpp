#include "submission.h"
#include "global.h"
using namespace std;
//using namespace boost::io;
using boost::format;
using boost::io::all_error_bits;
using boost::io::too_many_args_bit;
using boost::io::too_few_args_bit;

/*--------------------------
 * class submission
 * -------------------------*/

submission::submission(): res("Result not set") {}
submission::submission(int id, string lang, string exe, string src): submission() {
    this->id = id;
    this->exename = exe;
    this->srcname = src;
    this->lang = getLang(lang);

    execset.copySettings(opt);
    opt.std_in = "stdin.txt";
}

language submission::getLang(string lang) {
    mainlg.log("Language: " + lang, LVD2);
    language l;
    try {
        l = langs.at(lang);
        l.complier = formatCMD(l.complier);
        l.compargs = formatCMD(l.compargs);
        l.executer = formatCMD(l.executer);
        l.execargs = formatCMD(l.execargs);
    } catch (out_of_range ex) {
        mainlg.log("Received a unsupport language.", LVWA);
        throw unsupport_language();
    } catch (exception ex) {
        mainlg.log("Error when parsing language format string", LVER);
    }
    return l;
}

std::string submission::formatCMD(std::string fmstr) {
    string str = fmstr;
    try {
        format fm(fmstr);
        fm.exceptions(all_error_bits ^ (too_many_args_bit | too_few_args_bit));
        str = (fm % srcname % exename).str();
    } catch (exception ex) {
        mainlg.log("Error when parsing language format string: ", LVER);
        mainlg.log(fmstr);
        mainlg.log(ex.what());
    }
    return str;
}

string submission::getCode() const {
    return code;
}

void submission::setCode(string code) {
    this->code = code;
}

string submission::getStdin() const {
    return stdin;
}

void submission::setStdin(string data) {
    this->stdin = data;
}

int submission::getId() const {
    return id;
}

exec_opt& submission::getOption() {
    return opt;
}

result submission::getResult() const {
    return res;
}

void submission::setResult(result res) {
    this->res = res;
}

void submission::extract(std::string file, std::string data) {
    string path = BoxDir + "/" + to_string(opt.getId()) + "/box/" + file;
    ofstream fs(path);
    mainlg.log("Source file: " + path, LVD2);
    if (!fs) {
        mainlg.log("Failed to opened file: " + path, LVER);
        mainlg.log("Box id: " + to_string(opt.getId()));
        throw ios_base::failure(strerror(errno));
    }
    fs << data;
    fs.flush();
    fs.close();
}

int submission::initBoxid() {
    opt.registerbox();
    opt.metafile = "./meta/task" + to_string(opt.getId());
    return opt.getId();
}

int submission::setup() {
    if (initBoxid() == -1)
        return 1;
    if (boxInit(opt)) {
        mainlg.log("Unable to create box.", LVER);
        mainlg.log("Box id: " + to_string(opt.getId()));
        return 2;
    }
    mainlg.log("Box id: " + to_string(opt.getId()) + " created.", LVDE);
    extract(srcname + "." + this->lang.srcext, this->code);
    extract(opt.std_in, this->stdin);
    return 0;
}

int submission::compile() {
    if (!lang.needCompile)
        return 0;

    exec_opt compile_opt = opt;
    compset.copySettings(compile_opt);
    compile_opt.metafile = opt.metafile;

    return boxExec(lang.complier + " " + lang.compargs, compile_opt, lang, false);
}

int submission::execute() {
    return boxExec(lang.executer + " " + lang.execargs, opt, lang);
}

int submission::clean() {
    if (boxDel(opt)) {
        mainlg.log("Unable to remove box.", LVER);
        mainlg.log("Box id: " + to_string(opt.getId()));
    }
    mainlg.log("Box id: " + to_string(opt.getId()) + " removed.", LVDE);
    opt.releasebox();
    if (unlink(opt.metafile.c_str())) {
        mainlg.log("Unable to delete meta file.", LVWA);
        mainlg.log(strerror(errno));
    }
    return 0;
}

/*--------------------------
 * struct result
 * -------------------------*/

result::result(string error) {
    time = -1;
    mem = -1;
    exitcode = 0;
    signal = 0;
    isKilled = true;
    std_out = "";
    std_err = error;
    type = TYPE_FAILED;
}

result::result (int boxid, meta metas) {
    time = metas.time_wall;
    mem = metas.max_rss;
    exitcode = metas.exitcode;
    signal = metas.exitsig;
    isKilled = metas.isKilled;
    try {
        ifstream outf(BoxDir + "/" + to_string(boxid) + "/box/stdout.log");
        if(outf.fail())
            throw ifstream::failure(strerror(errno));
        string s;
        while(getline(outf, s))
            std_out += s + "\n";
        outf.close();
    } catch (exception ex) {
        mainlg.log("Fail to open output", LVER);
        mainlg.log(ex.what());
    }
    try {
        ifstream errf(BoxDir + "/" + to_string(boxid) + "/box/stderr.log");
        if(errf.fail())
            throw ifstream::failure(strerror(errno));
        string s;
        while(getline(errf, s))
            std_err += s + "\n";
        errf.close();
    } catch (exception ex) {
        mainlg.log("Fail to open stderr", LVER);
        mainlg.log(ex.what());
    }
}
