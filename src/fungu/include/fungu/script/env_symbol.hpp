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
    friend class symbol_local;
public:
    symbol();
    ~symbol();
    symbol_local * push_local_object(object *, const frame *);
    void set_global_object(object *);
    object * lookup_object(const frame *)const;
private:
    symbol(const symbol &);
    
    symbol_local * m_local;
    object::shared_ptr m_global;
};

class symbol_local
{
public:
    symbol_local(symbol *, const frame *);
    ~symbol_local();
    void attach();
    void detach();
    void set_object(object *);
    object * get_object()const;
    object * lookup_object(const frame *)const;
    const frame * get_frame()const;
    symbol_local * get_next_frame_sibling()const;
private:
    symbol_local(const symbol_local &);
    bool is_latest_attachment()const;
    
    symbol * m_symbol;
    symbol_local * m_super;
    const frame * m_frame;
    symbol_local * m_frame_sibling;
    object::shared_ptr m_object;
};

#endif
