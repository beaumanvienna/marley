SUBDIRS = mednafen



AM_CPPFLAGS		= --std=c++11 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen
AM_LDFLAGS		= --std=c++11 -fno-pie -no-pie -s

bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp

marley_LDADD 	= mednafen/src/libmednafen_marley.a
marley_LDADD 	+= -lSDL2 -lSDL2_image -lSDL2_ttf 
marley_LDADD 	+= -lasound -lm -ldl -lpthread -ljack -lpthread -lsndfile -lz


