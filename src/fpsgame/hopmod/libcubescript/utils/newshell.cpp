#include <iostream>
#include <string>
#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <asio.hpp>

#include "cubescript.hpp"

class cmdline_io_service:public asio::io_service::service
{
public:
    cmdline_io_service(asio::io_service & owner)
    : asio::io_service::service(owner),
      m_internal(),
      m_work(new asio::io_service::work(m_internal)),
      m_thread(boost::bind(&asio::io_service::run,&m_internal))
    {
        
    }
    
    ~cmdline_io_service()
    {
        m_internal.stop();
        m_thread.join();
    }
    
    void shutdown_service()
    {
        
    }
    
    template<typename CompletionHandler>
    void async_output(const std::string & text,CompletionHandler handler)
    {
        m_internal.post(boost::bind(&cmdline_io_service::do_output,
            this,boost::shared_ptr<asio::io_service::work>(new asio::io_service::work(get_io_service())),text,handler));
    }
    
    template<typename CompletionHandler>
    void async_input(CompletionHandler handler)
    {
        m_internal.post(boost::bind(&cmdline_io_service::do_input,this,
            boost::shared_ptr<asio::io_service::work>(new asio::io_service::work(get_io_service())),
            handler));
    }
    
    void output(const std::string & text)
    {   
        std::cout<<text;
    }
private:
    void do_output(boost::shared_ptr<asio::io_service::work> work,const std::string & text,boost::function0<void> handler)
    {
        std::cout<<text<<std::endl;
        get_io_service().post(handler);
    }
    
    void do_input(boost::shared_ptr<asio::io_service::work> work,boost::function1<void,std::string> handler)
    {
        std::string line;
        std::getline(std::cin,line);
        get_io_service().post(boost::bind(handler,line));
    }
    
    asio::io_service m_internal;
    boost::scoped_ptr<asio::io_service::work> m_work;
    boost::thread m_thread;
};

class script_service:public asio::io_service::service
{
private:
    class async_eval_expression:public asio::io_service::work
    {
    public:
        async_eval_expression(asio::io_service & owner,cubescript::cons::expression & exp,cubescript::construct::eval_completion_t handler)
         :asio::io_service::work(owner),m_exp(exp),m_result_handler(handler)
        {
            owner.post(boost::bind(&async_eval_expression::exec,this));
        }
    private:
        void exec()
        {
           m_exp.async_eval(boost::bind(&async_eval_expression::result_handler_proxy,this,_1,_2));
        }
        
        void result_handler_proxy(cubescript::exception *e,std::string result)
        {
            m_result_handler(e,result);
            //delete this;
        }
        
        cubescript::cons::expression & m_exp;
        cubescript::construct::eval_completion_t m_result_handler;
    };
public:
    script_service(asio::io_service & owner):asio::io_service::service(owner)
    {
        
    }
    
    void shutdown_service()
    {
        
    }
    
    template<typename ResultHandler>
    void async_eval(cubescript::cons::expression & exp,ResultHandler handler)
    {
        new async_eval_expression(get_io_service(),exp,handler);
    }
};

class application
{
public:
    application(asio::io_service & io_base,cmdline_io_service * user_io,script_service * scripting)
    :m_io(io_base),m_user_io(user_io),m_scripting(scripting)
    {
        m_exp=NULL;
        prompt_user();
        
        cubescript::function1<cubescript::void_t,std::string> fn_echo(boost::bind(&application::echo,this,_1));
        cubescript::bind<cubescript::function1<cubescript::void_t,std::string> >(fn_echo,"echo",&m_local);
        
        cubescript::function2<cubescript::void_t,std::string,std::string> fn_connect(boost::bind(&application::connect,this,_1,_2));
        cubescript::bind<cubescript::function2<cubescript::void_t,std::string,std::string> >(fn_connect,"connect",&m_local);
        
    }
private:
    cubescript::void_t echo(std::string out)
    {
        out+="\r\n";
        m_user_io->output(out);
    }
    
    cubescript::void_t connect(std::string hostname,std::string servicename)
    {
        asio::ip::tcp::resolver::query query(hostname,servicename);
        asio::ip::tcp::resolver resolver(m_io);
        asio::ip::tcp::resolver::iterator ep_it;
        try
        {
            ep_it=resolver.resolve(query);
        }
        catch(asio::system_error error)
        {
            throw cubescript::runtime_error("runtime.network.hostname.unresolved");
        }
        asio::ip::tcp::resolver::iterator ep_end;
        
        asio::ip::tcp::socket * socket=new asio::ip::tcp::socket(m_io);
        
        asio::error_code error = asio::error::host_not_found;
        while (error && ep_it != ep_end)
        {
            socket->close();
            socket->connect(*ep_it++,error);
        }
        if(error) throw cubescript::runtime_error("runtime.network.host.no_service");
        
    }
    
    void process_result(cubescript::exception * e,std::string)
    {
        if(e)
        {
            std::string output=e->what();
            output+="\r\n";
            m_user_io->output(output);
        }
        
        
        std::cout<<(void *)m_exp<<std::endl;
        
        delete m_exp;
        m_exp=NULL;
        
        prompt_user();
    }

    void process_input(std::string line)
    {
        if(!m_exp)
        {
            m_exp=new cubescript::cons::expression;
            m_exp->set_domain(&m_local);
        }
        
        std::stringstream input;
        input<<line<<"\r\n";
        
        if(m_exp->parse(input))
            m_scripting->async_eval(*m_exp,boost::bind(&application::process_result,this,_1,_2));
        else get_input();
    }
    
    void prompt_user()
    {
        m_user_io->output("> ");
        get_input();
    }
    
    void get_input()
    {
        boost::function1<void,std::string> fn;
        fn=boost::bind(&application::process_input,this,_1);
        m_user_io->async_input(fn);
    }
    
private:
    asio::io_service & m_io;
    cmdline_io_service * m_user_io;
    script_service * m_scripting;

    cubescript::cons::expression * m_exp;
    cubescript::domain_chain m_local;
};

int main()
{
    asio::io_service base_service;
    cmdline_io_service user(base_service);
    cubescript::runtime::register_core_functions();
    script_service script(base_service);
    application app(base_service,&user,&script);
    base_service.run();
    return 0;
}
