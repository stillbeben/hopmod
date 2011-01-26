#ifndef HOPMOD_LUA_EVENT_HPP
#define HOPMOD_LUA_EVENT_HPP

#include "push_function.hpp"
#include <cassert>
#include <iostream>

namespace lua{

class event_base
{
public:
    event_base(const char * text_id, int numeric_id = -1);
    const char * text_id()const;
    int numeric_id()const;
    static int assign_numeric_id();
private:
    const char * m_text_id;
    int m_numeric_id;
};

class event_environment
{
public:
    event_environment(lua_State *);
    void register_event_idents(event_base **);
    lua_State * push_listeners_table(const char * text_id, int num_id);
    void report_error(const char * text_id, const char * message);
private:
    lua_State * m_state;
    int m_text_id_index;
    int m_numeric_id_index;
};

template<typename Tuple>
class event:public event_base
{
public:
    event(const char * text_id, int numeric_id = -1)
     :event_base(text_id, numeric_id)
    {
        
    }
    
    bool operator()(event_environment & environment, const Tuple & args)
    {
        lua_State * L = environment.push_listeners_table(text_id(), numeric_id());
        assert(L && lua_type(L, -1) == LUA_TTABLE);
        
        bool prevent_default = false;
        
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            if(lua_type(L, -1) == LUA_TFUNCTION)
            {
                lua_pushvalue(L, -3);
                lua::push(L, args);
                if(lua_pcall(L, 1 + boost::tuples::length<Tuple>::value, 1, 0) == 0)
                    prevent_default = prevent_default || lua_toboolean(L, -1);   
                else
                    environment.report_error(text_id(), lua_tostring(L, -1));
            }
            lua_pop(L, 1);
        }
        return prevent_default;
    }
};

} //namespace lua

#endif

