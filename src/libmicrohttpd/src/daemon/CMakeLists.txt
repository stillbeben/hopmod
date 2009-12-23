
set(SOURCES
    connection.c
    daemon.c
    internal.c
    memorypool.c
    postprocessor.c
    reason_phrase.c
    response.c
)

include_directories(
    ../include
    ../../
)

add_library(microhttpd STATIC ${SOURCES})

install(TARGETS microhttpd ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DESTINATION})
