#ifndef CONFIG_H
#define CONFIG_H
#include <fstream>
#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <vector>
#include "logger.h"
#include "utils.h"

class config_section
{
public:
    config_section();
    config_section(std::string name);
    std::string getName() const;
    bool getBool(std::string key) const;
    int getInt(std::string key) const;
    std::string getString(std::string key) const;
    bool trygetBool(bool& buf, std::string key) const;
    bool trygetInt(int& buf, std::string key) const;
    bool trygetString(std::string& buf, std::string key) const;
    bool isExist(std::string key) const;
    void insert(std::string key, std::string value);
private:
    std::string name;
    std::map<std::string, std::string> data;
};

class config : public config_section
{
public:
    config();
    config(std::string path);
    const config_section& operator[] (std::string key) const;
    const std::vector<std::string>& getSections() const;
private:
    static const std::regex reg_empty;
    static const std::regex reg_comment;
    static const std::regex reg_section;
    static const std::regex reg_setting;
    static const std::regex reg_conffile;
    bool LoadFile(std::string path);
    std::vector<std::string> seclist;
    std::map<std::string, config_section> sec;
};

#endif // CONFIG_H
