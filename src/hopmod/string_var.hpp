#ifndef HOPMOD_STRING_VAR_HPP
#define HOPMOD_STRING_VAR_HPP

#include "cube.h"
#include "game.h"
#include <fungu/script/env.hpp>
#include <fungu/script/variable.hpp>

namespace fungu{
namespace script{

template<>
class variable<string>:public env::object
{
public:
    variable(string & str)
     :m_string(str)
    {
        
    }
    
    object_type get_object_type()const
    {
        return DATA_OBJECT;
    }
    
    void assign(const any & value)
    {
        const_string tmp = value.to_string();
        std::size_t len = tmp.length();
        if(len > MAXSTRLEN - 1) throw error(INVALID_VALUE,boost::make_tuple(std::string("string is too long")));
        strncpy(m_string, tmp.begin(), len);
        m_string[len] = '\0';
    }
    
    result_type apply(apply_arguments & args,env::frame *)
    {
        assign(args.safe_front());
        args.pop_front();
        try{return value();}
        catch(error){return any::null_value();}
    }
    
    result_type value()
    {
        return const_string(m_string,m_string+strlen(m_string)-1);
    }
private:
    string & m_string;
};

} //namespace script
} //namespace fungu

#endif
