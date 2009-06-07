/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS

/**
    @brief Local environment
*/
class frame
{
public:
    /**
        @brief Start a new scope
    */
    frame(env * envir);

    /**
        @brief Continue scope of parent frame - construct an inner frame.
    */
    frame(frame * outer_frame);

    /**
        Local bindings are deleted at frame object destruction.
    */
    ~frame();
    
    /**
        @brief Create local symbol binding.        
    */
    void bind_object(object *, object_id);

    /**
        @brief Create local symbol binding.
    */
    void bind_object(object *, symbol *);
    
    /**
        @brief Get the object bound to a local symbol.
        
        Returns NULL if the symbol is not found.
    */
    object * lookup_object(object_id id)const;
    
    /**
        @brief Get the object bound to a local symbol.
        
        Returns NULL if the symbol is not found.
    */
    object * lookup_object(const symbol *)const;

    /**
        @brief Get the object bound to a local symbol.
        
        Throws error(UNKNOWN_SYMBOL) exception if the symbol is not found.
    */
    object * lookup_required_object(object_id id)const;
    
    /**
        @brief Get the object bound to a local symbol.
        
        Throws error(UNKNOWN_SYMBOL) exception if the symbol is not found.
    */
    object * lookup_required_object(const symbol *)const;
    
    /**
        @brief Tell the evaluator not to evaluate anymore expressions.
    */
    void signal_return();
    
    /**
        @brief Tell the evaluator not to evaluate anymore expressions.
        @param value set the explicit result value
    */
    void signal_return(result_type value);
    
    /**
        @brief Undo a signal_return() call.
    */
    void unset_return();
    
    /**
        @brief Get the explicit result value.
    */
    result_type get_result_value();
    
    /**
        @brief Has signal_return() been called?
    */
    bool has_expired()const;
    
    /**
        @brief Return a pointer of the associated environment object.
    */
    env * get_env()const;
    
    /**
        @brief Get the root frame of this scope.
    */
    frame * get_scope_frame()const;
    
    /**
        @brief Get the latest symbol_local object created.
    */
    symbol_local * get_last_bind()const;
    
    /**
        @brief Register frame-related runtime functions with a global environment.
    */
    static void register_functions(env &);
private:
    frame(const frame &);

    /**
        @brief Reattach local symbol bindings to environment.
    */
    void attach_locals();
    
    /**
        @brief Detach local symbol bindings from environment.
        
        Once detached, the local symbols will remain allocated but will be
        invisible to symbol lookups.
    */
    void detach_locals();

    /**
        @brief Are local symbols currently detached from the environment?
    */
    bool is_detached_from_env()const;

    result_type signal_return_scriptfun(object::call_arguments & args);
    result_type set_result_scriptfun(object::call_arguments & args);
    result_type bind_myvar_scriptfun(object::call_arguments & args);
    result_type bind_localvar_scriptfun(object::call_arguments & args);
    result_type bind_globalvar_scriptfun(object::call_arguments & args);
    result_type bind_var_scriptfun(object::call_arguments & args,int scope);
    result_type getvar_scriptfun(object::call_arguments & args);
    result_type setvar_scriptfun(object::call_arguments & args);
    result_type issymbol_scriptfun(object::call_arguments & args);
    result_type isnull_scriptfun(object::call_arguments & args);
    result_type retnull_scriptfun(object::call_arguments & args);
    result_type isprocedure_scriptfun(object::call_arguments & args);

    class va_memfun:public object
    {
    public:
        typedef result_type (frame::*pointer)(call_arguments &);
        va_memfun(pointer ptr):m_fun(ptr){}
        result_type call(call_arguments & args,frame * x)
        {
            return (x->*m_fun)(args);
        }
        result_type value(){return get_shared_ptr();}
    private:
        pointer m_fun;
    };
    
    env * m_env;
    frame * m_scope;
    
    symbol_local * m_last_binding;
    
    bool m_expired;
    result_type m_result;
    
    bool m_detached;
};

#endif
