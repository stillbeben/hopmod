// Original source code copied from engine/master.cpp found in Sauerbraten's source tree.

#include "cube.h"

#include "hopmod/script_pipe.hpp"
#include "hopmod/scripting.hpp"
#include "hopmod/masterserver_client.cpp"
#include "hopmod/string_var.hpp"
#include "hopmod/signals.hpp"

#include <enet/time.h>
#include <signal.h>
#include <iostream>

#include <fungu/script.hpp>
using namespace fungu;

#define INPUT_LIMIT 4096
#define OUTPUT_LIMIT (64*1024)
#define CLIENT_TIME (3*60*1000)
#define AUTH_TIME (60*1000)
#define AUTH_LIMIT 100
#define CLIENT_LIMIT 8192
#define DUP_LIMIT 16
#define KEEPALIVE_TIME (65*60*1000)
#define SERVER_LIMIT (10*1024)
#define DEFAULT_SERVER_PORT 28787
#define DELEGATE_CONCURRENT_REQUEST_LIMIT 65535

struct userinfo
{
    char *name;
    char *domain;
    void *pubkey;
};

hashtable<char *, userinfo> users;
hashtable<char *, hashtable<char *, userinfo *> > domains;

void adduser(const char *name, const char * domain, const char *pubkey)
{
    char * namecopy = newstring(name);
    userinfo &u = users[namecopy];
    u.name = namecopy;
    u.domain = NULL;
    u.pubkey = parsepubkey(pubkey);
    
    if(domain && domain[0])
    {
        u.domain = newstring(domain);
        domains[u.domain][namecopy] = &u;
    }
}

void deleteuser(const char * name,const char * domain)
{
    char * namecopy = newstring(name);
    char * domaincopy = newstring(domain);
    
    users.remove(namecopy);
    if(domain && domain[0]) domains[domaincopy].remove(namecopy);
}

void clearusers()
{
    enumerate(users, userinfo, u, { delete[] u.name; delete [] u.domain; freepubkey(u.pubkey); });
    users.clear();
    domains.clear();
}

struct client;

struct authreq
{
    enet_uint32 reqtime; 
    uint id;
    void *answer;
    bool delegated;
    int rootserver_id;
    client * c;
};

struct client
{
    ENetAddress address;
    ENetSocket socket;
    char input[INPUT_LIMIT];
    vector<char> output;
    int inputpos, outputpos;
    enet_uint32 connecttime, lastinput;
    int servport;
    vector<authreq> authreqs;

    client() : inputpos(0), outputpos(0), servport(-1) {}
};  
vector<client *> clients;

void outputf(client &c, const char *fmt, ...);

ENetSocket serversocket = ENET_SOCKET_NULL;

time_t starttime;
enet_uint32 servtime = 0;

string rootserver_hostname = "sauerbraten.org";
int rootserver_port = DEFAULT_SERVER_PORT;
void rootserver_input(masterserver_client & client, const char * reply, int argc, const char * const * argv);
masterserver_client rootserver(rootserver_input);
authreq * rootserver_id[DELEGATE_CONCURRENT_REQUEST_LIMIT];

bool use_rootserver(){return rootserver_hostname[0];}

void initialize_rootserver()
{
    authreq ** first = &rootserver_id[0];
    authreq ** last = &rootserver_id[DELEGATE_CONCURRENT_REQUEST_LIMIT-1];
    for(; first <= last; first++) *first = NULL;
}

int get_free_rootserver_id()
{
    authreq ** first = &rootserver_id[0];
    authreq ** last = &rootserver_id[DELEGATE_CONCURRENT_REQUEST_LIMIT-1];
    for(; first <= last; first++) if(!*first) return first - &rootserver_id[0]; 
    return -1;
}

