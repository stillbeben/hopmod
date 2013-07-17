find_package(Boost COMPONENTS signals system thread REQUIRED)

option(WITH_GEOIP "Enable GeoIP support" ON)
find_package(GeoIP)

if(GeoIP_FOUND)
    if(WITH_GEOIP)
    add_definitions(-DWITH_GEOIP)
    set(GEOIP_INCLUDE_DIRS ${GeoIP_INCLUDE_DIRS})
    set(GEOIP_LIBRARY ${GeoIP_LIBRARY})
endif(WITH_GEOIP)
else(GeoIP_FOUND)
    if(WITH_GEOIP)
        message(FATAL_ERROR "missing required library: GeoIP. To build without GeoIP support use option -DWITH_GEOIP=No")
    endif(WITH_GEOIP)
endif(GeoIP_FOUND)

option(WITH_OPENSSL "Enable SSL support" ON)
find_package(OpenSSL)

if(WITH_OPENSSL AND OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)
    set(LUA_NET_SSL_SOURCES
        hopmod/lua/modules/net/ssl_context.cpp
        hopmod/lua/modules/net/ssl_tcp_stream.cpp)
    add_definitions(-DKSSL_DEBUG)
else(WITH_OPENSSL AND OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)
    add_definitions(-DDISABLE_SSL -DWITHOUT_OPENSSL)
endif(WITH_OPENSSL AND OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR)

include(CheckLibraryExists)
check_library_exists(pthread pthread_self "" PTHREAD_EXISTS)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    check_library_exists(rt clock_gettime "" RTLIB_EXISTS)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

check_library_exists(z gzopen "" ZLIB_EXISTS)
if(NOT ${ZLIB_EXISTS})
    message(FATAL_ERROR "missing required library: ZLIB")
endif(NOT ${ZLIB_EXISTS})

add_subdirectory(enet)
add_subdirectory(lua)
add_subdirectory(fungu)
add_subdirectory(lsqlite3-8)
add_subdirectory(hopmod/standalone/luapp)

find_package(MYSQL)

if(MYSQL_FOUND)
add_subdirectory(luasql)
endif(MYSQL_FOUND) 

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/hopmod
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ENET_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    ${FUNGU_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${GEOIP_INCLUDE_DIRS})

add_definitions(-Wall -fsigned-char -DSTANDALONE -DREVISION=$ENV{REVISION} -DLUA_COMPAT_MODULE ${LSQLITE3_DEFINITIONS})
add_definitions(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING)

IF(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG") 
    add_definitions( -O3)
ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")


set(SAUERTOOLS_SOURCES
    shared/tools.cpp
    shared/stream.cpp
    shared/crypto.cpp
    )

add_library(sauertools STATIC ${SAUERTOOLS_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(sauertools -lrt)
else(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
find_library(coreServices CoreServices)
    target_link_libraries(sauertools ${coreServices})
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

target_link_libraries(sauertools -lz ${ENET_LIBRARY})

set(LUA_MODULES_SOURCES
    hopmod/lib/md5.c
    hopmod/utils.cpp
    hopmod/net/address.cpp
    hopmod/net/address_mask.cpp
    hopmod/net/address_prefix.cpp
    hopmod/lua/event.cpp
    hopmod/lua/push_function.cpp
    hopmod/lua/library_extensions.cpp
    hopmod/lua/pcall.cpp
    hopmod/lua/error_handler.cpp
    hopmod/lua/push.cpp
    hopmod/lua/register_class.cpp
    hopmod/lua/modules/module.cpp
    hopmod/lua/modules/crypto.cpp
    hopmod/lua/modules/timer.cpp
    hopmod/lua/modules/geoip.cpp
    hopmod/lua/modules/filesystem.cpp
    hopmod/lua/modules/packlib.c
    hopmod/lua/modules/cubescript.cpp
    hopmod/lua/modules/net/module.cpp
    hopmod/lua/modules/net/tcp_socket.cpp
    hopmod/lua/modules/net/weak_ref.cpp
    hopmod/lua/modules/net/ipmask.cpp
    hopmod/lua/modules/net/file_stream.cpp
    hopmod/lua/modules/net/tcp_acceptor.cpp
    hopmod/lua/modules/net/resolver.cpp
    ${LUA_NET_SSL_SOURCES}
    hopmod/cubescript/cubescript.cpp
    hopmod/cubescript/lua_command_stack.cpp
    hopmod/cubescript/lua/pcall.cpp
    hopmod/httpserver/filesystem_resource.cpp
    hopmod/httpserver/lua_module.cpp )

add_library(lua_modules STATIC ${LUA_MODULES_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(lua_modules -pthread)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(lua_modules 
    sauertools
    ${LUA_LIBRARY}
    ${GEOIP_LIBRARY}
    ${FUNGU_STRING_LIBRARY}
    ${FUNGU_HTTP_LIBRARY}
    ${LSQLITE3_LIBRARY}
    ${OPENSSL_LIBRARIES})

SET(GAME_SERVER_SOURCES
    engine/server.cpp
    fpsgame/server.cpp
    hopmod/startup.cpp
    hopmod/scheduler.cpp
    hopmod/lua.cpp
    hopmod/core_bindings.cpp
    hopmod/events.cpp
    hopmod/signals.cpp)

add_executable(sauer_server ${GAME_SERVER_SOURCES})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    target_link_libraries(sauer_server -lrt)
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

target_link_libraries(sauer_server 
    -lz
    sauertools
    lua_modules
    -lboost_signals
    -lboost_thread
    -lboost_system
    ${ENET_LIBRARY})

add_executable(keygen hopmod/standalone/keygen.cpp)
target_link_libraries(keygen sauertools)

add_executable(monitor hopmod/standalone/monitor.cpp hopmod/lib/program_arguments.cpp)

install(TARGETS 
    sauer_server
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
