AC_INIT([libcubescript], [1.0.0])
AC_CONFIG_SRCDIR([src/cubescript.cpp])
AM_INIT_AUTOMAKE

AC_PROG_CXX
AC_PROG_RANLIB

AC_ARG_ENABLE([debug],[--enable-debug    Build in debug mode],[debug=true],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_ARG_ENABLE([tests],[--enable-tests    Build unit tests],[
    
    AM_PATH_CPPUNIT(1.12.0,,[
        AC_MSG_ERROR([
            You are missing the CppUnit testing framework (version >= 1.12.0).
        ])
    ])
    
    build_tests=true
],[build_tests=false])
AM_CONDITIONAL([BUILD_TESTS], [test x$build_tests = xtrue])

AC_ARG_ENABLE([docs],[--enable-docs    Build library documentation],[
    
    AC_CHECK_PROG([DOXYGEN],[doxygen],,[
        AC_MSG_ERROR([
            You are missing the doxygen program. Check PATH variable.
        ])
    ])
    
    docs=true
],[docs=false])
AM_CONDITIONAL([BUILD_DOCS],[test x$docs = xtrue])

AC_CONFIG_FILES([Makefile tests/Makefile])
AC_OUTPUT
