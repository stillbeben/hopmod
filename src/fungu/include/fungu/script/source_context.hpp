/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SOURCE_CONTEXT_HPP
#define FUNGU_SCRIPT_SOURCE_CONTEXT_HPP

#include <string>

namespace fungu{
namespace script{

class source_context
{
public:
    source_context():m_linenum(0){}
    virtual ~source_context(){}
    
    virtual source_context * clone() const = 0;
    
    virtual const char * get_uri_scheme() const = 0;
    
    virtual std::string get_location() const = 0;
    
    int get_line_number()const
    {
        return m_linenum;
    }
    
    void set_line_number(int linenum)
    {
        m_linenum = linenum;
    }
private:
    int m_linenum;
};

class file_source_context:public source_context
{
public:
    file_source_context(const std::string & filename)
     :m_filename(filename){}
    source_context * clone() const{return new file_source_context(*this);}
    const char * get_uri_scheme() const {return "file";}
    std::string get_location() const {return m_filename;}
private:
    std::string m_filename;
};

class local_source_context:public source_context
{
public:
    local_source_context()
    {
        set_line_number(1);
    }
    source_context * clone() const{return new local_source_context(*this);}
    const char * get_uri_scheme() const {return "local";}
    std::string get_location() const {return "";}
};

} //namespace script
} //namespace fungu

#endif
