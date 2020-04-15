libmednafen_marley_a_SOURCES	+=	nes/nes.cpp nes/x6502.cpp nes/cart.cpp nes/fds.cpp nes/ines.cpp nes/input.cpp nes/nsf.cpp nes/nsfe.cpp nes/unif.cpp nes/vsuni.cpp nes/sound.cpp nes/fds-sound.cpp

if WANT_DEBUGGER
libmednafen_marley_a_SOURCES	+=	nes/debug.cpp
libmednafen_marley_a_SOURCES	+=	nes/dis6502.cpp
endif

include nes/ppu/Makefile.am.inc
include nes/boards/Makefile.am.inc
include nes/input/Makefile.am.inc

libmednafen_marley_a_SOURCES	+=	nes/ntsc/nes_ntsc.cpp
