
# directories
SUBDIRS = pcsx2
SUBDIRS += mupen64plus
SUBDIRS += mednafen
SUBDIRS += dolphin
SUBDIRS += ppsspp
SUBDIRS += tests
SUBDIRS += screen_manager

#installation files
man6_MANS = resources/marley.6

applauncherdir = /usr/share/applications
applauncher_DATA = resources/marley.desktop

appicondir = /usr/share/icons/
appicon_DATA = resources/marley.ico

#additional make clean targets
clean-local:
	rm -rf confdefs.h
	rm -rf tests/.debug_build
	rm -rf resources/dolphin_data_sys.zip
	rm -rf resources/pcsx2
	rm -rf resources/*.cpp
	rm -rf resources/*.h
	rm -rf resources/ppsspp_assets.zip
	rm -rf ppsspp/ffmpeg/libavutil/*.o
	rm -rf ppsspp/ffmpeg/libswscale/*.o
	rm -rf ppsspp/ffmpeg/libavcodec/*.o
	rm -rf ppsspp/ffmpeg/libavformat/*.o
	rm -rf ppsspp/ffmpeg/libswresample/*.o
	rm -rf ppsspp/ffmpeg/libavutil/*.a
	rm -rf ppsspp/ffmpeg/libswscale/*.a
	rm -rf ppsspp/ffmpeg/libavcodec/*.a
	rm -rf ppsspp/ffmpeg/libavformat/*.a
	rm -rf ppsspp/ffmpeg/libswresample/*.a
	rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libavutil.a
	rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libavformat.a
	rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libswscale.a
	rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libswresample.a
	rm -rf ppsspp/ffmpeg/linux/x86_64/lib/libavcodec.a
	rm -rf ppsspp/ffmpeg/libavutil/x86/*.o
	rm -rf ppsspp/ffmpeg/libswscale/x86/*.o
	rm -rf ppsspp/ffmpeg/libavcodec/x86/*.o
	rm -rf ppsspp/ffmpeg/libavcodec/x86/*.o
	rm -rf ppsspp/ffmpeg/libswresample/x86/*.o
	rm -rf ppsspp/assets/ui_atlas.meta ppsspp/assets/ui_atlas.zim
	rm -rf ppsspp/ext/native/tools/build

#front end compiler settings
if DEBUG
AM_CXXFLAGS		= --std=c++17 -ggdb3 -g3 -O0 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core @GTK_HEADERS_INCLUDE@
AM_LDFLAGS		= --std=c++17 -ggdb3 -g3 -O0 -fno-pie -no-pie -L/usr/lib/x86_64-linux-gnu
else
AM_CXXFLAGS		= --std=c++17 -O2 -fno-pie -no-pie -I/usr/include/SDL2/ -I../mednafen -Idolphin/Source/Core @GTK_HEADERS_INCLUDE@
AM_LDFLAGS		= --std=c++17 -O2 -fno-pie -no-pie -s -L/usr/lib/x86_64-linux-gnu
endif

if DEBUGOUTPUT
AM_CXXFLAGS		+= -DDEBUG_OUTPUT
endif

#project target

bin_PROGRAMS 	= marley		
marley_SOURCES 	= src/marley.cpp src/gui.cpp src/controller.cpp src/statemachine.cpp src/emu.cpp src/wii.cpp resources/res.cpp src/mdf2iso.cpp

#generate built-in resources
RESOURCE_LIST= resources/../pictures/PS4-DualShock.bmp resources/marley.ico resources/mupen64plus-core/data/font.ttf resources/../pictures/Xbox-360-S-Controller.bmp resources/../pictures/shutdown_inactive.bmp resources/../pictures/Off_inactive.bmp resources/../pictures/path_to_games.bmp resources/../pictures/Setup.bmp resources/../pictures/path_to_fw.bmp resources/../pictures/PS3-DualShock.bmp resources/dolphin_data_sys.zip resources/settings_general.png resources/../pictures/Wiimote.bmp resources/../pictures/path_to_games_inactive.bmp resources/../pictures/generic-controller.bmp resources/../pictures/noController.bmp resources/../pictures/noGames.bmp resources/../pictures/beach.bmp resources/../pictures/firmware_PSX.bmp resources/../pictures/Setup_inactive.bmp resources/../pictures/config_inactive.bmp resources/splash_spritesheet.png resources/../pictures/rudder_grey.bmp resources/marley.desktop resources/../pictures/rudder.bmp resources/../pictures/shutdown.bmp resources/../pictures/barrel.bmp resources/../pictures/path_to_fw_inactive.bmp resources/../pictures/splash.bmp resources/../pictures/Play_inactive.bmp resources/dejavu-fonts-ttf/DejaVuSansMono-Bold.ttf resources/beach.png resources/wind.ogg resources/ppsspp_assets.zip resources/settings_pcsx2.png resources/../pictures/Off.bmp resources/splash_overlay.png resources/settings_dolphin.png resources/../pictures/Play.bmp resources/gamecontrollerdb.txt resources/../pictures/config.bmp resources/../pictures/SNES-controller.bmp
resources/res.cpp: resources/res.xml resources/res.h $(RESOURCE_LIST)
	glib-compile-resources resources/res.xml --target=resources/res.cpp --sourcedir=resources/ --generate-source

resources/res.h: resources/res.xml $(RESOURCE_LIST) resources/ppsspp_assets.zip resources/dolphin_data_sys.zip
	glib-compile-resources resources/res.xml --target=resources/res.h --sourcedir=resources/ --generate-header

src/controller.cpp: resources/res.cpp
src/gui.cpp: resources/res.cpp
src/marley.cpp: resources/res.cpp

ppsspp/ext/native/tools/build/atlastool: ppsspp/assets/ui_atlas.zim

ppsspp/assets/ui_atlas.zim: ppsspp/source_assets/fonts/8-bit.ttf  ppsspp/source_assets/fonts/Roboto-Condensed.ttf ppsspp/source_assets/image/tab_r.png ppsspp/source_assets/image/white.png ppsspp/source_assets/image/1.png ppsspp/source_assets/image/background.png ppsspp/source_assets/image/clouds.png ppsspp/source_assets/image/flag_hk.png ppsspp/source_assets/image/generic-controller.png ppsspp/source_assets/image/lines_r.png ppsspp/source_assets/image/rect_line.png ppsspp/source_assets/image/square.png ppsspp/source_assets/image/updir.png ppsspp/source_assets/image/2.png ppsspp/source_assets/image/back.png ppsspp/source_assets/image/controller_not_found.png ppsspp/source_assets/image/flag_jp.png ppsspp/source_assets/image/grid.png ppsspp/source_assets/image/logo.png ppsspp/source_assets/image/rect.png ppsspp/source_assets/image/start.png ppsspp/source_assets/image/up_line.png ppsspp/source_assets/image/3.png ppsspp/source_assets/image/back_r.png ppsspp/source_assets/image/cross.png ppsspp/source_assets/image/flag_ko.png ppsspp/source_assets/image/grid_r.png ppsspp/source_assets/image/logo.svg ppsspp/source_assets/image/round_line.png ppsspp/source_assets/image/stick_bg_line.png ppsspp/source_assets/image/up_line_r.png ppsspp/source_assets/image/4.png ppsspp/source_assets/image/barrel.png ppsspp/source_assets/image/dir_line.png ppsspp/source_assets/image/flag_us.png ppsspp/source_assets/image/home.png ppsspp/source_assets/image/L.png ppsspp/source_assets/image/round.png ppsspp/source_assets/image/stick_bg.png ppsspp/source_assets/image/Wiimote.png ppsspp/source_assets/image/5.png ppsspp/source_assets/image/beach.png ppsspp/source_assets/image/dir.png ppsspp/source_assets/image/folder_line.png ppsspp/source_assets/image/home_r.png ppsspp/source_assets/image/off.png ppsspp/source_assets/image/R.png ppsspp/source_assets/image/stick_line.png ppsspp/source_assets/image/Xbox-360-S-Controller.png ppsspp/source_assets/image/6.png ppsspp/source_assets/image/button.png ppsspp/source_assets/image/dolphin.png ppsspp/source_assets/image/folder_line_r.png ppsspp/source_assets/image/icon_regular_72.png ppsspp/source_assets/image/off_r.png ppsspp/source_assets/image/select.png ppsspp/source_assets/image/stick.png ppsspp/source_assets/image/zip.pdn ppsspp/source_assets/image/active_item.png ppsspp/source_assets/image/button_selected.png ppsspp/source_assets/image/dropshadow.png ppsspp/source_assets/image/folder.png ppsspp/source_assets/image/icon_regular.png ppsspp/source_assets/image/PS3-DualShock.png ppsspp/source_assets/image/shoulder_line.png ppsspp/source_assets/image/triangle.png ppsspp/source_assets/image/arrow.png ppsspp/source_assets/image/checkedbox.png ppsspp/source_assets/image/flag_as.png ppsspp/source_assets/image/gear.png ppsspp/source_assets/image/inactive_item.png ppsspp/source_assets/image/PS4-DualShock.png ppsspp/source_assets/image/shoulder.png ppsspp/source_assets/image/umd256.png ppsspp/source_assets/image/background2.png ppsspp/source_assets/image/circle.png ppsspp/source_assets/image/flag_eu.png ppsspp/source_assets/image/gear_r.png ppsspp/source_assets/image/lines.png ppsspp/source_assets/image/psp_display.png ppsspp/source_assets/image/SNES-controller.png ppsspp/source_assets/image/umd48.png ppsspp/source_assets/ouya/OUYA_A.png ppsspp/source_assets/ouya/OUYA_DPAD.png ppsspp/source_assets/ouya/OUYA_L1.png ppsspp/source_assets/ouya/OUYA_LS_DOWN.png ppsspp/source_assets/ouya/OUYA_LS_RIGHT.png ppsspp/source_assets/ouya/OUYA_R1.png ppsspp/source_assets/ouya/OUYA_RS_DOWN.png ppsspp/source_assets/ouya/OUYA_RS_RIGHT.png ppsspp/source_assets/ouya/OUYA_TOUCHPAD.png ppsspp/source_assets/ouya/OUYA_DPAD_DOWN.png ppsspp/source_assets/ouya/OUYA_DPAD_RIGHT.png ppsspp/source_assets/ouya/OUYA_L2.png ppsspp/source_assets/ouya/OUYA_LS_LEFT.png ppsspp/source_assets/ouya/OUYA_LS_UP.png ppsspp/source_assets/ouya/OUYA_R2.png ppsspp/source_assets/ouya/OUYA_RS_LEFT.png ppsspp/source_assets/ouya/OUYA_RS_UP.png ppsspp/source_assets/ouya/OUYA_U.png ppsspp/source_assets/ouya/OUYA_DPAD_LEFT.png ppsspp/source_assets/ouya/OUYA_DPAD_UP.png ppsspp/source_assets/ouya/OUYA_L3.png ppsspp/source_assets/ouya/OUYA_LS.png ppsspp/source_assets/ouya/OUYA_O.png ppsspp/source_assets/ouya/OUYA_R3.png ppsspp/source_assets/ouya/OUYA_RS.png ppsspp/source_assets/ouya/OUYA_SYSTEM.png ppsspp/source_assets/ouya/OUYA_Y.png
	$(info   ++++++++++++++++ atlas ++++++++++++++++)
	cd ppsspp && ./buildatlas.sh

resources/ppsspp_assets.zip: ppsspp/assets/ui_atlas.zim
	cd ppsspp && zip -r ../resources/ppsspp_assets.zip assets && cd ..

resources/dolphin_data_sys.zip: 
	cd dolphin && zip -r ../resources/dolphin_data_sys.zip Data/sys && cd ..

#ppsspp

ppsspp/build/lib/libCommon.a:
	$(info   ++++++++++++++++ pppsspp ++++++++++++++++)
	$(MAKE) -C ppsspp all

#pcsx2

pcsx2/build/pcsx2/libPCSX2.a: pcsx2

pcsx2:
	$(info   ++++++++++++++++ pcsx2 ++++++++++++++++)
	$(MAKE) -C pcsx2 all

#mupen64plus

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

#screen_manager, static
SCREEN_MAN_LD = \
	screen_manager/build/UI/libui.a \
	screen_manager/build/Common/libcommon.a \
	screen_manager/build/SDL/libsdl.a

marley_LDADD 	+= $(SCREEN_MAN_LD) $(SCREEN_MAN_LD) $(SCREEN_MAN_LD)

#mupen64plus, static
marley_LDADD 	+= mupen64plus/Source/input-sdl/_obj/config.o mupen64plus/Source/input-sdl/_obj/sdl_key_converter.o mupen64plus/Source/input-sdl/_obj/plugin.o mupen64plus/Source/input-sdl/_obj/autoconfig.o mupen64plus/Source/input-sdl/_obj/osal_dynamiclib_unix.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLtextures.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLcombiner.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLgeometry.o mupen64plus/Source/video-glide64mk2/_obj/Glitch64/OGLglitchmain.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters_hq2x.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/Ext_TxFilter.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxDbg.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxReSample.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxImage.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/fxt1.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/s2tc/s2tc_libtxc_dxtn.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/s2tc/s2tc_algorithm.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/wrapper.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/tc-1.1+/texstore.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxFilterExport.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters_hq4x.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxCache.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters_2xsai.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TextureFilters.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxUtil.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxTexCache.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxHiResCache.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxQuantize.o mupen64plus/Source/video-glide64mk2/_obj/GlideHQ/TxFilter.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/rdp.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/FBtoScreen.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Debugger.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/TexBuffer.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Ini.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/DepthBufferRender.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Combine.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Keys.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Config.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/CRC.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/TexCache.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/osal_dynamiclib_unix.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Util.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/3dmath.o mupen64plus/Source/video-glide64mk2/_obj/Glide64/Main.o mupen64plus/Source/core/_obj/osal/files_unix.o mupen64plus/Source/core/_obj/osal/dynamiclib_unix.o mupen64plus/Source/core/_obj/md5/md5.o mupen64plus/Source/core/_obj/asm_defines/asm_defines.o mupen64plus/Source/core/_obj/osd/oglft_c.o mupen64plus/Source/core/_obj/osd/osd.o mupen64plus/Source/core/_obj/api/config.o mupen64plus/Source/core/_obj/api/vidext.o mupen64plus/Source/core/_obj/api/frontend.o mupen64plus/Source/core/_obj/api/debugger.o mupen64plus/Source/core/_obj/api/callbacks.o mupen64plus/Source/core/_obj/api/common.o mupen64plus/Source/core/_obj/device/r4300/r4300_core.o mupen64plus/Source/core/_obj/device/r4300/cp1.o mupen64plus/Source/core/_obj/device/r4300/interrupt.o mupen64plus/Source/core/_obj/device/r4300/idec.o mupen64plus/Source/core/_obj/device/r4300/tlb.o mupen64plus/Source/core/_obj/device/r4300/recomp.o mupen64plus/Source/core/_obj/device/r4300/cp0.o mupen64plus/Source/core/_obj/device/r4300/pure_interp.o mupen64plus/Source/core/_obj/device/r4300/cached_interp.o mupen64plus/Source/core/_obj/device/r4300/x86_64/dynarec.o mupen64plus/Source/core/_obj/device/r4300/x86_64/assemble.o mupen64plus/Source/core/_obj/device/r4300/x86_64/dyna_start.o mupen64plus/Source/core/_obj/device/r4300/x86_64/regcache.o mupen64plus/Source/core/_obj/device/rdram/rdram.o mupen64plus/Source/core/_obj/device/rcp/pi/pi_controller.o mupen64plus/Source/core/_obj/device/rcp/ai/ai_controller.o mupen64plus/Source/core/_obj/device/rcp/vi/vi_controller.o mupen64plus/Source/core/_obj/device/rcp/mi/mi_controller.o mupen64plus/Source/core/_obj/device/rcp/ri/ri_controller.o mupen64plus/Source/core/_obj/device/rcp/si/si_controller.o mupen64plus/Source/core/_obj/device/rcp/rsp/rsp_core.o mupen64plus/Source/core/_obj/device/rcp/rdp/rdp_core.o mupen64plus/Source/core/_obj/device/rcp/rdp/fb.o mupen64plus/Source/core/_obj/device/memory/memory.o mupen64plus/Source/core/_obj/device/pif/cic.o mupen64plus/Source/core/_obj/device/pif/pif.o mupen64plus/Source/core/_obj/device/pif/bootrom_hle.o mupen64plus/Source/core/_obj/device/pif/n64_cic_nus_6105.o mupen64plus/Source/core/_obj/device/gb/gb_cart.o mupen64plus/Source/core/_obj/device/gb/mbc3_rtc.o mupen64plus/Source/core/_obj/device/gb/m64282fp.o mupen64plus/Source/core/_obj/device/device.o mupen64plus/Source/core/_obj/device/dd/dd_controller.o mupen64plus/Source/core/_obj/device/cart/cart_rom.o mupen64plus/Source/core/_obj/device/cart/cart.o mupen64plus/Source/core/_obj/device/cart/eeprom.o mupen64plus/Source/core/_obj/device/cart/af_rtc.o mupen64plus/Source/core/_obj/device/cart/flashram.o mupen64plus/Source/core/_obj/device/cart/sram.o mupen64plus/Source/core/_obj/device/controllers/paks/mempak.o mupen64plus/Source/core/_obj/device/controllers/paks/transferpak.o mupen64plus/Source/core/_obj/device/controllers/paks/rumblepak.o mupen64plus/Source/core/_obj/device/controllers/paks/biopak.o mupen64plus/Source/core/_obj/device/controllers/game_controller.o mupen64plus/Source/core/_obj/plugin/dummy_input.o mupen64plus/Source/core/_obj/plugin/plugin.o mupen64plus/Source/core/_obj/plugin/dummy_rsp.o mupen64plus/Source/core/_obj/plugin/dummy_video.o mupen64plus/Source/core/_obj/plugin/dummy_audio.o mupen64plus/Source/core/_obj/main/savestates.o mupen64plus/Source/core/_obj/main/sdl_key_converter.o mupen64plus/Source/core/_obj/main/screenshot.o mupen64plus/Source/core/_obj/main/workqueue.o mupen64plus/Source/core/_obj/main/eventloop.o mupen64plus/Source/core/_obj/main/main.o mupen64plus/Source/core/_obj/main/util.o mupen64plus/Source/core/_obj/main/rom.o mupen64plus/Source/core/_obj/main/cheat.o mupen64plus/Source/core/_obj/backends/plugins_compat/audio_plugin_compat.o mupen64plus/Source/core/_obj/backends/plugins_compat/input_plugin_compat.o mupen64plus/Source/core/_obj/backends/clock_ctime_plus_delta.o mupen64plus/Source/core/_obj/backends/file_storage.o mupen64plus/Source/core/_obj/backends/api/video_capture_backend.o mupen64plus/Source/core/_obj/backends/dummy_video_capture.o mupen64plus/Source/core/_obj/oglft/OGLFT.o mupen64plus/Source/core/_obj/minizip/zip.o mupen64plus/Source/core/_obj/minizip/ioapi.o mupen64plus/Source/core/_obj/minizip/unzip.o mupen64plus/Source/ui-console/_obj/plugin.o mupen64plus/Source/ui-console/_obj/core_interface.o mupen64plus/Source/ui-console/_obj/main.o mupen64plus/Source/ui-console/_obj/debugger.o mupen64plus/Source/ui-console/_obj/osal_files_unix.o mupen64plus/Source/ui-console/_obj/osal_dynamiclib_unix.o mupen64plus/Source/ui-console/_obj/compare_core.o mupen64plus/Source/ui-console/_obj/cheat.o mupen64plus/Source/rsp-hle/_obj/audio.o mupen64plus/Source/rsp-hle/_obj/alist_audio.o mupen64plus/Source/rsp-hle/_obj/re2.o mupen64plus/Source/rsp-hle/_obj/alist_nead.o mupen64plus/Source/rsp-hle/_obj/alist_naudio.o mupen64plus/Source/rsp-hle/_obj/plugin.o mupen64plus/Source/rsp-hle/_obj/hvqm.o mupen64plus/Source/rsp-hle/_obj/alist.o mupen64plus/Source/rsp-hle/_obj/hle.o mupen64plus/Source/rsp-hle/_obj/jpeg.o mupen64plus/Source/rsp-hle/_obj/mp3.o mupen64plus/Source/rsp-hle/_obj/memory.o mupen64plus/Source/rsp-hle/_obj/osal_dynamiclib_unix.o mupen64plus/Source/rsp-hle/_obj/cicx105.o mupen64plus/Source/rsp-hle/_obj/musyx.o mupen64plus/Source/audio-sdl/_obj/circular_buffer.o mupen64plus/Source/audio-sdl/_obj/volume.o mupen64plus/Source/audio-sdl/_obj/main.o mupen64plus/Source/audio-sdl/_obj/sdl_backend.o mupen64plus/Source/audio-sdl/_obj/resamplers/trivial.o mupen64plus/Source/audio-sdl/_obj/resamplers/src.o mupen64plus/Source/audio-sdl/_obj/resamplers/resamplers.o mupen64plus/Source/audio-sdl/_obj/osal_dynamiclib_unix.o

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
marley_LDADD 	+= dolphin/build/Source/Core/DolphinNoGUI/libdolphin-emu-nogui.a
marley_LDADD 	+= dolphin/build/Source/Core/UICommon/libuicommon.a 
marley_LDADD 	+= dolphin/build/Source/Core/../../Externals/cpp-optparse/libcpp-optparse.a 

#ppsspp, dynamic
marley_LDADD 	+= -lGLEW

#mupen64plus, dynamic
marley_LDADD 	+= -lGL -lstdc++ -lGLU -lfreetype -lboost_system -lboost_filesystem -lsamplerate

#pcsx2, static
marley_LDADD 	+= pcsx2/build/pcsx2/libPCSX2.a
marley_LDADD 	+= pcsx2/build/common/src/x86emitter/libx86emitter.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities.a
marley_LDADD 	+= pcsx2/build/common/src/Utilities/libUtilities_NO_TLS.a
if DEBUG
marley_LDADD 	+= pcsx2/build/3rdparty/fmt/fmt/libfmtd.a
else
marley_LDADD 	+= pcsx2/build/3rdparty/fmt/fmt/libfmt.a
endif
marley_LDADD 	+= pcsx2/build/plugins/onepad/libonepad.a
marley_LDADD 	+= pcsx2/build/plugins/GSdx/libGSdx.a
marley_LDADD 	+= pcsx2/build/plugins/dev9null/libdev9null-0.5.0.a
marley_LDADD 	+= pcsx2/build/plugins/USBnull/libUSBnull-0.7.0.a


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
marley_LDADD 	+= -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lXfixes
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
marley_LDADD 	+= -lxcb
marley_LDADD 	+= -lX11-xcb
marley_LDADD 	+= -lSoundTouch

