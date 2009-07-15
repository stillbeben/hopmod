/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_STRINGUTILS_HPP
#define FUNGU_STRINGUTILS_HPP

#include <string>

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
    
} //namespace fungu

#endif
