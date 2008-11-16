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

#include <sstream>
#include "cubescript/constructs/word.hpp"
#include "cubescript/constructs/string.hpp"
#include "cubescript/constructs/macro.hpp"
#include "cubescript/constructs/block.hpp"
#include "cubescript/constructs/comment.hpp"
#include "cubescript/constructs/expression.hpp"
#include "cubescript/domain.hpp"
#include "cubescript/symbol.hpp"
#include "cubescript/error.hpp"

namespace cubescript{
namespace cons{

cons::word::word()
{
    m_word.reserve(32);
}

cons::word::~word()
{
    
}
    
bool word::parse(std::istream & input)
{
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(in_set(c)) m_word+=c;
        else
        {
            input.putback(c);
            return true;
        }
    }
}

std::string word::eval()
{
    std::string result = m_word;
    
    if(m_word.length() && m_word[0]=='$')
    {
        unsigned int resolutions;
        for(resolutions=1; resolutions < m_word.length()-1 && m_word[resolutions]=='$'; resolutions++) ;
        result = result.substr(resolutions);
        for(unsigned int i=0; i<resolutions; i++)
        {
            symbol * s;
            try
            {
                s = get_domain()->require_symbol(result);
            }
            catch(const error_key & key)
            {
                throw new deref_error_context(key,result,i);
            }
            result = s->value();
        }
    }
    
    return result;
}

std::string word::formed()const
{
    return m_word;
}

bool word::in_set(char c)const
{
    //excluded charset
    if( c=='\"' || c=='(' || c==')' || c=='[' || c==']' 
        || c==' ' || c=='\r' || c=='\n' || c=='\t' || c==';') return false;
    
    return true;
}

string::string()
{
    m_str.reserve(64);
}

bool string::parse(std::istream & input)
{
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(m_str.length() && m_str[m_str.length()-1]=='\\')
            m_str.erase(m_str.end()-1);
        else if(c=='\"' || c=='\r' || c=='\n')
        {
            input.putback(c);
            
            return true;
        }
        
        m_str+=c;
    }
}

std::string string::eval()
{
    return m_str;
}

std::string string::formed()const
{
    return compose_string(m_str);
}

class macro_symbol:public word
{
public:
    std::string eval()
    {
        return get_domain()->require_symbol(word::eval())->value();
    }
};

macro::macro()
{
    m_escape=1;
    m_subt=NULL;
}

macro::~macro()
{
    if(m_subt) delete m_subt;
}

bool macro::parse(std::istream & input)
{
    if(m_subt) return parse_subt(input);
    
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c=='@')
        {
            ++m_escape;
        }
        else
        {
            if(c=='(') m_subt=new expression_container;
            else
            {
                input.putback(c);
                m_subt=new macro_symbol;
            }
            return parse_subt(input);
        }
    }
}

std::string macro::eval()
{
    return m_subt->eval();
}

std::string macro::formed()const
{
    std::stringstream writer;
    for(int i=1; i<=m_escape; ++i) writer<<"@";
    writer<<m_subt->formed();
    return writer.str();
}

int macro::get_escape_level()const
{
    return m_escape;
}

bool macro::parse_subt(std::istream & input)
{
    m_subt->set_domain(get_domain());
    
    bool done=m_subt->parse(input);
    if(!done) return false;
    
    bool is_expression=dynamic_cast<expression_container *>(m_subt);
    if(is_expression)
    {
        char termc;
        input.get(termc);
        assert(!input.fail());
        if(termc!=')') throw error_key("syntax.expected_terminator.expression");
    }
    
    return true;
}

block::block()
{
    m_code.reserve(256);
    m_nested=0;
    m_macro=NULL;
}

bool block::parse(std::istream & input)
{
    while(true)
    {
        if(m_macro && !parse_macro(input)) return false;
        
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c=='@')
        {
            m_macro=new macro;
            if(!parse_macro(input)) return false;
        }
        else
        {
            if(c=='[') ++m_nested;
            else if(c==']')
            {
                if(!m_nested)
                {
                    input.putback(c);
                    return true;
                }
                --m_nested;
            }
            
            m_code+=c;
        }
    }
}

std::string block::eval()
{
    return m_code;
}

std::string block::formed()const
{
    std::stringstream writer;
    writer<<"["<<m_code<<"]";
    return writer.str();
}

bool block::parse_macro(std::istream & input)
{
    m_macro->set_domain(get_domain());
    if(!m_macro->parse(input)) return false;
    
    if( m_macro->get_escape_level() > m_nested ) m_code+=m_macro->eval();
    else m_code+=m_macro->formed();
    
    delete m_macro;
    m_macro=NULL;
    
    return true;
}

bool comment::parse(std::istream & input)
{
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c=='\r' || c=='\n')
        {
            if(m_line.length()<1 || m_line[0]!='/')
                throw error_key("syntax.expected_comment_token");
            input.putback(c);
            return true;
        }
        
        m_line+=c;
    }
}

std::string comment::eval()
{
    return "";
}

std::string comment::formed()const
{
    std::stringstream writer;
    writer<<"//"<<m_line;
    return writer.str();
}

} //namespace cons

std::string compose_string(const std::string & txt)
{
    if(txt.length()==0) return "";
    
    std::string result;
    result.reserve(txt.length()+2);
    
    result="\"";
    
    for(unsigned int i=0; i<txt.length(); ++i)
    {
        char c=txt[i];
        if(c=='\"' || c=='\r' || c=='\n') result+='\\';
        result+=c;
    }
    
    result+="\"";
    
    return result;
}

} //namespace cubescript
