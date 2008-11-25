#include <sqlite3.h>
#include <cubescript.hpp>
#include <boost/bind.hpp>
#include <string>
#include "get_ticks.cpp"
#include <iostream>

class sqlite3db:public cubescript::proto_object
{
public:
    sqlite3db();
    sqlite3db(const sqlite3db &);
    cubescript::proto_object * clone()const{return new sqlite3db(*this);}
    void open(const std::string &);
    void eval(std::list<std::string> &,cubescript::domain *);
    void eval(const std::string &,const std::string &,cubescript::domain *);
    void close();
    void set_onerror_code(const std::string &);
    sqlite_int64 get_last_rowid()const;
    void set_busy_timeout(int);
    void set_onbusy_code(const std::string &);
private:
    int get_column_index(const std::string &,sqlite3_stmt *);
    std::string get_column_text(const std::string &,sqlite3_stmt *);
    void error_callback();
    void busy_callback(const std::string &);
    cubescript::domain * get_current_domain();
private:
    sqlite3 * m_db;
    std::string m_onerror;
    std::string m_onbusy;
    cubescript::domain * m_clsr;
    cubescript::function1<void,const std::string &> m_func_open;
    cubescript::functionV<void> m_func_eval;
    cubescript::function0<void> m_func_close;
    cubescript::function1<void,const std::string &> m_func_onerror;
    cubescript::function0<sqlite_int64> m_func_last_rowid;
    cubescript::function1<void,int> m_func_busy_timeout;
    cubescript::function1<void,const std::string &> m_func_onbusy;
};

sqlite3db::sqlite3db()
 :m_db(NULL),m_clsr(NULL),
  m_func_open(boost::bind(&sqlite3db::open,this,_1)),
  m_func_eval(boost::bind((void (sqlite3db::*)(std::list<std::string> &,cubescript::domain *))&sqlite3db::eval,this,_1,_2)),
  m_func_close(boost::bind(&sqlite3db::close,this)),
  m_func_onerror(boost::bind(&sqlite3db::set_onerror_code,this,_1)),
  m_func_last_rowid(boost::bind(&sqlite3db::get_last_rowid,this)),
  m_func_busy_timeout(boost::bind(&sqlite3db::set_busy_timeout,this,_1)),
  m_func_onbusy(boost::bind(&sqlite3db::set_onbusy_code,this,_1))
{
    add_member("open",&m_func_open);
    add_member("eval",&m_func_eval);
    add_member("close",&m_func_close);
    add_member("onerror",&m_func_onerror);
    add_member("last_rowid",&m_func_last_rowid);
    add_member("busy_timeout",&m_func_busy_timeout);
    add_member("onbusy",&m_func_onbusy);
}

sqlite3db::sqlite3db(const sqlite3db & src)
 :proto_object(src),
  m_db(src.m_db),
  m_onerror(src.m_onerror),
  m_onbusy(src.m_onbusy),
  m_clsr(NULL),
  m_func_open(boost::bind(&sqlite3db::open,this,_1)),
  m_func_eval(boost::bind((void (sqlite3db::*)(std::list<std::string> &,cubescript::domain *))&sqlite3db::eval,this,_1,_2)),
  m_func_close(boost::bind(&sqlite3db::close,this)),
  m_func_onerror(boost::bind(&sqlite3db::set_onerror_code,this,_1)),
  m_func_last_rowid(boost::bind(&sqlite3db::get_last_rowid,this)),
  m_func_busy_timeout(boost::bind(&sqlite3db::set_busy_timeout,this,_1)),
  m_func_onbusy(boost::bind(&sqlite3db::set_onbusy_code,this,_1))
{
    add_member("open",&m_func_open);
    add_member("eval",&m_func_eval);
    add_member("close",&m_func_close);
    add_member("onerror",&m_func_onerror);
    add_member("last_rowid",&m_func_last_rowid);
    add_member("busy_timeout",&m_func_busy_timeout);
    add_member("onbusy",&m_func_onbusy);
}

void sqlite3db::open(const std::string & filename)
{
    int openerr=sqlite3_open(filename.c_str(),&m_db);
    if(openerr!=SQLITE_OK) switch(openerr)
    {
        case SQLITE_PERM: throw cubescript::error_key("runtime.function.sqlite3_open.permission_denied");
        case SQLITE_BUSY: throw cubescript::error_key("runtime.function.sqlite3_open.db_locked");
        case SQLITE_READONLY: throw cubescript::error_key("runtime.function.sqlite3_open.db_readonly");
        default: throw cubescript::error_key("runtime.function.sqlite3_open.failed");
    }
}

void sqlite3db::eval(std::list<std::string> & arglist,cubescript::domain * aDomain)
{
    std::string statement=cubescript::functionN::pop_arg<std::string>(arglist);
    std::string rowcode("");
    if(!arglist.empty()) rowcode=cubescript::functionN::pop_arg<std::string>(arglist);
    eval(statement,rowcode,aDomain);
}

