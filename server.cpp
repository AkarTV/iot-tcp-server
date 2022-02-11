#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<algorithm>

#include "server.h"

//SERVER FUNCTIONS___________________________________

//Create the listening socket
Server* Server::start(EventSelector* sel, int port)
{
    int socket_fd, opt, res;
    struct sockaddr_in addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
        return 0;
    //set the socket option to reuse its address immediately after closing server (against port sticking) 
    opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    res = bind(socket_fd, (sockaddr*) &addr, sizeof(addr));
    if(res == -1)
        return 0;
    res = listen(socket_fd, max_q_len);
    if(res == -1)
        return 0;
    return new Server(sel, socket_fd);
}

Server::Server(EventSelector* sel, int fd) : FdHandler(fd), selector(sel)
{
    selector->add(this);
    sessions.reserve(expected_connections-1); //except server
}

Server::~Server()
{
    selector->remove(this);
}

//Accept connection if there is such a request and make new session socket
void Server::handle()
{
    int session_fd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    session_fd = accept(get_fd(), (sockaddr*) &addr, &len);
    if(session_fd == -1)
        return;
    Session* ses = new Session(this, session_fd);
    selector->add(ses); 
}

void Server::remove_session(Session* s)
{
    sessions.erase(std::remove(sessions.begin(), sessions.end(), s), sessions.end());
    selector->remove(s);
}

void Server::send_to(std::string& name, std::string& msg) const
{
    auto dest_iter = std::find_if(sessions.cbegin(), sessions.cend(), [&name](auto& x){return x->get_name() == name;});
    (*dest_iter)->send(msg.c_str());
}

void Server::send_all_clients(const char* msg) const
{
    for(auto& ses : sessions){
        if(!ses->is_device())
            ses->send(msg);
    }
}

std::string Server::get_devices_names() const
{
    std::string names;
    for(auto& ses : sessions){
        if(ses->is_device())
            names.append(ses->get_name());
        names.append(" ");
    }
    names.append("\n");
    return names;
}


//SESSION FUNCTIONS___________________________________________


void Session::send(const char* msg) const
{
    write(get_fd(), msg, strlen(msg));
}

Session::Session(Server* master_, int fd) : FdHandler(fd), master(master_) 
{
    send("Enter the password: ");
}

void Session::make_command(const char* c) const
{
    if(*c == 'a'){
        send(master->get_devices_names().c_str());
    }
}

void Session::send_to_device(const char* c) const
{
    std::string recipient;
    for(; *c!='>'; ++c)
        recipient.append({*c});
    ++c;
    std::string mes;
    for(; *c!=0; ++c)
        mes.append({*c});
    mes.append({'\n'});
    master->send_to(recipient, mes);
}

void Session::clear_buf()
{
    for(size_t i = 0; buffer[i]!=0; ++i)
        buffer[i]=0;
}

void Session::handle()
{
    //read from buffer
    int rc = read(get_fd(), buffer, buff_size);
    if(rc<1){
        if(name.substr(0, 3) == "/d/"){
            std::string left_mes = name + " has left.\n";      
            master->send_all_clients(left_mes.c_str());   
        }
        master->remove_session(this);
        return;
    }

    //remove '\n' and '\r' symbols in the buffer
    for(size_t i = 0; i < sizeof(buffer); ++i){
            if(buffer[i] == '\n'){
                buffer[i] = 0;
                if(i>0 && buffer[i-1] == '\r')
                    buffer[i-1] = 0;
                break;
            }
        }

    if(!connected){
        //chek the password
        if(strcmp(pass, buffer) == 0){
            connected = true;
            clear_buf();
            send("Enter your name (for devices enter \'/d/\' before name): ");
        }
        else{
            send("Incorrect password!");
            master->remove_session(this);
            return;
        }
    }
    else //device or client connected (password accepted)
    { 
        if(name.empty()){
            //fill the name field (first message from the new connection)
            name = buffer;
            master->add_session(this);
            if(name.substr(0, 3) == "/d/"){
                device = true;
                std::string new_device_mes = name + " registered.\n";
                master->send_all_clients(new_device_mes.c_str());
            }
            return;
        }

        if(!device) //client
        {
            if(buffer[0]=='<'){
                send_to_device(&buffer[1]);
            }
            if(buffer[0]=='/'){
                make_command(&buffer[1]);
            }
        }

        if(device){
            std::string mes = name + ": " + buffer + '\n';
            master->send_all_clients(mes.c_str());
        }
        
        clear_buf();
    }
}