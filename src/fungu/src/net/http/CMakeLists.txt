
set(HTTP_SOURCES
    connection.cpp
    header.cpp
    info.cpp
    request.cpp
    request_line.cpp
    response.cpp
    status.cpp)

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../../../include)
add_library(fungu_http STATIC ${HTTP_SOURCES})
target_link_libraries(fungu_http fungu_string -pthread boost_system fungu_script)

install(TARGETS fungu_http ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DESTINATION})
