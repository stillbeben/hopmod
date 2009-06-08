#include "signals.hpp"
#include "scripting.hpp"

#include <fungu/script/slot_factory.hpp>
#include <fungu/script/lua/lua_function.hpp>
using namespace fungu;

static script::env * env = NULL;
static script::slot_factory slots;

boost::signal<void ()> signal_started;
boost::signal<void ()> signal_shutdown;
boost::signal<void ()> signal_reloadhopmod;

boost::signal<void (int)> signal_connect;
boost::signal<void (int,const char *)> signal_disconnect;
boost::signal<void (const char *,const char *)> signal_failedconnect;
boost::signal<void (int)> signal_active;
boost::signal<void (int,const char *,const char *)> signal_rename;
boost::signal<void (int)> signal_renaming;
boost::signal<void (int,const char *,const char *)> signal_reteam;
boost::signal<int (int,const char *,const char *), proceed> signal_chteamrequest;
boost::signal<void (int,int,std::string,std::string)> signal_kick;
boost::signal<int (int,const char *), proceed> signal_text;
boost::signal<int (int,const char *), proceed> signal_sayteam;
boost::signal<void ()> signal_intermission;
boost::signal<void ()> signal_finishedgame;
boost::signal<void (int)> signal_timeupdate;
boost::signal<void (const char *,const char *)> signal_mapchange;
boost::signal<int (int,const char *,const char *), proceed> signal_mapvote;
boost::signal<void ()> signal_setnextgame;
boost::signal<void ()> signal_gamepaused;
boost::signal<void ()> signal_gameresumed;
boost::signal<void (const char *,const char *)> signal_setmastermode;
boost::signal<void (int,int)> signal_spectator;
boost::signal<void (int,const char *,int)> signal_setmaster;
boost::signal<void (int,int)> signal_teamkill;
boost::signal<void (int,const char *,const char *,bool)> signal_auth;
boost::signal<void (int,const char *,const char *)> signal_authreq;
boost::signal<void (int,const char *)> signal_authrep;
boost::signal<void (int,int,int)> signal_addbot;
boost::signal<void (int)> signal_delbot;
boost::signal<void (int)> signal_botleft;
boost::signal<void (int,const char *)> signal_beginrecord;
boost::signal<void (int,int)> signal_endrecord;
boost::signal<void (int)> signal_mapcrcfail;
boost::signal<void (const char *,const char *)> signal_votepassed;

boost::signal<void ()> signal_rootserver_failedconnect;

static script::any proceed_error_handler(script::error_trace * errinfo)
{
    report_script_error(errinfo);
    return true;
}

static script::any normal_error_handler(script::error_trace * errinfo)
{
    report_script_error(errinfo);
    return script::any::null_value();
}

static int lua_event_handler_function(lua_State * L)
{
    int argc = lua_gettop(L);
    if(argc < 2) return luaL_error(L,"missing arguments");
    const char * name = lua_tostring(L,1);
    if(!name) return luaL_argerror(L, 1,"expecting string type for name");
    if(lua_type(L,2) != LUA_TFUNCTION) return luaL_argerror(L,2,"expecting function type for handler");
    lua_pushvalue(L,2);
    
    script::env::object::shared_ptr luaFunctionObject = new script::lua::lua_function(L);
    luaFunctionObject->set_adopted();
    
    int handle = slots.create_slot(name, luaFunctionObject, env);
    lua_pushinteger(L, handle);
    return 1;
}

static int cubescript_event_handler_function(const std::string & name, script::any obj)
{
    if(obj.get_type() != typeid(script::env::object::shared_ptr)) throw script::error(script::BAD_CAST);
    return slots.create_slot(name, script::any_cast<script::env::object::shared_ptr>(obj), env);
}

void destroy_slot(int handle)
{
    slots.destroy_slot(handle);
}

void cleanup()
{
    slots.clear();
    slots.deallocate_destroyed_slots();
}

