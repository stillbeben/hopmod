set(LUASQL_MYSQL_LIBRARY luasql_mysql CACHE INTERNAL "luasql_mysql library")

include_directories(${LUA_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR})

set(SOURCES
    src/luasql.c
    src/ls_mysql.c)

add_library(${LUASQL_MYSQL_LIBRARY} SHARED src/luasql.c src/ls_mysql.c)
target_link_libraries(${LUASQL_MYSQL_LIBRARY} -lz ${MYSQL_LIBRARIES} ${LUA_LIBRARY})

install(TARGETS 
            ${LUASQL_MYSQL_LIBRARY}
        LIBRARY DESTINATION 
            ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION
            ${INSTALL_ARCHIVE_DIR})

