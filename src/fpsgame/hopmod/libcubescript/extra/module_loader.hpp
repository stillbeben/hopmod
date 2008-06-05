
#ifndef MODULE_LOADER_HPP
#define MODULE_LOADER_HPP

namespace cubescript
{

class module
{
public:
    class domain_accessor;
    template<typename> class symbol_delegate;
    typedef void (* throw_exception_function)(const error_key &);
    virtual void initialise(throw_exception_function throw_func){m_throw_function=throw_func;}
    virtual void register_symbols(domain_accessor *)=0;
    void throw_exception(const error_key & e){m_throw_function(e);}
private:
    throw_exception_function m_throw_function;
};

class module::domain_accessor
{
public:
    domain_accessor(domain *);
    void register_symbol(const char *,symbol *);
private:
    domain * m_domain;
};

template<typename T>
class module::symbol_delegate:public symbol
{
public:
    symbol_delegate(module * manager,const T & src):m_manager(manager),m_source(src){}
    std::string apply(std::list<std::string> & arglist,domain * aDomain)
    {
        try{return m_source.apply(arglist,aDomain);}
        catch(const error_key & e){m_manager->throw_exception(e);}
    }
    std::string value()const
    {
        try{return m_source.value();}
        catch(const error_key & e){m_manager->throw_exception(e);}
    }
private:
    T m_source;
    module * m_manager;
};

#define IMPLEMENT_CUBESCRIPT_MODULE(name) extern "C" { \
    static name g_module_instance; \
    cubescript::module * get_csmodule_object(){return &g_module_instance;} } 

}

#endif