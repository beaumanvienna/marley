SUBDIRS = pcsx2
SUBDIRS += mupen64plus
SUBDIRS += mednafen
SUBDIRS += dolphin
SUBDIRS += ppsspp
SUBDIRS += tests

#marley
if DEBUG
AM_CXXFLAGS		= --std=c++17 -ggdb3 -g3 -O0 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core @GTK_HEADERS_INCLUDE@
AM_LDFLAGS		= --std=c++17 -ggdb3 -g3 -O0 -fno-pie -no-pie -L/usr/lib/x86_64-linux-gnu
else
AM_CXXFLAGS		= --std=c++17 -O2 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core @GTK_HEADERS_INCLUDE@
AM_LDFLAGS		= --std=c++17 -O2 -fno-pie -no-pie -s -L/usr/lib/x86_64-linux-gnu
endif


bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp src/emu.cpp src/wii.cpp resources/res.cpp resources/pcsx2_res.cpp

#generate built-in resources
BUILT_SOURCES = resources/res.cpp
resources/res.cpp: resources/res.xml resources/res.h @RESOURCE_LIST@ 
	glib-compile-resources resources/res.xml --target=resources/res.cpp --sourcedir=resources/ --generate-source

resources/res.h: resources/res.xml @RESOURCE_LIST@ resources/ppsspp_assets.zip resources/dolphin_data_sys.zip
	glib-compile-resources resources/res.xml --target=resources/res.h --sourcedir=resources/ --generate-header

resources/ppsspp_assets.zip: 
	cd ppsspp && zip -r ../resources/ppsspp_assets.zip assets && cd ..

resources/dolphin_data_sys.zip: 
	cd dolphin && zip -r ../resources/dolphin_data_sys.zip Data/sys && cd ..
	
#pcsx2

pcsx2/build/pcsx2/libPCSX2.a: pcsx2 resources/pcsx2_res.cpp

resources/pcsx2/libCDVDnull.so: 		pcsx2/build/plugins/CDVDnull/libCDVDnull.so
resources/pcsx2/libcdvdGigaherz.so: 	pcsx2/build/plugins/cdvdGigaherz/src/libcdvdGigaherz.so
resources/pcsx2/libUSBnull-0.7.0.so: 	pcsx2/build/plugins/USBnull/libUSBnull-0.7.0.so
resources/pcsx2/libspu2x-2.0.0.so: 		pcsx2/build/plugins/spu2-x/src/libspu2x-2.0.0.so
resources/pcsx2/libFWnull-0.7.0.so: 	pcsx2/build/plugins/FWnull/libFWnull-0.7.0.so
resources/pcsx2/libdev9ghzdrk-0.4.so: 	pcsx2/build/plugins/dev9ghzdrk/libdev9ghzdrk-0.4.so
resources/pcsx2/libdev9null-0.5.0.so: 	pcsx2/build/plugins/dev9null/libdev9null-0.5.0.so


#generate pcsx2 built-in resources

PCSX2_RESOURCES =  resources/pcsx2/libCDVDnull.so
PCSX2_RESOURCES += resources/pcsx2/libcdvdGigaherz.so
PCSX2_RESOURCES += resources/pcsx2/libUSBnull-0.7.0.so
PCSX2_RESOURCES += resources/pcsx2/libspu2x-2.0.0.so
PCSX2_RESOURCES += resources/pcsx2/libFWnull-0.7.0.so
PCSX2_RESOURCES += resources/pcsx2/libdev9ghzdrk-0.4.so
PCSX2_RESOURCES += resources/pcsx2/libdev9null-0.5.0.so

$(PCSX2_RESOURCES):
	cp -f $< $@


resources/pcsx2_res.cpp: resources/pcsx2_res.xml resources/pcsx2_res.h $(PCSX2_RESOURCES)
	glib-compile-resources resources/pcsx2_res.xml --target=resources/pcsx2_res.cpp --sourcedir=resources/ --generate-source

