#standalone ppsspp test version
COMPILER_ARTIFACTS = -I/usr/include/SDL2 -I../../screen_manager/

LINKER_OBJECTS = \
	../../ppsspp/build/lib/libglslang.a \
	../../ppsspp/build/lib/libxxhash.a \
	../../ppsspp/build/lib/liblibzip.a \
	../../ppsspp/build/lib/libspirv-cross-cpp.a \
	../../ppsspp/build/lib/libspirv-cross-glsl.a \
	../../ppsspp/build/lib/libudis86.a \
	../../ppsspp/build/lib/libOSDependent.a \
	../../ppsspp/build/lib/libspirv-cross-core.a \
	../../ppsspp/build/lib/libspirv-cross-msl.a \
	../../ppsspp/build/lib/libsfmt19937.a \
	../../ppsspp/build/lib/libPPSSPPSDL.a \
	../../ppsspp/build/lib/libspirv-cross-hlsl.a \
	../../ppsspp/build/lib/libcityhash.a \
	../../ppsspp/build/lib/libkirk.a \
	../../ppsspp/build/lib/libarmips.a \
	../../ppsspp/build/lib/libnative.a \
	../../ppsspp/build/lib/libOGLCompiler.a \
	../../ppsspp/build/lib/libgason.a \
	../../ppsspp/build/lib/libxbrz.a \
	../../ppsspp/build/lib/libSPVRemapper.a \
	../../ppsspp/build/lib/libCore.a \
	../../ppsspp/build/lib/libHLSL.a \
	../../ppsspp/build/lib/libCommon.a \
	../../ppsspp/build/lib/libSPIRV.a \
	../../ppsspp/build/lib/libsnappy.a \
	../../ppsspp/ffmpeg/linux/x86_64/lib/libavcodec.a \
	../../ppsspp/ffmpeg/linux/x86_64/lib/libavutil.a \
	../../ppsspp/ffmpeg/linux/x86_64/lib/libswscale.a \
	../../ppsspp/ffmpeg/linux/x86_64/lib/libavformat.a \
	../../ppsspp/ffmpeg/linux/x86_64/lib/libswresample.a \
	../../ppsspp/build/lib/libminiupnpc.a
	
LINKER_OBJECTS 	+= -lSDL2
LINKER_OBJECTS 	+= -lGLEW
LINKER_OBJECTS 	+= -lpthread
LINKER_OBJECTS 	+= -lz
LINKER_OBJECTS  += -ldl
LINKER_OBJECTS 	+= -lGL
LINKER_OBJECTS 	+= -lrt
LINKER_OBJECTS 	+= -lpng
LINKER_OBJECTS 	+= -lGLU

all: PPSSPP

PPSSPP: main.cpp
	$(info   *************** tests make ppsspp ***************)
	$(MAKE) -C ../../ppsspp/build all
	g++ $(COMPILER_ARTIFACTS) -g3 -O0 -ggdb -o PPSSPP main.cpp ../../screen_manager/UI/Scale.cpp $(LINKER_OBJECTS) $(LINKER_OBJECTS) $(LINKER_OBJECTS)

clean:
	$(info   *************** tests ppsspp clean ***************)
	rm -f *.o PPSSPP
