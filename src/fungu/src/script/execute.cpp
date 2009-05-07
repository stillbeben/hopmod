/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"
#include "fungu/script/expression.hpp"
#include "fungu/script/execute.hpp"
#include "fungu/script/eval_stream.hpp"
#include "fungu/script/code_block.hpp"
#include "fungu/script/constant.hpp"

#include <stdio.h>
#include <errno.h>

namespace fungu{
namespace script{

namespace execute_detail{

typedef script::eval_stream file_eval_stream;

inline void file_eval(expression * expr,env::frame * frame){expr->eval(frame);}

} //namespace execute_detail

int execute_file(const char * filename, env::frame * aFrame)
{
    FILE * file_stream = fopen(filename,"r");
    if(!file_stream) return ENOENT;
    
    file_source_context file_context(filename);
    env * environment = aFrame->get_env();
    const source_context * prev_context = environment->get_source_context();
    environment->set_source_context(&file_context);
    
    env::closure_frame * file_frame = env::closure_frame::new_(aFrame->get_env());
    
    #define COMMON_CLEANUP \
        environment->set_source_context(prev_context); \
        file_frame->detach_bindings_from_env(); \
        file_frame->unset_return(); \
        env::closure_frame::delete_(file_frame); \
        fclose(file_stream);
    
    constant<const char *> filename_const(filename);
    filename_const.set_temporary_flag();
    file_frame->bind_object(&filename_const,FUNGU_OBJECT_ID("FILENAME"));
    
    execute_detail::file_eval_stream reader(file_frame, &execute_detail::file_eval);
    
    char tmpbuf[1024];
    int line = 1;
    
    while(!file_frame->has_expired() && !feof(file_stream) && !ferror(file_stream))
    {
        if(!fgets(tmpbuf, sizeof(tmpbuf), file_stream)) continue;
        
        try
        {
            reader.feed(tmpbuf, strlen(tmpbuf)+feof(file_stream));
        }
        catch(error_info *)
        {
            COMMON_CLEANUP;
            throw;
        }
        catch(error_exception)
        {
            COMMON_CLEANUP;
            throw;
        }
        
        file_context.set_line_number(++line);
    }
    
    if(reader.is_parsing_expression())
    {
        COMMON_CLEANUP;
        throw error(UNEXPECTED_EOF);
    }
    
    int code = feof(file_stream) ? 0 : errno;
    COMMON_CLEANUP;
    return code;
}

void throw_if_error(int errcode)
{
    if(errcode)
        throw error(OPERATION_ERROR,boost::make_tuple(std::string(strerror(errcode))));
}

result_type execute_text(const_string code,env::frame * parent_scope)
{
    return code_block::temporary_source(code,NULL)
        .compile(parent_scope).eval_each_expression(parent_scope);
}

} //namespace script
} //namespace fungu
