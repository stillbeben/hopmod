add_subdirectory(script)
add_subdirectory(net)

include_directories(../include ${BOOST_INCLUDE_DIR})
add_library(fungu_string STATIC string.cpp)

install(TARGETS fungu_string ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DESTINATION})
