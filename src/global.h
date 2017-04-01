#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
using std::string;

extern bool daerunning;
extern int daepid;
extern bool stopping;

/*--------------------
 * config variables
 * -------------------*/

extern bool DaemonMode;
extern string PIDFile;
extern string LOCKFile;
extern string BoxDir;
extern string WorkingDir;
extern string LogFile;
extern string LangFile;
extern string ServerAddr;
extern short ServerPort;
extern string ServerToken;

#endif // GLOBAL_H
