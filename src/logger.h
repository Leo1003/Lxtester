#ifndef LOGGER_H
#define LOGGER_H
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

enum loglevel
{
    LVUNDEF = -1,
    LVD2,
    LVDE,
    LVIN,
    LVWA,
    LVER,
    LVFA,
    LVNU
};

class logger {
public:
    logger(string name);
    void log(string mess, loglevel loglv = LVNU);
    static string to_string(loglevel lv);
    static loglevel getGlobalLevel();
    static void setGlobalLevel(loglevel lv);
private:
    string logname;
    loglevel lastlv;
    static loglevel globalset;
};

#endif
