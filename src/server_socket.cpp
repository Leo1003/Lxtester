#include "server_socket.h"
using namespace std;
using namespace sio;

ServerSocket::ServerSocket(string host, short port, string token)
{
    stringstream ss;
    ss<<"wss://"<<host<<":"<<port;
    this->addr = ss.str();
    this->token = token;
    log("Server address is: " + addr, LVDE);
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
    cli.connect(addr, map<string,string>(), query);
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
    log("Creating object message", LVDE);
    shared_ptr<object_message> mess = static_pointer_cast<object_message>(object_message::create());
    log("Converted object message", LVDE);
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
    log("Inserted result message", LVDE);
    log("Emitting...", LVDE);
    s->emit("Result", static_pointer_cast<message>(mess));
    log("Emitted.", LVDE);
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
    log("Failed to connect to server.", LVER);
    //TODO:Lost connection event occured here!
    _cv.notify_all();
}

void ServerSocket::on_error(const sio::message::ptr& message)
{
    ULOCK
    failed = true;
    log("Authentication error", LVFA);
    log("Can't connect to server.");
    //TODO:Authentication error event occured here!
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

