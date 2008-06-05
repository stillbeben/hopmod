bin_PROGRAMS = utests

utests_CPPFLAGS=-I../include -I../boost_1_34_0 `cppunit-config --cflags`
utests_LDFLAGS=`cppunit-config --libs`
utests_LDADD = ../libcubescript.a
utests_SOURCES = \
    units/lexcasting.cpp \
    units/cons_word.cpp \
    units/cons_string.cpp \
    units/cons_macro.cpp \
    units/cons_block.cpp \
    units/cons_expression.cpp \
    units/alias.cpp \
    ut_runner.cpp