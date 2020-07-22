SUBDIRS = pcsx2
SUBDIRS += mupen64plus
SUBDIRS += mednafen
SUBDIRS += dolphin
SUBDIRS += ppsspp
SUBDIRS += tests

#marley
AM_CPPFLAGS		= --std=c++17 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core
AM_LDFLAGS		= --std=c++17 -fno-pie -no-pie -s -L/usr/lib/x86_64-linux-gnu 

bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp src/emu.cpp src/wii.cpp

#pcsx2
COPY_FILES =  pcsx2/build/plugins/libCDVDnull.so 
COPY_FILES += pcsx2/build/plugins/libcdvdGigaherz.so 
COPY_FILES += pcsx2/build/plugins/libUSBnull-0.7.0.so 
COPY_FILES += pcsx2/build/plugins/libspu2x-2.0.0.so
COPY_FILES += pcsx2/build/plugins/libFWnull-0.7.0.so  
COPY_FILES += pcsx2/build/plugins/libdev9ghzdrk-0.4.so 
COPY_FILES += pcsx2/build/plugins/libdev9null-0.5.0.so 

pcsx2/build/pcsx2/libPCSX2.a: pcsx2 $(COPY_FILES)

pcsx2/build/plugins/libCDVDnull.so: 		pcsx2/build/plugins/CDVDnull/libCDVDnull.so
pcsx2/build/plugins/libcdvdGigaherz.so: 	pcsx2/build/plugins/cdvdGigaherz/src/libcdvdGigaherz.so
pcsx2/build/plugins/libUSBnull-0.7.0.so: 	pcsx2/build/plugins/USBnull/libUSBnull-0.7.0.so
pcsx2/build/plugins/libspu2x-2.0.0.so: 		pcsx2/build/plugins/spu2-x/src/libspu2x-2.0.0.so
pcsx2/build/plugins/libFWnull-0.7.0.so: 	pcsx2/build/plugins/FWnull/libFWnull-0.7.0.so
pcsx2/build/plugins/libdev9ghzdrk-0.4.so: 	pcsx2/build/plugins/dev9ghzdrk/libdev9ghzdrk-0.4.so
pcsx2/build/plugins/libdev9null-0.5.0.so: 	pcsx2/build/plugins/dev9null/libdev9null-0.5.0.so

pcsx2/build/plugins/%:
	cp -f $< $@

pcsx2:
	$(info   ++++++++++++++++ pcsx2 ++++++++++++++++)
	$(MAKE) -C pcsx2 all

#mupen64plus
mupen64plus/Source/mupen.a: mupen64plus

mupen64plus:
	$(info   ++++++++++++++++ mupen64plus ++++++++++++++++)
	$(MAKE) -C mupen64plus all

#dolphin
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

