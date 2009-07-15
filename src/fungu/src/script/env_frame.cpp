/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

env::frame::frame(env * envir)
 :m_env(envir),
  m_scope(this),
  m_last_binding(NULL),
  m_expired(false),
  m_detached(false)
{
    
}

env::frame::frame(env::frame * outer_frame)
 :m_env(outer_frame->m_env),
  m_scope(outer_frame->m_scope),
  m_last_binding(NULL),
  m_expired(false),
  m_detached(false)
{

}

env::frame::~frame()
{
    delete m_last_binding;
}

void env::frame::bind_object(env::object * obj, object_id id)
{
    return bind_object(obj, m_env->create_symbol(id));
}

void env::frame::bind_object(env::object * obj, env::symbol * sym)
{
    symbol_local * local = sym->push_local_object(obj, this);
    m_last_binding = local;
}

env::object * env::frame::lookup_object(object_id id)const
{
    const symbol * sym = m_env->lookup_symbol(id);
    if(!sym) return NULL;
    return lookup_object(sym);
}

env::object * env::frame::lookup_object(const env::symbol * sym)const
{
    return sym->lookup_object(this);
}

env::object * env::frame::lookup_required_object(object_id id)const
{
    object * obj = lookup_object(id);
    if(!obj) throw error(UNKNOWN_SYMBOL, boost::make_tuple(id.copy()));
    return obj;
}

env::object * env::frame::lookup_required_object(const env::symbol * sym)const
{
    object * obj = lookup_object(sym);
    if(!obj) throw error(UNKNOWN_SYMBOL); //FIXME add id tuple
    return obj;
}

bool env::frame::has_expired()const
{
    return m_scope->m_expired;
}

void env::frame::signal_return()
{
    m_scope->m_expired = true;
}

void env::frame::signal_return(result_type value)
{
    m_scope->m_result = value;
    m_scope->m_expired = true;
}

void env::frame::unset_return()
{
    m_scope->m_result.reset();
    m_scope->m_expired = false;
}

result_type env::frame::get_result_value()
{
    return m_scope->m_result;
}

env * env::frame::get_env()const
{
    return m_env;
}

env::frame * env::frame::get_scope_frame()const
{
    return m_scope;
}

void env::frame::register_functions(env & e)
{
    static va_memfun return_scriptfun(&frame::signal_return_scriptfun);
    e.bind_global_object(&return_scriptfun,FUNGU_OBJECT_ID("return"));

    static va_memfun result_scriptfun(&frame::set_result_scriptfun);
    e.bind_global_object(&result_scriptfun,FUNGU_OBJECT_ID("result"));

    static va_memfun bind_myvar_scriptfun_obj(&frame::bind_myvar_scriptfun);
    e.bind_global_object(&bind_myvar_scriptfun_obj,FUNGU_OBJECT_ID("my"));
    e.bind_global_object(&bind_myvar_scriptfun_obj,FUNGU_OBJECT_ID("local"));

    static va_memfun bind_globalvar_scriptfun_obj(&frame::bind_globalvar_scriptfun);
    e.bind_global_object(&bind_globalvar_scriptfun_obj,FUNGU_OBJECT_ID("global"));

    static va_memfun getvar_scriptfun_obj(&frame::getvar_scriptfun);
    e.bind_global_object(&getvar_scriptfun_obj,FUNGU_OBJECT_ID("get"));

    static va_memfun setvar_scriptfun_obj(&frame::setvar_scriptfun);
    e.bind_global_object(&setvar_scriptfun_obj,FUNGU_OBJECT_ID("set"));

    static va_memfun issymbol_scriptfun_obj(&frame::issymbol_scriptfun);
    e.bind_global_object(&issymbol_scriptfun_obj,FUNGU_OBJECT_ID("symbol?"));

    static va_memfun isnull_scriptfun_obj(&frame::isnull_scriptfun);
    e.bind_global_object(&isnull_scriptfun_obj,FUNGU_OBJECT_ID("null?"));

    static va_memfun retnull_scriptfun_obj(&frame::retnull_scriptfun);
    e.bind_global_object(&retnull_scriptfun_obj,FUNGU_OBJECT_ID("null"));

    static va_memfun isprocedure_scriptfun_obj(&frame::isprocedure_scriptfun);
    e.bind_global_object(&isprocedure_scriptfun_obj,FUNGU_OBJECT_ID("procedure?"));
}

