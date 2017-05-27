#include "global.h"

bool refresh = false;
bool daerunning = false;
int daepid = 0;
bool stopping = false;
bool stopping2 = false;

pid_t nowrunning = 0;

exec_opt execset;
exec_opt compset;

bool DaemonMode = true;
string PIDFile = "/tmp/lxtester.pid";
string LOCKFile = "/tmp/lxtester.lock";
string BoxDir = "/tmp/box";
int MaxWorker = 10;
string WorkingDir = ".";
string LogFile = "/tmp/lxtester.log";
string LangFile = "languages.conf";
string OptionFile = "options.conf";
string IsoBinFile = "./isolate/isolate";
string ServerAddr = "localhost";
short ServerPort = 80;
string ServerToken;
