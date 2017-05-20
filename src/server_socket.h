#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "logger.h"
#include "submission.h"
#include "utils.h"
#include "../lib/sio_client.h"
#include "../lib/sio_message.h"
#include "../lib/sio_socket.h"
#define ULOCK std::unique_lock<std::mutex> _ul = unique_lock<mutex>(_lock);

enum ConnectionStatus
{
    NotConnected,
    Connected,
    Failed,
    Errored,
    Disconnected
};

enum JobType
{
    None,
    Submission,
    Cancel
};

struct Job
{
    JobType type;
    submission sub;
    int submissionid;
};

class ServerSocket
{
public:
    ServerSocket(std::string host, short port, std::string token);
    ~ServerSocket();
    ConnectionStatus getStatus() const;
    void connect();
    void suspend();
    void resume();
    void disconnect();
    Job getJob();
    size_t countJob() const;
    void sendResult(const submission& sub);
private:
    sio::client cli;
    sio::socket::ptr s;
    std::mutex _lock;
    std::condition_variable _cv;
    bool unlocked;
    std::string addr;
    std::string token;
    std::queue<Job> jobque;
    logger lg;
    ConnectionStatus stat;

    inline void resetmt();
    void _connect();
    //Listeners
    void on_connected();
    void on_failed();
    void on_error(sio::message::ptr const& message);
    void on_closed(sio::client::close_reason const& reason);
    //Events
    void _job(sio::event& event);
    void _cancel(sio::event& event);
};

#endif // SERVER_SOCKET_H
