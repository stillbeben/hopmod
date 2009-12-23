
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../)
add_definitions(-DBOOST_SIGNALS_DYN_LINK=1)

set(BOOST_SIGNALS_SOURCES
    src/connection.cpp
    src/named_slot_map.cpp
    src/signal_base.cpp
    src/slot.cpp
    src/trackable.cpp)

add_library(boost_signals SHARED ${BOOST_SIGNALS_SOURCES})

install(TARGETS boost_signals LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION})
