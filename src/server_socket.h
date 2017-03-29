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
#include "submission.h"
#include "utils.h"
#include "../lib/sio_client.h"
#include "../lib/sio_message.h"
#include "../lib/sio_socket.h"
#define ULOCK std::unique_lock<std::mutex> _ul = unique_lock<mutex>(_lock);

class ServerSocket
{
public:
    ServerSocket(std::string host, short port, std::string token);
    ~ServerSocket();
    bool getConnected() const;
    int connect();
    int disconnect();
    bool getSubmission(submission*& sub);
    void sendResult(const submission& sub);
private:
    sio::client cli;
    sio::socket::ptr s;
    std::mutex _lock;
    std::condition_variable _cv;
    bool unlocked, failed, shutdowned;
    std::string addr;
    std::string token;
    std::queue<submission> jobque;
    
    inline void resetmt();
    //Listeners
    void on_connected();
    void on_failed();
    void on_error(sio::message::ptr const& message);
    void on_closed(sio::client::close_reason const& reason);
    //Events
    void _job(const std::string& name, sio::message::ptr const& mess, bool need_ack, sio::message::list& ack_message);
    void _callback(const std::string& name, sio::message::ptr const& mess, bool need_ack, sio::message::list& ack_message);
};

#endif // SERVER_SOCKET_H
