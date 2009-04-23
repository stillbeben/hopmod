AC_INIT

AC_PROG_CXX

AC_HAVE_LIBRARY([z],,[AC_MSG_ERROR("You are missing a required library: zlib.")])

AC_HAVE_LIBRARY([GeoIP],[AC_SUBST(WITHOUT_GEOIP,0)],[
    AC_SUBST(WITHOUT_GEOIP,1)
    AC_MSG_WARN("Building WITHOUT GeoIP support.")
])

AC_HAVE_LIBRARY([microhttpd],[AC_SUBST(WITHOUT_MICROHTTPD,0)],[
    AC_SUBST(WITHOUT_MICROHTTPD,1)
    AC_MSG_WARN("Building WITHOUT script_socket support.")
])

AC_HAVE_LIBRARY([ncurses],[],[AC_MSG_WARN("Unable to find ncurses library which means the build for luashell will fail.")])

AC_LANG_PUSH([C++])
AC_CHECK_HEADER([boost/detail/lightweight_test.hpp],[AC_SUBST(WITHOUT_TEST_PROJECT,0)],[
    AC_SUBST(WITHOUT_TEST_PROJECT,1)
    AC_MSG_WARN("Will not be building and running the tests.")])
AC_LANG_POP()

AC_CHECK_FUNC(gethostbyaddr_r, [AC_SUBST(DEFINE_HAS_GETHOSTBYADDR_R,"<define>HAS_GETHOSTBYADDR_R")])
AC_CHECK_FUNC(gethostbyname_r, [AC_SUBST(DEFINE_HAS_GETHOSTBYNAME_R,"<define>HAS_GETHOSTBYNAME_R")])
AC_CHECK_FUNC(poll, [AC_SUBST(DEFINE_HAS_POLL,"<define>HAS_POLL")])
AC_CHECK_FUNC(fcntl, [AC_SUBST(DEFINE_HAS_FCNTL,"<define>HAS_FCNTL")])
AC_CHECK_FUNC(inet_pton, [AC_SUBST(DEFINE_HAS_INET_PTON,"<define>HAS_INET_PTON")])
AC_CHECK_FUNC(inet_ntop, [AC_SUBST(DEFINE_HAS_INET_NTOP,"<define>HAS_INET_NTOP")])

AC_CHECK_MEMBER(struct msghdr.msg_flags, [AC_SUBST(DEFINE_HAS_MSGHDR_FLAGS,"<define>HAS_MSGHDR_FLAGS")], , [#include <sys/socket.h>])

AC_CHECK_TYPE(socklen_t, [AC_SUBST(DEFINE_HAS_SOCKLEN_T,"<define>HAS_SOCKLEN_T")], , 
              #include <sys/types.h>
              #include <sys/socket.h>
)

AC_EGREP_HEADER(MSG_MAXIOVLEN, /usr/include/sys/socket.h, AC_SUBST(DEFINE_ENET_BUFFER_MAXIMUM,"<define>ENET_BUFFER_MAXIMUM=$MSG_MAXIOVLEN"))
AC_EGREP_HEADER(MSG_MAXIOVLEN, socket.h, AC_SUBST(DEFINE_ENET_BUFFER_MAXIMUM,"<define>ENET_BUFFER_MAXIMUM=$MSG_MAXIOVLEN"))

AC_MSG_CHECKING(whether to use CRC32)
AC_ARG_ENABLE(crc32,
    [  --enable-crc32   enable CRC32 packet verification ],
    [if test "$enableval" = yes; then
        AC_MSG_RESULT(yes)
        AC_SUBST(DEFINE_USE_CRC32,"<define>USE_CRC32")
     else
        AC_MSG_RESULT(no)
     fi],
    [AC_MSG_RESULT(no)])

AC_OUTPUT([Jamroot])
