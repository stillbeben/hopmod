#include "cube.h"
#include "game.h"
#include "hopmod.hpp"
#include "player_command.hpp"
#include "extapi.hpp"
#include <fungu/script.hpp>
#include <fungu/script/variable.hpp>

using namespace fungu;

namespace fungu{
namespace script{
template<>
class variable<string>:public env::object
{
public:
    variable(string & str)
     :m_string(str)
    {
        
    }
    
    object_type get_object_type()const
    {
        return DATA_OBJECT;
    }
    
    void assign(const any & value)
    {
        const_string tmp = value.to_string();
        std::size_t len = tmp.length();
        if(len > MAXSTRLEN - 1) throw error(INVALID_VALUE,boost::make_tuple(std::string("string is too long")));
        strncpy(m_string, tmp.begin(), len);
        m_string[len] = '\0';
    }
    
    result_type apply(apply_arguments & args,env::frame *)
    {
        assign(args.safe_front());
        args.pop_front();
        try{return value();}
        catch(error){return any::null_value();}
    }
    
    result_type value()
    {
        return const_string(m_string,m_string+strlen(m_string)-1);
    }
private:
    string & m_string;
};
} //namespace script
} //namespace fungu

static std::vector<script::any> player_kick_defargs;

static void setup_default_arguments()
{
    player_kick_defargs.clear();
    player_kick_defargs.push_back(14400);
    player_kick_defargs.push_back(const_string(FUNGU_LITERAL_STRING("server")));
    player_kick_defargs.push_back(const_string());
    
}

void register_server_script_bindings(script::env & env)
{
    setup_default_arguments();
    
    //player-oriented functions
    script::bind_global_func<void (int,const char *)>(server::player_msg, FUNGU_OBJECT_ID("player_msg"), env);
    script::bind_global_func<bool (int,const char *)>(process_player_command, FUNGU_OBJECT_ID("process_player_command"), env);
    script::bind_global_func<void (int,int,const std::string &,const std::string &)>(server::kick, FUNGU_OBJECT_ID("player_kick"), env, &player_kick_defargs);
    
    //server-oriented functions and variable
    script::bind_global_func<void (bool)>(server::pausegame,FUNGU_OBJECT_ID("pausegame"),env);
    script::bind_global_func<void (const char *)>(server::sendservmsg, FUNGU_OBJECT_ID("msg"), env);
    script::bind_global_func<void ()>(server::shutdown, FUNGU_OBJECT_ID("shutdown"), env);
    script::bind_global_func<void (int)>(server::changetime, FUNGU_OBJECT_ID("changetime"), env);
    script::bind_global_func<void ()>(server::clearbans, FUNGU_OBJECT_ID("clearbans"), env);
    
    script::bind_global_var(server::serverdesc, FUNGU_OBJECT_ID("servername"), env);
    script::bind_global_ro_var(server::smapname, FUNGU_OBJECT_ID("map"), env);
    script::bind_global_var(server::serverpass, FUNGU_OBJECT_ID("server_password"), env);
    script::bind_global_wo_var(server::masterpass, FUNGU_OBJECT_ID("admin_password"), env);
    script::bind_global_ro_var(server::currentmaster, FUNGU_OBJECT_ID("master"), env);
    script::bind_global_ro_var(server::minremain, FUNGU_OBJECT_ID("timeleft"), env);
    script::bind_global_ro_var(totalmillis, FUNGU_OBJECT_ID("uptime"), env);
    script::bind_global_var(maxclients, FUNGU_OBJECT_ID("maxplayers"), env);
    script::bind_global_var(server::next_gamemode, FUNGU_OBJECT_ID("next_mode"), env);
    script::bind_global_var(server::next_mapname, FUNGU_OBJECT_ID("next_map"), env);
    script::bind_global_var(server::next_gametime, FUNGU_OBJECT_ID("next_gametime"), env);
    
    //script_socket functions
    script::bind_global_func<bool ()>(script_socket_supported, FUNGU_OBJECT_ID("script_socket_supported?"), env);
    script::bind_global_func<bool (unsigned short)>(open_script_socket, FUNGU_OBJECT_ID("script_socket_server"), env);
    script::bind_global_func<void ()>(close_script_socket, FUNGU_OBJECT_ID("close_script_socket_server"), env);
    
    //geoip functions
    script::bind_global_func<bool ()>(geoip_supported, FUNGU_OBJECT_ID("geoip_supported?"), env);
    script::bind_global_func<bool (const char *)>(load_geoip_database, FUNGU_OBJECT_ID("load_geoip_database"), env);
    script::bind_global_func<const char * (const char *)>(ip_to_country, FUNGU_OBJECT_ID("ip_to_country"), env);
    script::bind_global_func<const char * (const char *)>(ip_to_country_code, FUNGU_OBJECT_ID("ip_to_country_code"), env);
    
    script::bind_global_var(server::mastermode, FUNGU_OBJECT_ID("mastermode"), env);//TODO change to managed var
    script::bind_global_const((int)MM_OPEN, FUNGU_OBJECT_ID("MM_OPEN"), env);
    script::bind_global_const((int)MM_VETO, FUNGU_OBJECT_ID("MM_VETO"), env);
    script::bind_global_const((int)MM_LOCKED, FUNGU_OBJECT_ID("MM_LOCKED"), env);
    script::bind_global_const((int)MM_PRIVATE, FUNGU_OBJECT_ID("MM_PRIVATE"), env);
    script::bind_global_const((int)MM_PASSWORD, FUNGU_OBJECT_ID("MM_PASSWORD"), env);
}
