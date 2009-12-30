
set(MYSQL_DIR "/usr/local/mysql" CACHE STRING "Directory of MySQL installation")

include_directories(${LUA_INCLUDE_DIR} ${MYSQL_DIR}/include)
link_directories(${MYSQL_DIR}/lib)

add_library(luasql_mysql SHARED src/luasql.c src/ls_mysql.c)
target_link_libraries(luasql_mysql -lz -lmysqlclient lua)

install(TARGETS luasql_mysql LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION})
