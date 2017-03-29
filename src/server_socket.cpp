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
    cli.connect(addr,query);
    resetmt();
    ULOCK
    if(!unlocked && !failed)
        _cv.wait(_ul);
    _ul.unlock();
    if(failed)
        return 1;
    s = cli.socket("lxtester");
    s->on_error(bind(&ServerSocket::on_error, this, placeholders::_1));
    s->on("Job", bind(&ServerSocket::_job, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4));
    s->on("CallBack", bind(&ServerSocket::_callback, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4));
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
    resetmt();
    if(!unlocked && !failed)
        _cv.wait(_ul);
    _ul.unlock();
    return 0;
}

bool ServerSocket::getConnected() const
{
    return cli.opened();
}

bool ServerSocket::getSubmission(submission& sub)
{
    ULOCK
    if(jobque.empty())
    {
        return false;
    }
    sub = jobque.front();
    jobque.pop();
    return true;
}

void ServerSocket::sendResult(const submission& sub)
{
    ULOCK
    object_message mess = *(static_pointer_cast<object_message>(object_message::create()));
    mess.insert("id", int_message::create(sub.getId()));
    result re = sub.getResult();
    mess.insert("time", int_message::create(re.time));
    mess.insert("memory", int_message::create(re.mem));
    mess.insert("exitcode", int_message::create(re.exitcode));
    mess.insert("signal", int_message::create(re.signal));
    mess.insert("killed", bool_message::create(re.isKilled));
    mess.insert("output", string_message::create(re.std_out));
    mess.insert("error", string_message::create(re.std_err));
    s->emit("Result", message::ptr(&mess));
    resetmt();
    if(!unlocked && !failed)
        _cv.wait(_ul);
    _ul.unlock();
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

void ServerSocket::on_error(const sio::message::ptr& message)
{
    ULOCK
    log("Socket Error", LVER);
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
    try
    {
        object_message msg = *(dynamic_pointer_cast<object_message>(mess));
        int id = msg["id"]->get_int();
        string l = msg["language"]->get_string();
        string exefile = msg["exefile"]->get_string();
        string srcfile = msg["srcfile"]->get_string();
        submission sub(id, l, exefile, srcfile);

        sub.setCode(msg["code"]->get_string());
        jobque.push(sub);
    }
    catch(exception ex)
    {
        log("Failed to receive job", LVFA);
        log(ex.what());
    }
}

void ServerSocket::_callback(const std::string& name, const sio::message::ptr& mess, bool need_ack, sio::message::list& ack_message)
{
    ULOCK
    unlocked = true;
    log("A message had been sucessfully sent to the server.", LVDE);
    _cv.notify_all();
}

