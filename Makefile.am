SUBDIRS = pcsx2
SUBDIRS += mupen64plus
SUBDIRS += mednafen
SUBDIRS += dolphin
SUBDIRS += ppsspp

pcsx2/build/pcsx2/libPCSX2.a: pcsx2

pcsx2:
	$(info   ++++++++++++++++ pcsx2 ++++++++++++++++)
	$(MAKE) -C pcsx2 all

mupen64plus/Source/mupen.a: mupen64plus

mupen64plus:
	$(info   ++++++++++++++++ mupen64plus ++++++++++++++++)
	$(MAKE) -C mupen64plus all


dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a: dolphin
dolphin/build/Source/Core/Core/libcore.a: dolphin
dolphin/build/Source/Core/UICommon/libuicommon.a: dolphin
dolphin/build/Source/Core/../../Externals/imgui/libimgui.a: dolphin
dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a: dolphin
dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a: dolphin
dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a: dolphin
dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a: dolphin
dolphin/build/Source/Core/VideoCommon/libvideocommon.a: dolphin
dolphin/build/Source/Core/Core/libcore.a: dolphin
dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a: dolphin
dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a: dolphin
dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a: dolphin
dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a: dolphin
dolphin/build/Source/Core/VideoCommon/libvideocommon.a: dolphin
dolphin/build/Source/Core/AudioCommon/libaudiocommon.a: dolphin
dolphin/build/Source/Core/../../Externals/soundtouch/libSoundTouch.a: dolphin
dolphin/build/Source/Core/../../Externals/FreeSurround/libFreeSurround.a: dolphin
dolphin/build/Source/Core/../../Externals/cubeb/libcubeb.a: dolphin
dolphin/build/Source/Core/DiscIO/libdiscio.a: dolphin
dolphin/build/Source/Core/InputCommon/libinputcommon.a: dolphin
dolphin/build/Source/Core/../../Externals/hidapi/libhidapi.a: dolphin
dolphin/build/Source/Core/../../Externals/glslang/libglslang.a: dolphin
dolphin/build/Source/Core/../../Externals/imgui/libimgui.a: dolphin
dolphin/build/Source/Core/../../Externals/xxhash/libxxhash.a: dolphin
dolphin/build/Source/Core/Common/libcommon.a: dolphin
dolphin/build/Source/Core/../../Externals/enet/libenet.a: dolphin
dolphin/build/Source/Core/../../Externals/pugixml/libpugixml.a: dolphin
dolphin/build/Source/Core/../../Externals/fmt/libfmt.a: dolphin
dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a: dolphin
dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a: dolphin
dolphin/build/Source/Core/../../Externals/minizip/libminizip.a: dolphin
dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a: dolphin

dolphin:
	$(info   ++++++++++++++++ dolphin ++++++++++++++++)
	$(MAKE) -C dolphin all

AM_CPPFLAGS		= --std=c++17 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core
AM_LDFLAGS		= --std=c++17 -fno-pie -no-pie -s -L/usr/lib/x86_64-linux-gnu 

bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp src/emu.cpp src/wii.cpp

#mednafen
marley_LDADD 	= mednafen/src/libmednafen_marley.a
marley_LDADD 	+= -lSDL2 -lSDL2_image -lSDL2_ttf 
marley_LDADD 	+= -lasound -lm -ldl -lpthread -ljack -lpthread -lsndfile -lz

#ppsspp
marley_LDADD 	+= ppsspp/build/lib/libglslang.a
marley_LDADD 	+= ppsspp/build/lib/libxxhash.a
marley_LDADD 	+= ppsspp/build/lib/liblibzip.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-cpp.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-glsl.a
marley_LDADD 	+= ppsspp/build/lib/libdiscord-rpc.a
marley_LDADD 	+= ppsspp/build/lib/libudis86.a
marley_LDADD 	+= ppsspp/build/lib/libOSDependent.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-core.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-msl.a
marley_LDADD 	+= ppsspp/build/lib/libsfmt19937.a
marley_LDADD 	+= ppsspp/build/lib/libPPSSPPSDL.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-hlsl.a
marley_LDADD 	+= ppsspp/build/lib/libcityhash.a
marley_LDADD 	+= ppsspp/build/lib/libkirk.a
marley_LDADD 	+= ppsspp/build/lib/libpng17.a
marley_LDADD 	+= ppsspp/build/lib/libarmips.a
marley_LDADD 	+= ppsspp/build/lib/libnative.a
marley_LDADD 	+= ppsspp/build/lib/libOGLCompiler.a
marley_LDADD 	+= ppsspp/build/lib/libgason.a
marley_LDADD 	+= ppsspp/build/lib/libxbrz.a
marley_LDADD 	+= ppsspp/build/lib/libSPVRemapper.a
marley_LDADD 	+= ppsspp/build/lib/librg_etc1.a
marley_LDADD 	+= ppsspp/build/lib/libCore.a
marley_LDADD 	+= ppsspp/build/lib/libHLSL.a
marley_LDADD 	+= ppsspp/build/lib/libCommon.a
marley_LDADD 	+= ppsspp/build/lib/libSPIRV.a
marley_LDADD 	+= ppsspp/build/lib/libsnappy.a
marley_LDADD 	+= -lswresample -lGLEW
marley_LDADD 	+= ppsspp/build/lib/libglslang.a
marley_LDADD 	+= ppsspp/build/lib/libxxhash.a
marley_LDADD 	+= ppsspp/build/lib/liblibzip.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-cpp.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-glsl.a
marley_LDADD 	+= ppsspp/build/lib/libdiscord-rpc.a
marley_LDADD 	+= ppsspp/build/lib/libudis86.a
marley_LDADD 	+= ppsspp/build/lib/libOSDependent.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-core.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-msl.a
marley_LDADD 	+= ppsspp/build/lib/libsfmt19937.a
marley_LDADD 	+= ppsspp/build/lib/libPPSSPPSDL.a
marley_LDADD 	+= ppsspp/build/lib/libspirv-cross-hlsl.a
marley_LDADD 	+= ppsspp/build/lib/libcityhash.a
marley_LDADD 	+= ppsspp/build/lib/libkirk.a
marley_LDADD 	+= ppsspp/build/lib/libpng17.a
marley_LDADD 	+= ppsspp/build/lib/libarmips.a
marley_LDADD 	+= ppsspp/build/lib/libnative.a
marley_LDADD 	+= ppsspp/build/lib/libOGLCompiler.a
marley_LDADD 	+= ppsspp/build/lib/libgason.a
marley_LDADD 	+= ppsspp/build/lib/libxbrz.a
marley_LDADD 	+= ppsspp/build/lib/libSPVRemapper.a
marley_LDADD 	+= ppsspp/build/lib/librg_etc1.a
marley_LDADD 	+= ppsspp/build/lib/libCore.a
marley_LDADD 	+= ppsspp/build/lib/libHLSL.a
marley_LDADD 	+= ppsspp/build/lib/libCommon.a
marley_LDADD 	+= ppsspp/build/lib/libSPIRV.a
marley_LDADD 	+= ppsspp/build/lib/libsnappy.a

