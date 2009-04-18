/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS

class object
{
public:
    typedef arguments_container apply_arguments;
    typedef env::frame frame;
    typedef boost::intrusive_ptr<object> shared_ptr;
    
    object();
    virtual ~object();
    
    class member_iterator
    {
    public:
        virtual ~member_iterator(){}
        virtual const_string get_name()const=0;
        virtual object * get_object()const=0;
        virtual bool next()=0;
    };

    enum object_type
    {
        UNCLASSIFIED_OBJECT = 0,
        FUNCTION_OBJECT,
        DATA_OBJECT
    };
    
    virtual object_type get_object_type()const;
    
    virtual result_type apply(apply_arguments &,frame *) = 0;
    #ifdef FUNGU_WITH_LUA
    virtual int apply(lua_State *);
    #endif
    virtual result_type value();
    virtual void assign(const any &);
    virtual object * lookup_member(const_string);
    virtual member_iterator * first_member()const;
    
    virtual const source_context * get_source_context()const;
    
    void add_ref();
    object & unref();
    unsigned int get_refcount()const;
    object & set_temporary_flag();
    object & set_adopted_flag();
    bool is_temporary()const;
    bool is_adopted()const;
    shared_ptr get_shared_ptr();
    static shared_ptr get_shared_ptr(object * obj);
private:
    unsigned int m_refcount;
    
    enum
    {
        FLAG_TEMPORARY = 1,
        FLAG_ADOPTED = 2
    };
    unsigned int m_flags;
};

#endif
