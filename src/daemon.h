#ifndef DAEMON_H
#define DAEMON_H
#include <fcntl.h>
#include <map>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "config.h"
#include "global.h"
#include "server_socket.h"
#include "submission.h"
#include "utils.h"

int maind();
void reconnect();
void signal_handler(int);
void child_handler();
pid_t testWorkFlow(submission& sub);
bool DetectDaemon();

#endif // DAEMON_H