void rootserver_input(masterserver_client & client, const char * reply, int argc, const char * const * argv)
{
    if(argc == 0) return;
    
    uint id = static_cast<uint>(atoi(argv[0]));
    bool finished_request = true;
    
    if(id > DELEGATE_CONCURRENT_REQUEST_LIMIT || !rootserver_id[id])
    {
        std::cerr<<"Root server replied with an invalid root-id ("<<id<<")"<<std::endl;
        return;
    }
    
    authreq * req = rootserver_id[id];
    
    if(strcmp(reply, "chalauth") == 0)
    {
        if(argc > 1) 
        {
            outputf(*req->c, "chalauth %u %s\n", req->id, argv[1]);
            finished_request = false;
            std::cout<<"Forwarded challenge to user (auth id "<<req->id<<")"<<std::endl;
        }
    }
    else if(strcmp(reply, "succauth") == 0) outputf(*req->c, "succauth %u\n", req->id);
    else if(strcmp(reply, "failauth") == 0) outputf(*req->c, "failauth %u\n", req->id);
    
    if(finished_request)
    {
        rootserver_id[id] = NULL;
        
        loopv(req->c->authreqs) 
            if(req->c->authreqs[i].id == req->id)
            {
                req->c->authreqs.remove(i--);
                break;
            }
    }
}

bool connect_to_rootserver()
{
    ENetAddress address;
    if(enet_address_set_host(&address, rootserver_hostname) < 0)
    {
        std::cerr<<"Could not resolve hostname "<<rootserver_hostname<<" for auth server connection."<<std::endl;
        signal_rootserver_failedconnect();
        return false;
    }
    
    address.port = rootserver_port;
    
    if(rootserver.connect(address) == false)
    {
        std::cerr<<"Unable to connect to auth server at "<<rootserver_hostname<<":"<<DEFAULT_SERVER_PORT<<std::endl;
        signal_rootserver_failedconnect();
        return false;
    }
    
    return true;
}

bool delegate_reqauth(client &c, uint id, char *name, char * domain)
{    
    bool using_domain = domain[0];
    
    if(!use_rootserver())
    {
        outputf(c, "failauth %u\n", id);
        return false;
    }
    
    if(rootserver.is_connected() == false && connect_to_rootserver() == false)
    {
        outputf(c, "failauth %u\n", id);
        return false;
    }
    
    int rsid = get_free_rootserver_id();
    if(rsid == -1)
    {
        std::cerr<<"Auth request failed for "<<name<<"@"<<(using_domain ? domain : "sauerbraten.org")<<" because there are too many concurrent requests to the root server."<<std::endl;
        outputf(c, "failauth %u\n", id);
        return false;
    }
    
    if(c.authreqs.length() >= AUTH_LIMIT)
    {
        outputf(c, "failauth %u\n", c.authreqs[0].id);
        freechallenge(c.authreqs[0].answer);
        c.authreqs.remove(0);
    }
    
    authreq &a = c.authreqs.add();
    a.reqtime = servtime;
    a.id = id;
    a.delegated = true;
    a.rootserver_id = rsid;
    a.c = &c;
    
    rootserver_id[rsid] = &a;
    
    char * reqargs[4];
    defformatstring(authid)("%i", rsid);
    reqargs[0] = authid;
    reqargs[1] = name;
    reqargs[2] = using_domain ? domain : NULL;
    reqargs[3] = NULL;
    
    rootserver.send_request("reqauth", reqargs);
    
    std::cout<<"Forwarded request to root server (auth-id "<<id<<" root-id "<<rsid<<")"<<std::endl;
    
    return true;
}

void fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char msg[256];
    vsprintf(msg, fmt, args);
    std::cerr<<msg<<std::endl;
    va_end(args);
    exit(EXIT_FAILURE);
}

void conoutfv(int type, const char *fmt, va_list args)
{
    char msg[256];
    vsprintf(msg, fmt, args);
    std::cout<<msg<<std::endl;
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args);
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}

void log_status(const char * msg){std::cout<<msg<<std::endl;}
void log_error(const char * msg){std::cout<<msg<<std::endl;}

