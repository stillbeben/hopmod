/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ANYVAR_MACRO_HPP
#define FUNGU_SCRIPT_ANYVAR_MACRO_HPP

// this class is currently defined in two places: in variable.hpp and in env_frame.hpp as a nested class.

#ifdef FUNGU_WITH_LUA
    #define DEFINE_ANY_VARIABLE_CLASS \
    class any_variable:public env::object \
    { \
    public: \
        any_variable():m_procedure(false){} \
    \
        void assign(const any & value) \
        {\
            m_any = value;\
            m_procedure = value.get_type() == typeid(env::object::shared_ptr);\
            if(!m_procedure && any_is_string(m_any))\
                m_any = const_string(m_any.to_string().copy());\
        }\
        \
        result_type apply(apply_arguments & args,env::frame * frame)\
        {\
            if(m_procedure)\
                return any_cast<env::object::shared_ptr>(m_any)->apply(args,frame);\
            else\
            {\
                assign(args.safe_front());\
                args.pop_front();\
                return m_any;\
            }\
            \
            return false;\
        }\
        int apply(lua_State * L) \
        {\
            if(m_procedure) \
                return any_cast<env::object::shared_ptr>(m_any)->apply(L);\
            else return luaL_error(L, "not a function"); \
        }\
        \
        result_type value() \
        { \
            if(m_procedure) \
                return any_cast<env::object::shared_ptr>(m_any)->value(); \
            return m_any; \
        }\
        const source_context * get_source_context()const \
        { \
            if(m_procedure) \
                return any_cast<env::object::shared_ptr>(m_any)->get_source_context(); \
            return NULL; \
        } \
        object * lookup_member(const_string id) \
        { \
            if(m_procedure) \
                return any_cast<shared_ptr>(m_any)->lookup_member(id); \
            else return NULL; \
        } \
        const any & get_any()const{return m_any;} \
    private:\
        any m_any;\
        bool m_procedure;\
    };
#else
    #define DEFINE_ANY_VARIABLE_CLASS \
    class any_variable:public env::object \
    { \
    public: \
        any_variable():m_procedure(false){} \
    \
        void assign(const any & value) \
        {\
            m_any = value;\
            m_procedure = value.get_type() == typeid(env::object::shared_ptr);\
            if(!m_procedure && any_is_string(m_any))\
                m_any = const_string(m_any.to_string().copy());\
        }\
        \
        result_type apply(apply_arguments & args,env::frame * frame)\
        {\
            if(m_procedure)\
                return any_cast<env::object::shared_ptr>(m_any)->apply(args,frame);\
            else\
            {\
                assign(args.safe_front());\
                args.pop_front();\
                return m_any;\
            }\
            \
            return false;\
        }\
        \
        result_type value() \
        { \
            if(m_procedure) \
                return any_cast<env::object::shared_ptr>(m_any)->value(); \
            return m_any; \
        }\
        const source_context * get_source_context()const \
        { \
            if(m_procedure) \
                return any_cast<env::object::shared_ptr>(m_any)->get_source_context(); \
            return NULL; \
        } \
        object * lookup_member(const_string id) \
        { \
            if(m_procedure) \
                return any_cast<shared_ptr>(m_any)->lookup_member(id); \
            else return NULL; \
        } \
        const any & get_any()const{return m_any;} \
    private:\
        any m_any;\
        bool m_procedure;\
    };
#endif

#endif
