#include "cube.h"
#include "game.h"
#include "hopmod.hpp"
#include "player_command.hpp"
#include "extapi.hpp"
#include "string_var.hpp"
#include <fungu/script.hpp>
#include <fungu/script/variable.hpp>
using namespace fungu;

static std::vector<script::any> player_kick_defargs;
static std::vector<script::any> changemap_defargs;

static void setup_default_arguments()
{
    player_kick_defargs.clear();
    player_kick_defargs.push_back(14400);
    player_kick_defargs.push_back(std::string("server"));
    player_kick_defargs.push_back(std::string(""));
    
    changemap_defargs.clear();
    changemap_defargs.push_back(const_string());
    changemap_defargs.push_back(-1);
}

void register_server_script_bindings(script::env & env)
{
    setup_default_arguments();
    
    //player-oriented functions
    script::bind_global_func<void (int,const char *)>(server::player_msg, FUNGU_OBJECT_ID("player_msg"), env);
    script::bind_global_func<bool (int,const char *)>(process_player_command, FUNGU_OBJECT_ID("process_player_command"), env);
    script::bind_global_func<void (int,int,const std::string &,const std::string &)>(server::kick, FUNGU_OBJECT_ID("kick"), env, &player_kick_defargs);
    script::bind_global_func<const char * (int)>(server::player_name, FUNGU_OBJECT_ID("player_name"), env);
    script::bind_global_func<const char * (int)>(server::player_team, FUNGU_OBJECT_ID("player_team"), env);
    script::bind_global_func<const char * (int)>(server::player_privilege, FUNGU_OBJECT_ID("player_priv"), env);
    script::bind_global_func<int (int)>(server::player_privilege_code, FUNGU_OBJECT_ID("player_priv_code"), env);
    script::bind_global_const((int)PRIV_NONE, FUNGU_OBJECT_ID("PRIV_NONE"), env);
    script::bind_global_const((int)PRIV_MASTER, FUNGU_OBJECT_ID("PRIV_MASTER"), env);
    script::bind_global_const((int)PRIV_ADMIN, FUNGU_OBJECT_ID("PRIV_ADMIN"), env);
    script::bind_global_func<int (int)>(server::player_id, FUNGU_OBJECT_ID("player_id"), env);
    script::bind_global_func<int (int)>(server::player_sessionid, FUNGU_OBJECT_ID("player_sessionid"), env);
    script::bind_global_func<int (int)>(server::player_ping, FUNGU_OBJECT_ID("player_ping"), env);
    script::bind_global_func<int (int)>(server::player_lag, FUNGU_OBJECT_ID("player_lag"), env);
    script::bind_global_func<const char * (int)>(server::player_ip, FUNGU_OBJECT_ID("player_ip"), env);
    script::bind_global_func<int (int)>(server::player_iplong, FUNGU_OBJECT_ID("player_iplong"), env);
    script::bind_global_func<const char * (int)>(server::player_status, FUNGU_OBJECT_ID("player_status"), env);
    script::bind_global_func<int (int)>(server::player_status_code, FUNGU_OBJECT_ID("player_status_code"), env);
    script::bind_global_func<int (int)>(server::player_frags, FUNGU_OBJECT_ID("player_frags"), env);
    script::bind_global_func<int (int)>(server::player_deaths, FUNGU_OBJECT_ID("player_deaths"), env);
    script::bind_global_func<int (int)>(server::player_suicides, FUNGU_OBJECT_ID("player_suicides"), env);
    script::bind_global_func<int (int)>(server::player_teamkills, FUNGU_OBJECT_ID("player_teamkills"), env);
    script::bind_global_func<int (int)>(server::player_damage, FUNGU_OBJECT_ID("player_damage"), env);
    script::bind_global_func<int (int)>(server::player_damagewasted, FUNGU_OBJECT_ID("player_damagewasted"), env);
    script::bind_global_func<int (int)>(server::player_maxhealth, FUNGU_OBJECT_ID("player_maxhealth"), env);
    script::bind_global_func<int (int)>(server::player_health, FUNGU_OBJECT_ID("player_health"), env);
    script::bind_global_func<int (int)>(server::player_gun, FUNGU_OBJECT_ID("player_gun"), env);
    script::bind_global_func<int (int)>(server::player_hits, FUNGU_OBJECT_ID("player_hits"), env);
    script::bind_global_func<int (int)>(server::player_shots, FUNGU_OBJECT_ID("player_shots"), env);
    script::bind_global_func<int (int)>(server::player_accuracy, FUNGU_OBJECT_ID("player_accuracy"), env);
    script::bind_global_func<int (int)>(server::player_timeplayed, FUNGU_OBJECT_ID("player_timeplayed"), env);
    script::bind_global_func<int (int)>(server::player_win, FUNGU_OBJECT_ID("player_win"), env);
    script::bind_global_func<void (int)>(server::player_slay, FUNGU_OBJECT_ID("player_slay"), env);
    script::bind_global_func<bool (int,const char *)>(server::player_changeteam, FUNGU_OBJECT_ID("changeteam"), env);
    script::bind_global_func<int (int)>(server::player_bots, FUNGU_OBJECT_ID("player_bots"), env);
    script::bind_global_func<int (int)>(server::player_authreq, FUNGU_OBJECT_ID("player_authreq"), env);
    script::bind_global_func<int (int)>(server::player_rank, FUNGU_OBJECT_ID("player_rank"), env);
    script::bind_global_func<bool (int)>(server::player_isbot, FUNGU_OBJECT_ID("player_isbot"), env);
    script::bind_global_func<int (int)>(server::player_mapcrc, FUNGU_OBJECT_ID("player_mapcrc"), env);
    
    script::bind_global_const((int)CS_ALIVE, FUNGU_OBJECT_ID("ALIVE"), env);
    script::bind_global_const((int)CS_DEAD, FUNGU_OBJECT_ID("DEAD"), env);
    script::bind_global_const((int)CS_SPAWNING, FUNGU_OBJECT_ID("SPAWNING"), env);
    script::bind_global_const((int)CS_LAGGED, FUNGU_OBJECT_ID("LAGGED"), env);
    script::bind_global_const((int)CS_SPECTATOR, FUNGU_OBJECT_ID("SPECTATOR"), env);
    script::bind_global_const((int)CS_EDITING, FUNGU_OBJECT_ID("EDITING"), env);
    script::bind_global_func<int (int)>(server::player_connection_time, FUNGU_OBJECT_ID("player_connection_time"), env);
    script::bind_global_func<void (int)>(server::player_spec, FUNGU_OBJECT_ID("spec"), env);
    script::bind_global_func<void (int)>(server::player_unspec, FUNGU_OBJECT_ID("unspec"), env);
    script::bind_global_func<void ()>(server::unsetmaster, FUNGU_OBJECT_ID("unsetmaster"), env);
    script::bind_global_func<void (int)>(server::server_setmaster, FUNGU_OBJECT_ID("setmaster"), env);
    script::bind_global_func<void (int)>(server::server_setadmin, FUNGU_OBJECT_ID("setadmin"), env);
    script::bind_global_func<void (int)>(server::set_invadmin, FUNGU_OBJECT_ID("set_invadmin"), env);
    script::bind_global_func<void (int)>(server::unset_invadmin, FUNGU_OBJECT_ID("unset_invadmin"), env);
    
    script::bind_global_func<std::vector<int> ()>(server::cs_player_list, FUNGU_OBJECT_ID("players"), env);
    script::bind_global_func<std::vector<int> ()>(server::cs_spec_list, FUNGU_OBJECT_ID("spectators"), env);
    script::bind_global_func<std::vector<int> ()>(server::cs_bot_list, FUNGU_OBJECT_ID("bots"), env);
    register_lua_function(&server::lua_player_list, "players");
    register_lua_function(&server::lua_spec_list, "spectators");
    register_lua_function(&server::lua_bot_list, "bots");
    
    register_lua_function(&server::lua_gamemodeinfo, "gengamemodeinfo");
    
    //team-oriented functions
    script::bind_global_func<void (const char*,const char*)>(server::team_msg,FUNGU_OBJECT_ID("team_msg"), env);
    script::bind_global_func<std::vector<std::string> ()>(server::get_teams, FUNGU_OBJECT_ID("teams"), env);
    register_lua_function(&server::lua_team_list, "teams");
    script::bind_global_func<int (const char *)>(server::get_team_score, FUNGU_OBJECT_ID("team_score"), env);
    script::bind_global_func<int (const char *)>(server::team_win, FUNGU_OBJECT_ID("team_win"), env);
    script::bind_global_func<int (const char *)>(server::team_draw, FUNGU_OBJECT_ID("team_draw"), env);
    script::bind_global_func<std::vector<int> (const char *)>(server::get_team_players, FUNGU_OBJECT_ID("team_players"), env);
    register_lua_function(&server::lua_team_players, "team_players");
    
    //server-oriented functions and variables
    script::bind_global_func<void ()>(server::reload_hopmod, FUNGU_OBJECT_ID("reloadscripts"), env);
    script::bind_global_func<void (bool)>(server::pausegame,FUNGU_OBJECT_ID("pausegame"),env);
    script::bind_global_func<void (const char *)>(server::sendservmsg, FUNGU_OBJECT_ID("msg"), env);
    script::bind_global_func<void ()>(server::shutdown, FUNGU_OBJECT_ID("shutdown"), env);
    script::bind_global_func<void (int)>(server::changetime, FUNGU_OBJECT_ID("changetime"), env);
    script::bind_global_func<void (const char *,const char *,int)>(server::changemap,FUNGU_OBJECT_ID("changemap"), env, &changemap_defargs);
    script::bind_global_func<bool (const char *)>(server::writebanlist, FUNGU_OBJECT_ID("writebanlist"), env);
    script::bind_global_func<bool (const char *)>(server::loadbanlist, FUNGU_OBJECT_ID("loadbanlist"), env);
    script::bind_global_func<void (const char *)>(server::addpermban, FUNGU_OBJECT_ID("permban"), env);
    script::bind_global_func<void (const char *)>(server::unsetban, FUNGU_OBJECT_ID("unsetban"), env);
    script::bind_global_func<void ()>(server::clearbans, FUNGU_OBJECT_ID("clearbans"), env);
    script::bind_global_func<void ()>(exec_restarter, FUNGU_OBJECT_ID("exec_restarter"), env);
    script::bind_global_func<void (int)>(server::delbot, FUNGU_OBJECT_ID("delbot"), env);
    script::bind_global_func<int ()>(server::recorddemo, FUNGU_OBJECT_ID("recorddemo"), env);
    script::bind_global_func<void ()>(server::enddemorecord, FUNGU_OBJECT_ID("stopdemo"), env);
    
    script::bind_global_var(server::serverdesc, FUNGU_OBJECT_ID("servername"), env);
    script::bind_global_ro_var(server::smapname, FUNGU_OBJECT_ID("map"), env);
    script::bind_global_var(server::serverpass, FUNGU_OBJECT_ID("server_password"), env);
    script::bind_global_wo_var(server::masterpass, FUNGU_OBJECT_ID("admin_password"), env);
    script::bind_global_ro_var(server::currentmaster, FUNGU_OBJECT_ID("master"), env);
    script::bind_global_ro_var(server::minremain, FUNGU_OBJECT_ID("timeleft"), env);
    script::bind_global_ro_var(totalmillis, FUNGU_OBJECT_ID("uptime"), env);
    script::bind_global_var(maxclients, FUNGU_OBJECT_ID("maxplayers"), env);
    script::bind_global_var(serverip, FUNGU_OBJECT_ID("serverip"), env);
    script::bind_global_var(serverport, FUNGU_OBJECT_ID("serverport"), env);
    script::bind_global_var(mastername, FUNGU_OBJECT_ID("masterserver"), env);
    script::bind_global_var(allowupdatemaster, FUNGU_OBJECT_ID("updatemaster"), env);
    script::bind_global_var(server::next_gamemode, FUNGU_OBJECT_ID("next_mode"), env);
    script::bind_global_var(server::next_mapname, FUNGU_OBJECT_ID("next_map"), env);
    script::bind_global_var(server::next_gametime, FUNGU_OBJECT_ID("next_gametime"), env);
    script::bind_global_var(server::reassignteams, FUNGU_OBJECT_ID("reassignteams"), env);
    script::bind_global_funvar<int>(server::getplayercount, FUNGU_OBJECT_ID("playercount"), env);
    script::bind_global_funvar<int>(server::getspeccount, FUNGU_OBJECT_ID("speccount"), env);
    script::bind_global_funvar<int>(server::getbotcount, FUNGU_OBJECT_ID("botcount"), env);
    script::bind_global_var(server::aiman::botlimit, FUNGU_OBJECT_ID("botlimit"), env);
    script::bind_global_var(server::aiman::botbalance, FUNGU_OBJECT_ID("botbalance"), env);
    script::bind_global_func<void (bool)>(server::enable_master_auth, FUNGU_OBJECT_ID("use_master_auth"), env);
    script::bind_global_func<bool ()>(server::using_master_auth, FUNGU_OBJECT_ID("using_master_auth"), env);
    script::bind_global_funvar<const char *>(server::gamemodename, FUNGU_OBJECT_ID("gamemode"), env);
    
    script::bind_global_var(server::allow_mm_veto, FUNGU_OBJECT_ID("allow_mm_veto"), env);
    script::bind_global_var(server::allow_mm_locked, FUNGU_OBJECT_ID("allow_mm_locked"), env);
    script::bind_global_var(server::allow_mm_private, FUNGU_OBJECT_ID("allow_mm_private"), env);
    
    script::bind_global_var(server::allow_item[I_SHELLS-I_SHELLS], FUNGU_OBJECT_ID("allow_shells"), env);
    script::bind_global_var(server::allow_item[I_BULLETS-I_SHELLS], FUNGU_OBJECT_ID("allow_bullets"), env);
    script::bind_global_var(server::allow_item[I_ROCKETS-I_SHELLS], FUNGU_OBJECT_ID("allow_rockets"), env);
    script::bind_global_var(server::allow_item[I_ROUNDS-I_SHELLS], FUNGU_OBJECT_ID("allow_rounds"), env);
    script::bind_global_var(server::allow_item[I_GRENADES-I_SHELLS], FUNGU_OBJECT_ID("allow_grenades"), env);
    script::bind_global_var(server::allow_item[I_CARTRIDGES-I_SHELLS], FUNGU_OBJECT_ID("allow_cartridges"), env);
    script::bind_global_var(server::allow_item[I_HEALTH-I_SHELLS], FUNGU_OBJECT_ID("allow_health"), env);
    script::bind_global_var(server::allow_item[I_BOOST-I_SHELLS], FUNGU_OBJECT_ID("allow_healthboost"), env);
    script::bind_global_var(server::allow_item[I_GREENARMOUR-I_SHELLS], FUNGU_OBJECT_ID("allow_greenarmour"), env);
    script::bind_global_var(server::allow_item[I_YELLOWARMOUR-I_SHELLS], FUNGU_OBJECT_ID("allow_yellowarmour"), env);
    script::bind_global_var(server::allow_item[I_QUAD-I_SHELLS], FUNGU_OBJECT_ID("allow_quad"), env);
    script::bind_global_var(server::reservedslots, FUNGU_OBJECT_ID("reservedslots"), env);
    
    //script_socket functions
    script::bind_global_func<bool ()>(script_socket_supported, FUNGU_OBJECT_ID("script_socket_supported?"), env);
    script::bind_global_func<bool (unsigned short,const char *)>(open_script_socket, FUNGU_OBJECT_ID("script_socket_server"), env);
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
    
    script::bind_global_var(server::sv_text_hit_length, FUNGU_OBJECT_ID("flood_protect_text"), env);
    script::bind_global_var(server::sv_sayteam_hit_length, FUNGU_OBJECT_ID("flood_protect_sayteam"), env);
    script::bind_global_var(server::sv_mapvote_hit_length, FUNGU_OBJECT_ID("flood_protect_mapvote"), env);
    script::bind_global_var(server::sv_switchname_hit_length, FUNGU_OBJECT_ID("flood_protect_switchname"), env);
    script::bind_global_var(server::sv_switchteam_hit_length, FUNGU_OBJECT_ID("flood_protect_switchteam"), env);
    
    //utility
    script::bind_global_func<std::string (const char *)>(resolve_hostname, FUNGU_OBJECT_ID("gethostbyname"), env);
    
    script::bind_global_var(tx_bytes, FUNGU_OBJECT_ID("tx_bytes"), env);
    script::bind_global_var(rx_bytes, FUNGU_OBJECT_ID("rx_bytes"), env);
    script::bind_global_var(tx_packets, FUNGU_OBJECT_ID("tx_packets"), env);
    script::bind_global_var(rx_packets, FUNGU_OBJECT_ID("rx_packets"), env);
    
    register_lua_function(&server::lua_genkeypair, "genkeypair");
    register_lua_function(&server::lua_genchallenge, "genchallenge");
    register_lua_function(&server::lua_checkchallenge, "checkchallenge");
    register_lua_function(&server::lua_freechalanswer, "freechalanswer");
    script::bind_global_func<void (int)>(server::delegateauth, FUNGU_OBJECT_ID("delegateauth"), env);
    script::bind_global_func<void (int,const char *)>(server::relayauthanswer, FUNGU_OBJECT_ID("relayauthanswer"), env);
    script::bind_global_func<void (int,const char *)>(server::sendauthreq, FUNGU_OBJECT_ID("sendauthreq"), env);
    script::bind_global_func<void (int,const char *)>(server::sendauthchallenge, FUNGU_OBJECT_ID("sendauthchallenge"), env);
    script::bind_global_func<void (int)> (server::signal_auth_success, FUNGU_OBJECT_ID("signal_auth_success"), env);
    script::bind_global_func<void (int)> (server::signal_auth_failure, FUNGU_OBJECT_ID("signal_auth_failure"), env);
}
