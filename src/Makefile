HOPMOD_CXXFLAGS= \
    `fpsgame/hopmod/libcubescript/cppflags.sh fpsgame/hopmod/libcubescript`

HOPMOD_OBJS= \
    fpsgame/hopmod/textformat.o \
    fpsgame/hopmod/stopwatch.o \
    fpsgame/hopmod/eventhandler.o \
    fpsgame/hopmod/script_pipe.o \
    fpsgame/hopmod/sleep.o \
    fpsgame/hopmod/module.o \
    fpsgame/hopmod/module_loader.o

CXXOPTFLAGS= -O3 -fomit-frame-pointer
INCLUDES= -Ishared -Iengine -Ifpsgame -Irpggame -Ienet/include -I/usr/X11R6/include `sdl-config --cflags`
CXXFLAGS= -Wall -fsigned-char $(CXXOPTFLAGS) $(INCLUDES) -g $(HOPMOD_CXXFLAGS) 

PLATFORM_PREFIX=native

SERVER_LIBS=-Lenet -lenet -lz -Lfpsgame/hopmod/libcubescript -lcubescript -ldl

SERVER_OBJS= \
	shared/tools-standalone.o \
	engine/server-standalone.o \
	fpsgame/fps-standalone.o \
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
	-$(RM) $(SERVER_OBJS) $(CLIENT_OBJS) ../bin/sauer_server ../bin/mkfifochan ../lib/geoip.csm
	$(MAKE)	-C enet/ clean
	$(MAKE) -C fpsgame/hopmod/libcubescript/ clean
    
%-standalone.o:
	$(CXX) $(CXXFLAGS) -DSTANDALONE -c -o $@ $(subst -standalone.o,.cpp,$@)

mkfifochan:
	$(CC) fpsgame/hopmod/mkfifochan.c -o../bin/mkfifochan

plugins:
	$(CXX) fpsgame/hopmod/modules/geoip.cpp fpsgame/hopmod/module_loader.cpp fpsgame/hopmod/libcubescript/libcubescript.a -DCUBESCRIPT_MODULE $(HOPMOD_CXXFLAGS) -lGeoIP -shared -fPIC -o../lib/geoip.csm

server:	libenet libcubescript $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../bin/sauer_server $(SERVER_OBJS) $(SERVER_LIBS)  

install: all
