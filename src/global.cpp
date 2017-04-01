#include "global.h"

bool daerunning = false;
int daepid = 0;
bool stopping = false;

bool DaemonMode = true;
string PIDFile = "/tmp/lxtester.pid";
string LOCKFile = "/tmp/lxtester.lock";
string BoxDir = "/tmp/box";
string WorkingDir = ".";
string LogFile = "/tmp/lxtester.log";
string LangFile = "languages.conf";
string ServerAddr = "localhost";
short ServerPort = 80;
string ServerToken;
