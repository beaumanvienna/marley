#standalone mupen64plus test version
COMPILER_ARTIFACTS = -I/usr/include/SDL2  -fno-pie -no-pie

LINKER_OBJECTS = \
	../../mupen64plus/Source/mupen.a \

LINKER_OBJECTS 	+= -lSDL2
LINKER_OBJECTS 	+= -lGL 
LINKER_OBJECTS 	+= -lstdc++ 
LINKER_OBJECTS 	+= -lGLU 
LINKER_OBJECTS 	+= -lfreetype 
LINKER_OBJECTS 	+= -lboost_system 
LINKER_OBJECTS 	+= -lboost_filesystem 
LINKER_OBJECTS 	+= -lsamplerate
LINKER_OBJECTS 	+= -ldl
LINKER_OBJECTS 	+= -lpng
LINKER_OBJECTS 	+= -lz

all: MUPEN64PLUS

MUPEN64PLUS: main.cpp
	$(info   *************** tests make mupen64plus ***************)
	$(MAKE) -C ../../mupen64plus/ all -j8
	g++ $(COMPILER_ARTIFACTS)  -o MUPEN64PLUS main.cpp $(LINKER_OBJECTS) 
#-g3 -O0 -ggdb
clean:
	$(info   *************** tests mupen64plus clean ***************)
	rm -f *.o MUPEN64PLUS
