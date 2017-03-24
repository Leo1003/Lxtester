#include "server_socket.h"
using namespace std;
using namespace sio;

ServerSocket::ServerSocket(string host, short port, string token)
{
    stringstream ss;
    ss<<"wss://"<<host<<":"<<port;
    this->addr = ss.str();
    this->token = token;
    resetmt();
    cli.set_reconnect_attempts(3);
    cli.set_reconnect_delay(2000);
    cli.set_reconnect_delay_max(2500);
    cli.set_open_listener(bind(&ServerSocket::on_connected, this));
    cli.set_fail_listener(bind(&ServerSocket::on_failed, this));
    cli.set_close_listener(bind(&ServerSocket::on_closed, this, placeholders::_1));
}

ServerSocket::~ServerSocket()
{
    disconnect();
}

int ServerSocket::connect()
{
    if(cli.opened())
        return 0;
    map<string,string> query;
    query["passtoken"] = token;
    resetmt();
    cli.connect(addr,query);
    ULOCK
    if(!unlocked && !failed)
        _cv.wait(_ul);
    _ul.unlock();
    if(failed)
        return 1;
    s = cli.socket("lxtester");
    
    return 0;
}

int ServerSocket::disconnect()
{
    if(!cli.opened())
        return 127;
    ULOCK
    log("Disconnecting from server...", LVIN);
    shutdowned = true;
    cli.close();
    if(!unlocked)
        _cv.wait(_ul);
    _ul.unlock();
    return 0;
}

bool ServerSocket::getConnected() const
{
    return cli.opened();
}

submission ServerSocket::getSubmission()
{
    ULOCK
    if(jobque.empty())
    {
        submission emp(-1, "");
        return emp;
    }
    submission s = jobque.front();
    jobque.pop();
    return s;
}

inline void ServerSocket::resetmt()
{
    unlocked = false;
    failed = false;
}

void ServerSocket::on_connected()
{
    ULOCK
    unlocked = true;
    log("Connected to server.", LVIN);
    _cv.notify_all();
}

void ServerSocket::on_failed()
{
    ULOCK
    failed = true;
    log("Socket failed", LVWA);
    _cv.notify_all();
}

void ServerSocket::on_closed(client::close_reason const& reason)
{
    ULOCK
    if(reason != client::close_reason_normal)
    {
        while(!cli.opened() && !shutdowned)
        {
            log("Lost connect to server.", LVER);
            log("Trying to reconnect...", LVIN);
            connect();
            if(failed)
            {
                log("Reconnect failed.", LVER);
                log("Waiting 60 seconds...", LVIN);
                sleep(60);
            }
        }
    }
    else
    {
        log("Disconnected.", LVIN);
    }
    unlocked = true;
    _cv.notify_all();
}

void ServerSocket::_job(const string& name, const message::ptr& mess, bool need_ack, message::list& ack_message)
{
    ULOCK
    map<string, message::ptr> msg = mess->get_map();
    int id = msg["id"]->get_int();
    string l = msg["language"]->get_string();
    submission sub(id, l);
    if(msg["customname"]->get_bool())
    {
        string exefile = msg["exefile"]->get_string();
        string srcfile = msg["srcfile"]->get_string();
        //Override the origin submission object.
        sub = submission(id, l, exefile, srcfile);
    }
    sub.setCode(msg["code"]->get_string());
    jobque.push(sub);
}

