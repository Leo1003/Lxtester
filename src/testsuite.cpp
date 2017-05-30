#include "testsuite.h"
#include "global.h"
using namespace std;

std::map<std::string, language> langs;

void loadLangs(std::string confpath) {
    logger lg("LangLoader");
    try {
        config conf(confpath);
        vector<string> list = conf.getSections();
        for_each(list.begin(), list.end(), [&](string &name) {
            language l;
            config_section lconf = conf[name];
            l.name = lconf.getName();
            l.needCompile = lconf.getBool("needCompile");
            if(l.needCompile) {
                l.complier = lconf.getString("Compiler");
                l.compargs = lconf.getString("CompileArgs");
            }
            l.executer = lconf.getString("Executer");
            l.execargs = lconf.getString("ExecuteArgs");
            l.srcext = lconf.getString("SourceExt");
            if (lconf.isExist("Environment")) {
                string envstr = lconf.getString("Environment");
                string s;
                while(s = delimstring(envstr, ','), s != "") {
                    l.env.push_back(s);
                }
            }
            if (lconf.isExist("MountRules")) {
                string envstr = lconf.getString("MountRules");
                string s;
                while(s = delimstring(envstr, ','), s != "") {
                    l.dirrules.push_back(s);
                }
            }
            langs[name] = l;
            lg.log("Found language: " + name, LVDE);
        });
    } catch (exception ex) {
        lg.log("Error when loading language config", LVER);
        lg.log(ex.what());
    }
}

void extractOption(config_section conf, exec_opt &o) {
    conf.trygetInt(o.outputLength, "OutputLength");
    conf.trygetInt(o.time, "Time");
    conf.trygetInt(o.mem, "Memory");
    conf.trygetInt(o.fsize, "FileSize");
    conf.trygetInt(o.processes, "Processes");
    conf.trygetInt(o.stack, "Stack");
}

void loadSandboxOption(string confpath) {
    logger lg("OptionLoader");
    try {
        lg.log("Loading sandbox option...", LVDE);
        config conf(confpath);
        //Global Option
        extractOption(conf, execset);
        extractOption(conf, compset);
        //Independent Option
        extractOption(conf["Execute"], execset);
        extractOption(conf["Compile"], compset);
        lg.log("Loaded sandbox option.", LVIN);
    } catch (exception ex) {
        lg.log("Error when loading sandbox options", LVER);
        lg.log(ex.what());
    }
}
