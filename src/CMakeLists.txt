add_subdirectory(enet)
add_subdirectory(lua)
add_subdirectory(boost)
add_subdirectory(fungu)
add_subdirectory(lua-sqlite3)
add_subdirectory(authserver EXCLUDE_FROM_ALL)
add_subdirectory(luasql EXCLUDE_FROM_ALL)

include(PCHSupport)
include(CheckLibraryExists)

check_library_exists(GeoIP GeoIP_open "" GEOIP_EXISTS)
check_library_exists(z gzopen "" ZLIB_EXISTS)
check_library_exists(pthread pthread_self "" PTHREAD_EXISTS)
check_library_exists(rt clock_gettime "" RTLIB_EXISTS)

if(NOT ${GEOIP_EXISTS})
    message(FATAL_ERROR "missing required library: GeoIP")
endif(NOT ${GEOIP_EXISTS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/hopmod
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ENET_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${FUNGU_INCLUDE_DIR})

add_definitions(-DSTANDALONE -DFUNGU_WITH_LUA -DHOPMOD_GAMESERVER_EVENTS)

set(SCRIPTING_LIBRARIES lua fungu_script)

set(SAUERTOOLS_SOURCES
    shared/tools.cpp
    shared/stream.cpp
    shared/crypto.cpp
    hopmod/utils.cpp
    hopmod/net/address.cpp
    hopmod/net/address_mask.cpp
    hopmod/net/address_prefix.cpp)

add_library(sauertools STATIC ${SAUERTOOLS_SOURCES})

set(LUA_MODULES_SOURCES
    hopmod/lua/crypto.cpp
    hopmod/md5.c
    hopmod/lua/net.cpp
    hopmod/lua/timer.cpp
    hopmod/lua/cubescript.cpp
    hopmod/lua/geoip.cpp
    hopmod/lua/filesystem.cpp
    hopmod/lua/packlib.c)

add_library(lua_modules STATIC ${LUA_MODULES_SOURCES})
target_link_libraries(lua_modules -lGeoIP -pthread sauertools ${SCRIPTING_LIBRARIES})

add_library(serverscripting STATIC hopmod/scripting.cpp hopmod/signals.cpp)
target_link_libraries(serverscripting lua_modules boost_signals boost_system ${SCRIPTING_LIBRARIES})

add_library(scriptpipe STATIC hopmod/script_pipe.cpp)
target_link_libraries(scriptpipe serverscripting)

SET(SAUER_SERVER_SOURCES
    engine/server.cpp
    fpsgame/server.cpp
    hopmod/startup.cpp
    hopmod/scripting.cpp
    hopmod/script_bindings.cpp
    hopmod/script_pipe.cpp
    hopmod/parse_player_command.cpp
    hopmod/scheduler.cpp
    hopmod/playerid.cpp
    hopmod/restarter.cpp
    hopmod/httpserver/httpserver.cpp
    hopmod/httpserver/serverexec.cpp
    hopmod/httpserver/filesystem_resource.cpp
    hopmod/httpserver/lua_module.cpp)

add_executable(sauer_server ${SAUER_SERVER_SOURCES})
target_link_libraries(sauer_server sauertools serverscripting scriptpipe enet fungu_http -lz -lrt)

add_executable(keygen hopmod/keygen.cpp)
target_link_libraries(keygen sauertools -lz)

add_executable(luapp hopmod/luapp.cpp)
target_link_libraries(luapp lua boost_system lua_modules -lrt)

install(TARGETS sauer_server sauertools lua_modules serverscripting scriptpipe
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DESTINATION} 
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION} 
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DESTINATION})

install(TARGETS keygen luapp
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DESTINATION}/utils)
