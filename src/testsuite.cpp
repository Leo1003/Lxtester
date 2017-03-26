#include "testsuite.h"
using namespace std;

std::map<std::string, language> langs;
std::map<pid_t, int> pidmap;

void loadLangs(std::string confpath)
{
    try{
        config conf(confpath);
        vector<string> list = conf.getSections();
        for_each(list.begin(), list.end(), [&](string &name) throw(out_of_range){
            language l;
            config_section lconf = conf[name];
            l.name = lconf.getName();
            l.needComplie = lconf.getBool("needComplie");
            l.complier = lconf.getString("Compiler");
            l.compargs = lconf.getString("CompileArgs");
            l.executer = lconf.getString("Executer");
            l.execargs = lconf.getString("ExecuteArgs");
            langs[name] = l;
        });
    }
    catch (exception ex)
    {
        log("Error when loading language config", LVER);
        log(ex.what());
    }
}

