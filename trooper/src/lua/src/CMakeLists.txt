
set(SOURCES
    lapi.c      
    lcode.c   
    ldo.c    
    lgc.c     
    llex.c      
    loadlib.c   
    loslib.c   
    lstring.c  
    ltablib.c      
    lzio.c
    lauxlib.c   
    ldblib.c  
    ldump.c  
    linit.c   
    lmathlib.c  
    lobject.c   
    lparser.c  
    lstrlib.c  
    ltm.c      
    lundump.c  
    lbaselib.c  
    ldebug.c  
    lfunc.c  
    liolib.c  
    lmem.c
    lopcodes.c
    lstate.c
    ltable.c
    lvm.c
)

add_library(lua SHARED ${SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    add_definitions(-DLUA_USE_LINUX)
    target_link_libraries(lua dl)
endif(${CMAKE_SYSTEM_NAME} MATCHES Linux)

if(${CMAKE_SYSTEM_NAME} MATCHES BSD)
    add_definitions(-DLUA_USE_POSIX -DLUA_USE_DLOPEN)
endif(${CMAKE_SYSTEM_NAME} MATCHES BSD)

install(TARGETS lua LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION})