#mupen64plus
marley_LDADD 	+= mupen64plus/Source/mupen.a
marley_LDADD 	+= -lGL -lstdc++ -lGLU -lfreetype -lboost_system -lboost_filesystem -lsamplerate

#dolphin
marley_LDADD 	+= dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a
marley_LDADD 	+= dolphin/build/Source/Core/Core/libcore.a 
marley_LDADD 	+= dolphin/build/Source/Core/UICommon/libuicommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/imgui/libimgui.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoCommon/libvideocommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/Core/libcore.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a 
marley_LDADD 	+= dolphin/build/Source/Core/VideoCommon/libvideocommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/AudioCommon/libaudiocommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/soundtouch/libSoundTouch.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/FreeSurround/libFreeSurround.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cubeb/libcubeb.a 
marley_LDADD 	+= dolphin/build/Source/Core/DiscIO/libdiscio.a 
marley_LDADD 	+= dolphin/build/Source/Core/InputCommon/libinputcommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/hidapi/libhidapi.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/glslang/libglslang.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/imgui/libimgui.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/xxhash/libxxhash.a 
marley_LDADD 	+= dolphin/build/Source/Core/Common/libcommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/enet/libenet.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/pugixml/libpugixml.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/fmt/libfmt.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/minizip/libminizip.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a  
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5.9.5
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libasound.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libpulse.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/liblzo2.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libz.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libbluetooth.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libavformat.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libavcodec.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libswscale.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libavutil.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5.9.5 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.9.5 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libmbedtls.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libmbedx509.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libmbedcrypto.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libcurl.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libICE.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libX11.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libXext.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libSM.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libEGL.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libOpenGL.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libGLX.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libGLU.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libminiupnpc.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libusb-1.0.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libevdev.so 
marley_LDADD 	+= /lib/x86_64-linux-gnu/libudev.so 
marley_LDADD 	+= -lXi 
marley_LDADD 	+= -lsfml-network 
marley_LDADD 	+= -lsfml-system 
marley_LDADD 	+= -ludev 
marley_LDADD 	+= -lc 
marley_LDADD 	+= -lpng 
marley_LDADD    += -ldl
marley_LDADD 	+= -lrt 
marley_LDADD 	+= -lXrandr 
marley_LDADD 	+= -lpulse 
marley_LDADD 	+= -lsndio 

#pcsx2
marley_LDADD 	+= pcsx2/build/pcsx2/libPCSX2.a
marley_LDADD 	+= pcsx2/build/pcsx2/libPCSX2.a
marley_LDADD 	+= pcsx2/build/common/src/x86emitter/libx86emitter.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
marley_LDADD 	+= -lwx_baseu-3.0 
marley_LDADD 	+= -lwx_gtk2u_core-3.0 
marley_LDADD 	+= -lwx_gtk2u_adv-3.0 
marley_LDADD 	+= -lglib-2.0
marley_LDADD 	+= -lgobject-2.0
marley_LDADD 	+= -latk-1.0
marley_LDADD 	+= -lgio-2.0
marley_LDADD 	+= -lgthread-2.0
marley_LDADD 	+= -lgmodule-2.0
marley_LDADD 	+= -lgdk_pixbuf-2.0
marley_LDADD 	+= -lcairo
marley_LDADD 	+= -lpango-1.0
marley_LDADD 	+= -lpangocairo-1.0
marley_LDADD 	+= -lpangoft2-1.0
marley_LDADD 	+= -lpangoxft-1.0
marley_LDADD 	+= -lgdk-x11-2.0
marley_LDADD 	+= -lgtk-x11-2.0
marley_LDADD 	+= -laio
marley_LDADD 	+= -lwx_baseu-3.0
marley_LDADD 	+= -lwx_gtk2u_core-3.0 
marley_LDADD 	+= -lwx_gtk2u_adv-3.0 
