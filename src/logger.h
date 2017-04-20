#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "utils.h"
using namespace std;

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
