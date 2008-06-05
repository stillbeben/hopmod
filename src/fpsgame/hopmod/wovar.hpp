
#ifndef WOVAR_HPP
#define WOVAR_HPP

#include <cubescript.hpp>

namespace cubescript
{

template<typename T>
class wo_variable_ref:public variable_ref<T>
{
public:
    wo_variable_ref(T & ref):variable_ref<T>(ref){}
    std::string value()const{throw error_key("runtime.variable.write_only");}
};

class wo_cstr_variable:public cstr_variable
{
public:
    wo_cstr_variable(char * str,size_t maxlen):cstr_variable(str,maxlen){}
    std::string value()const{throw error_key("runtime.variable.write_only");}
};

}

#endif
