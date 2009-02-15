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

#include <sys/stat.h>
#include <errno.h>
#include <regex.h>
#include <time.h>
#include <sstream>
#include <fstream>
#include <boost/bind.hpp>
#include "cubescript/runtime.hpp"
#include "cubescript/function.hpp"
#include "cubescript/constant.hpp"
#include "cubescript/variable.hpp"
#include "cubescript/exec_helpers.hpp"
#include "cubescript/object.hpp"

namespace cubescript{
namespace runtime{

void assign_alias(std::list<std::string> & args,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(args);
    std::string code=functionN::pop_arg<std::string>(args);
    alias * aAlias=aDomain->get_or_create_symbol<alias>(id,alias(),domain::REGISTER_ROOT | domain::SEARCH_PARENTS);
    while(aAlias->pop()) ;
    aAlias->push(code);
}

void push_alias(std::list<std::string> & args,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(args);
    std::string code=functionN::pop_arg<std::string>(args);
    alias * aAlias=aDomain->get_or_create_symbol<alias>(id,alias());
    aAlias->push(code);
}

void pop_alias(std::list<std::string> & args,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(args);
    alias * aAlias=aDomain->get_or_create_symbol<alias>(id,alias());
    aAlias->pop();
}

inline int math_add(int a,int b){return a + b;}
inline int math_sub(int a,int b){return a - b;}
inline int math_mul(int a,int b){return a * b;}
inline int math_div(int a,int b){if(b==0) throw error_key("runtime.divide_by_zero"); return a / b;}
inline int math_mod(int a,int b){return a % b;}
inline int math_min(int a,int b){return a < b? a : b;}
inline int math_max(int a,int b){return a > b? a : b;}
inline float math_fadd(float a,float b){return a+b;}
inline float math_fsub(float a,float b){return a-b;}
inline float math_fmul(float a,float b){return a*b;}
inline float math_fdiv(float a,float b){return a/b;}
#include <math.h>
inline float math_round(float a){return roundf(a);}
inline float math_ceil(float a){return ceilf(a);}
inline float math_floor(float a){return floorf(a);}
inline int math_abs(int a){return abs(a);}
inline float math_fabs(float a){return fabs(a);}

inline bool comp_equal(int a,int b){return a == b;}
inline bool comp_nequal(int a,int b){return a != b;}
inline bool comp_gt(int a,int b){return a > b;}
inline bool comp_gte(int a,int b){return a >= b;}
inline bool comp_lt(int a,int b){return a < b;}
inline bool comp_lte(int a,int b){return a<= b;}

inline bool logic_not(bool a){return !a;}
inline bool logic_or(alias_function & a,alias_function & b){return parse_type<bool>(a.run(args0())) || parse_type<bool>(b.run(args0()));}
inline bool logic_and(alias_function & a,alias_function & b){return parse_type<bool>(a.run(args0())) && parse_type<bool>(b.run(args0()));}
inline bool logic_xor(bool a,bool b){return a ^ b;}

std::string control_if(std::list<std::string> & args,domain * context)
{
    bool has_true_arg=args.size()>1;
    bool has_false_arg=args.size()>2;
    
    bool cond=functionN::pop_arg<bool>(args);
    std::string true_block=has_true_arg ? functionN::pop_arg<std::string>(args) :"";
    std::string false_block=has_false_arg ? functionN::pop_arg<std::string>(args) :"";
    
    alias runner;
    
    if(cond) {if(has_true_arg) runner.push(true_block);}
    else if(has_false_arg) runner.push(false_block);
    
    std::list<std::string> noargs;
    return runner.apply(noargs,context);
}

void control_loop(std::list<std::string> & args,domain * parent_domain)
{
    std::string counter=functionN::pop_arg<std::string>(args);
    int n=functionN::pop_arg<int>(args);
    std::string body=functionN::pop_arg<std::string>(args);
    
    domain tmp_domain(parent_domain);

    alias alias_counter;
    tmp_domain.register_symbol(counter,&alias_counter);
    
    alias alias_body;
    alias_body.push(body);
    
    std::list<std::string> noargs;
    
    for(int i=0; i<n; ++i)
    {
        alias_counter.push(print_type<int>(i));
        alias_body.apply(noargs,&tmp_domain);
        alias_counter.pop();
    }
}

void control_while(std::list<std::string> & args,domain * context)
{
    alias cond;
    cond.push(functionN::pop_arg<std::string>(args));
    
    alias body;
    body.push(functionN::pop_arg<std::string>(args));
    
    std::list<std::string> noargs;
    
    while(parse_type<bool>(cond.apply(noargs,context))) body.apply(noargs,context);
}

bool predicate_symbol(std::list<std::string> & args,domain * context)
{
    std::string name=functionN::pop_arg<std::string>(args);
    symbol * s=context->lookup_symbol(name);
    return s;
}

inline
std::string list_at(std::vector<std::string> list,unsigned int n)
{
    if(n<0 || n>=list.size()) throw error_key("runtime.out_of_bounds");
    return list[n];
}

inline 
int list_len(std::vector<std::string> list)
{
    return list.size();
}

void _throw(std::string key)
{
    if(key.compare(0,8,"runtime.")!=0) throw error_key("runtime.invalid_throw");
    static std::string s_key; //FIXME not thread safe!
    s_key=key;
    throw error_key(s_key.c_str());
}

inline bool strcmp(std::string a,std::string b){return a==b;}

std::string concat(std::list<std::string> & args,domain *)
{
    std::string result;
    while(!args.empty())
    {
        result+=functionN::pop_arg<std::string>(args);
        if(args.size()>0) result+=" ";
    }
    return result;
}

std::string concatword(std::list<std::string> & args,domain *)
{
    std::string result;
    while(!args.empty()) result+=functionN::pop_arg<std::string>(args);
    return result;
}

std::string format(std::list<std::string> & args,domain *)
{
    std::string result;
    
    std::string _template=functionN::pop_arg<std::string>(args);
    
    size_t totalc=_template.length();
    
    std::vector<std::string> elements;
    while(!args.empty())
    {
        std::string el=functionN::pop_arg<std::string>(args);
        totalc+=el.length();
        elements.push_back(el);
    }
    
    result.reserve(totalc);
    
    std::string::size_type len=_template.length();
    for(unsigned int i=0; i<len; ++i)
    {
        if(_template[i]=='%')
        {
            if(i==len-1) throw error_key("runtime.function.format.invalid_template");
            char digitc=_template[i+1];
            if(!(digitc>='1' && digitc<='9')) throw error_key("runtime.function.format.invalid_template");
            unsigned int n=digitc-'1';
            if(n>=elements.size()) throw error_key("runtime.function.format.missing_template_argument");
            result+=elements[n];
            ++i;
        }
        else result+=_template[i];
    }
    
    return result;
}

std::vector<std::string> split(const std::string & str,const std::string & delims)
{
    std::vector<std::string> tokens(1);
    for(unsigned int i=0; i<str.length(); ++i)
    {
        bool match=false;
        for(unsigned int j=0; j<delims.length(); ++j)
            if(delims[j]==str[i])
            {
                match=true;
                break;
            }
        if(match) tokens.push_back(std::string());
        else tokens.back()+=str[i];
    }
    return tokens;
}

bool match(const std::string & pattern,const std::string & str)
{
    regex_t compiled;
    int compret=regcomp(&compiled,pattern.c_str(),0);
    if(compret!=0) throw error_key("runtime.function.match.regcomp_failed");
    int execret=regexec(&compiled,str.c_str(),0,NULL,0);
    regfree(&compiled);
    
    bool matched = false;
    if(execret==0) matched = true;
    else if(execret==REG_ESPACE)
        throw error_key("runtime.function.match.nomem");
    return matched;
}

void foreach(const std::list<std::string> & list,alias_function & code)
{
    for(std::list<std::string>::const_iterator it=list.begin();
        it!=list.end();
        ++it )
    {
        arguments args;
        code.run(args & (*it));
    }
}

time_t get_time_now()
{
    return time(NULL);
}

std::string format_date(time_t local_timestamp)
{
    const tm * fields = localtime(&local_timestamp);
    std::ostringstream result;
    result<<1900 + fields->tm_year<<"-";
    result<<(fields->tm_mon+1 < 10 ? "0":"")<<fields->tm_mon+1<<"-";
    result<<(fields->tm_mday < 10 ? "0":"")<<fields->tm_mday;
    return result.str();
}

std::string format_time(time_t local_timestamp)
{
    const tm * fields=localtime(&local_timestamp);
    std::ostringstream result;
    
    result<<(fields->tm_hour < 10 ? "0":"")<<fields->tm_hour<<":";
    result<<(fields->tm_min < 10 ? "0":"")<<fields->tm_min<<":";
    result<<(fields->tm_sec < 10 ? "0":"")<<fields->tm_sec;
    
    int tz_off=fields->tm_gmtoff;
    
    if(tz_off < 0)
    {
        tz_off=abs(tz_off);
        result<<"-";
    }
    else result<<"+";
    
    int tz_hours=tz_off/3600;
    int tz_mins=(tz_off-(tz_hours*3600))/60;
    
    result<<(tz_hours < 10 ? "0":"")<<tz_hours<<":";
    result<<(tz_mins < 10 ? "0":"")<<tz_mins;
    
    return result.str();
}

std::string format_datetime(time_t local_timestamp)
{
    std::ostringstream result;
    result<<format_date(local_timestamp)<<"T"<<format_time(local_timestamp);
    return result.str();
}

const char * get_weekday(time_t local_timestamp)
{
    tm * fields=localtime(&local_timestamp);
    static char * days[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    return *(days+fields->tm_wday);
}

const char * get_month(time_t local_timestamp)
{
    tm * fields=localtime(&local_timestamp);
    static char * months[]={"January","February","March","April","May","June","July","August","September","October","November","December"};
    return *(months+fields->tm_mon);
}

static std::string exec_catch_code(const char * key,const std::string & code,domain * aDomain)
{
    nullary_setter rethrow;
    domain catch_context(aDomain,domain::TEMPORARY_DOMAIN);
    catch_context.register_symbol("rethrow",&rethrow);
    
    alias runcatch;
    runcatch.push(code);
    
    arguments args;
    std::string result = runcatch.apply(args & std::string(key),&catch_context);
    
    if(rethrow.is_set()) throw;
    return result;
}

std::string _try(std::list<std::string> & arglist,domain * aDomain)
{
    //try <operation> <argument 1> ... <catch error block>
    std::string op=functionN::pop_arg<std::string>(arglist);
    if(arglist.empty()) throw error_key("runtime.function.try.missing_catch_argument");
    std::string catchcode=arglist.back();
    arglist.pop_back();
    
    std::string result;
    
    try
    {
        result = aDomain->require_symbol(op)->apply(arglist,aDomain);
    }
    catch(const error_key & e)
    {
        /*nullary_setter rethrow;
        domain catch_context(aDomain,domain::TEMPORARY_DOMAIN);
        catch_context.register_symbol("rethrow",&rethrow);
        
        arglist.clear();
        alias runcatch;
        runcatch.push(catchcode);
        
        arguments args;
        std::string result=runcatch.apply(args & e.get_key(),&catch_context);
        
        if(rethrow.is_set()) throw;
        return result;*/
        result = exec_catch_code(e.get_key(),catchcode,aDomain);
    }
    catch(error_context * e)
    {
        result = exec_catch_code(e->get_key(),catchcode,aDomain);
    }
    
    arglist.clear();
    return result;
}

std::string dump(std::list<std::string> & arglist,domain * aDomain)
{
    bool dump2file=false;
    std::ostringstream memory;
    std::ofstream file;
    std::ostream * output;
    
    if(arglist.size())
    {
        file.open(functionN::pop_arg<std::string>(arglist).c_str(),std::ios_base::out);
        dump2file=true;
        output=&file;
    }
    else output=&memory;
    
    aDomain->save(*output);
    
    if(!dump2file) return memory.str();
    else return "";
}

void create_object(std::list<std::string> & arglist,domain * aDomain)
{
    std::string className=functionN::pop_arg<std::string>(arglist);
    std::string objectName=functionN::pop_arg<std::string>(arglist);
    
    aDomain->register_symbol(objectName,
        aDomain->get_symbol_of_kind<proto_object>(className)->clone(),
        domain::ADOPT_SYMBOL | domain::REGISTER_FIRSTNTMP);
}

void delete_object(std::list<std::string> & arglist,domain * aDomain)
{
    std::string objectName=functionN::pop_arg<std::string>(arglist);
    aDomain->register_symbol(objectName,new null,domain::ADOPT_SYMBOL);
}

std::string get_symbol_value(std::list<std::string> & arglist,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(arglist);
    return aDomain->require_symbol(id)->value();
}

void register_base_functions(domain * aDomain)
{
    static functionV<void> func_assign_alias(&assign_alias);
    static functionV<void> func_push_alias(&push_alias);
    static functionV<void> func_pop_alias(&pop_alias);
    
    aDomain->register_symbol("alias",&func_assign_alias);
    aDomain->register_symbol("push",&func_push_alias);
    aDomain->register_symbol("pop",&func_pop_alias);

    static function1<void,std::string> func_throw(&_throw);
    static functionV<std::string> func_try(&_try);
    
    aDomain->register_symbol("throw",&func_throw);
    aDomain->register_symbol("try",&func_try);
    
    static functionV<void> func_new(&create_object);
    static functionV<void> func_delete(&delete_object);
    
    aDomain->register_symbol("new",&func_new);
    aDomain->register_symbol("delete",&func_delete);
    
    static functionV<std::string> func_value(&get_symbol_value);
    
    aDomain->register_symbol("value",&func_value);
}

void register_core_functions(domain * aDomain)
{
    register_base_functions(aDomain);
    
    static function2<int,int,int> func_math_add(&math_add);
    static function2<int,int,int> func_math_sub(&math_sub);
    static function2<int,int,int> func_math_mul(&math_mul);
    static function2<int,int,int> func_math_div(&math_div);
    static function2<int,int,int> func_math_mod(&math_mod);
    static function2<int,int,int> func_math_min(&math_min);
    static function2<int,int,int> func_math_max(&math_max);
    static function2<float,float,float> func_math_fadd(&math_fadd);
    static function2<float,float,float> func_math_fsub(&math_fsub);
    static function2<float,float,float> func_math_fmul(&math_fmul);
    static function2<float,float,float> func_math_fdiv(&math_fdiv);
    static function1<float,float> func_math_round(&math_round);
    static function1<float,float> func_math_ceil(&math_ceil);
    static function1<float,float> func_math_floor(&math_floor);
    static function1<int,int> func_math_abs(&math_abs);
    static function1<float,float> func_math_fabs(&math_fabs);
    
    aDomain->register_symbol("+",&func_math_add);
    aDomain->register_symbol("-",&func_math_sub);
    aDomain->register_symbol("*",&func_math_mul);
    aDomain->register_symbol("div",&func_math_div);
    aDomain->register_symbol("mod",&func_math_mod);
    aDomain->register_symbol("min",&func_math_min);
    aDomain->register_symbol("max",&func_math_max);
    aDomain->register_symbol("fadd",&func_math_fadd);
    aDomain->register_symbol("fsub",&func_math_fsub);
    aDomain->register_symbol("fmul",&func_math_fmul);
    aDomain->register_symbol("fdiv",&func_math_fdiv);
    aDomain->register_symbol("round",&func_math_round);
    aDomain->register_symbol("ceil",&func_math_ceil);
    aDomain->register_symbol("floor",&func_math_floor);
    aDomain->register_symbol("abs",&func_math_abs);
    aDomain->register_symbol("fabs",&func_math_fabs);

    static function2<bool,int,int> func_comp_equal(&comp_equal);
    static function2<bool,int,int> func_comp_nequal(&comp_nequal);
    static function2<bool,int,int> func_comp_gt(&comp_gt);
    static function2<bool,int,int> func_comp_gte(&comp_gte);
    static function2<bool,int,int> func_comp_lt(&comp_lt);
    static function2<bool,int,int> func_comp_lte(&comp_lte);
    
    aDomain->register_symbol("=",&func_comp_equal);
    aDomain->register_symbol("!=",&func_comp_nequal);
    aDomain->register_symbol(">",&func_comp_gt);
    aDomain->register_symbol(">=",&func_comp_gte);
    aDomain->register_symbol("<",&func_comp_lt);
    aDomain->register_symbol("<=",&func_comp_lte);
    
    static function1<bool,bool> func_logic_not(&logic_not);
    static function2<bool,alias_function &,alias_function &> func_logic_or(&logic_or);
    static function2<bool,alias_function &,alias_function &> func_logic_and(&logic_and);
    static function2<bool,bool,bool> func_logic_xor(&logic_xor);
    
    aDomain->register_symbol("!",&func_logic_not);
    aDomain->register_symbol("||",&func_logic_or);
    aDomain->register_symbol("&&",&func_logic_and);
    aDomain->register_symbol("^",&func_logic_xor);
    
    static functionV<std::string> func_control_if(&control_if);
    static functionV<void> func_control_loop(&control_loop);
    static functionV<void> func_control_while(&control_while);
    
    aDomain->register_symbol("if",&func_control_if);
    aDomain->register_symbol("loop",&func_control_loop);
    aDomain->register_symbol("while",&func_control_while);
    
    static functionV<bool> func_predicate_symbol(&predicate_symbol);
    
    aDomain->register_symbol("symbol?",&func_predicate_symbol);
    
    static function2<std::string,std::vector<std::string>,unsigned int> func_list_at(&list_at);
    static function1<int,std::vector<std::string> > func_list_len(&list_len);
    static function2<void,const std::list<std::string>,alias_function &> func_foreach(&foreach);
    
    aDomain->register_symbol("at",&func_list_at);
    aDomain->register_symbol("listlen",&func_list_len);
    aDomain->register_symbol("foreach",&func_foreach);
    
    static function2<bool,std::string,std::string> func_strcmp(&strcmp);
    static functionV<std::string> func_concat(&concat);
    static functionV<std::string> func_concatword(&concatword);
    static functionV<std::string> func_format(&format);
    static function2<std::vector<std::string>,const std::string &,const std::string &> func_split(&split);
    static function2<bool,const std::string &,const std::string &> func_match(&match);
    
    aDomain->register_symbol("strcmp",&func_strcmp);
    aDomain->register_symbol("concat",&func_concat);
    aDomain->register_symbol("concatword",&func_concatword);
    aDomain->register_symbol("format",&func_format);
    aDomain->register_symbol("split",&func_split);
    aDomain->register_symbol("match",&func_match);
    
    static const char * package="libcubescript 0.5";
    static const char * package_string="libcubescript";
    static const char * package_version="0.5";
    static const char * package_bugreport="graham.daws@gmail.com";
    
    static constant<const char *> const_engine(package);
    static constant<const char *> const_engine_string(package_string);
    static constant<const char *> const_engine_version(package_version);
    static constant<const char *> const_engine_bugreport(package_bugreport);

    aDomain->register_symbol("SCRIPT_ENGINE",&const_engine);
    aDomain->register_symbol("SCRIPT_ENGINE_STRING",&const_engine_string);
    aDomain->register_symbol("SCRIPT_ENGINE_VERSION",&const_engine_version);
    aDomain->register_symbol("SCRIPT_ENGINE_BUGREPORT",&const_engine_bugreport);
    
    static function0<time_t> func_time(&get_time_now);
    static function1<std::string,time_t> func_format_date(&format_date);
    static function1<std::string,time_t> func_format_time(&format_time);
    static function1<std::string,time_t> func_format_datetime(&format_datetime);
    static function1<const char *,time_t> func_get_weekday(&get_weekday);
    static function1<const char *,time_t> func_get_month(&get_month);
    
    aDomain->register_symbol("now",&func_time);
    aDomain->register_symbol("date",&func_format_date);
    aDomain->register_symbol("time",&func_format_time);
    aDomain->register_symbol("datetime",&func_format_datetime);
    aDomain->register_symbol("weekday",&func_get_weekday);
    aDomain->register_symbol("month",&func_get_month);
    
    static functionV<std::string> func_dump(&dump);
    
    aDomain->register_symbol("dump",&func_dump);
    
    static object_adaptor class_core(aDomain);
    aDomain->register_symbol("core",&class_core);
}

inline
void exec(std::list<std::string> & args,domain * context)
{
    std::string filename=functionN::pop_arg<std::string>(args);
    exec_file(filename,context);
}

bool filesystem_path(const std::string & filename)
{
    struct stat info;
    if(::stat(filename.c_str(),&info)==-1)
    {
        if(errno!=ENOENT) throw error_key("runtime.function.filesystem_path.unexpected_error");
        return false;
    }
    
    return true;
}

int get_file_modified(const std::string & filename)
{
    struct stat info;
    if(::stat(filename.c_str(),&info)==-1) throw error_key("runtime.function.file_modified");
#if __FreeBSD__
    return info.st_mtime;
#else
    return info.st_mtim.tv_sec;
#endif
}

void include(std::list<std::string> & args,domain * aDomain)
{
    const std::vector<std::string> & include_path=*aDomain->get_symbol_of_kind<variable<std::vector<std::string> > >("include_path");
    
    std::string filename=functionN::pop_arg<std::string>(args);
    std::string full_filename;
    
    for( std::vector<std::string>::const_iterator it=include_path.begin();
         it!=include_path.end();
         ++it )
    {
        full_filename = *it + "/" + filename;
        if(filesystem_path(full_filename)) break;
        full_filename.clear();
    }
    
    if(full_filename.empty()) throw error_key("runtime.function.include.file_not_found");
    
    exec_file(full_filename,aDomain);
}

inline
char * getenv(const std::string & name)
{
    return ::getenv(name.c_str());
}

inline
bool setenv(const std::string & name,const std::string & value)
{
    return ::setenv(name.c_str(),value.c_str(),true)==0;
}

inline void unsetenv(const std::string & name)
{
    ::unsetenv(name.c_str());
}

std::string system_exec(const std::string & filename)
{
    FILE * file=popen(filename.c_str(),"r");
    if(!file) throw error_key("runtime.function.system_exec.failed");
    
    std::ostringstream result;
    
    char readbuffer[255];
    while(!feof(file))
    {
        size_t readlen=fread(readbuffer,1,sizeof(readbuffer)-1,file);
        readbuffer[readlen]='\0';
        
        result<<readbuffer;
    }
    
    pclose(file);
    
    return result.str();
}

void register_system_functions(domain * aDomain)
{
    static functionV<void> func_exec(&exec);
    aDomain->register_symbol("exec",&func_exec);
    
    variable< std::vector<std::string> > * var_include_path=new variable<std::vector<std::string> >;
    aDomain->register_symbol("include_path",var_include_path,domain::ADOPT_SYMBOL);
    
    static functionV<void> func_include(&include);
    aDomain->register_symbol("include",&func_include);
    
    static function1<bool,const std::string &> func_path(&filesystem_path);
    static function1<int,const std::string &> func_file_modified(&get_file_modified);
    
    aDomain->register_symbol("path?",&func_path);
    aDomain->register_symbol("file_modified",&func_file_modified);
    
    static function1<char *,const std::string &> func_getenv(&getenv);
    aDomain->register_symbol("getenv",&func_getenv);
    
    static function2<bool,const std::string &,const std::string &> func_setenv(&setenv);
    aDomain->register_symbol("setenv",&func_setenv);
    
    static function1<void,const std::string &> func_unsetenv(&unsetenv);
    aDomain->register_symbol("unsetenv",&func_unsetenv);
    
    static function1<std::string,const std::string &> func_system(&system_exec);
    aDomain->register_symbol("system",&func_system);
    
    #ifdef OS_PLATFORM
        static const char * os_platform=OS_PLATFORM;
    #else
        static const char * os_platform="";
    #endif
    
    static constant<const char *> const_os_platform(os_platform);
    
    aDomain->register_symbol("OS_PLATFORM",&const_os_platform);
}

} //namespace runtime
} //namespace cubescript
