#standalone pcsx2 test version

LINKER_OBJECTS = ../../pcsx2/build/pcsx2/libPCSX2.a
LINKER_OBJECTS += ../../pcsx2/build/pcsx2/libPCSX2.a
LINKER_OBJECTS += ../../pcsx2/build/common/src/x86emitter/libx86emitter.a
LINKER_OBJECTS += ../../pcsx2/build/common/src/Utilities/libUtilities.a
LINKER_OBJECTS += ../../pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
LINKER_OBJECTS += -lwx_baseu-3.0
LINKER_OBJECTS += -lwx_gtk2u_core-3.0
LINKER_OBJECTS += -lwx_gtk2u_adv-3.0
LINKER_OBJECTS += -lglib-2.0
LINKER_OBJECTS += -lgobject-2.0
LINKER_OBJECTS += -latk-1.0
LINKER_OBJECTS += -lgio-2.0
LINKER_OBJECTS += -lgthread-2.0
LINKER_OBJECTS += -lgmodule-2.0
LINKER_OBJECTS += -lgdk_pixbuf-2.0
LINKER_OBJECTS += -lcairo
LINKER_OBJECTS += -lpango-1.0
LINKER_OBJECTS += -lpangocairo-1.0
LINKER_OBJECTS += -lpangoft2-1.0
LINKER_OBJECTS += -lpangoxft-1.0
LINKER_OBJECTS += -lgdk-x11-2.0
LINKER_OBJECTS += -lgtk-x11-2.0
LINKER_OBJECTS += -laio
LINKER_OBJECTS += -lwx_baseu-3.0
LINKER_OBJECTS += -lwx_gtk2u_core-3.0
LINKER_OBJECTS += -lwx_gtk2u_adv-3.0
LINKER_OBJECTS += -lGL
LINKER_OBJECTS += -lstdc++
LINKER_OBJECTS += -lGLU
LINKER_OBJECTS += -lfreetype
LINKER_OBJECTS += -ludev
LINKER_OBJECTS += -ldl
LINKER_OBJECTS += -lrt
LINKER_OBJECTS += -lm
LINKER_OBJECTS += -lpthread
LINKER_OBJECTS += -lz

all: 
	$(info   *************** pcsx2 all ***************)
	gcc -g3 -O0 -ggdb -o PCSX2 main.cpp $(LINKER_OBJECTS)


clean:
	$(info   *************** pcsx2 clean ***************)
	rm -f *.o PCSX2

