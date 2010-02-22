
include(CheckLibraryExists)
check_library_exists(sqlite3 sqlite3_open "" SQLITE3_EXISTS)

if(${SQLITE3_EXISTS})

    add_library(luasqlite3 SHARED libluasqlite3.c)
    target_link_libraries(luasqlite3 lua -lsqlite3)
    include_directories(${LUA_INCLUDE_DIR})

    install(TARGETS luasqlite3 LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION})
    install(FILES sqlite3.lua libluasqlite3-loader.lua DESTINATION ${INSTALL_LUA_PACKAGE_DESTINATION})
    
endif(${SQLITE3_EXISTS})
