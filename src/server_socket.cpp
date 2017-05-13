#include "server_socket.h"
using namespace std;
using namespace sio;

ServerSocket::ServerSocket(string host, short port, string token) : lg(logger("Socket"))
{
    stringstream ss;
    ss<<"wss://"<<host<<":"<<port;
    this->addr = ss.str();
    this->token = token;
    lg.log("Server address is: " + addr, LVIN);
    resetmt();
    stat = NotConnected;
    cli.set_reconnect_attempts(2);
    cli.set_reconnect_delay(2000);
    cli.set_reconnect_delay_max(2000);
    cli.set_open_listener(bind(&ServerSocket::on_connected, this));
    cli.set_fail_listener(bind(&ServerSocket::on_failed, this));
    cli.set_close_listener(bind(&ServerSocket::on_closed, this, placeholders::_1));
}

ServerSocket::~ServerSocket()
{
    disconnect();
}

void ServerSocket::connect()
{
    if(cli.opened())
        return;
    _connect();
    ULOCK
    resetmt();
    if(!unlocked)
        _cv.wait(_ul);
    _ul.unlock();
    if(!cli.opened())
        return;
    stat = Connected;
    s = cli.socket("lxtester");
    s->on_error(bind(&ServerSocket::on_error, this, placeholders::_1));
    s->on("Job", bind(&ServerSocket::_job, this, placeholders::_1));
    s->on("Cancel", bind(&ServerSocket::_cancel, this, placeholders::_1));
}

void ServerSocket::_connect()
{
    map<string,string> query;
    query["passtoken"] = token;
    cli.connect(addr, map<string,string>(), query);
}

void ServerSocket::disconnect()
{
    if(!cli.opened())
        return;
    ULOCK
    lg.log("Disconnecting from server...", LVIN);
    cli.close();
    resetmt();
    if(!unlocked)
        _cv.wait(_ul);
    _ul.unlock();
}

ConnectionStatus ServerSocket::getStatus() const
{
    if(stat == Connected)
    {
        if(cli.opened())
            return Connected;
        else
            return NotConnected;
    }
    else
        return stat;
}

Job ServerSocket::getJob()
{
    ULOCK
    Job j;
    if(jobque.empty())
    {
        j.type = None;
        return j;
    }
    j = move(jobque.front());
    jobque.pop();
    return j;
}

void ServerSocket::sendResult(const submission& sub)
{
    ULOCK
    lg.log("Creating object message", LVD2);
    shared_ptr<object_message> mess = static_pointer_cast<object_message>(object_message::create());
    lg.log("Converted object message", LVD2);
    mess->insert("id", int_message::create(sub.getId()));
    result re = sub.getResult();
    mess->insert("type", int_message::create(re.type));
    mess->insert("time", int_message::create(re.time));
    mess->insert("memory", int_message::create(re.mem));
    mess->insert("exitcode", int_message::create(re.exitcode));
    mess->insert("signal", int_message::create(re.signal));
    mess->insert("killed", bool_message::create(re.isKilled));
    mess->insert("output", string_message::create(re.std_out));
    mess->insert("error", string_message::create(re.std_err));
    lg.log("Inserted result message", LVD2);
    lg.log("Emitting...", LVD2);
    s->emit("Result", static_pointer_cast<message>(mess));
    lg.log("Emitted.", LVD2);
}


inline void ServerSocket::resetmt()
{
    unlocked = false;
}

void ServerSocket::on_connected()
{
    ULOCK
    unlocked = true;
    stat = Connected;
    lg.log("Connected to server.", LVIN);
    _cv.notify_all();
}

void ServerSocket::on_failed()
{
    lg.log("on_failed", LVD2);
    ULOCK
    stat = Failed;
     unlocked = true;
    _cv.notify_all();
}

void ServerSocket::on_error(const sio::message::ptr& message)
{
    lg.log("on_error", LVD2);
    ULOCK
    unlocked = true;
    stat = Errored;
    lg.log(message->get_string(), LVFA);
    lg.log("An error has occurred when connecting to server.");
    _cv.notify_all();
}

void ServerSocket::on_closed(client::close_reason const& reason)
{
    ULOCK
    if(reason != client::close_reason_normal)
    {
        stat = Failed;
        lg.log("Abnormal Disconnected.", LVWA);
    }
    else
    {
        stat = Disconnected;
        lg.log("Disconnected.", LVIN);
    }
    unlocked = true;
    _cv.notify_all();
}

void ServerSocket::_job(sio::event& event)
{
    ULOCK
    try
    {
        map<string, message::ptr> msg = event.get_message()->get_map();
        int id = msg.at("id")->get_int();
        string l = msg.at("language")->get_string();
        string exefile = msg.at("exefile")->get_string();
        string srcfile = msg.at("srcfile")->get_string();
        submission sub(id, l, exefile, srcfile);

        sub.setCode(msg.at("code")->get_string());
        sub.setStdin(msg.at("stdin")->get_string());
        Job j;
        j.type = Submission;
        j.submissionid = sub.getId();
        j.sub = move(sub);
        jobque.push(move(j));
    }
    catch(out_of_range ex)
    {
        lg.log("Server sent a bad submission format!", LVWA);
        lg.log(ex.what());
    }
    catch(exception ex)
    {
        lg.log("Failed to receive job", LVFA);
        lg.log(ex.what());
    }
}

void ServerSocket::_cancel(sio::event& event)
{
    ULOCK
    try
    {
        map<string, message::ptr> msg = event.get_message()->get_map();
        Job j;
        j.type = Cancel;
        j.submissionid = msg.at("id")->get_int();
    }
    catch(out_of_range ex)
    {
        lg.log("Server sent a bad request format!", LVWA);
        lg.log(ex.what());
    }
    catch(exception ex)
    {
        lg.log("Failed to receive job", LVFA);
        lg.log(ex.what());
    }
}