void purgeclient(int n)
{
    client &c = *clients[n];
    enet_socket_destroy(c.socket);
    delete clients[n];
    clients.remove(n);
}

void output(client &c, const char *msg, int len = 0)
{
    if(!len) len = strlen(msg);
    c.output.put(msg, len);
}

void outputf(client &c, const char *fmt, ...)
{
    string msg;
    va_list args;
    va_start(args, fmt);
    vformatstring(msg, fmt, args);
    va_end(args);

    output(c, msg);
}

void setupserver(int port, const char *ip = NULL)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    if(ip)
    {
        if(enet_address_set_host(&address, ip)<0)
            fatal("failed to resolve server address: %s", ip);
    }
    serversocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    if(serversocket==ENET_SOCKET_NULL || 
       enet_socket_set_option(serversocket, ENET_SOCKOPT_REUSEADDR, 1) < 0 ||
       enet_socket_bind(serversocket, &address) < 0 ||
       enet_socket_listen(serversocket, -1) < 0)
        fatal("failed to create server socket");
    if(enet_socket_set_option(serversocket, ENET_SOCKOPT_NONBLOCK, 1)<0)
        fatal("failed to make server socket non-blocking");

    enet_time_set(0);
    
    starttime = time(NULL);
    char *ct = ctime(&starttime);
    if(strchr(ct, '\n')) *strchr(ct, '\n') = '\0';
    conoutf("*** Starting auth server on %s %d at %s ***", ip ? ip : "localhost", port, ct);
}

void purgeauths(client &c)
{
    int expired = 0;
    loopv(c.authreqs)
    {
        if(ENET_TIME_DIFFERENCE(servtime, c.authreqs[i].reqtime) >= AUTH_TIME) 
        {
            outputf(c, "failauth %u\n", c.authreqs[i].id);
            if(!c.authreqs[i].delegated) freechallenge(c.authreqs[i].answer);
            else rootserver_id[c.authreqs[i].rootserver_id] = NULL;
            expired = i + 1;
        }
        else break;
    }
    if(expired > 0) c.authreqs.remove(0, expired);
}

void reqauth(client &c, uint id, char *name, char * domain)
{
    purgeauths(c);
    
    time_t t = time(NULL);
    char *ct = ctime(&t);
    if(ct) 
    { 
        char *newline = strchr(ct, '\n');
        if(newline) *newline = '\0'; 
    }
    string ip;
    if(enet_address_get_host_ip(&c.address, ip, sizeof(ip)) < 0) copystring(ip, "-");
    conoutf("%s: attempting \"%s\" as %u from %s", ct ? ct : "-", name, id, ip);
    
    userinfo * u = NULL;
    bool using_domain = domain[0];
    
    if(using_domain)
    {
        hashtable<char *, userinfo *> * d = domains.access(domain);
        if(!d)
        {
            delegate_reqauth(c, id, name, domain);
            return;
        }
        
        u = *(d->access(name));
        if(!u)
        {
            delegate_reqauth(c, id, name, domain);
            return;
        }
    }
    else
    {
        u = users.access(name);
        if(!u)
        {
            delegate_reqauth(c, id, name, domain);
            return;
        }
    }
    
    if(c.authreqs.length() >= AUTH_LIMIT)
    {
        outputf(c, "failauth %u\n", c.authreqs[0].id);
        freechallenge(c.authreqs[0].answer);
        c.authreqs.remove(0);
    }
    
    authreq &a = c.authreqs.add();
    a.reqtime = servtime;
    a.id = id;
    a.delegated = false;
    uint seed[3] = { starttime, servtime, randomMT() };
    static vector<char> buf;
    buf.setsizenodelete(0);
    a.answer = genchallenge(u->pubkey, seed, sizeof(seed), buf);
    
    outputf(c, "chalauth %u %s\n", id, buf.getbuf());
}

