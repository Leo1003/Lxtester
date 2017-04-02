#include "config.h"
using namespace std;

/*--------------------------
 * class config_section
 * -------------------------*/

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
    int64_t v = 0;
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

bool config_section::isExist(std::string key) const
{
    if(data.find(key) == data.end())
        return false;
    else
        return true;
}

void config_section::insert(std::string key, std::string value)
{
    data[key] = value;
}

/*--------------------------
 * class config
 * -------------------------*/

const regex config::reg_empty(R"(^[\s]*$)");
const regex config::reg_comment(R"(^[\s]*#.*)");
const regex config::reg_section(R"(^[\s]*\[(.+)\].*)");
const regex config::reg_setting(R"(^[\s]*([\w]+)[\s]*=[\s]*([\S ]*?)[\s]*$)");

config::config() { }

config::config(std::string path) : config_section()
{
    try
    {
        ifstream conf(path);
        if(conf.fail())
        {
            throw ifstream::failure(strerror(errno));
        }
        log("Loading config: " + path, LVDE);
        int linec = 0;
        string buf, secpointer = "_";
        smatch sm;
        while(getline(conf, buf))
        {
            linec++;
            if(regex_match(buf, reg_empty) || regex_match(buf, reg_comment))
                continue;
            if(regex_match(buf, sm, reg_section))
            {
                log("Loading section: " + string(sm[1]), LVD2);
                config_section cs(sm[1]);
                sec[sm[1]] = cs;
                seclist.push_back(sm[1]);
                secpointer = sm[1];
                continue;
            }
            if(regex_match(buf, sm, reg_setting))
            {
                if(secpointer == "_")
                    this->insert(sm[1], sm[2]);
                else
                    sec[secpointer].insert(sm[1], sm[2]);
                continue;
            }
            log("Can't recognize config file: " + path + " , at line: " + to_string(linec), LVWA);
        }
        conf.close();
        log("Loaded config: " + path, LVDE);
    }
    catch(exception ex)
    {
        log("Something bad happened while parsing config file: " + path, LVFA);
        log(ex.what());
        throw ex;
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
        throw ex;
    }
}

const vector<string>& config::getSections() const
{
    return seclist;
}
