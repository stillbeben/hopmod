
lib_LIBRARIES = libcubescript.a

libcubescript_a_CPPFLAGS = -Iinclude -Wall -Wno-sign-compare -Wnon-virtual-dtor -pg
libcubescript_a_SOURCES = src/cubescript.cpp

if BUILD_TESTS
    TESTS_DIR = tests
endif

SUBDIRS = . $(TESTS_DIR)

if BUILD_DOCS
docs:
	doxygen
endif
