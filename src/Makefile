
HOPMOD_OBJS= \
    fpsgame/hopmod/tools.o \
    fpsgame/hopmod/textformat.o \
    fpsgame/hopmod/stopwatch.o \
    fpsgame/hopmod/eventhandler.o \
    fpsgame/hopmod/script_pipe.o \
    fpsgame/hopmod/script_socket.o \
    fpsgame/hopmod/schedule.o \
    fpsgame/hopmod/schedfunctions.o \
    fpsgame/hopmod/sqlite3.o \
    fpsgame/hopmod/geoip.o

LIBCUBESCRIPT_OBJS= \
    fpsgame/hopmod/libcubescript/src/error.o \
    fpsgame/hopmod/libcubescript/src/alias.o \
    fpsgame/hopmod/libcubescript/src/symbol.o \
    fpsgame/hopmod/libcubescript/src/domain.o \
    fpsgame/hopmod/libcubescript/src/construct.o \
    fpsgame/hopmod/libcubescript/src/constructs.o \
    fpsgame/hopmod/libcubescript/src/expression.o \
    fpsgame/hopmod/libcubescript/src/reference.o \
    fpsgame/hopmod/libcubescript/src/object.o \
    fpsgame/hopmod/libcubescript/src/exec_helpers.o \
    fpsgame/hopmod/libcubescript/src/runtime.o \

CXXOPTFLAGS= -g
INCLUDES= -I/usr/local/include -Ishared -Iengine -Ifpsgame -Ienet/include -Ifpsgame/hopmod/libcubescript/include
CXXFLAGS= -Wall -fsigned-char $(CXXOPTFLAGS) $(INCLUDES) -DSTANDALONE -DUSE_SQLITE3 -DUSE_GEOIP -DUSE_SCRIPT_SOCKET
LDFLAGS= -L/usr/local/lib -rpath=/usr/local/lib
PLATFORM_PREFIX=native

SERVER_LIBS=-Lenet -lenet -lz -lsqlite3 -lGeoIP -lmicrohttpd

SERVER_OBJS= \
	shared/tools-standalone.o \
	engine/server-standalone.o \
	fpsgame/fpsserver.o \
    $(LIBCUBESCRIPT_OBJS) \
    $(HOPMOD_OBJS)

default: all

all: server mkfifochan

enet/Makefile:
	cd enet; ./configure

libenet: enet/Makefile
	$(MAKE)	-C enet/ all

clean: enet/Makefile
	-$(RM) $(SERVER_OBJS) $(CLIENT_OBJS) ../bin/sauer_server ../bin/mkfifochan
	$(MAKE)	-C enet/ clean
    
%-standalone.o:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DSTANDALONE -c -o $@ $(subst -standalone.o,.cpp,$@)

mkfifochan:
	$(CC) fpsgame/hopmod/mkfifochan.c -o../bin/mkfifochan

server:	libenet $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o ../bin/sauer_server $(SERVER_OBJS) $(SERVER_LIBS)  

install: all

depend:
	makedepend -a -o-standalone.o -Y -Ishared -Iengine -Ifpsgame $(subst -standalone.o,.cpp,$(SERVER_OBJS))

# DO NOT DELETE

shared/tools.o: shared/pch.h shared/tools.h
shared/tools-standalone.o: shared/pch.h shared/tools.h
engine/server-standalone.o: shared/pch.h shared/cube.h
engine/server-standalone.o: shared/tools.h shared/geom.h shared/ents.h
engine/server-standalone.o: shared/igame.h
