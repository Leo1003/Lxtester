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

class ServerSocket
{
public:
    ServerSocket(std::string host, short port, std::string token);
    ~ServerSocket();
    ConnectionStatus getStatus() const;
    void connect();
    void disconnect();
    bool getSubmission(submission& sub);
    void sendResult(const submission& sub);
private:
    sio::client cli;
    sio::socket::ptr s;
    std::mutex _lock;
    std::condition_variable _cv;
    bool unlocked;
    std::string addr;
    std::string token;
    std::queue<submission> jobque;
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
    void _job(const std::string& name, sio::message::ptr const& mess, bool need_ack, sio::message::list& ack_message);
    void _callback(const std::string& name, sio::message::ptr const& mess, bool need_ack, sio::message::list& ack_message);
};

#endif // SERVER_SOCKET_H
