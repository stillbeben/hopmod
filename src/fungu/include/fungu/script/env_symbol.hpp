/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS

class symbol
{
public:
    symbol();
    ~symbol();
    symbol & bind_object(object *);
    symbol & allow_rebind();
    symbol & adopt_object();
    symbol & set_dynamically_scoped();
    void inherit_dynamic_chain(const symbol *);
    object * get_object()const;
    bool is_dynamically_scoped()const;
    bool in_dynamic_scope_chain()const;
private:
    void unbind_object();
    
    object * m_object;
    
    enum SYMBOL_FLAGS
    {
        SYMBOL_SHARED_OBJECT    = 1,
        SYMBOL_OVERWRITE        = 2,
        SYMBOL_DYNAMIC_CHAIN    = 4,
        SYMBOL_DYNAMIC_SCOPE    = 8
    };
    unsigned long m_flags;
};

#endif