#mednafen
marley_LDADD 	= mednafen/src/libmednafen_marley.a
marley_LDADD 	+= -lSDL2 -lSDL2_image -lSDL2_ttf 
marley_LDADD 	+= -lasound -lm -ldl -lpthread -ljack -lsndfile -lz

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
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libQt5Widgets.so
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libasound.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libpulse.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/liblzo2.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libz.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libbluetooth.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libavformat.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libavcodec.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libswscale.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libavutil.so 
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libQt5Gui.so
marley_LDADD 	+= /usr/lib/x86_64-linux-gnu/libQt5Core.so 
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
marley_LDADD 	+= pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Cache.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/COP0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/COP2.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Counters.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/GameDatabase.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Dump.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Elfheader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/EmuCmp.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/FiFo.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/FPU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Gif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Gif_Logger.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Gif_Unit.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/GS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/GSState.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Hw.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/HwRead.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/HwWrite.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Interpreter.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopBios.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopCounters.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopDma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopGte.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopHw.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopIrq.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopMem.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopSio2.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Mdec.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Memory.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MMI.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MTGS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MTVU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MultipartFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/OutputIsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Patch.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Patch_Memory.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Pcsx2Config.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/PluginManager.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/PrecompiledHeader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R3000A.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R3000AInterpreter.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R3000AOpcodeTables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R5900.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R5900OpcodeImpl.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R5900OpcodeTables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/SaveState.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ShiftJisToUnicode.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sif0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sif1.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/sif2.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sio.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/SourceLog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/SPR.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/System.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif0_Dma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif1_Dma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif1_MFIFO.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif_Codes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif_Transfer.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif_Unpack.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/vtlb.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUmicro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU0micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU0microInterp.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU1micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU1microInterp.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUflags.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUmicroMem.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUops.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/BlockdumpFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CdRom.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CDVDaccess.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CDVD.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CDVDisoReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/InputIsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/OutputIsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/ChunksCache.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CompressedFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CsoFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/GzippedFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/IsoFS/IsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/IsoFS/IsoFSCDVD.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/IsoFS/IsoFS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DebugInterface.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisassemblyManager.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/ExpressionParser.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MIPSAnalyst.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MipsAssembler.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MipsAssemblerTables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MipsStackWalk.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/Breakpoints.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/SymbolMap.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisR3000A.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisR5900asm.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisVU0Micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisVU1Micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/BiosDebugData.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppAssert.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppConfig.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppCorePlugins.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppCoreThread.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppEventSources.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppGameDatabase.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppUserMode.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppInit.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppMain.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppRes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/ConsoleLogger.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/CpuUsageProvider.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/AboutBoxDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/AssertionDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/BaseConfigurationDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/ConfirmationDialogs.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/ConvertMemoryCardDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/CreateMemoryCardDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/FirstTimeWizard.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/ImportSettingsDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/LogOptionsDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/McdConfigDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/PickUserModeDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/SysConfigDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/BreakpointWindow.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/CtrlDisassemblyView.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/CtrlRegisterList.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/CtrlMemView.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/DebuggerLists.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/DisassemblyDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/DebugEvents.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/ExecutorThread.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/FrameForGS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/GlobalCommands.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/i18n.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/IsoDropTarget.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MainFrame.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MainMenuClicks.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MemoryCardFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MemoryCardFolder.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MessageBoxes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MSWstuff.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/BaseApplicableConfigPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/BiosSelectorPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/CpuPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/DirPickerPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/GameFixesPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/GSWindowPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/LogOptionsPanels.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/MemoryCardListPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/MemoryCardListView.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/MiscPanelStuff.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/PathsPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/PluginSelectorPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/SpeedhacksPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/VideoPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/RecentIsoList.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Saveslots.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/SysState.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/UpdateUI.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPU_Fifo.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPUdither.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPUdma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/mpeg2lib/Idct.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/mpeg2lib/Mpeg.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/yuv2rgb.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/BiosTools.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/pgif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/LegacyDmac.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/Iop/IopHwRead.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/Iop/IopHwWrite.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/Iop/PsxBios.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/InputRecording.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/InputRecordingFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/NewRecordingFrame.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/PadData.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/RecordingControls.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/RecordingInputManager.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/VirtualPad.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/System/SysCoreThread.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/System/SysThreadBase.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Utilities/FileUtils.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/BaseblockEx.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iCOP0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iCore.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iFPU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iFPUd.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iMisc.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iMMI.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iR3000A.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iR3000Atables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iR5900Misc.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ir5900tables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iCore-32.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900-32.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Arit.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900AritImm.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Branch.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Jump.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900LoadStore.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Move.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900MultDiv.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Shift.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Templates.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/recVTLB.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/newVif_Dynarec.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/newVif_Unpack.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/newVif_UnpackSSE.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/microVU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ZipTools/thread_gzip.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ZipTools/thread_lzma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/CpuUsageProviderLnx.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Linux/LnxConsolePipe.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Linux/LnxKeyCodes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Linux/LnxFlatFileReader.cpp.o
marley_LDADD 	+= pcsx2/build/pcsx2/libPCSX2.a
marley_LDADD 	+= pcsx2/build/common/src/x86emitter/libx86emitter.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
marley_LDADD 	+= pcsx2/build/plugins/onepad/libonepad.a
marley_LDADD 	+= pcsx2/build/plugins/GSdx/libGSdx.a
marley_LDADD 	+= -lwx_baseu-3.0 

#marley_LDADD 	+= -lwx_gtk2u_core-3.0 
#marley_LDADD 	+= -lwx_gtk2u_adv-3.0 
#marley_LDADD 	+= -lgdk-x11-2.0
#marley_LDADD 	+= -lgtk-x11-2.0

marley_LDADD 	+= -lwx_gtk3u_core-3.0 
marley_LDADD 	+= -lwx_gtk3u_adv-3.0 
marley_LDADD 	+= -lgtk-3
marley_LDADD 	+= -lgdk-3

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
marley_LDADD 	+= -laio
marley_LDADD 	+= -llzma
