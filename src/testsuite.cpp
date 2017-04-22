#include "testsuite.h"
using namespace std;

std::map<std::string, language> langs;

void loadLangs(std::string confpath)
{
    logger lg("LangLoader");
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
            lg.log("Found language: " + name, LVDE);
        });
    }
    catch (exception ex)
    {
        lg.log("Error when loading language config", LVER);
        lg.log(ex.what());
    }
}

