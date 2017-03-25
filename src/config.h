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
    std::string getName() const;
    bool getBool(std::string key) const;
    int64_t getInt(std::string key) const;
    std::string getString(std::string key) const;
protected:
    config_section();
    config_section(std::string name);
    void insert(std::string key, std::string value);
private:
    std::string name;
    std::map<std::string, std::string> data;
};

class config : config_section
{
public:
    config(std::string path);
    const config_section& operator[] (std::string key) const;
private:
    std::map<std::string, config_section> sec;
};

#endif // CONFIG_H
