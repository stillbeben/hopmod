
#include "eventhandler.hpp"
#include "tools.hpp"
#include <algorithm>
#include <boost/bind.hpp>
#include <iostream>

void event_handler::push_handler_code(const std::string & code)
{
    m_handlers.push_back(cubescript::alias());
    m_handlers.back().push(code);
}

void event_handler::push_handler_code(const std::string & code,const std::string & name)
{
    push_handler_code(code);
    m_names[name]=--m_handlers.end();
}

void event_handler::cancel_handler(const std::string & name)
{
    std::map<std::string,handler_list::iterator>::iterator it=m_names.find(name);
    if(it==m_names.end())return;
    m_handlers.erase(it->second);
    m_names.erase(it);
}

void event_handler::run(std::list<std::string> & args,cubescript::domain * parent_domain)
{
    cubescript::domain local(parent_domain);
    
    cubescript::function0<cubescript::void_t> func_cancel_handler(boost::bind(&event_handler::cancel_handler,this));
    local.register_symbol("cancel_handler",&func_cancel_handler);
    
    int max=m_handlers.size();
    int count=0;
    
    for(std::list<cubescript::alias>::iterator it=m_handlers.begin(); count<max && it!=m_handlers.end(); ++it,++count)
    {
        m_cancel_handler=false;
        
        std::list<std::string> args_copy(args);
        
        it->apply(args_copy,&local);
        
        if(m_cancel_handler)
        {
            std::list<cubescript::alias>::iterator canIt=it++;
            m_handlers.erase(canIt);
        }
    }
}

void event_handler::clear_handlers()
{
    m_handlers.clear();
}

cubescript::void_t event_handler::cancel_handler()
{
    m_cancel_handler=true;
    return cubescript::void_t();
}

event_handler_service::event_handler_service(cubescript::domain * aDomain)
 :m_domain(aDomain)
{
    std::for_each(m_handler,m_handler+MaxEventSlots,assign<event_handler *>(NULL));
    
    cubescript::symbol * func_event_handler=new cubescript::function2<_void,int,const std::string &>(boost::bind(&event_handler_service::push_handler_code,this,_1,_2));
    m_domain->register_symbol("event_handler",func_event_handler,cubescript::domain::ADOPT_SYMBOL);
    
    cubescript::symbol * func_named_event_handler=new cubescript::function3<_void,int,const std::string &,const std::string &>(boost::bind(&event_handler_service::push_handler_code,this,_1,_2,_3));
    m_domain->register_symbol("named_event_handler",func_named_event_handler,cubescript::domain::ADOPT_SYMBOL);

    cubescript::symbol * func_cancel_named_handler=new cubescript::function2<_void,int,const std::string &>(boost::bind(&event_handler_service::cancel_named_handler,this,_1,_2));
    m_domain->register_symbol("cancel_named_handler",func_cancel_named_handler,cubescript::domain::ADOPT_SYMBOL);
}

void event_handler_service::register_event(const char * name,event_handler * handler)
{
    int id=0;
    for(;id<MaxEventSlots && m_handler[id]; id++);
    assert(id < MaxEventSlots);
    m_handler[id]=handler;
    cubescript::symbol * idsym=new cubescript::constant<int>(id);
    //this will cause trouble if symbol is used after this object has been destroyed
    m_domain->register_symbol(name,idsym,cubescript::domain::ADOPT_SYMBOL);
}

void event_handler_service::dispatch(event_handler * handler,std::list<std::string> & args,bool * veto)
{
    cubescript::domain local(m_domain,cubescript::domain::TEMPORARY_DOMAIN);
    
    bool tmp;
    bool * veto_ptr=veto;
    if(!veto_ptr) veto_ptr=&tmp;
    
    cubescript::variable_ref<bool> var_veto(*veto_ptr);
    local.register_symbol("veto",&var_veto);
    
    try
    {
        handler->run(args,&local);
    }
    catch(const cubescript::script_error<cubescript::symbol_error> & e)
        {std::cerr<<"error in script file "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<": "<<e.get_id()<<std::endl;}
    catch(const cubescript::script_error<cubescript::error_key> & e)
        {std::cerr<<"error in script file "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<" in expression \""<<e.get_expr()<<"\""<<std::endl;}
    catch(const cubescript::expr_error<cubescript::error_key> & e)
        {std::cerr<<"cubescript error: "<<e.what()<<" in expression \""<<e.get_expr()<<"\"."<<std::endl;}
    catch(const cubescript::symbol_error & e)
        {std::cerr<<"cubescript error: "<<e.what()<<": "<<e.get_id()<<std::endl;}
    catch(const cubescript::error_key & e)
        {std::cerr<<"cubescript error: "<<e.what()<<std::endl;}
}

void event_handler_service::clear_all_handlers()
{
    for(int i=0; i < MaxEventSlots; ++i)
        if(m_handler[i]) m_handler[i]->clear_handlers();
}

_void event_handler_service::push_handler_code(int id,const std::string & code)
{
    event_handler * handler=m_handler[id];
    if(!handler) throw cubescript::error_key("runtime.function.event_handler.bad_event_id");
    handler->push_handler_code(code);
    return _void();
}

_void event_handler_service::push_handler_code(int id,const std::string & code,const std::string & name)
{
    event_handler * handler=m_handler[id];
    if(!handler) throw cubescript::error_key("runtime.function.event_handler.bad_event_id");
    handler->push_handler_code(code,name);
    return _void();
}

_void event_handler_service::cancel_named_handler(int id,const std::string & name)
{
    event_handler * handler=m_handler[id];
    if(!handler) throw cubescript::error_key("runtime.function.event_handler.bad_event_id");
    handler->cancel_handler(name);
    return _void();
}
