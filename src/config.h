#ifndef CONFIG_H
#define CONFIG_H
#include <fstream>
#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <vector>
#include "utils.h"

class config_section
{
public:
    config_section();
    config_section(std::string name);
    std::string getName() const;
    bool getBool(std::string key) const;
    int64_t getInt(std::string key) const;
    std::string getString(std::string key) const;
    void insert(std::string key, std::string value);
private:
    std::string name;
    std::map<std::string, std::string> data;
};

class config : config_section
{
public:
    config();
    config(std::string path);
    const config_section& operator[] (std::string key) const;
    const std::vector<std::string>& getSections() const;
    
    using config_section::getBool;
    using config_section::getInt;
    using config_section::getString;
private:
    std::vector<std::string> seclist;
    std::map<std::string, config_section> sec;
};

#endif // CONFIG_H
