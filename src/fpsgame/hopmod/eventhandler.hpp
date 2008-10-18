
#ifndef HOPMOD_EVENTHANDLER_HPP
#define HOPMOD_EVENTHANDLER_HPP

#include <cubescript.hpp>
#include <list>
#include <string>
#include <map>

class event_handler
{
public:
    void push_handler_code(const std::string &);
    void push_handler_code(const std::string &,const std::string &);
    void cancel_handler(const std::string &);
    void run(std::list<std::string> &,cubescript::domain *);
    void clear_handlers();
private:
    void cancel_handler(); 
    bool m_cancel_handler;
    typedef std::list<cubescript::alias> handler_list;
    handler_list m_handlers;
    std::map<std::string,handler_list::iterator> m_names;
};

enum{MaxEventSlots = 64};

class event_handler_service
{
public:
    event_handler_service(cubescript::domain *);
    void register_event(const char *,event_handler *);
    void dispatch(event_handler *,std::list<std::string> &,bool *);
    void clear_all_handlers();
private:
    void push_handler_code(int,const std::string &);
    void push_handler_code(int,const std::string &,const std::string &);
    void cancel_named_handler(int,const std::string &);
    cubescript::domain * m_domain;
    event_handler * m_handler[MaxEventSlots];
};

#endif
