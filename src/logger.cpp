#include "logger.h"
using namespace std;

loglevel logger::globalset = LVIN;

logger::logger(string name) {
    lastlv = LVNU;
    logname = name;
}

void logger::log(string mess, loglevel loglv) {
    if (loglv < globalset)
        return;
    if (loglv == LVNU && lastlv < globalset)
        return;
    if (loglv != LVNU)
        lastlv = loglv;
    string lvmes = to_string(loglv);
    stringstream ident;
    if (globalset == LVD2) {
        ident << left << setw(12) << logname << "-> ";
    }
    string buf;
    stringstream ss(mess);
    stringstream errbuf;
    while (getline(ss, buf)) //Avoid split by other process
        errbuf << ident.str() << lvmes << " " << buf << "\n";
    cerr << errbuf.str();
    cerr.flush();
}

string logger::to_string(loglevel lv) {
    switch (lv) {
        case LVFA: return "[FATAL]";
        case LVER: return "[ERROR]";
        case LVWA: return "[WARN] ";
        case LVIN: return "[INFO] ";
        case LVDE: return "[DEBUG]";
        case LVD2: return "[DEBUG]";
        case LVNU: return "       ";
        default:   return "       ";
    }
}

loglevel logger::getGlobalLevel() {
    return globalset;
}

void logger::setGlobalLevel(loglevel lv) {
    globalset = lv;
}
