
set(SOURCES
    type.cpp
    constructs.cpp
    env.cpp
    execute.cpp
    objects.cpp
    corelib.cpp
    lua.cpp
)

add_definitions(-DFUNGU_WITH_LUA)

include_directories(
    ../../include
    ../../../boost
    ../../../lua/src
)

add_library(fungu_script STATIC ${SOURCES})

include(PCHSupport)
add_precompiled_header(fungu_script ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

target_link_libraries(fungu_script fungu_string lua)
