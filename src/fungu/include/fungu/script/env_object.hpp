/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS

/**
    @brief Environment object base class.
    
    Functions and variables and everything else the environment can interact
    with are types inherited from the object class. All the language operators
    are declared here as virtual functions for derived classes to override.
*/
class object
{
public:
    typedef callargs call_arguments;
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
    
    /**
        @brief Get information on this type of object.
        
        This method is useful for a foreign environment on determining how to
        treat an object.
        
        The base implementation returns UNCLASSIFIED_OBJECT.
    */
    virtual object_type get_object_type()const;
    
    /**
        @brief Function call (language operator).
    */
    virtual result_type call(call_arguments &,frame *) = 0;
    
    /**
        @brief Get value of object (language operator).
        
        The base implementation returns a shared ptr to this object.
    */
    virtual result_type value();
    
    /**
        @brief Assign a value to the object (language operator).
    
        The base implementation throws an exception error(NO_WRITE).
    */
    virtual void assign(const any &);
    
    /**
        @brief Get member object (language operator).
        
        The base implementation returns NULL.
    */
    virtual object * lookup_member(const_string);
    
    /**
        @brief Get iterator for the first member object.
        
        The base implementation returns NULL.
    */
    virtual member_iterator * first_member()const;
    
    #ifdef FUNGU_WITH_LUA
    virtual int call(lua_State *);
    virtual void value(lua_State *);
    #endif
    
    /**
        @brief Increment object reference count.
    */
    void add_ref();
    
    /**
        @brief Decrement object reference count.
    */
    object & unref();
    
    /**
        @brief Get value of object reference count.
    */
    unsigned int get_refcount()const;
    
    /**
        @brief Set object as temporary - meaning it cannot be shared.
        
        This method is for objects which are temporary and are stack
        allocated.
    */
    object & set_temporary();
    
    /**
        @brief Set object as adopted - meaning the last shared owner deletes the object.
        
        By default, objects have "native" ownership, meaning the sole owner of
        the object has the responsibility for the object's deallocation. When
        the original owner sets the object as adopted, it's then the case that
        it's up to the last reference holder to delete the object. This method
        must only be called by the object's creator (ideally, this attribute
        would be passed to the constructor and this method wouldn't exist).
        
        This scheme exists to allow both heap and stack allocated objects to be
        used. The adopted attribute/flag can only be set on heap allocated
        objects - DO NOT call this method on a stack allocated object.
        
        Stack allocated objects should call the set_temporary method.
        
        TODO: Find a reliable way to detect if the object is located on the stack.
    */
    object & set_adopted();
    
    /**
        @brief Is the object a temporary?
    */
    bool is_temporary()const;
    
    /**
        @brief Is the object adopted?
    */
    bool is_adopted()const;
    
    /**
        @brief Return a smart pointer pointing to this object.
        
        Peforms automatic reference counting and deallocation. If the object
        is a temporary then an exception error(UNSUPPORTED) will be thrown.
    */
    shared_ptr get_shared_ptr();
    
    static shared_ptr get_shared_ptr(object * obj);
private:
    unsigned int m_refcount;
    
    enum
    {
        TEMPORARY_OBJECT = 1,
        ADOPTED_OBJECT = 2
    };
    unsigned char m_flags;
};

#endif
