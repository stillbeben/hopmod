/*   libcubescript - A CubeScript engine.  */
/*
 *  Copyright (c) 2008 Graham Daws. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *          
 *      2. Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *         
 *      3. The names of the contributors and copyright holders must not be used
 *         to endorse or promote products derived from this software without
 *         specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef LIBCUBESCRIPT_LEXCAST_HPP
#define LIBCUBESCRIPT_LEXCAST_HPP

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include "error.hpp"
#include "constructs/string.hpp"
#include "constructs/word.hpp"

namespace cubescript{

template<typename T> inline void parse_type(const std::string & str,std::vector<T> & v);
template<typename T> inline void parse_type(const std::string & str,std::list<T> & v);

template<typename T>
void parse_type(const std::string & str,T & v)
{
    std::stringstream reader(str);
    reader.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    try{reader>>v;}
    catch(std::ios_base::failure){throw error_key("runtime.bad_cast");}
    catch(std::bad_cast){throw error_key("runtime.bad_cast");}
}

template<typename T> inline
T parse_type(const std::string & str)
{
    T tmp;
    parse_type(str,tmp);
    return tmp;
}

template<> inline 
std::string parse_type<std::string>(const std::string & str){return str;}

template<> inline 
bool parse_type<bool>(const std::string & str)
{
    if(str.length()==0 || str[0]=='0') return false;
    else return true;
}

template<> inline
int parse_type<int>(const std::string & str)
{
    if(!str.length()) return 0;
    int val; 
    parse_type(str,val);
    return val;
}

template<> inline
float parse_type<float>(const std::string & str)
{
    if(!str.length()) return 0.0;
    float val;
    parse_type(str,val);
    return val;
}

template<typename T>
T parse_pushback_container_list(const std::string & str)
{
    T container;
    std::stringstream input(str);
    
    while(input.peek()>0)
    {
        char c;
        input.get(c);
        
        if(c==' ' || c=='\t' || c=='\r' || c=='\n') continue;
        
        if(c=='\"')
        {
            cons::string element;
            element.parse(input);
            container.push_back(parse_type<typename T::value_type>(element.eval()));
            input.get(c);
        }
        else
        {
            input.putback(c);
            cons::word element;
            element.parse(input);
            container.push_back(parse_type<typename T::value_type>(element.eval()));
        }
    }
    
    return container;
}

template<typename T> inline
void parse_type(const std::string & str,std::vector<T> & v)
{
    v=parse_pushback_container_list< std::vector<T> >(str);
}

template<typename T> inline
void parse_type(const std::string & str,std::list<T> & v)
{
    v=parse_pushback_container_list< std::list<T> >(str);
}

template<typename T> inline std::string __print_type(std::vector<T>);
template<typename T> inline std::string __print_type(std::list<T> &);

template<typename T> 
std::string __print_type(T value)
{
    std::stringstream writer;
    writer<<value;
    return writer.str();
}

template<typename T> inline
std::string print_type(T value)
{
    return __print_type(value);
}

template<typename T>
std::string print_forward_container_list(const T & data)
{
    std::string list;
    list.reserve(64);
    
    for(typename T::const_iterator it=data.begin(); it!=data.end(); ++it)
    {
        list+=compose_string(print_type<typename T::value_type>(*it));
        list+=" ";
    }
    
    return list;
}

template<typename T> inline
std::string __print_type(std::vector<T> value)
{
    return print_forward_container_list(value);
}

template<typename T> inline
std::string __print_type(std::list<T> & value)
{
    return print_forward_container_list(value);
}

} //namespace cubescript

#endif
