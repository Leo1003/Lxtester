#include "server_socket.h"
using namespace std;
using namespace sio;

ServerSocket::ServerSocket(string proco, string host, short port, string token, string name) : lg(logger("Socket")) {
    stringstream ss;
    ss<<proco<<"://"<<host;
    if (port) {
        ss<<":"<<port;
    }
    this->addr = ss.str();
    this->token = token;
    this->lxtname = name;
    lg.log("Server address is: " + addr, LVIN);
    lg.log("Lxtester name is: " + name, LVIN);
    resetmt();
    stat = NotConnected;
    cli.set_reconnect_attempts(2);
    cli.set_reconnect_delay(2000);
    cli.set_reconnect_delay_max(2000);
    cli.set_open_listener(bind(&ServerSocket::on_connected, this));
    cli.set_fail_listener(bind(&ServerSocket::on_failed, this));
    cli.set_close_listener(bind(&ServerSocket::on_closed, this, placeholders::_1));
}

ServerSocket::~ServerSocket() {
    disconnect();
}

void ServerSocket::connect() {
    if (cli.opened())
        return;
    _connect();
    ULOCK
    resetmt();
    if (!unlocked)
        _cv.wait(_ul);
    _ul.unlock();
    if (!cli.opened())
        return;
    stat = Connected;
    s = cli.socket("lxtester");
    s->on_error(bind(&ServerSocket::on_error, this, placeholders::_1));
    s->on("Job", bind(&ServerSocket::_job, this, placeholders::_1));
    s->on("Cancel", bind(&ServerSocket::_cancel, this, placeholders::_1));
    shared_ptr<object_message> m = static_pointer_cast<object_message>(object_message::create());
    m->insert("name", string_message::create(lxtname));
    sleep(1);
    s->emit("Name", static_pointer_cast<message>(m));
}

void ServerSocket::_connect() {
    map<string,string> query;
    query["passtoken"] = token;
    cli.connect(addr, map<string,string>(), query);
}

void ServerSocket::disconnect() {
    if (!cli.opened())
        return;
    ULOCK
    lg.log("Disconnecting from server...", LVIN);
    cli.close();
    resetmt();
    if (!unlocked)
        _cv.wait(_ul);
    _ul.unlock();
}

void ServerSocket::suspend() {
    ULOCK
    s->emit("Suspend");
}

void ServerSocket::resume() {
    ULOCK
    s->emit("Resume");
}

ConnectionStatus ServerSocket::getStatus() const {
    if (stat == Connected) {
        if (cli.opened())
            return Connected;
        else
            return NotConnected;
    } else
        return stat;
}

Job ServerSocket::getJob() {
    ULOCK
    Job j;
    if (jobque.empty()) {
        j.type = None;
        return j;
    }
    j = jobque.front();
    jobque.pop_front();
    return j;
}

JobType ServerSocket::getJobType() const {
    return (jobque.empty() ? None : jobque.front().type);
}

size_t ServerSocket::countJob() const {
    return jobque.size();
}

shared_ptr<message> ServerSocket::generateMessage(const int id, const result r)
{
    lg.log("Creating object message", LVD2);
    shared_ptr<object_message> m = static_pointer_cast<object_message>(object_message::create());
    m->insert("id", int_message::create(id));
    m->insert("type", int_message::create(r.type));
    m->insert("time", int_message::create(r.time));
    m->insert("memory", int_message::create(r.mem));
    m->insert("exitcode", int_message::create(r.exitcode));
    m->insert("signal", int_message::create(r.signal));
    m->insert("killed", bool_message::create(r.isKilled));
    m->insert("output", string_message::create(r.std_out));
    m->insert("error", string_message::create(r.std_err));
    lg.log("Inserted result message", LVD2);
    return static_pointer_cast<message>(m);
}

void ServerSocket::sendResult(const submission& sub) {
    ULOCK
    lg.log("Emitting...", LVD2);
    s->emit("Result", generateMessage(sub.getId(), sub.getResult()));
    lg.log("Emitted.", LVD2);
}

void ServerSocket::sendFailed(int id, std::string msg)
{
    result r(msg);
    lg.log("Emitting...", LVD2);
    s->emit("Result", generateMessage(id, r));
    lg.log("Emitted.", LVD2);
}

inline void ServerSocket::resetmt() {
    unlocked = false;
}

void ServerSocket::on_connected() {
    ULOCK
    unlocked = true;
    stat = Connected;
    lg.log("Connected to server.", LVIN);
    _cv.notify_all();
}

void ServerSocket::on_failed() {
    lg.log("on_failed", LVD2);
    ULOCK
    stat = Failed;
     unlocked = true;
    _cv.notify_all();
}

void ServerSocket::on_error(const sio::message::ptr& message) {
    lg.log("on_error", LVD2);
    ULOCK
    unlocked = true;
    stat = Errored;
    lg.log(message->get_string(), LVFA);
    lg.log("An error has occurred when connecting to server.");
    _cv.notify_all();
}

void ServerSocket::on_closed(client::close_reason const& reason) {
    ULOCK
    if (reason != client::close_reason_normal) {
        stat = Failed;
        lg.log("Abnormal Disconnected.", LVWA);
    } else {
        stat = Disconnected;
        lg.log("Disconnected.", LVIN);
    }
    unlocked = true;
    _cv.notify_all();
}

void ServerSocket::_job(sio::event& event) {
    ULOCK
    map<string, message::ptr> msg;
    try {
        msg = event.get_message()->get_map();
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
        j.sub = sub;
        jobque.push_back(j);
    } catch (out_of_range ex) {
        lg.log("Server sent a bad submission format!", LVWA);
    } catch (unsupport_language ex) {
        sendFailed(msg.at("id")->get_int(), "Unsupported language");
    } catch (exception ex) {
        lg.log("Failed to receive job", LVFA);
        lg.log(ex.what());
    }
}

void ServerSocket::_cancel(sio::event& event) {
    ULOCK
    try {
        map<string, message::ptr> msg = event.get_message()->get_map();
        Job j;
        j.type = Cancel;
        j.submissionid = msg.at("id")->get_int();
        jobque.push_front(j);
    } catch (out_of_range ex) {
        lg.log("Server sent a bad request format!", LVWA);
        lg.log(ex.what());
    } catch (exception ex) {
        lg.log("Failed to receive job", LVFA);
        lg.log(ex.what());
    }
}
