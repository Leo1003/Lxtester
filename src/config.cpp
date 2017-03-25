#include "config.h"
using namespace std;

/*--------------------------
    class config_section
  --------------------------*/

config_section::config_section() : config_section("_") { }
config_section::config_section(std::string name)
{
    this->name = name;
}

bool config_section::getBool(std::string key) const
{
    string str = getString(key);
    regex t("y(?:es)|true", regex::icase);
    regex f("n(?:o)|false", regex::icase);
    if(regex_match(str, t))
        return true;
    if(regex_match(str, f))
        return false;
    log("Parsing config:" + key + " to boolean failed.", LVWA);
    return false;
}

int64_t config_section::getInt(std::string key) const
{
    string str = getString(key);
    int64_t v;
    try
    {
        v = stoll(str);
    }
    catch(invalid_argument ex)
    {
        log("Parsing config:" + key + " to integer failed.", LVWA);
    }
    return v;
}

string config_section::getString(string key) const
{
    try
    {
        return data.at(key);
    }
    catch(out_of_range ex)
    {
        log("Accessing non-existed config:" + key + ".", LVWA);
        return "";
    }
}

std::string config_section::getName() const
{
    return name;
}

void config_section::insert(std::string key, std::string value)
{
    data[key] = value;
}

/*--------------------------
    class config
  --------------------------*/

config::config(std::string path)
{
    try
    {
        ifstream conf(path);
        string buf, secname = "_";
        regex comment("^[\\s]*#.*");
        regex section("^[\\s]*\\[(.+)\\]");
        smatch sm;
        while(getline(conf, buf))
        {
            if(regex_match(buf, comment))
                continue;
            
        }
    }
    catch(exception ex)
    {
        
    }
}

const config_section& config::operator[](std::string key) const
{
    try
    {
        return sec.at(key);
    }
    catch(out_of_range ex)
    {
        log("Accessing non-existed section:" + key + ".", LVWA);
    }
}
