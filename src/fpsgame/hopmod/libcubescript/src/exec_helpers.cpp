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

#include <fstream>
#include <sstream>
#include "cubescript/exec_helpers.hpp"
#include "cubescript/alias.hpp"
#include "cubescript/constructs/expression.hpp"
#include "cubescript/bind.hpp"
#include "cubescript/lexcast.hpp"

static std::string extract_path(const std::string & filename)
{
    int i=filename.length()-1;
    while(i>=0 && filename[i]!='/' && filename[i]!='\\' ) --i;
    return filename.substr(0,i+1);
}

namespace cubescript{

std::string exec_block(const std::string & input,domain * d)
{
    std::list<std::string> noargs;
    alias runner;
    runner.push(input);
    return runner.apply(noargs,d);
}

std::string exec_script(std::istream & input,const std::string & filename,domain * aDomain)
{
    std::string result;
    int line_number=0;
    
    cons::expression * exp=NULL;
    
    while(!input.eof())
    {
        std::string line;
        std::getline(input,line);
        line+="\r\n";
        ++line_number;
        
        std::stringstream line_stream(line);
        
        while(line_stream.peek()>0)
        {
            try
            {
                if(!exp)
                {
                    exp=new cons::expression;
                    exp->set_domain(aDomain);
                }
                
                if(exp->parse(line_stream))
                {
                    result=exp->eval();
                    
                    char expterm=line_stream.get();
                    if(expterm=='\r') line_stream.get(); //lf;
                    
                    delete exp;
                    exp=NULL;
                }
            }
            catch(error_key & key){throw new script_error_context(key,filename,line_number);}
            catch(error_context * frame){throw new script_error_context(frame,filename,line_number);}
        }
    }
    
    return result;
}

std::string exec_file(const std::string & filename,domain * parent_domain)
{
    std::ifstream file(filename.c_str());
    
    domain local_domain(parent_domain,domain::TEMPORARY_DOMAIN);
    domain writable_domain(&local_domain);
    
    bind(filename,"FILENAME",&local_domain);
    bind(extract_path(filename),"SCRIPT_PATH",&local_domain);
    
    if(file.fail()) throw error_key("runtime.function.exec.file_not_found");
    std::string result=exec_script(file,filename,&writable_domain);
    
    domain * dst=parent_domain->get_first_nontmp_domain();
    if(dst) dst->unite(&writable_domain);
    
    return result;
}

static std::list<std::string> gs_noargs;

std::list<std::string> & args0()
{
    gs_noargs.clear();
    return gs_noargs;
}

std::string script_error_context::get_name()const{return "script";}

std::string script_error_context::get_description()const
{
    std::stringstream result;
    result<<m_file<<"("<<m_line<<")";
    return result.str();
}

std::string script_error_context::get_attribute(const std::string & name)const
{
    if(name=="line") return print_type<int>(m_line);
    else if(name=="file") return m_file;
    assert(false);
}

} //namespace cubescript
