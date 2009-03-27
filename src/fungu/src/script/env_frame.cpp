/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"
#include "fungu/script/any_variable.hpp"

namespace fungu{
namespace script{

env::frame::frame_symbol::frame_symbol(frame_symbol ** table_entry,frame * aFrame)
 :m_table_entry(table_entry), 
  m_prev_frame_binding(NULL),
  m_next_sibling(NULL),
  m_frame(aFrame)
{
    assert(m_table_entry);
    if(m_frame) m_next_sibling = m_frame->get_last_bind();
}

env::frame::frame_symbol::~frame_symbol()
{
    if(*m_table_entry == this) detach_from_env();
    delete m_next_sibling;
}

env::frame::frame_symbol & env::frame::frame_symbol::attach_to_env()
{
    m_prev_frame_binding = *m_table_entry;
    inherit_dynamic_chain(m_prev_frame_binding);
    *m_table_entry = this;
    return *this;
}

env::frame::frame_symbol * env::frame::frame_symbol::attach_global_to_env()
{
    assert(m_frame == m_frame->get_env()->get_global_scope());
    
    set_dynamically_scoped();
    frame_symbol * binding = *m_table_entry;
    if(!binding)
    {
        *m_table_entry = this;
        return this;
    }
    while(binding)
    {
        binding->inherit_dynamic_chain(this);
        
        if(!binding->m_prev_frame_binding)
        {
            if(binding->get_frame() == m_frame) return binding;
            else binding->m_prev_frame_binding = this;
        }
        
        binding = binding->m_prev_frame_binding;
    }
    return this;
}

env::frame::frame_symbol & env::frame::frame_symbol::detach_from_env()
{
    assert(*m_table_entry == this); //failure probably caused by out of order binding
    *m_table_entry = m_prev_frame_binding;
    m_prev_frame_binding = NULL;
    return *this;
}

env::frame::frame_symbol & env::frame::frame_symbol::detach_from_frame()
{
    assert(m_frame->m_last_binding == m_next_sibling);
    m_next_sibling = NULL;
    return *this;
}

env::frame * env::frame::frame_symbol::get_frame()const
{
    return m_frame;
}

env::frame::frame_symbol * env::frame::frame_symbol::get_next_sibling()const
{
    return m_next_sibling;
}

env::frame::frame_symbol * env::frame::frame_symbol::get_prev_frame_binding()const
{
    return m_prev_frame_binding;
}

env::frame::frame_symbol::frame_symbol(const frame_symbol &)
{
    
}

env::frame::frame(env * envir)
 :m_env(envir),
  m_outer_frame(this),
  m_last_binding(NULL),
  m_expired(false),
  m_closure_refcount(0),
  m_detached(false)
{
    
}

env::frame::frame(env::frame * outer_frame)
 :m_env(outer_frame->m_env),
  m_outer_frame(outer_frame->m_outer_frame),
  m_last_binding(NULL),
  m_expired(false),
  m_closure_refcount(0),
  m_detached(false)
{

}

env::frame::~frame()
{
    delete m_last_binding;
}

env::symbol & env::frame::bind_object(env::object * obj, object_identifier id)
{
    return bind_object(obj,env::register_symbol(id));
}

env::symbol & env::frame::bind_object(env::object * obj, frame_symbol ** table_entry)
{
    frame_symbol * new_symbol;
    
    if(*table_entry && (*table_entry)->get_frame()==this)
        new_symbol = *table_entry;
    else
    {
        new_symbol = new frame_symbol(table_entry, this);
        new_symbol->attach_to_env();
        m_last_binding = new_symbol;
    }
    
    //if bind_object ever begins to throw more than error(NO_BIND) then
    //this should probably be put inside a try and catch block to delete
    //new_symbol allocation.
    new_symbol->bind_object(obj);

    return *new_symbol;
}

env::symbol & env::frame::bind_global_object(env::object * obj, object_identifier id)
{
    if(m_env->get_global_scope() != this) 
        return m_env->get_global_scope()->bind_global_object(obj,id);

    frame_symbol * new_symbol = new frame_symbol(env::register_symbol(id), this);
    frame_symbol * existing_symbol = new_symbol->attach_global_to_env();

    if(existing_symbol != new_symbol)
    {
        new_symbol->detach_from_frame();
        delete new_symbol;
    }
    else m_last_binding = new_symbol;

    existing_symbol->bind_object(obj);

    notify_bind_event(id,obj);

    return *existing_symbol;
}

env::object * env::frame::lookup_object(object_identifier id)const
{
    frame_symbol ** entry = env::lookup_symbol(id);
    if(!entry || !(*entry)) return NULL;
    return lookup_object(entry);
}

env::object * env::frame::lookup_object(const env::frame::frame_symbol * const * handle)const
{
    const frame_symbol * entry = *handle;
    if(!entry) return NULL;
    frame * entry_frame = entry->get_frame();
    if(entry_frame)
    {
        if(entry_frame->m_outer_frame != m_outer_frame)
        {
            const frame_symbol * valid = NULL;
            while(entry && entry->in_dynamic_scope_chain())
            {
                if((entry->get_frame()==NULL /*universal binding*/ ||
                    entry->get_frame()->m_env == m_env)
                    && entry->is_dynamically_scoped())
                {
                    valid = entry;
                    break;
                }
                entry = entry->get_prev_frame_binding();
            }
            if(valid) entry = valid;
            else return NULL;
        }
    }
    return entry->get_object();
}

env::object * env::frame::lookup_required_object(object_identifier id)const
{
    object * obj = lookup_object(id);
    if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(id.copy()));
    return obj;
}

env::object * env::frame::lookup_required_object(const frame_symbol * const * handle)const
{
    object * obj = lookup_object(handle);
    if(!obj) throw error(UNKNOWN_SYMBOL); //FIXME add id tuple
    return obj;
}

bool env::frame::has_expired()const
{
    return m_outer_frame->m_expired;
}

void env::frame::signal_return()
{
    m_outer_frame->m_expired = true;
}

void env::frame::signal_return(result_type value)
{
    m_outer_frame->m_result = value;
    m_outer_frame->m_expired = true;
}

result_type env::frame::get_result_value()
{
    return m_outer_frame->m_result;
}

env * env::frame::get_env()const
{
    return m_env;
}

void env::frame::register_universal_functions()
{
    static va_memfun return_scriptfun(&frame::signal_return_scriptfun);
    bind_universal_object(&return_scriptfun,FUNGU_OBJECT_ID("return"));

    static va_memfun result_scriptfun(&frame::set_result_scriptfun);
    bind_universal_object(&result_scriptfun,FUNGU_OBJECT_ID("result"));

    static va_memfun bind_myvar_scriptfun_obj(&frame::bind_myvar_scriptfun);
    bind_universal_object(&bind_myvar_scriptfun_obj,FUNGU_OBJECT_ID("my"));

    static va_memfun bind_localvar_scriptfun_obj(&frame::bind_localvar_scriptfun);
    bind_universal_object(&bind_localvar_scriptfun_obj,FUNGU_OBJECT_ID("local"));

    static va_memfun bind_globalvar_scriptfun_obj(&frame::bind_globalvar_scriptfun);
    bind_universal_object(&bind_globalvar_scriptfun_obj,FUNGU_OBJECT_ID("global"));

    static va_memfun getvar_scriptfun_obj(&frame::getvar_scriptfun);
    bind_universal_object(&getvar_scriptfun_obj,FUNGU_OBJECT_ID("get"));

    static va_memfun setvar_scriptfun_obj(&frame::setvar_scriptfun);
    bind_universal_object(&setvar_scriptfun_obj,FUNGU_OBJECT_ID("set"));

    static va_memfun issymbol_scriptfun_obj(&frame::issymbol_scriptfun);
    bind_universal_object(&issymbol_scriptfun_obj,FUNGU_OBJECT_ID("symbol?"));

    static va_memfun isnull_scriptfun_obj(&frame::isnull_scriptfun);
    bind_universal_object(&isnull_scriptfun_obj,FUNGU_OBJECT_ID("null?"));

    static va_memfun retnull_scriptfun_obj(&frame::retnull_scriptfun);
    bind_universal_object(&retnull_scriptfun_obj,FUNGU_OBJECT_ID("null"));

    static va_memfun isprocedure_scriptfun_obj(&frame::isprocedure_scriptfun);
    bind_universal_object(&isprocedure_scriptfun_obj,FUNGU_OBJECT_ID("procedure?"));

    static va_memfun getsrc_scriptfun_obj(&frame::getsrc_scriptfun);
    bind_universal_object(&getsrc_scriptfun_obj,FUNGU_OBJECT_ID("source"));
}

void env::frame::detach_bindings_from_env()
{
    if(m_detached) return;

    frame_symbol * binding = m_last_binding;
    while(binding)
    {
        binding->detach_from_env();
        binding = binding->get_next_sibling();
    }

    m_detached = true;
}

void env::frame::attach_bindings_to_env()
{
    if(!m_detached) return;

    frame_symbol * binding = m_last_binding;
    while(binding)
    {
        binding->attach_to_env();
        binding = binding->get_next_sibling();
    }
}

bool env::frame::is_detached_from_env()const
{
    return m_detached;
}

void env::frame::unset_return()
{
    m_expired = false;
    m_result.reset();
}

env::frame::frame_symbol * env::frame::get_last_bind()const
{
    return m_last_binding;
}

unsigned int env::frame::get_closure_refcount()const
{
    return m_closure_refcount;
}

void env::frame::incr_closure_refcount()
{
    assert(m_closure_refcount);
    m_closure_refcount++;
}

env::frame & env::frame::decr_closure_refcount()
{
    assert(m_closure_refcount);
    m_closure_refcount--;
    return *this;
}

void env::frame::setup_closure_refcount()
{
    m_closure_refcount = 1;
}

env::frame::frame(const env::frame &)
{
    assert(false);
}

result_type env::frame::signal_return_scriptfun(env::object::apply_arguments & args)
{
    if(!args.empty())
    {
        signal_return(args.front());
        args.pop_front();
    }
    else signal_return();
    return any::null_value();
}

result_type env::frame::set_result_scriptfun(env::object::apply_arguments & args)
{
    m_outer_frame->m_result = args.safe_front();
    args.pop_front();
    return m_result;
}

result_type env::frame::bind_myvar_scriptfun(env::object::apply_arguments & args)
{
    return bind_var_scriptfun(args,1);
}

result_type env::frame::bind_localvar_scriptfun(env::object::apply_arguments & args)
{
    return bind_var_scriptfun(args,2);
}

result_type env::frame::bind_globalvar_scriptfun(env::object::apply_arguments & args)
{
    return bind_var_scriptfun(args,3);
}

result_type env::frame::bind_var_scriptfun(env::object::apply_arguments & args,int scope)
{
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    const_string id = args.casted_front<const_string>();
    args.pop_front();

    any_variable * var = new any_variable;
    
    switch(scope)
    {
        case 1: //my
            bind_object(var,id.copy()).adopt_object();
            break;
        case 2: //local
            bind_object(var,id.copy()).adopt_object().set_dynamically_scoped();
            break;
        case 3: //global
            m_env->bind_global_object(var,id.copy()).adopt_object();
            break;
    }

    if(!args.empty())
    {
        var->assign(args.front());
        args.pop_front();
    }

    return var->value();
}

result_type env::frame::getvar_scriptfun(env::object::apply_arguments & args)
{
    any subject = args.safe_front();
    args.pop_front();

    object * obj;
    if(subject.get_type() == typeid(env::object::shared_ptr))
        obj = any_cast<env::object::shared_ptr>(subject).get();
    else obj = lookup_required_object(subject.to_string());

    return obj->value();
}

result_type env::frame::setvar_scriptfun(env::object::apply_arguments & args)
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

result_type env::frame::issymbol_scriptfun(env::object::apply_arguments & args)
{
    const_string id = args.safe_casted_front<const_string>();
    args.pop_front();
    return (bool)lookup_object(id);
}

result_type env::frame::isnull_scriptfun(env::object::apply_arguments & args)
{
    any value = args.safe_front();
    args.pop_front();
    return value.empty();
}

result_type env::frame::retnull_scriptfun(env::object::apply_arguments & args)
{
    return any();
}

result_type env::frame::isprocedure_scriptfun(env::object::apply_arguments & args)
{
    bool isProcedure = args.safe_front().get_type() == typeid(env::object::shared_ptr);
    args.pop_front();
    return isProcedure;
}

result_type env::frame::getsrc_scriptfun(env::object::apply_arguments & args)
{
    const_string symbol_id = args.safe_casted_front<const_string>();
    args.pop_front();
    const source_context * source_ctx = lookup_required_object(symbol_id)->get_source_context();
    std::string source;
    if(source_ctx)
        source = source_ctx->get_location() + std::string(":") + lexical_cast<std::string>(source_ctx->get_line_number()); 
    return const_string(source);
}

void env::frame::notify_bind_event(const_string id,object * obj)
{
    if(m_env->m_bind_observer)
        m_env->m_bind_observer(id,obj);
}

env::closure_frame * env::closure_frame::new_(env * envir)
{
    return new closure_frame(envir);
}

env::closure_frame * env::closure_frame::new_inner(frame * outer_frame)
{
    return new closure_frame(outer_frame);
}

void env::closure_frame::delete_(closure_frame * aClosureFrame)
{
    if(aClosureFrame->decr_closure_refcount().get_closure_refcount() == 0)
        delete aClosureFrame;
}

void env::closure_frame::attach_bindings_to_env()
{
    if(m_outer_closure)
    {
        m_outer_was_detached = m_outer_closure->is_detached_from_env();
        m_outer_closure->attach_bindings_to_env();
    }
    frame::attach_bindings_to_env();
}

void env::closure_frame::detach_bindings_from_env()
{
    if(m_outer_closure && m_outer_was_detached)
        m_outer_closure->detach_bindings_from_env();
    frame::detach_bindings_from_env();
}

env::closure_frame::closure_frame(env * envir)
 :frame(envir), m_outer_closure(NULL)
{
    setup_closure_refcount();
}

env::closure_frame::closure_frame(frame * outer_frame)
 :frame(outer_frame)
{
    setup_closure_refcount();
    if(outer_frame->get_closure_refcount())
    {
        m_outer_closure = static_cast<closure_frame *>(outer_frame);
        m_outer_closure->incr_closure_refcount();
        m_outer_was_detached = false;
    }
}

env::closure_frame::~closure_frame()
{
    if(m_outer_closure && 
        m_outer_closure->decr_closure_refcount().get_closure_refcount() == 0) delete m_outer_closure;
}

} //namespace script
} //namespace fungu
