
#ifndef MODULE_LOADER_HPP
#define MODULE_LOADER_HPP

#include <cubescript.hpp>

namespace cubescript{class module;}
extern "C" {cubescript::module * get_csmodule_object();}
#define IMPLEMENT_CUBESCRIPT_MODULE(name) extern "C" { \
    cubescript::module * get_csmodule_object(){ static name s_module_instance; \
    return &s_module_instance;}}

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
    symbol_delegate(const T & src):m_source(src){}
    std::string apply(std::list<std::string> & arglist,domain * aDomain)
    {
        try{return m_source.apply(arglist,aDomain);}
        catch(const error_key & e){get_csmodule_object()->throw_exception(e);}
    }
    std::string value()const
    {
        try{return m_source.value();}
        catch(const error_key & e){get_csmodule_object()->throw_exception(e);}
    }
private:
    T m_source;
};

void register_module_loader(domain *);

}




#endif