void register_signals(script::env & env)
{
    ::env = &env;
    
    signal_shutdown.connect(&cleanup, boost::signals::at_front);
    
    slots.register_signal(signal_started, "started", normal_error_handler);
    slots.register_signal(signal_shutdown,"shutdown",normal_error_handler, boost::signals::at_front);
    slots.register_signal(signal_reloadhopmod, "reloadhopmod", normal_error_handler);
    
    slots.register_signal(signal_connect,"connect",normal_error_handler);
    slots.register_signal(signal_disconnect,"disconnect",normal_error_handler);
    slots.register_signal(signal_failedconnect, "failedconnect",normal_error_handler);
    slots.register_signal(signal_active, "active", normal_error_handler);
    slots.register_signal(signal_rename,"rename",normal_error_handler);
    slots.register_signal(signal_renaming, "renaming", normal_error_handler);
    slots.register_signal(signal_reteam, "reteam", normal_error_handler);
    slots.register_signal(signal_chteamrequest, "chteamrequest", proceed_error_handler);
    slots.register_signal(signal_kick,"kick",normal_error_handler);
    slots.register_signal(signal_text,"text",proceed_error_handler);
    slots.register_signal(signal_sayteam,"sayteam",proceed_error_handler);
    slots.register_signal(signal_intermission,"intermission", normal_error_handler);
    slots.register_signal(signal_finishedgame, "finishedgame", normal_error_handler);
    slots.register_signal(signal_timeupdate,"timeupdate", normal_error_handler);
    slots.register_signal(signal_mapchange,"mapchange", normal_error_handler);
    slots.register_signal(signal_mapvote, "mapvote", proceed_error_handler);
    slots.register_signal(signal_setnextgame, "setnextgame", normal_error_handler);
    slots.register_signal(signal_gamepaused, "gamepaused", normal_error_handler);
    slots.register_signal(signal_gameresumed, "gameresumed", normal_error_handler);
    slots.register_signal(signal_setmastermode, "setmastermode", normal_error_handler);
    slots.register_signal(signal_spectator, "spectator", normal_error_handler);
    slots.register_signal(signal_setmaster, "setmaster", normal_error_handler);
    slots.register_signal(signal_teamkill, "teamkill", normal_error_handler);
    slots.register_signal(signal_auth, "auth", normal_error_handler);
    slots.register_signal(signal_authreq, "authreq", normal_error_handler);
    slots.register_signal(signal_authrep, "authrep", normal_error_handler);
    slots.register_signal(signal_addbot, "addbot", normal_error_handler);
    slots.register_signal(signal_delbot, "delbot", normal_error_handler);
    slots.register_signal(signal_botleft, "botleft", normal_error_handler);
    slots.register_signal(signal_beginrecord, "beginrecord", normal_error_handler);
    slots.register_signal(signal_endrecord, "endrecord", normal_error_handler);
    slots.register_signal(signal_mapcrcfail, "mapcrcfail", normal_error_handler);
    slots.register_signal(signal_votepassed, "votepassed", normal_error_handler);

    slots.register_signal(signal_rootserver_failedconnect, "failedrootconnect", normal_error_handler);

    script::bind_global_func<int (const std::string &,script::any)>(cubescript_event_handler_function, FUNGU_OBJECT_ID("event_handler"), env);
    script::bind_global_func<void (int)>(destroy_slot, FUNGU_OBJECT_ID("cancel_handler"), env);
    
    register_lua_function(lua_event_handler_function,"event_handler");
}

void cleanup_dead_slots()
{
    slots.deallocate_destroyed_slots();
}

void disconnect_all_slots()
{
    signal_started.disconnect_all_slots();
    signal_shutdown.disconnect_all_slots();
    signal_reloadhopmod.disconnect_all_slots();
    
    signal_connect.disconnect_all_slots();
    signal_disconnect.disconnect_all_slots();
    signal_failedconnect.disconnect_all_slots();
    signal_active.disconnect_all_slots();
    signal_rename.disconnect_all_slots();
    signal_renaming.disconnect_all_slots();
    signal_reteam.disconnect_all_slots();
    signal_chteamrequest.disconnect_all_slots();
    signal_kick.disconnect_all_slots();
    signal_text.disconnect_all_slots();
    signal_sayteam.disconnect_all_slots();
    signal_intermission.disconnect_all_slots();
    signal_finishedgame.disconnect_all_slots();
    signal_timeupdate.disconnect_all_slots();
    signal_mapchange.disconnect_all_slots();
    signal_mapvote.disconnect_all_slots();
    signal_setnextgame.disconnect_all_slots();
    signal_gamepaused.disconnect_all_slots();
    signal_gameresumed.disconnect_all_slots();
    signal_setmastermode.disconnect_all_slots();
    signal_spectator.disconnect_all_slots();
    signal_setmaster.disconnect_all_slots();
    signal_teamkill.disconnect_all_slots();
    signal_auth.disconnect_all_slots();
    signal_authreq.disconnect_all_slots();
    signal_authrep.disconnect_all_slots();
    signal_addbot.disconnect_all_slots();
    signal_delbot.disconnect_all_slots();
    signal_botleft.disconnect_all_slots();
    signal_beginrecord.disconnect_all_slots();
    signal_endrecord.disconnect_all_slots();
    signal_mapcrcfail.disconnect_all_slots();
    signal_votepassed.disconnect_all_slots();
    
    signal_rootserver_failedconnect.disconnect_all_slots();
}
