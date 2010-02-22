add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
add_library(boost_system STATIC src/error_code.cpp)
install(TARGETS boost_system ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DESTINATION})
