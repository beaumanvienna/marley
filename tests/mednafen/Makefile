#standalone ppsspp test version
COMPILER_ARTIFACTS = --std=c++17 -fno-pie -no-pie -I/usr/include/SDL2

LINKER_OBJECTS = \
	../../mednafen/src/libmednafen_marley.a

LINKER_OBJECTS 	+= -lSDL2
LINKER_OBJECTS 	+= -lSDL2_image 
LINKER_OBJECTS 	+= -lSDL2_ttf
LINKER_OBJECTS 	+= -lasound 
LINKER_OBJECTS 	+= -lm 
LINKER_OBJECTS 	+= -ldl 
LINKER_OBJECTS 	+= -lpthread 
LINKER_OBJECTS 	+= -ljack 
LINKER_OBJECTS 	+= -lsndfile -lz
LINKER_OBJECTS 	+= -lz

all: MEDNAFEN

MEDNAFEN: main.cpp
	$(info   *************** tests make mednafen ***************)
	$(MAKE) -C ../../mednafen/ all -j8
	g++ $(COMPILER_ARTIFACTS) -o MEDNAFEN main.cpp $(LINKER_OBJECTS)
#-g3 -O0 -ggdb
clean:
	$(info   *************** tests mednafen clean ***************)
	rm -f *.o MEDNAFEN
