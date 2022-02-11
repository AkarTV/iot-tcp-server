#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "event_selector.h"

const int max_q_len = 16; 
const int buff_size = 200;
const char pass[] = "1234";

class Session;

class Server : public FdHandler {
    public:
    static Server* start(EventSelector* sel, int port);
    void remove_session(Session* s);
    void send_to(std::string& name, std::string& msg) const;
    void send_all_clients(const char* msg) const;
    std::string get_devices_names() const;
    void add_session(Session* ses) { sessions.push_back(ses); }
    ~Server();

    private:
    Server(EventSelector* sel, int fd);
    void handle();
    EventSelector* selector;
    std::vector<Session*> sessions;
};

class Session : public FdHandler {
    public:
    Session(Server* master_, int fd);
    void send(const char* msg) const;
    void handle();
    bool is_device() const { return device; }
    std::string get_name() const { return name; }
    void clear_buf();
    void make_command(const char* c) const;
    void send_to_device(const char* c) const;

    private:
    Server* master;
    std::string name = "";
    char buffer[buff_size];
    bool connected = false;
    bool device = false;
};
#endif