#the header needs to be generated upfront
#trick glib-compile-resources with touch
BUILT_SOURCES += resources/pcsx2_res.h
resources/pcsx2_res.h: resources/pcsx2_res.xml
	rm -rf resources/pcsx2/
	mkdir resources/pcsx2/
	touch $(PCSX2_RESOURCES)
	glib-compile-resources resources/pcsx2_res.xml --target=resources/pcsx2_res.h --sourcedir=resources/ --generate-header
	rm -f resources/pcsx2/*.so

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
if DEBUG
dolphin/build/Source/Core/../../Externals/fmt/libfmtd.a: dolphin
else
dolphin/build/Source/Core/../../Externals/fmt/libfmt.a: dolphin
endif
dolphin/build/Source/Core/../../Externals/bzip2/libbzip2.a: dolphin
dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a: dolphin
dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a: dolphin
dolphin/build/Source/Core/../../Externals/minizip/libminizip.a: dolphin
dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a: dolphin

dolphin:
	$(info   ++++++++++++++++ dolphin ++++++++++++++++)
	$(MAKE) -C dolphin all

#mednafen, static
marley_LDADD 	= mednafen/src/libmednafen_marley.a

#ppsspp, static
PPSSPP_LD = \
	ppsspp/build/lib/libglslang.a \
	ppsspp/build/lib/libxxhash.a \
	ppsspp/build/lib/liblibzip.a \
	ppsspp/build/lib/libspirv-cross-cpp.a \
	ppsspp/build/lib/libspirv-cross-glsl.a \
	ppsspp/build/lib/libdiscord-rpc.a \
	ppsspp/build/lib/libudis86.a \
	ppsspp/build/lib/libOSDependent.a \
	ppsspp/build/lib/libspirv-cross-core.a \
	ppsspp/build/lib/libspirv-cross-msl.a \
	ppsspp/build/lib/libsfmt19937.a \
	ppsspp/build/lib/libPPSSPPSDL.a \
	ppsspp/build/lib/libspirv-cross-hlsl.a \
	ppsspp/build/lib/libcityhash.a \
	ppsspp/build/lib/libkirk.a \
	ppsspp/build/lib/libarmips.a \
	ppsspp/build/lib/libnative.a \
	ppsspp/build/lib/libOGLCompiler.a \
	ppsspp/build/lib/libgason.a \
	ppsspp/build/lib/libxbrz.a \
	ppsspp/build/lib/libSPVRemapper.a \
	ppsspp/build/lib/libCore.a \
	ppsspp/build/lib/libHLSL.a \
	ppsspp/build/lib/libCommon.a \
	ppsspp/build/lib/libSPIRV.a \
	ppsspp/build/lib/libsnappy.a \
	ppsspp/ffmpeg/linux/x86_64/lib/libavcodec.a \
	ppsspp/ffmpeg/linux/x86_64/lib/libavutil.a \
	ppsspp/ffmpeg/linux/x86_64/lib/libswscale.a \
	ppsspp/ffmpeg/linux/x86_64/lib/libavformat.a \
	ppsspp/ffmpeg/linux/x86_64/lib/libswresample.a \
	ppsspp/build/lib/libminiupnpc.a
marley_LDADD 	+= $(PPSSPP_LD) $(PPSSPP_LD) $(PPSSPP_LD)

#mupen64plus, static
marley_LDADD 	+= mupen64plus/Source/mupen.a

#dolphin, static
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
if DEBUG
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/fmt/libfmtd.a 
else
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/fmt/libfmt.a 
endif
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/bzip2/libbzip2.a
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/minizip/libminizip.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a  


#ppsspp, dynamic
marley_LDADD 	+= -lGLEW

#mupen64plus, dynamic
marley_LDADD 	+= -lGL -lstdc++ -lGLU -lfreetype -lboost_system -lboost_filesystem -lsamplerate

#pcsx2, static
marley_LDADD 	+= pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Cache.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/COP0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/COP2.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Counters.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/GameDatabase.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Dump.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Elfheader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/EmuCmp.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/FiFo.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/FPU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Gif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Gif_Logger.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Gif_Unit.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/GS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/GSState.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Hw.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/HwRead.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/HwWrite.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Interpreter.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopBios.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopCounters.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopDma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopGte.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopHw.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopIrq.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopMem.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IopSio2.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Mdec.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Memory.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MMI.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MTGS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MTVU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/MultipartFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/OutputIsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Patch.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Patch_Memory.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Pcsx2Config.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/PluginManager.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/PrecompiledHeader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R3000A.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R3000AInterpreter.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R3000AOpcodeTables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R5900.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R5900OpcodeImpl.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/R5900OpcodeTables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/SaveState.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ShiftJisToUnicode.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sif0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sif1.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/sif2.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Sio.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/SourceLog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/SPR.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/System.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif0_Dma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif1_Dma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif1_MFIFO.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif_Codes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif_Transfer.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Vif_Unpack.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/vtlb.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUmicro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU0micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU0microInterp.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU1micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VU1microInterp.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUflags.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUmicroMem.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/VUops.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/BlockdumpFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CdRom.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CDVDaccess.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CDVD.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CDVDisoReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/InputIsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/OutputIsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/ChunksCache.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CompressedFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/CsoFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/GzippedFileReader.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/IsoFS/IsoFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/IsoFS/IsoFSCDVD.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/CDVD/IsoFS/IsoFS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DebugInterface.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisassemblyManager.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/ExpressionParser.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MIPSAnalyst.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MipsAssembler.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MipsAssemblerTables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/MipsStackWalk.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/Breakpoints.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/SymbolMap.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisR3000A.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisR5900asm.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisVU0Micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/DisVU1Micro.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/DebugTools/BiosDebugData.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppAssert.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppConfig.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppCorePlugins.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppCoreThread.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppEventSources.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppGameDatabase.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppUserMode.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppInit.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppMain.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/AppRes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/ConsoleLogger.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/CpuUsageProvider.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/AboutBoxDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/AssertionDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/BaseConfigurationDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/ConfirmationDialogs.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/ConvertMemoryCardDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/CreateMemoryCardDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/FirstTimeWizard.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/ImportSettingsDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/LogOptionsDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/McdConfigDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/PickUserModeDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Dialogs/SysConfigDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/BreakpointWindow.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/CtrlDisassemblyView.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/CtrlRegisterList.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/CtrlMemView.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/DebuggerLists.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/DisassemblyDialog.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Debugger/DebugEvents.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/ExecutorThread.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/FrameForGS.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/GlobalCommands.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/i18n.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/IsoDropTarget.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MainFrame.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MainMenuClicks.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MemoryCardFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MemoryCardFolder.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MessageBoxes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/MSWstuff.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/BaseApplicableConfigPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/BiosSelectorPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/CpuPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/DirPickerPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/GameFixesPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/GSWindowPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/LogOptionsPanels.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/MemoryCardListPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/MemoryCardListView.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/MiscPanelStuff.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/PathsPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/PluginSelectorPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/SpeedhacksPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Panels/VideoPanel.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/RecentIsoList.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/Saveslots.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/SysState.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/UpdateUI.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPU_Fifo.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPUdither.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/IPUdma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/mpeg2lib/Idct.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/mpeg2lib/Mpeg.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/IPU/yuv2rgb.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/BiosTools.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/pgif.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/LegacyDmac.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/Iop/IopHwRead.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/Iop/IopHwWrite.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ps2/Iop/PsxBios.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/InputRecording.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/InputRecordingFile.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/NewRecordingFrame.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/PadData.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/RecordingControls.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/RecordingInputManager.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Recording/VirtualPad.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/System/SysCoreThread.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/System/SysThreadBase.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Utilities/FileUtils.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/BaseblockEx.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iCOP0.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iCore.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iFPU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iFPUd.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iMisc.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iMMI.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iR3000A.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iR3000Atables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/iR5900Misc.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ir5900tables.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iCore-32.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900-32.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Arit.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900AritImm.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Branch.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Jump.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900LoadStore.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Move.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900MultDiv.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Shift.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/iR5900Templates.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/ix86-32/recVTLB.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/newVif_Dynarec.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/newVif_Unpack.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/newVif_UnpackSSE.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/x86/microVU.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ZipTools/thread_gzip.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/ZipTools/thread_lzma.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/gui/CpuUsageProviderLnx.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Linux/LnxConsolePipe.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Linux/LnxKeyCodes.cpp.o pcsx2/build/pcsx2/CMakeFiles/PCSX2.dir/Linux/LnxFlatFileReader.cpp.o
marley_LDADD 	+= pcsx2/build/pcsx2/libPCSX2.a
marley_LDADD 	+= pcsx2/build/common/src/x86emitter/libx86emitter.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
marley_LDADD 	+= pcsx2/build/plugins/onepad/libonepad.a
marley_LDADD 	+= pcsx2/build/plugins/GSdx/libGSdx.a


#dolphin, static
marley_LDADD 	+= -lQt5Widgets
marley_LDADD 	+= -lasound 
marley_LDADD 	+= -llzo2
marley_LDADD 	+= -lavformat
marley_LDADD 	+= -lavcodec
marley_LDADD 	+= -lswscale
marley_LDADD 	+= -lavutil
marley_LDADD 	+= -lQt5Gui
marley_LDADD 	+= -lQt5Core
marley_LDADD 	+= -lcurl  
marley_LDADD 	+= -lICE 
marley_LDADD 	+= -lX11
marley_LDADD 	+= -lXext  
marley_LDADD 	+= -lSM 
marley_LDADD 	+= -lGLX 
marley_LDADD 	+= -lminiupnpc 
marley_LDADD 	+= -lusb-1.0 
marley_LDADD 	+= -levdev 
marley_LDADD 	+= -ludev  
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
marley_LDADD 	+= -lpthread
marley_LDADD 	+= -lSDL2
marley_LDADD 	+= -lz
marley_LDADD 	+= -lEGL
marley_LDADD 	+= -lGL
marley_LDADD 	+= -lGLU
marley_LDADD 	+= -lOpenGL
marley_LDADD 	+= -ljack 
marley_LDADD 	+= -lzstd 
marley_LDADD 	+= -lEGL
marley_LDADD 	+= -llzma 
marley_LDADD 	+= -lbluetooth
marley_LDADD 	+= -lmbedtls
marley_LDADD 	+= -lmbedx509
marley_LDADD 	+= -lmbedcrypto 

#mednafen, dynamic
marley_LDADD 	+= -lSDL2 -lSDL2_image -lSDL2_ttf 
marley_LDADD 	+= -lasound -lm -ldl -lpthread -ljack -lsndfile -lz

#pcsx2, dynamic
marley_LDADD 	+= -lwx_baseu-3.0 
if PCSX2_USE_GTK3
marley_LDADD 	+= -lwx_gtk3u_core-3.0 
marley_LDADD 	+= -lwx_gtk3u_adv-3.0 
marley_LDADD 	+= -lgtk-3
marley_LDADD 	+= -lgdk-3
else
marley_LDADD 	+= -lwx_gtk2u_core-3.0
marley_LDADD 	+= -lwx_gtk2u_adv-3.0
marley_LDADD 	+= -lgdk-x11-2.0
marley_LDADD 	+= -lgtk-x11-2.0
endif

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
marley_LDADD 	+= -lXfixes
