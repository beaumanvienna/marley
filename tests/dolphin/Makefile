#standalone dolphin test version


DEBUG=$(shell ls ../.debug_build 2>/dev/null)


ifeq ($(DEBUG), ../.debug_build)
COMPILER_ARTIFACTS = --std=c++17 -g3 -O0 -ggdb -I/usr/include/SDL2 -I../../dolphin/Source/Core
else
COMPILER_ARTIFACTS = --std=c++17 -I/usr/include/SDL2 -I../../dolphin/Source/Core
endif



LINKER_OBJECTS 	=  ../../dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/Core/libcore.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/UICommon/libuicommon.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/imgui/libimgui.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoCommon/libvideocommon.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/Core/libcore.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/VideoCommon/libvideocommon.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/AudioCommon/libaudiocommon.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/soundtouch/libSoundTouch.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/FreeSurround/libFreeSurround.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/cubeb/libcubeb.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/DiscIO/libdiscio.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/InputCommon/libinputcommon.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/hidapi/libhidapi.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/glslang/libglslang.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/imgui/libimgui.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/xxhash/libxxhash.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/Common/libcommon.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/enet/libenet.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/pugixml/libpugixml.a 
ifeq ($(DEBUG), ../.debug_build)
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/fmt/libfmtd.a 
else
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/fmt/libfmt.a 
endif
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/bzip2/libbzip2.a
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/minizip/libminizip.a 
LINKER_OBJECTS 	+= ../../dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a  
LINKER_OBJECTS 	+= -lQt5Widgets
LINKER_OBJECTS 	+= -lasound 
LINKER_OBJECTS 	+= -llzo2
LINKER_OBJECTS 	+= -lavformat
LINKER_OBJECTS 	+= -lavcodec
LINKER_OBJECTS 	+= -lswscale
LINKER_OBJECTS 	+= -lavutil
LINKER_OBJECTS 	+= -lQt5Gui
LINKER_OBJECTS 	+= -lQt5Core
LINKER_OBJECTS 	+= -lcurl  
LINKER_OBJECTS 	+= -lICE 
LINKER_OBJECTS 	+= -lX11
LINKER_OBJECTS 	+= -lXext  
LINKER_OBJECTS 	+= -lSM 
LINKER_OBJECTS 	+= -lGLX 
LINKER_OBJECTS 	+= -lminiupnpc 
LINKER_OBJECTS 	+= -lusb-1.0 
LINKER_OBJECTS 	+= -levdev 
LINKER_OBJECTS 	+= -ludev  
LINKER_OBJECTS 	+= -lXi 
LINKER_OBJECTS 	+= -lsfml-network 
LINKER_OBJECTS 	+= -lsfml-system 
LINKER_OBJECTS 	+= -ludev 
LINKER_OBJECTS 	+= -lc 
LINKER_OBJECTS 	+= -lpng 
LINKER_OBJECTS 	+= -ldl
LINKER_OBJECTS 	+= -lrt 
LINKER_OBJECTS 	+= -lXrandr 
LINKER_OBJECTS 	+= -lpulse 
LINKER_OBJECTS 	+= -lpthread
LINKER_OBJECTS 	+= -lSDL2
LINKER_OBJECTS 	+= -lz
LINKER_OBJECTS 	+= -lEGL
LINKER_OBJECTS 	+= -lGL
LINKER_OBJECTS 	+= -lGLU
LINKER_OBJECTS 	+= -lOpenGL
LINKER_OBJECTS 	+= -ljack 
LINKER_OBJECTS 	+= -lzstd 
LINKER_OBJECTS 	+= -lEGL
LINKER_OBJECTS 	+= -llzma 
LINKER_OBJECTS 	+= -lbluetooth
LINKER_OBJECTS 	+= -lmbedtls
LINKER_OBJECTS 	+= -lmbedx509
LINKER_OBJECTS 	+= -lmbedcrypto


all: DOLPHIN

DOLPHIN: main.cpp
	$(info   *************** tests make dolphin ***************)
	$(MAKE) -C ../../dolphin/build all
	g++ $(COMPILER_ARTIFACTS) -o DOLPHIN main.cpp $(LINKER_OBJECTS)

clean:
	$(info   *************** tests dolphin clean ***************)
	rm -f *.o DOLPHIN

