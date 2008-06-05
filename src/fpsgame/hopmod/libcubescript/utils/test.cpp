
#include "cubescript.hpp"
#include "script_pipe.hpp"
#include "sleep.hpp"

int main()
{
    cubescript::domain local;
    cubescript::runtime::register_core_functions(&local);
    cubescript::runtime::register_system_functions(&local);
    
    script_pipe_service sp_server(std::cerr);
    sp_server.register_function(&local);
    
    sleep_service sleep_server;
    sleep_server.register_function(&local);
    
    cubescript::exec_block("script_pipe mkfifochan [serverexec 777]",&local);
    //cubescript::exec_block("script_pipe [/home/graham/scripts/test.sh] [test]",&local);
    //cubescript::exec_block("script_pipe stderrtest []",&local);
    
    while(true)
    {
        sleep_server.run();
        sp_server.run();
        usleep(5000);
    }
    
    return 0;
}
