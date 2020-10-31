
# directories
SUBDIRS = pcsx2
SUBDIRS += mupen64plus
SUBDIRS += mednafen
#SUBDIRS += dolphin
SUBDIRS += ppsspp
SUBDIRS += tests

#installation files
man6_MANS = resources/marley.6

applauncherdir = /usr/share/applications
applauncher_DATA = resources/marley.desktop

appicondir = /usr/share/icons/
appicon_DATA = resources/marley.ico

#additional make clean targets
clean-local:
	-rm -rf confdefs.h
	-rm -rf tests/.debug_build
	-rm -rf resources/dolphin_data_sys.zip
	-rm -rf resources/pcsx2
	-rm -rf resources/*.cpp
	-rm -rf resources/*.h
	-rm -rf resources/ppsspp_assets.zip
	-rm -rf ppsspp/ffmpeg/libavutil/*.o
	-rm -rf ppsspp/ffmpeg/libswscale/*.o
	-rm -rf ppsspp/ffmpeg/libavcodec/*.o
	-rm -rf ppsspp/ffmpeg/libavformat/*.o
	-rm -rf ppsspp/ffmpeg/libswresample/*.o
	-rm -rf ppsspp/ffmpeg/libavutil/*.a
	-rm -rf ppsspp/ffmpeg/libswscale/*.a
	-rm -rf ppsspp/ffmpeg/libavcodec/*.a
	-rm -rf ppsspp/ffmpeg/libavformat/*.a
	-rm -rf ppsspp/ffmpeg/libswresample/*.a
	-rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libavutil.a
	-rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libavformat.a
	-rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libswscale.a
	-rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libswresample.a
	-rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libavcodec.a
	-rm -rf ppsspp/ffmpeg/libavutil/x86/*.o
	-rm -rf ppsspp/ffmpeg/libswscale/x86/*.o
	-rm -rf ppsspp/ffmpeg/libavcodec/x86/*.o
	-rm -rf ppsspp/ffmpeg/libavcodec/x86/*.o
	-rm -rf ppsspp/ffmpeg/libswresample/x86/*.o

#front end compiler settings
if DEBUG
AM_CXXFLAGS		= --std=c++17 -ggdb3 -g3 -O0 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core @GTK_HEADERS_INCLUDE@
AM_LDFLAGS		= --std=c++17 -ggdb3 -g3 -O0 -fno-pie -no-pie -L/usr/lib/x86_64-linux-gnu
else
AM_CXXFLAGS		= --std=c++17 -O2 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core @GTK_HEADERS_INCLUDE@
AM_LDFLAGS		= --std=c++17 -O2 -fno-pie -no-pie -s -L/usr/lib/x86_64-linux-gnu
endif

#project target

bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp src/emu.cpp src/wii.cpp resources/res.cpp resources/pcsx2_res.cpp src/mdf2iso.cpp

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

resources/pcsx2/libUSBnull-0.7.0.so: 	pcsx2/build/plugins/USBnull/libUSBnull-0.7.0.so
resources/pcsx2/libdev9null-0.5.0.so: 	pcsx2/build/plugins/dev9null/libdev9null-0.5.0.so

#generate pcsx2 built-in resources

PCSX2_RESOURCES = resources/pcsx2/libUSBnull-0.7.0.so
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

mupen64plus:
	$(info   ++++++++++++++++ mupen64plus ++++++++++++++++)
	$(MAKE) -C mupen64plus all

#dolphin
#dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a: dolphin
#dolphin/build/Source/Core/Core/libcore.a: dolphin
#dolphin/build/Source/Core/UICommon/libuicommon.a: dolphin
#dolphin/build/Source/Core/../../Externals/imgui/libimgui.a: dolphin
#dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a: dolphin
#dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a: dolphin
#dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a: dolphin
#dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a: dolphin
#dolphin/build/Source/Core/VideoCommon/libvideocommon.a: dolphin
#dolphin/build/Source/Core/Core/libcore.a: dolphin
#dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a: dolphin
#dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a: dolphin
#dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a: dolphin
#dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a: dolphin
#dolphin/build/Source/Core/VideoCommon/libvideocommon.a: dolphin
#dolphin/build/Source/Core/AudioCommon/libaudiocommon.a: dolphin
#dolphin/build/Source/Core/../../Externals/soundtouch/libSoundTouch.a: dolphin
#dolphin/build/Source/Core/../../Externals/FreeSurround/libFreeSurround.a: dolphin
#dolphin/build/Source/Core/../../Externals/cubeb/libcubeb.a: dolphin
#dolphin/build/Source/Core/DiscIO/libdiscio.a: dolphin
#dolphin/build/Source/Core/InputCommon/libinputcommon.a: dolphin
#dolphin/build/Source/Core/../../Externals/hidapi/libhidapi.a: dolphin
#dolphin/build/Source/Core/../../Externals/glslang/libglslang.a: dolphin
#dolphin/build/Source/Core/../../Externals/imgui/libimgui.a: dolphin
#dolphin/build/Source/Core/../../Externals/xxhash/libxxhash.a: dolphin
#dolphin/build/Source/Core/Common/libcommon.a: dolphin
#dolphin/build/Source/Core/../../Externals/enet/libenet.a: dolphin
#dolphin/build/Source/Core/../../Externals/pugixml/libpugixml.a: dolphin
#if DEBUG
#dolphin/build/Source/Core/../../Externals/fmt/libfmtd.a: dolphin
#else
#dolphin/build/Source/Core/../../Externals/fmt/libfmt.a: dolphin
#endif
#dolphin/build/Source/Core/../../Externals/bzip2/libbzip2.a: dolphin
#dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a: dolphin
#dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a: dolphin
#dolphin/build/Source/Core/../../Externals/minizip/libminizip.a: dolphin
#dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a: dolphin

#dolphin:
#	$(info   ++++++++++++++++ dolphin ++++++++++++++++)
#	$(MAKE) -C dolphin all

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
marley_LDADD 	+= mupen64plus/Source/input-sdl/_obj/config.o mupen64plus/Source/input-sdl/_obj/sdl_key_converter.o mupen64plus/Source/input-sdl/_obj/plugin.o mupen64plus/Source/input-sdl/_obj/autoconfig.o mupen64plus/Source/input-sdl/_obj/osal_dynamiclib_unix.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLtextures.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLcombiner.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLgeometry.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLglitchmain.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters_hq2x.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/Ext_TxFilter.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxDbg.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxReSample.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxImage.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/fxt1.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/s2tc/s2tc_libtxc_dxtn.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/s2tc/s2tc_algorithm.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/wrapper.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/texstore.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxFilterExport.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters_hq4x.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxCache.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters_2xsai.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxUtil.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxTexCache.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxHiResCache.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxQuantize.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxFilter.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/rdp.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/FBtoScreen.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Debugger.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/TexBuffer.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Ini.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/DepthBufferRender.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Combine.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Keys.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Config.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/CRC.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/TexCache.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/osal_dynamiclib_unix.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Util.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/3dmath.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Main.o mupen64plus/Source/core/_obj/osal/files_unix.o mupen64plus/Source/core/_obj/osal/dynamiclib_unix.o mupen64plus/Source/core/_obj/md5/md5.o mupen64plus/Source/core/_obj/asm_defines/asm_defines.o mupen64plus/Source/core/_obj/osd/oglft_c.o mupen64plus/Source/core/_obj/osd/osd.o mupen64plus/Source/core/_obj/api/config.o mupen64plus/Source/core/_obj/api/vidext.o mupen64plus/Source/core/_obj/api/frontend.o mupen64plus/Source/core/_obj/api/debugger.o mupen64plus/Source/core/_obj/api/callbacks.o mupen64plus/Source/core/_obj/api/common.o mupen64plus/Source/core/_obj/device/r4300/r4300_core.o mupen64plus/Source/core/_obj/device/r4300/cp1.o mupen64plus/Source/core/_obj/device/r4300/interrupt.o mupen64plus/Source/core/_obj/device/r4300/idec.o mupen64plus/Source/core/_obj/device/r4300/tlb.o mupen64plus/Source/core/_obj/device/r4300/recomp.o mupen64plus/Source/core/_obj/device/r4300/cp0.o mupen64plus/Source/core/_obj/device/r4300/pure_interp.o mupen64plus/Source/core/_obj/device/r4300/cached_interp.o mupen64plus/Source/core/_obj/device/r4300/x86_64/dynarec.o mupen64plus/Source/core/_obj/device/r4300/x86_64/assemble.o mupen64plus/Source/core/_obj/device/r4300/x86_64/dyna_start.o mupen64plus/Source/core/_obj/device/r4300/x86_64/regcache.o mupen64plus/Source/core/_obj/device/rdram/rdram.o mupen64plus/Source/core/_obj/device/rcp/pi/pi_controller.o mupen64plus/Source/core/_obj/device/rcp/ai/ai_controller.o mupen64plus/Source/core/_obj/device/rcp/vi/vi_controller.o mupen64plus/Source/core/_obj/device/rcp/mi/mi_controller.o mupen64plus/Source/core/_obj/device/rcp/ri/ri_controller.o mupen64plus/Source/core/_obj/device/rcp/si/si_controller.o mupen64plus/Source/core/_obj/device/rcp/rsp/rsp_core.o mupen64plus/Source/core/_obj/device/rcp/rdp/rdp_core.o mupen64plus/Source/core/_obj/device/rcp/rdp/fb.o mupen64plus/Source/core/_obj/device/memory/memory.o mupen64plus/Source/core/_obj/device/pif/cic.o mupen64plus/Source/core/_obj/device/pif/pif.o mupen64plus/Source/core/_obj/device/pif/bootrom_hle.o mupen64plus/Source/core/_obj/device/pif/n64_cic_nus_6105.o mupen64plus/Source/core/_obj/device/gb/gb_cart.o mupen64plus/Source/core/_obj/device/gb/mbc3_rtc.o mupen64plus/Source/core/_obj/device/gb/m64282fp.o mupen64plus/Source/core/_obj/device/device.o mupen64plus/Source/core/_obj/device/dd/dd_controller.o mupen64plus/Source/core/_obj/device/cart/cart_rom.o mupen64plus/Source/core/_obj/device/cart/cart.o mupen64plus/Source/core/_obj/device/cart/eeprom.o mupen64plus/Source/core/_obj/device/cart/af_rtc.o mupen64plus/Source/core/_obj/device/cart/flashram.o mupen64plus/Source/core/_obj/device/cart/sram.o mupen64plus/Source/core/_obj/device/controllers/paks/mempak.o mupen64plus/Source/core/_obj/device/controllers/paks/transferpak.o mupen64plus/Source/core/_obj/device/controllers/paks/rumblepak.o mupen64plus/Source/core/_obj/device/controllers/paks/biopak.o mupen64plus/Source/core/_obj/device/controllers/game_controller.o mupen64plus/Source/core/_obj/plugin/dummy_input.o mupen64plus/Source/core/_obj/plugin/plugin.o mupen64plus/Source/core/_obj/plugin/dummy_rsp.o mupen64plus/Source/core/_obj/plugin/dummy_video.o mupen64plus/Source/core/_obj/plugin/dummy_audio.o mupen64plus/Source/core/_obj/main/savestates.o mupen64plus/Source/core/_obj/main/sdl_key_converter.o mupen64plus/Source/core/_obj/main/screenshot.o mupen64plus/Source/core/_obj/main/workqueue.o mupen64plus/Source/core/_obj/main/eventloop.o mupen64plus/Source/core/_obj/main/main.o mupen64plus/Source/core/_obj/main/util.o mupen64plus/Source/core/_obj/main/rom.o mupen64plus/Source/core/_obj/main/cheat.o mupen64plus/Source/core/_obj/backends/plugins_compat/audio_plugin_compat.o mupen64plus/Source/core/_obj/backends/plugins_compat/input_plugin_compat.o mupen64plus/Source/core/_obj/backends/clock_ctime_plus_delta.o mupen64plus/Source/core/_obj/backends/file_storage.o mupen64plus/Source/core/_obj/backends/api/video_capture_backend.o mupen64plus/Source/core/_obj/backends/dummy_video_capture.o mupen64plus/Source/core/_obj/oglft/OGLFT.o mupen64plus/Source/core/_obj/minizip/zip.o mupen64plus/Source/core/_obj/minizip/ioapi.o mupen64plus/Source/core/_obj/minizip/unzip.o mupen64plus/Source/ui-console/_obj/plugin.o mupen64plus/Source/ui-console/_obj/core_interface.o mupen64plus/Source/ui-console/_obj/main.o mupen64plus/Source/ui-console/_obj/debugger.o mupen64plus/Source/ui-console/_obj/osal_files_unix.o mupen64plus/Source/ui-console/_obj/osal_dynamiclib_unix.o mupen64plus/Source/ui-console/_obj/compare_core.o mupen64plus/Source/ui-console/_obj/cheat.o mupen64plus/Source/rsp-hle/_obj/audio.o mupen64plus/Source/rsp-hle/_obj/alist_audio.o mupen64plus/Source/rsp-hle/_obj/re2.o mupen64plus/Source/rsp-hle/_obj/alist_nead.o mupen64plus/Source/rsp-hle/_obj/alist_naudio.o mupen64plus/Source/rsp-hle/_obj/plugin.o mupen64plus/Source/rsp-hle/_obj/hvqm.o mupen64plus/Source/rsp-hle/_obj/alist.o mupen64plus/Source/rsp-hle/_obj/hle.o mupen64plus/Source/rsp-hle/_obj/jpeg.o mupen64plus/Source/rsp-hle/_obj/mp3.o mupen64plus/Source/rsp-hle/_obj/memory.o mupen64plus/Source/rsp-hle/_obj/osal_dynamiclib_unix.o mupen64plus/Source/rsp-hle/_obj/cicx105.o mupen64plus/Source/rsp-hle/_obj/musyx.o mupen64plus/Source/audio-sdl/_obj/circular_buffer.o mupen64plus/Source/audio-sdl/_obj/volume.o mupen64plus/Source/audio-sdl/_obj/main.o mupen64plus/Source/audio-sdl/_obj/sdl_backend.o mupen64plus/Source/audio-sdl/_obj/resamplers/trivial.o mupen64plus/Source/audio-sdl/_obj/resamplers/src.o mupen64plus/Source/audio-sdl/_obj/resamplers/resamplers.o mupen64plus/Source/audio-sdl/_obj/osal_dynamiclib_unix.o

#dolphin, static
#marley_LDADD 	+= dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a
#marley_LDADD 	+= dolphin/build/Source/Core/Core/libcore.a 
#marley_LDADD 	+= dolphin/build/Source/Core/UICommon/libuicommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/imgui/libimgui.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoCommon/libvideocommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/Core/libcore.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Null/libvideonull.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/OGL/libvideoogl.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Software/libvideosoftware.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoBackends/Vulkan/libvideovulkan.a 
#marley_LDADD 	+= dolphin/build/Source/Core/VideoCommon/libvideocommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/AudioCommon/libaudiocommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/soundtouch/libSoundTouch.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/FreeSurround/libFreeSurround.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cubeb/libcubeb.a 
#marley_LDADD 	+= dolphin/build/Source/Core/DiscIO/libdiscio.a 
#marley_LDADD 	+= dolphin/build/Source/Core/InputCommon/libinputcommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/hidapi/libhidapi.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/glslang/libglslang.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/imgui/libimgui.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/xxhash/libxxhash.a 
#marley_LDADD 	+= dolphin/build/Source/Core/Common/libcommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/enet/libenet.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/pugixml/libpugixml.a 
#if DEBUG
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/fmt/libfmtd.a 
#else
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/fmt/libfmt.a 
#endif
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/bzip2/libbzip2.a
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/Bochs_disasm/libbdisasm.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/minizip/libminizip.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/discord-rpc/src/libdiscord-rpc.a  
#marley_LDADD 	+= dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a
#marley_LDADD 	+= dolphin/build/Source/Core/UICommon/libuicommon.a 
#marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a 

#ppsspp, dynamic
marley_LDADD 	+= -lGLEW

#mupen64plus, dynamic
marley_LDADD 	+= -lGL -lstdc++ -lGLU -lfreetype -lboost_system -lboost_filesystem -lsamplerate

#pcsx2, static
marley_LDADD 	+= pcsx2/build/pcsx2/libPCSX2.a
marley_LDADD 	+= pcsx2/build/common/src/x86emitter/libx86emitter.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
marley_LDADD 	+= pcsx2/build/3rdparty/fmt/fmt/libfmt.a
marley_LDADD 	+= pcsx2/build/plugins/onepad/libonepad.a
marley_LDADD 	+= pcsx2/build/plugins/GSdx/libGSdx.a


#dolphin, dynamic
marley_LDADD 	+= -lQt5Widgets
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
marley_LDADD 	+= -ldl
marley_LDADD 	+= -lrt 
marley_LDADD 	+= -lXrandr 
marley_LDADD 	+= -lpulse 
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
marley_LDADD 	+= -l@GTK_BASE@
marley_LDADD 	+= -l@GTK_CORE@
marley_LDADD 	+= -l@GTK__ADV@
marley_LDADD 	+= -l@GTK_X@
marley_LDADD 	+= -l@GDK_X@

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
marley_LDADD 	+= -lxcb
marley_LDADD 	+= -lX11-xcb
marley_LDADD 	+= -lSoundTouch

