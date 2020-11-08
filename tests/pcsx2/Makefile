#standalone pcsx2 test version

COMPILER_ARTIFACTS = --std=c++17 -I/usr/include/SDL2

#USE_FRONTEND=YES

LINKER_OBJECTS = ../../pcsx2/build/pcsx2/libPCSX2.a
LINKER_OBJECTS += ../../pcsx2/build/common/src/x86emitter/libx86emitter.a
LINKER_OBJECTS += ../../pcsx2/build/common/src/Utilities/libUtilities.a
LINKER_OBJECTS += ../../pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
LINKER_OBJECTS += ../../pcsx2/build/3rdparty/fmt/fmt/libfmt.a
LINKER_OBJECTS += ../../pcsx2/build/plugins/onepad/libonepad.a
LINKER_OBJECTS += ../../pcsx2/build/plugins/GSdx/libGSdx.a
LINKER_OBJECTS += ../../pcsx2/build/plugins/dev9null/libdev9null-0.5.0.a
LINKER_OBJECTS += ../../pcsx2/build/plugins/USBnull/libUSBnull-0.7.0.a

PCSX2_USE_GTK3 = $(shell find /usr/ -name "*wx_gtk3u_core-3.0*.so" 2>/dev/null)
ifeq ($(PCSX2_USE_GTK3),)
LINKER_OBJECTS 	+= -lwx_baseu-3.0
LINKER_OBJECTS 	+= -lwx_gtk2u_core-3.0
LINKER_OBJECTS 	+= -lwx_gtk2u_adv-3.0
LINKER_OBJECTS 	+= -lgdk-x11-2.0
LINKER_OBJECTS 	+= -lgtk-x11-2.0
GTK_VERSION      = GTK2
else
LINKER_OBJECTS 	+= -l$(shell find /usr/ -name "*wx_baseu-3.0*.so"      2>/dev/null| xargs basename | sed 's/lib//' | sed 's/\.so//')
LINKER_OBJECTS 	+= -l$(shell find /usr/ -name "*wx_gtk3u_core-3.0*.so" 2>/dev/null| xargs basename | sed 's/lib//' | sed 's/\.so//')
LINKER_OBJECTS 	+= -l$(shell find /usr/ -name "*wx_gtk3u_adv-3.0*.so"  2>/dev/null| xargs basename | sed 's/lib//' | sed 's/\.so//')
LINKER_OBJECTS 	+= -lgtk-3
LINKER_OBJECTS 	+= -lgdk-3
GTK_VERSION      = GTK3
endif

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
LINKER_OBJECTS += -laio
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
LINKER_OBJECTS += -lpng
LINKER_OBJECTS += -llzma

LINKER_OBJECTS += -lSDL2
LINKER_OBJECTS += -lX11 
LINKER_OBJECTS += -lX11-xcb
LINKER_OBJECTS += -lxcb
LINKER_OBJECTS += -lEGL
LINKER_OBJECTS += -lSoundTouch
LINKER_OBJECTS += -lasound

ifeq ($(USE_FRONTEND),YES)
LINKER_OBJECTS += -lSDL2_image -lSDL2_ttf -lXfixes
COMPILER_ARTIFACTS += $(shell pkg-config --cflags gtk+-3.0)
COMPILER_ARTIFACTS += -DPACKAGE_VERSION=\"9.9.9\"
SOURCE_FILES = ../../resources/res.cpp
SOURCE_FILES += ../../src/controller.cpp
SOURCE_FILES += ../../src/emu.cpp
SOURCE_FILES += ../../src/gui.cpp
SOURCE_FILES += ../../src/marley.cpp
SOURCE_FILES += ../../src/mdf2iso.cpp
SOURCE_FILES += ../../src/statemachine.cpp
SOURCE_FILES += ../../src/wii.cpp
else
SOURCE_FILES = main.cpp
endif


all: PCSX2

PCSX2: $(SOURCE_FILES) Makefile
	$(info   *************** tests make pcsx2 ***************)
	$(info $(GTK_VERSION))
	$(MAKE) -C ../../pcsx2/build
	g++ $(COMPILER_ARTIFACTS) -o PCSX2 $(SOURCE_FILES) $(LINKER_OBJECTS)

clean:
	$(info   *************** tests pcsx2 clean ***************)
	rm -f *.o PCSX2 GSdx_opengl_debug_hw.txt

