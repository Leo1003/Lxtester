#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
#include "logger.h"
#include "runner.h"
using std::string;

extern bool daerunning;
extern int daepid;
extern bool stopping, stopping2, reset;

extern exec_opt execset;
extern exec_opt compset;

/*--------------------
 * config variables
 * -------------------*/

extern bool DaemonMode;
extern string PIDFile;
extern string LOCKFile;
extern string BoxDir;
extern int MaxWorker;
extern string WorkingDir;
extern string LogFile;
extern string LangFile;
extern string OptionFile;
extern string IsoBinFile;
extern string ServerAddr;
extern short ServerPort;
extern string ServerToken;
extern class logger mainlg;

#endif // GLOBAL_H
