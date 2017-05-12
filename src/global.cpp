#include "global.h"

bool reset = false;
bool daerunning = false;
int daepid = 0;
bool stopping = false;
bool stopping2 = false;

bool DaemonMode = true;
string PIDFile = "/tmp/lxtester.pid";
string LOCKFile = "/tmp/lxtester.lock";
string BoxDir = "/tmp/box";
string WorkingDir = ".";
string LogFile = "/tmp/lxtester.log";
string LangFile = "languages.conf";
string IsoBinFile = "./isolate/isolate";
string ServerAddr = "localhost";
short ServerPort = 80;
string ServerToken;
