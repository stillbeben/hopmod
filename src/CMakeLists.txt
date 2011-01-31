add_subdirectory(enet)
add_subdirectory(lua)
add_subdirectory(boost)
add_subdirectory(fungu)
add_subdirectory(lsqlite3-7)
add_subdirectory(hopmod/standalone/luapp)
add_subdirectory(authserver EXCLUDE_FROM_ALL)
add_subdirectory(luasql EXCLUDE_FROM_ALL)

include(CheckLibraryExists)

check_library_exists(GeoIP GeoIP_open "" GEOIP_EXISTS)
check_library_exists(z gzopen "" ZLIB_EXISTS)
check_library_exists(pthread pthread_self "" PTHREAD_EXISTS)
check_library_exists(rt clock_gettime "" RTLIB_EXISTS)

if(NOT ${GEOIP_EXISTS})
    message(FATAL_ERROR "missing required library: GeoIP")
endif(NOT ${GEOIP_EXISTS})

if(NOT ${ZLIB_EXISTS})
    message(FATAL_ERROR "missing required library: ZLIB")
endif(NOT ${ZLIB_EXISTS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/hopmod
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ENET_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS})

add_definitions(-Wall -DSTANDALONE ${LSQLITE3_DEFINITIONS})

set(SCRIPTING_LIBRARIES ${LUA_LIBRARY} ${FUNGU_STRING_LIBRARY})

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
    hopmod/lib/md5.c
    hopmod/lua/push_function.cpp
    hopmod/lua/modules/crypto.cpp    
    hopmod/lua/modules/net.cpp
    hopmod/lua/modules/timer.cpp
    
    hopmod/lua/modules/cubescript.cpp
    hopmod/cubescript/cubescript.cpp
    hopmod/cubescript/lua_command_stack.cpp
    hopmod/cubescript/lua/pcall.cpp
    
    hopmod/lua/modules/geoip.cpp
    hopmod/lua/modules/filesystem.cpp
    hopmod/lua/modules/packlib.c)

add_library(lua_modules STATIC ${LUA_MODULES_SOURCES})
target_link_libraries(lua_modules -lGeoIP -pthread sauertools ${SCRIPTING_LIBRARIES})

SET(GAME_SERVER_SOURCES
    engine/server.cpp
    fpsgame/server.cpp
    hopmod/startup.cpp
    hopmod/scheduler.cpp
    hopmod/playerid.cpp
    hopmod/restarter.cpp
    hopmod/httpserver/filesystem_resource.cpp
    hopmod/httpserver/lua_module.cpp
    
    hopmod/lua.cpp
    hopmod/lua/library_extensions.cpp
    hopmod/core_bindings.cpp
    hopmod/lua/event.cpp
    hopmod/events.cpp
    hopmod/signals.cpp)

add_definitions(${LSQLITE3_DEFINITIONS})
add_executable(sauer_server ${GAME_SERVER_SOURCES})
target_link_libraries(sauer_server sauertools lua_modules ${BOOST_SIGNALS_LIBRARY} ${BOOST_THREAD_LIBRARY} ${LSQLITE3_LIBRARY} ${ENET_LIBRARY} ${FUNGU_HTTP_LIBRARY} -lz -lrt)

add_executable(keygen hopmod/standalone/keygen.cpp)
target_link_libraries(keygen sauertools)

add_executable(monitor hopmod/standalone/monitor.cpp hopmod/lib/program_arguments.cpp)

install(TARGETS 
            sauer_server
            sauertools
            lua_modules
            monitor
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR} 
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR} 
        ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})

install(TARGETS 
            keygen
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}/utils)

install(FILES 
            hopmod/cubescript/cubescript_library.lua 
        DESTINATION ${PROJECT_SOURCE_DIR}/script/package)

