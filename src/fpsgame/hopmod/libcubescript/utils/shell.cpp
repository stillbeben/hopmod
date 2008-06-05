
#include "cubescript.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

void_ echo(const std::string & str)
{
    std::cout<<str<<std::endl;
}

class foo:public cubescript::proto_object
{
public:
    foo()
     :m_func_bar(boost::bind(&foo::bar,this))
    {
        add_member("bar",&m_func_bar);
    }
    
    foo(const foo & src)
     :cubescript::proto_object(src),
      m_func_bar(boost::bind(&foo::bar,this))
    {
        add_member("bar",&m_func_bar);
    }
    
    cubescript::proto_object * clone()const{return new foo(*this);}
private:
    cubescript::function0<std::string> m_func_bar;
    std::string bar()const{return "hello world";}
};

int main()
{
    cubescript::domain shell_domain;
    
    cubescript::runtime::register_core_functions(&shell_domain);
    cubescript::runtime::register_system_functions(&shell_domain);
    
    cubescript::bind(echo,"echo",&shell_domain);
    
    cubescript::exec_file("autoexec.cfg",&shell_domain);
    
    shell_domain.register_symbol("foo",new foo);
    
    while(!std::cin.eof())
    {
        cubescript::cons::expression * code=new cubescript::cons::expression;
        code->set_domain(&shell_domain);
        
        std::cout<<"> ";
        
        bool done=false;
        
        try
        {
            while(!done)
            {
                std::string line;
                std::getline(std::cin,line);
                line+="\r\n";
                std::stringstream reader(line);
                done=code->parse(reader);
            }
            
            code->eval();
        }
        catch(cubescript::script_error<cubescript::symbol_error> & e)
        {
            std::cout<<"symbol error in "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<": "<<e.get_id()<<std::endl;
        }
        catch(cubescript::script_error<cubescript::error_key> & e)
        {
            std::cout<<"error in "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<std::endl;
        }
        catch(cubescript::symbol_error & e)
        {
            std::cout<<"symbol error: "<<e.what()<<": "<<e.get_id()<<std::endl;
        }
        catch(cubescript::error_key & e)
        {
            std::cout<<"error "<<e.what()<<std::endl;
        }
        
        delete code;
    }
    
    return 0;
}
