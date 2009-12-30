
set(MYSQL_DIR "/usr/local/mysql" CACHE STRING "Directory of MySQL installation")

include_directories(${LUA_INCLUDE_DIR} ${MYSQL_DIR}/include)

add_library(luasql_mysql SHARED src/luasql.c src/ls_mysql.c)
target_link_libraries(luasql_mysql -z -lmysqlclient lua)
link_directories(${MYSQL_DIR}/lib)

install(TARGETS luasql_mysql LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION})
