/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS

class frame
{
public:
    class frame_symbol:public symbol
    {
    public:
        frame_symbol(frame_symbol ** table_entry,frame * aFrame);
        ~frame_symbol();
        
        frame_symbol & attach_to_env();
        frame_symbol * attach_global_to_env();
        
        frame_symbol & detach_from_env();
        frame_symbol & detach_from_frame();
        
        frame * get_frame()const;
        frame_symbol * get_next_sibling()const;
        frame_symbol * get_prev_frame_binding()const;
    private:
        frame_symbol(const frame_symbol &);
    
        frame_symbol ** m_table_entry;
        frame_symbol * m_prev_frame_binding;
        frame_symbol * m_next_sibling;
        frame * m_frame;
    };
    
    frame(env * envir);
    frame(frame * outer_frame);
    ~frame();
    
    symbol & bind_object(object * obj, object_identifier id);
    symbol & bind_object(object * obj, frame_symbol ** table_entry);
    symbol & bind_global_object(object * obj, object_identifier id);
    
    object * lookup_object(object_identifier id)const;
    object * lookup_object(const frame::frame_symbol * const * handle)const;
    object * lookup_required_object(object_identifier id)const;
    object * lookup_required_object(const frame_symbol * const * handle)const;
    
    bool has_expired()const;
    
    void signal_return();
    void signal_return(result_type value);
    
    result_type get_result_value();
    
    env * get_env()const;
    
    static void register_universal_functions();
    
    void detach_bindings_from_env();
    void attach_bindings_to_env();
    
    bool is_detached_from_env()const;
    
    void unset_return();
    
    frame_symbol * get_last_bind()const;
    
    unsigned int get_closure_refcount()const;
    void incr_closure_refcount();
    frame & decr_closure_refcount();
protected:
    void setup_closure_refcount();
private:
    frame(const frame &);
    
    result_type signal_return_scriptfun(object::apply_arguments & args);
    result_type set_result_scriptfun(object::apply_arguments & args);
    
    result_type bind_myvar_scriptfun(object::apply_arguments & args);
    result_type bind_localvar_scriptfun(object::apply_arguments & args);
    result_type bind_globalvar_scriptfun(object::apply_arguments & args);
    result_type bind_var_scriptfun(object::apply_arguments & args,int scope);
    
    result_type getvar_scriptfun(object::apply_arguments & args);
    result_type setvar_scriptfun(object::apply_arguments & args);
    
    result_type issymbol_scriptfun(object::apply_arguments & args);
    result_type isnull_scriptfun(object::apply_arguments & args);
    
    result_type retnull_scriptfun(object::apply_arguments & args);
    
    result_type isprocedure_scriptfun(object::apply_arguments & args);
    
    result_type getsrc_scriptfun(object::apply_arguments & args);
    
    void notify_bind_event(const_string id,object * obj);
    
    class va_memfun:public object
    {
    public:
        typedef result_type (frame::*pointer)(apply_arguments &);
        va_memfun(pointer ptr):m_fun(ptr){}
        result_type apply(apply_arguments & args,frame * x)
        {
            return (x->*m_fun)(args);
        }
        result_type value(){return get_shared_ptr();}
    private:
        pointer m_fun;
    };
    
    env * m_env;
    frame * m_outer_frame;  //used to get to the root of a lexical scope
    frame_symbol * m_last_binding;
    bool m_expired;
    result_type m_result;
    
    unsigned int m_closure_refcount;
    
    bool m_detached;
};

class closure_frame:public frame
{
public:
    static closure_frame * new_(env * envir);
    static closure_frame * new_inner(frame * outer_frame);
    static void delete_(closure_frame * aClosureFrame);
    void attach_bindings_to_env();
    void detach_bindings_from_env();
private:
    closure_frame(env * envir);
    closure_frame(frame * outer_frame);
    ~closure_frame();
    
    closure_frame * m_outer_closure;
    bool m_outer_was_detached;
};

#endif
