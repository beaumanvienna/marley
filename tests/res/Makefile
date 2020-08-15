# test script for resource system
GTK_HEADERS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-2.0 )

COMPILER_ARTIFACTS = --std=c++17 -I/usr/include/SDL2 $(GTK_HEADERS) 

LINKER_OBJECTS = -lSDL2 -lSDL2_image -lSDL2_ttf $(GTK_LIBS)



GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

resources = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies res.xml)

all: main.cpp res.c res.h
	$(info   *************** tests make res ***************)
	g++ $(COMPILER_ARTIFACTS) -o RES main.cpp res.c $(LINKER_OBJECTS)

res.c: res.xml $(resources)
	$(GLIB_COMPILE_RESOURCES) res.xml --target=res.c --sourcedir=. --generate-source

res.h: res.xml $(resources)
	$(GLIB_COMPILE_RESOURCES) res.xml --target=res.h --sourcedir=. --generate-header

clean:
	$(info   *************** tests res clean ***************)
	rm -f *.o RES res.c res.h