void confauth(client &c, uint id, const char *val)
{
    purgeauths(c);

    loopv(c.authreqs) if(c.authreqs[i].id == id)
    {
        string ip;
        if(enet_address_get_host_ip(&c.address, ip, sizeof(ip)) < 0) copystring(ip, "-");
        
        if(c.authreqs[i].delegated)
        {
            if(rootserver.is_connected() == false && connect_to_rootserver() == false)
            {
                outputf(c, "failauth %u\n", id);
                return;
            }
            
            const char * reqargs[3];
            defformatstring(authid)("%i", c.authreqs[i].rootserver_id);
            reqargs[0] = authid;
            reqargs[1] = val;
            reqargs[2] = NULL;
            
            rootserver.send_request("confauth", reqargs);
        }
        else
        {
            if(checkchallenge(val, c.authreqs[i].answer))
            {
                outputf(c, "succauth %u\n", id);
                conoutf("succeeded %u from %s", id, ip);
            }
            else 
            {
                outputf(c, "failauth %u\n", id);
                conoutf("failed %u from %s", id, ip);
            }
            freechallenge(c.authreqs[i].answer);
            c.authreqs.remove(i--);
        }
        return;
    }
    outputf(c, "failauth %u\n", id);
}

bool checkclientinput(client &c)
{
    if(c.inputpos<0) return true;
    char *end = (char *)memchr(c.input, '\n', c.inputpos);
    while(end)
    {
        *end++ = '\0';
        c.lastinput = servtime;
        
        uint id;
        string user, domain, val;
        user[0]='\0';
        domain[0]='\0';
        val[0]='\0';
        
        if(sscanf(c.input, "reqauth %u %100s %100s", &id, user, domain) >= 2)
        {
            reqauth(c, id, user, domain);
        }
        else if(sscanf(c.input, "confauth %u %100s", &id, val) == 2)
        {
            confauth(c, id, val);
        }
        
        c.inputpos = &c.input[c.inputpos] - end;
        memmove(c.input, end, c.inputpos);

        end = (char *)memchr(c.input, '\n', c.inputpos);
    }
    return c.inputpos<(int)sizeof(c.input);
}

ENetSocketSet readset, writeset;

