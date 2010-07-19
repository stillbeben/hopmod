include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame
    ${CMAKE_CURRENT_SOURCE_DIR}/../hopmod
    ${ENET_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${FUNGU_INCLUDE_DIR})

add_definitions(-DSTANDALONE -DFUNGU_WITH_LUA -DHOPMOD_AUTHSERVER_EVENTS)

add_executable(sauer_authserver authserver.cpp)
target_link_libraries(sauer_authserver sauertools serverscripting scriptpipe enet)

install(TARGETS sauer_authserver RUNTIME DESTINATION ${INSTALL_RUNTIME_DESTINATION})
