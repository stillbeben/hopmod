
lib_LIBRARIES = libcubescript.a

libcubescript_a_CPPFLAGS = -Iinclude -Iboost_1_34_0 -Wall -Wno-sign-compare -Wnon-virtual-dtor
libcubescript_a_SOURCES = src/cubescript.cpp

if BUILD_TESTS
    TESTS_DIR = tests
endif

SUBDIRS = . $(TESTS_DIR)

if BUILD_DOCS
docs:
	doxygen
endif
