HOPMOD_CXXFLAGS= \
    `fpsgame/hopmod/libcubescript/cppflags.sh fpsgame/hopmod/libcubescript` -DSTANDALONE -DUSE_SQLITE3 -DUSE_GEOIP -DUSE_SCRIPT_SOCKET

HOPMOD_OBJS= \
    fpsgame/hopmod/textformat.o \
    fpsgame/hopmod/stopwatch.o \
    fpsgame/hopmod/eventhandler.o \
    fpsgame/hopmod/script_pipe.o \
    fpsgame/hopmod/script_socket.o \
    fpsgame/hopmod/schedule.o \
    fpsgame/hopmod/schedfunctions.o \
    fpsgame/hopmod/sqlite3.o \
    fpsgame/hopmod/geoip.o

CXXOPTFLAGS= -g
INCLUDES= -Ishared -Iengine -Ifpsgame -Ienet/include
CXXFLAGS= -Wall -fsigned-char $(CXXOPTFLAGS) $(INCLUDES) $(HOPMOD_CXXFLAGS)

PLATFORM_PREFIX=native

SERVER_LIBS=-Lenet -lenet -lz -Lfpsgame/hopmod/libcubescript -lcubescript -ldl -lsqlite3 -lGeoIP -lmicrohttpd

SERVER_OBJS= \
	shared/tools-standalone.o \
	engine/server-standalone.o \
	fpsgame/fpsserver.o \
    $(HOPMOD_OBJS)

default: all

all: server mkfifochan plugins

enet/Makefile:
	cd enet; ./configure

libenet: enet/Makefile
	$(MAKE)	-C enet/ all

fpsgame/hopmod/libcubescript/Makefile:
	cd fpsgame/hopmod/libcubescript; ./configure

libcubescript: fpsgame/hopmod/libcubescript/Makefile
	$(MAKE) -C fpsgame/hopmod/libcubescript/ all

clean: enet/Makefile
	-$(RM) $(SERVER_OBJS) $(CLIENT_OBJS) ../bin/sauer_server ../bin/mkfifochan
	$(MAKE)	-C enet/ clean
	$(MAKE) -C fpsgame/hopmod/libcubescript/ clean
    
%-standalone.o:
	$(CXX) $(CXXFLAGS) -DSTANDALONE -c -o $@ $(subst -standalone.o,.cpp,$@)

mkfifochan:
	$(CC) fpsgame/hopmod/mkfifochan.c -o../bin/mkfifochan

plugins:

server:	libenet libcubescript $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../bin/sauer_server $(SERVER_OBJS) $(SERVER_LIBS)  

install: all