void checkclients()
{
    bool check_rootserver = rootserver.is_connected();
    
    ENetSocketSet readset, writeset;
    ENetSocket maxsock = serversocket;
    ENET_SOCKETSET_EMPTY(readset);
    ENET_SOCKETSET_EMPTY(writeset);
    ENET_SOCKETSET_ADD(readset, serversocket);
    
    if(check_rootserver)
    {
        maxsock = rootserver.get_socket_descriptor();
        ENET_SOCKETSET_ADD(readset, maxsock);
    }
    
    loopv(clients)
    {
        client &c = *clients[i];
        if(c.outputpos < c.output.length()) ENET_SOCKETSET_ADD(writeset, c.socket);
        else ENET_SOCKETSET_ADD(readset, c.socket);
        maxsock = max(maxsock, c.socket);
    }
    if(enet_socketset_select(maxsock, &readset, &writeset, 1000)<=0) return;

    if(ENET_SOCKETSET_CHECK(readset, serversocket))
    {
        ENetAddress address;
        ENetSocket clientsocket = enet_socket_accept(serversocket, &address);
        if(clients.length()>=CLIENT_LIMIT) enet_socket_destroy(clientsocket);
        else if(clientsocket!=ENET_SOCKET_NULL)
        {
            int dups = 0, oldest = -1;
            loopv(clients) if(clients[i]->address.host == address.host) 
            {
                dups++;
                if(oldest<0 || clients[i]->connecttime < clients[oldest]->connecttime) oldest = i;
            }
            if(dups >= DUP_LIMIT) purgeclient(oldest);
                
            client *c = new client;
            c->address = address;
            c->socket = clientsocket;
            c->connecttime = servtime;
            c->lastinput = servtime;
            clients.add(c);
        }
    }
    
    if(check_rootserver)
    {
        if(ENET_SOCKETSET_CHECK(readset, rootserver.get_socket_descriptor()))
            rootserver.flush_input();
    }
    
    loopv(clients)
    {
        client &c = *clients[i];
        if((c.outputpos < c.output.length()) && ENET_SOCKETSET_CHECK(writeset, c.socket))
        {
            const char *data = c.output.getbuf();
            int len = c.output.length();
            ENetBuffer buf;
            buf.data = (void *)&data[c.outputpos];
            buf.dataLength = len-c.outputpos;
            int res = enet_socket_send(c.socket, NULL, &buf, 1);
            if(res>=0) 
            {
                c.outputpos += res;
                if(c.outputpos>=len)
                {
                    c.output.setsizenodelete(0);
                    c.outputpos = 0;
                }
            }
            else { purgeclient(i--); continue; }
        }
        if(ENET_SOCKETSET_CHECK(readset, c.socket))
        {
            ENetBuffer buf;
            buf.data = &c.input[c.inputpos];
            buf.dataLength = sizeof(c.input) - c.inputpos;
            int res = enet_socket_receive(c.socket, NULL, &buf, 1);
            if(res>0)
            {
                c.inputpos += res;
                c.input[min(c.inputpos, (int)sizeof(c.input)-1)] = '\0';
                if(!checkclientinput(c)) { purgeclient(i--); continue; }
            }
            else { purgeclient(i--); continue; }
        }
        if(c.output.length() > OUTPUT_LIMIT) { purgeclient(i--); continue; }
        if(ENET_TIME_DIFFERENCE(servtime, c.lastinput) >= CLIENT_TIME) { purgeclient(i--); continue; }
    }
}

static void shutdown_from_signal(int i)
{
    signal_shutdown();
    exit(0);
}

static void _shutdown()
{
    shutdown_from_signal(SIGTERM);
}

int main(int argc, char **argv)
{
    struct sigaction terminate_action;
    sigemptyset(&terminate_action.sa_mask);
    terminate_action.sa_handler = shutdown_from_signal;
    terminate_action.sa_flags = 0;
    sigaction(SIGINT, &terminate_action, NULL);
    sigaction(SIGTERM, &terminate_action, NULL);
    
    const char * confname = "conf/authserver.conf";
    
    int port = DEFAULT_SERVER_PORT;
    string ip = "";
    
    init_scripting();
    
    script::env & e = get_script_env();
    
    script::bind_var(port, "serverport", e);
    script::bind_var(ip, "serverip", e);
    
    script::bind_var(rootserver_hostname, "rootserver", e);
    script::bind_var(rootserver_port, "rootserver_port", e);
    
    script::bind_freefunc(adduser, "adduser", e);
    script::bind_freefunc(deleteuser, "deleteuser", e);
    script::bind_freefunc(clearusers, "clearusers", e);
    
    script::bind_freefunc(log_status, "log", e);
    script::bind_freefunc(log_error, "logerror", e);
    
    script::bind_freefunc(_shutdown, "shutdown", e);
    
    register_signals(e);
    
    init_script_pipe();
    open_script_pipe("authexec", 511, e);
    
    try
    {
        fungu::script::execute_file(confname, e);
    }
    catch(fungu::script::error_trace * error)
    {
        report_script_error(error);
    }
    
    setupserver(port, (ip[0] ? ip : NULL));
    
    signal_started();
    
    std::cout<<"*READY*"<<std::endl;
    std::cout.flush();
    
    for(;;)
    {
        servtime = enet_time_get();
        checkclients();
        
        if(rootserver.is_connected() && rootserver.has_queued_output())
            rootserver.flush_output();
        
        run_script_pipe_service(servtime);
        
        cleanup_dead_slots();
    }
    
    return EXIT_SUCCESS;
}

