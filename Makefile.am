SUBDIRS = mednafen
SUBDIRS += dolphin

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

mupen64plus/Source/audio-sdl/mupen64plus-audio-sdl.a: mupen64plus
mupen64plus/Source/core/libmupen64plus.a: mupen64plus
mupen64plus/Source/input-sdl/mupen64plus-input-sdl.a: mupen64plus
mupen64plus/Source/rsp-hle/mupen64plus-rsp-hle.a: mupen64plus
mupen64plus/Source/ui-console/mupen64plus: mupen64plus
mupen64plus/Source/video-glide64mk2/mupen64plus-video-glide64mk2.a: mupen64plus
mupen64plus/Source/video-rice/mupen64plus-video-rice.a: mupen64plus

mupen64plus:
	$(info   ++++++++++++++++ mupen64plus ++++++++++++++++)
	$(MAKE) -C mupen64plus all

AM_CPPFLAGS		= --std=c++17 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core
AM_LDFLAGS		= --std=c++17 -fno-pie -no-pie -s

bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp src/emu.cpp src/wii.cpp

marley_LDADD 	= mednafen/src/libmednafen_marley.a
marley_LDADD 	+= -lSDL2 -lSDL2_image -lSDL2_ttf 
marley_LDADD 	+= -lasound -lm -ldl -lpthread -ljack -lpthread -lsndfile -lz

marley_LDADD 	+= mupen64plus/Source/audio-sdl/mupen64plus-audio-sdl.a
marley_LDADD 	+= mupen64plus/Source/core/libmupen64plus.a
marley_LDADD 	+= mupen64plus/Source/input-sdl/mupen64plus-input-sdl.a
marley_LDADD 	+= mupen64plus/Source/rsp-hle/mupen64plus-rsp-hle.a
marley_LDADD 	+= mupen64plus/Source/ui-console/mupen64plus
marley_LDADD 	+= mupen64plus/Source/video-glide64mk2/mupen64plus-video-glide64mk2.a
marley_LDADD 	+= mupen64plus/Source/video-rice/mupen64plus-video-rice.a

marley_LDADD 	+= dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a
#marley_LDADD 	+= dolphin/build/Source/Core/DolphinQt/libdolphin-emu.a
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
marley_LDADD 	+= -ldl 
marley_LDADD 	+= -lrt 
marley_LDADD 	+= -lXrandr 
marley_LDADD 	+= -lpulse 
marley_LDADD 	+= -lsndio 


