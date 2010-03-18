
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

target_link_libraries(fungu_script fungu_string lua)
