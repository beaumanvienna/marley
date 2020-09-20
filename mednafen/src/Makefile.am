AUTOMAKE_OPTIONS = subdir-objects
DEFS = @DEFS@ @SNDFILE_CFLAGS@
DEFAULT_INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_builddir)/intl

SUBDIRS			=
noinst_LIBRARIES	= libmednafen_marley.a
libmednafen_marley_a_LIBADD		=
libmednafen_marley_a_DEPENDENCIES	=
libmednafen_marley_a_SOURCES 	= 	debug.cpp error.cpp mempatcher.cpp settings.cpp endian.cpp Time.cpp mednafen.cpp git.cpp file.cpp general.cpp memory.cpp netplay.cpp state.cpp state_rewind.cpp movie.cpp player.cpp PSFLoader.cpp SSFLoader.cpp SNSFLoader.cpp SPCReader.cpp tests.cpp qtrecord.cpp IPSPatcher.cpp 
libmednafen_marley_a_SOURCES	+=	VirtualFS.cpp NativeVFS.cpp Stream.cpp MemoryStream.cpp ExtMemStream.cpp FileStream.cpp MTStreamReader.cpp


SUBDIRS 		+=	drivers
libmednafen_marley_a_LIBADD          +=	drivers/main_marley.o drivers/args.o drivers/help.o drivers/ers.o drivers/sound.o drivers/netplay.o drivers/input.o drivers/mouse.o drivers/keyboard.o drivers/Joystick.o drivers/Joystick_SDL.o drivers/console.o drivers/cheat.o drivers/fps.o drivers/video-state.o drivers/remote.o drivers/rmdui.o drivers/opengl.o drivers/shader.o drivers/nongl.o drivers/nnx.o drivers/video.o drivers/hqxx-common.o drivers/hq2x.o drivers/hq3x.o drivers/hq4x.o drivers/scale2x.o drivers/scale3x.o drivers/scalebit.o drivers/2xSaI.o drivers/debugger.o drivers/gfxdebugger.o drivers/memdebugger.o drivers/logdebugger.o drivers/prompt.o 
libmednafen_marley_a_DEPENDENCIES   +=	drivers/libmdfnsdl_marley.a


SUBDIRS 		+=	drivers
libmednafen_marley_a_LIBADD          +=	drivers/main_marley.o drivers/args.o drivers/help.o drivers/ers.o drivers/sound.o drivers/netplay.o drivers/input.o drivers/mouse.o drivers/keyboard.o drivers/Joystick.o drivers/Joystick_SDL.o drivers/console.o drivers/cheat.o drivers/fps.o drivers/video-state.o drivers/remote.o drivers/rmdui.o drivers/opengl.o drivers/shader.o drivers/nongl.o drivers/nnx.o drivers/video.o drivers/hqxx-common.o drivers/hq2x.o drivers/hq3x.o drivers/hq4x.o drivers/scale2x.o drivers/scale3x.o drivers/scalebit.o drivers/2xSaI.o drivers/debugger.o drivers/gfxdebugger.o drivers/memdebugger.o drivers/logdebugger.o drivers/prompt.o 


SUBDIRS			+=	sexyal
libmednafen_marley_a_LIBADD		+=	sexyal/sexyal.o sexyal/convert.o sexyal/drivers/dummy.o   sexyal/drivers/oss.o  sexyal/drivers/jack.o sexyal/drivers/sdl.o sexyal/drivers/alsa.o 
libmednafen_marley_a_DEPENDENCIES	+=	sexyal/libsexyal.a

include cdplay/Makefile.am.inc
include demo/Makefile.am.inc

if WANT_APPLE2_EMU
include apple2/Makefile.am.inc
endif

if WANT_GB_EMU
include gb/Makefile.am.inc
endif

if WANT_GBA_EMU
include gba/Makefile.am.inc
endif

if WANT_LYNX_EMU
include lynx/Makefile.am.inc
endif

if WANT_MD_EMU
include md/Makefile.am.inc
endif

if WANT_NES_EMU
include nes/Makefile.am.inc
endif

#if WANT_NES_NEW_EMU
#include nes_new/Makefile.am.inc
#endif

if WANT_NGP_EMU
include ngp/Makefile.am.inc
endif

if WANT_PCE_EMU
include pce/Makefile.am.inc
endif

if WANT_PCE_FAST_EMU
include pce_fast/Makefile.am.inc
endif

if WANT_PCFX_EMU
include pcfx/Makefile.am.inc
endif

if WANT_PSX_EMU
include psx/Makefile.am.inc
endif

if WANT_SMS_EMU
include sms/Makefile.am.inc
endif

if WANT_SNES_EMU
include snes/Makefile.am.inc
endif

if WANT_SNES_FAUST_EMU
include snes_faust/Makefile.am.inc
endif

SS_SUBDIRS =
if WANT_SS_EMU
SS_SUBDIRS 		=	ss
libmednafen_marley_a_LIBADD          +=     ss/db.o ss/cdb.o ss/sound.o ss/cart.o ss/cart/backup.o ss/cart/cs1ram.o ss/cart/debug.o ss/cart/extram.o ss/cart/rom.o ss/cart/ar4mp.o ss/ss.o ss/scu_dsp_gen.o ss/scu_dsp_mvi.o ss/scu_dsp_jmp.o ss/scu_dsp_misc.o ss/vdp1.o ss/vdp1_line.o ss/vdp1_sprite.o ss/vdp1_poly.o ss/vdp2.o ss/vdp2_render.o ss/smpc.o ss/input/multitap.o ss/input/gamepad.o ss/input/3dpad.o ss/input/mouse.o ss/input/wheel.o ss/input/mission.o ss/input/gun.o ss/input/keyboard.o ss/input/jpkeyboard.o
libmednafen_marley_a_DEPENDENCIES   +=      ss/libss.a
endif

if WANT_SSFPLAY_EMU
SS_SUBDIRS 		=	ss
libmednafen_marley_a_LIBADD          +=     ss/ssf.o
libmednafen_marley_a_DEPENDENCIES   +=      ss/libssfplay.a
endif

SUBDIRS	+= $(SS_SUBDIRS)

if WANT_VB_EMU
include vb/Makefile.am.inc
endif

if WANT_WSWAN_EMU
include wswan/Makefile.am.inc
endif


if WANT_DEBUGGER
include desa68/Makefile.am.inc
endif

include hw_cpu/Makefile.am.inc
include hw_misc/Makefile.am.inc
include hw_sound/Makefile.am.inc
include hw_video/Makefile.am.inc
include mthreading/Makefile.am.inc
include cdrom/Makefile.am.inc
include sound/Makefile.am.inc
include net/Makefile.am.inc
include quicklz/Makefile.am.inc
include compress/Makefile.am.inc
include hash/Makefile.am.inc
include string/Makefile.am.inc
include video/Makefile.am.inc
include resampler/Makefile.am.inc
include cputest/Makefile.am.inc
include cheat_formats/Makefile.am.inc

if HAVE_EXTERNAL_LZO2

else
include minilzo/Makefile.am.inc
endif

if HAVE_EXTERNAL_TREMOR

else
include tremor/Makefile.am.inc
endif

if HAVE_EXTERNAL_MPCDEC

else
include mpcdec/Makefile.am.inc
endif

if HAVE_EXTERNAL_TRIO

else
include trio/Makefile.am.inc
endif



