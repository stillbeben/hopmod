/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_STRINGUTILS_HPP
#define FUNGU_STRINGUTILS_HPP

#include "string.hpp"
#include <string>
#include <boost/functional/hash.hpp>
#include <ostream>
#include <istream>

namespace fungu{

/**
    @brief Trim newline characters from end of string.
*/
inline std::string * trim_newlines(std::string * str)
{
    std::size_t len=str->length();
    if(len)
    {
        std::string::iterator erase_first=str->end();

        char last=(*str)[len-1];
        if(last=='\n')
        {
            --erase_first;
            if(len > 1 && (*str)[len-2]=='\r') --erase_first;
        }
        else if(last=='\r') --erase_first;
        
        if(erase_first!=str->end())
        {
            str->erase(erase_first,str->end());
            trim_newlines(str);
        }
    }
    
    return str;
}

inline bool scan_newline(const char ** readptr)
{
    if(**readptr == '\r' && *((*readptr)+1)=='\n')
        *readptr += 2;
    else
        if(**readptr == '\n')
            (*readptr)++;
        else
            return false;
    return true;
}

template<typename T>
size_t hash_value(basic_const_string<T> str)
{
    return boost::hash_range(str.begin(),str.end());
}

template<typename T>
std::string & operator+=(std::string & dst,const basic_const_string<T> & src)
{
    dst.append(src.begin(),src.end());
    return dst;
}

template<typename T>
std::ostream & operator<<(std::ostream & dst,const basic_const_string<T> & src)
{
    dst.write(src.begin(),src.length());
    return dst;
}

template<typename T>
std::istream & operator>>(std::istream & src,basic_const_string<T> & dst)
{
    std::basic_string<T> str;
    str.reserve(32);
    while(src.good())
    {
        T e;
        src.read(&e,sizeof(T));
        if(src.good()) str.append(1,e);
    }
    dst = basic_const_string<T>(str);
    src.clear();
    return src;
}

} //namespace fungu

#endif