void env::frame::attach_locals()
{
    assert(m_detached);
    if(!m_detached) return;

    for(symbol_local * local = m_last_binding; local; 
        local = local->get_next_frame_sibling() ) local->attach();
    
    m_detached = false;
}

void env::frame::detach_locals()
{
    assert(!m_detached);
    if(m_detached) return;
    
    for(symbol_local * local = m_last_binding; local; 
        local = local->get_next_frame_sibling() ) local->detach();
    
    m_detached = true;
}

bool env::frame::is_detached_from_env()const
{
    return m_detached;
}

env::symbol_local * env::frame::get_last_bind()const
{
    return m_last_binding;
}

env::frame::frame(const env::frame &)
{
    assert(false);
}

result_type env::frame::signal_return_scriptfun(env::object::call_arguments & args)
{
    if(!args.empty())
    {
        signal_return(args.front());
        args.pop_front();
    }
    else signal_return();
    return any::null_value();
}

result_type env::frame::set_result_scriptfun(env::object::call_arguments & args)
{
    m_scope->m_result = args.safe_front();
    args.pop_front();
    return m_result;
}

result_type env::frame::bind_myvar_scriptfun(env::object::call_arguments & args)
{
    return bind_var_scriptfun(args,1);
}

result_type env::frame::bind_localvar_scriptfun(env::object::call_arguments & args)
{
    return bind_var_scriptfun(args,2);
}

result_type env::frame::bind_globalvar_scriptfun(env::object::call_arguments & args)
{
    return bind_var_scriptfun(args,3);
}

result_type env::frame::bind_var_scriptfun(env::object::call_arguments & args,int scope)
{
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    const_string id = args.casted_front<const_string>();
    args.pop_front();

    any_variable * var = new any_variable;
    var->set_adopted();
    
    switch(scope)
    {
        case 1: //my
            bind_object(var, id.copy());
            break;
        case 3: //global
            m_env->bind_global_object(var, id.copy());
            break;
    }

    if(!args.empty())
    {
        var->assign(args.front());
        args.pop_front();
    }

    return var->value();
}

result_type env::frame::getvar_scriptfun(env::object::call_arguments & args)
{
    any subject = args.safe_front();
    args.pop_front();

    object * obj;
    if(subject.get_type() == typeid(env::object::shared_ptr))
        obj = any_cast<env::object::shared_ptr>(subject).get();
    else obj = lookup_required_object(subject.to_string());

    return obj->value();
}

result_type env::frame::setvar_scriptfun(env::object::call_arguments & args)
{
    any subject = args.safe_front();
    args.pop_front();

    object * obj;
    if(subject.get_type() == typeid(env::object::shared_ptr))
        obj = any_cast<env::object::shared_ptr>(subject).get();
    else obj = lookup_required_object(subject.to_string());

    obj->assign(args.safe_front());
    args.pop_front();

    return obj->value();
}

result_type env::frame::issymbol_scriptfun(env::object::call_arguments & args)
{
    const_string id = args.safe_casted_front<const_string>();
    args.pop_front();
    return (bool)lookup_object(id);
}

result_type env::frame::isnull_scriptfun(env::object::call_arguments & args)
{
    any value = args.safe_front();
    args.pop_front();
    return value.empty();
}

result_type env::frame::retnull_scriptfun(env::object::call_arguments & args)
{
    return any();
}

result_type env::frame::isprocedure_scriptfun(env::object::call_arguments & args)
{
    bool isProcedure = args.safe_front().get_type() == typeid(env::object::shared_ptr);
    args.pop_front();
    return isProcedure;
}

} //namespace script
} //namespace fungu