void sqlite3db::eval(const std::string & statement,const std::string & rowcode,cubescript::domain * aDomain)
{
    if(!m_db) throw cubescript::error_key("runtime.function.sqlite3_eval.db_closed");
    
    sqlite3_stmt * sqlstmt;
    const char * remaining=NULL;
    
    if(::sqlite3_prepare_v2(m_db,statement.c_str(),statement.length(),&sqlstmt,&remaining)!=SQLITE_OK)
    {
        // delay handling of busy error until after bindings to retrieve the resolved sql
        if(sqlite3_errcode(m_db) != SQLITE_BUSY) 
        {
            error_callback();
            throw cubescript::error_key("runtime.function.sqlite3_eval.sql_error");
        }
    }
    
    if(!sqlstmt) return;
    
    int paramcount=sqlite3_bind_parameter_count(sqlstmt);
    std::vector<std::string> bindings;
    bindings.reserve(paramcount);
    for(int i=1; i<=paramcount; i++)
    {
        const char * name=sqlite3_bind_parameter_name(sqlstmt,i);
        if(name)
        {
            std::string value = aDomain->require_symbol(&name[1])->value();
            
            std::string binding_entry = name + std::string("=") + value;
            bindings.push_back(binding_entry);
            
            char prefix=name[0];
            if(prefix=='?')
                sqlite3_bind_int(sqlstmt,i,cubescript::parse_type<int>(value));
            else
                sqlite3_bind_text(sqlstmt,i,value.c_str(),-1,SQLITE_TRANSIENT);
        }
    }
    
    cubescript::domain eval_context(aDomain,cubescript::domain::TEMPORARY_DOMAIN);
    m_clsr = &eval_context;
    
    cubescript::function1<std::string,const std::string &> func_column(boost::bind(&sqlite3db::get_column_text,this,_1,sqlstmt));
    cubescript::variable<bool> var_cancel;
    cubescript::variable<std::vector<std::string> > var_bindings;
    var_bindings.readonly(true);
    eval_context.register_symbol("column",&func_column);
    eval_context.register_symbol("cancel",&var_cancel);
    eval_context.register_symbol("bindings",&var_bindings);
    var_cancel=false;
    
    bool done=false;
    while(!done)
    {
        int i=sqlite3_step(sqlstmt);
        switch(i)
        {
            case SQLITE_ROW:
                exec_block(rowcode,&eval_context);
                done=var_cancel;
                break;
            case SQLITE_DONE:
                done=true;
                break;
            case SQLITE_BUSY:
            {
                busy_callback(sqlite3_sql(sqlstmt));
                
                sqlite3_finalize(sqlstmt);
                m_clsr = NULL;
                throw cubescript::error_key("runtime.function.sqlite3_eval.db_locked");
            }
            case SQLITE_ERROR:
            case SQLITE_MISUSE:
            default:
                error_callback();
                sqlite3_finalize(sqlstmt);
                m_clsr = NULL;
                throw cubescript::error_key("runtime.function.sqlite3_eval.eval_error");
        }
    }
    
    sqlite3_finalize(sqlstmt);
    m_clsr = NULL;
    
    if(*remaining) eval(remaining,rowcode,aDomain);
}

void sqlite3db::close()
{
    if(m_db)
    {
        sqlite3_close(m_db);
        m_db=NULL;
    }
}

void sqlite3db::set_onerror_code(const std::string & code)
{
    m_onerror=code;
}

sqlite_int64 sqlite3db::get_last_rowid()const
{
    if(!m_db) throw cubescript::error_key("runtime.function.sqlite3_last_rowid.db_closed");
    return sqlite3_last_insert_rowid(m_db);
}

int sqlite3db::get_column_index(const std::string & name,sqlite3_stmt * stmt)
{
    int count=sqlite3_column_count(stmt);
    for(int i=0; i<count; i++) if(name==sqlite3_column_name(stmt,i)) return i;
    throw cubescript::error_key("runtime.function.sqlite3_eval.column_not_found");
    return -1;
}

std::string sqlite3db::get_column_text(const std::string & name,sqlite3_stmt * stmt)
{
    const char * text=(const char *)sqlite3_column_text(stmt,get_column_index(name,stmt));
    if(!text) text="";
    return std::string(text);
}

void sqlite3db::error_callback()
{
    if(m_onerror.length() && m_db)
    {
        cubescript::alias_function func(m_onerror,get_current_domain());
        cubescript::arguments args;
        try
        {
            func.run(args & std::string((const char *)sqlite3_errmsg(m_db)));
        }catch(cubescript::error_key &){}
    }
}

void sqlite3db::busy_callback(const std::string & sql)
{
    if(m_onbusy.length() && m_db)
    {
        cubescript::alias_function func(m_onbusy,get_current_domain());
        cubescript::arguments args;
        try
        {
            func.run(args & sql);
        }catch(cubescript::error_key &){}
    }
}

cubescript::domain * sqlite3db::get_current_domain()
{
    return m_clsr ? m_clsr : get_parent_domain();
}

void sqlite3db::set_busy_timeout(int ms)
{
    sqlite3_busy_timeout(m_db,ms);
}

void sqlite3db::set_onbusy_code(const std::string & code)
{
    m_onbusy = code;
}

namespace cubescript{
void register_sqlite3(domain * aDomain)
{
    aDomain->register_symbol("sqlite3",new sqlite3db);
}
}
