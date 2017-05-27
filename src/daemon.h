#ifndef DAEMON_H
#define DAEMON_H
#include <fcntl.h>
#include <map>
#include <signal.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "config.h"
#include "server_socket.h"
#include "submission.h"
#include "utils.h"

void enterDaemon();
void maind();
void reconnect();
void signal_handler(int);
void child_handler();
pid_t testWorkFlow(submission& sub);
void worker_signal_handler(int);
bool DetectDaemon();

#endif // DAEMON_H
