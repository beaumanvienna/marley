noinst_LIBRARIES	+=	libngp.a
libngp_a_CFLAGS		=	@AM_CFLAGS@ @NO_STRICT_ALIASING_FLAGS@
libngp_a_CXXFLAGS	=	@AM_CXXFLAGS@ @NO_STRICT_ALIASING_FLAGS@
libngp_a_SOURCES 	=	ngp/bios.cpp ngp/biosHLE.cpp ngp/dma.cpp ngp/flash.cpp ngp/gfx.cpp ngp/T6W28_Apu.cpp	\
	ngp/gfx_scanline_mono.cpp ngp/gfx_scanline_colour.cpp ngp/interrupt.cpp ngp/mem.cpp ngp/neopop.cpp \
	ngp/rom.cpp ngp/rtc.cpp ngp/sound.cpp ngp/Z80_interface.cpp \
	ngp/TLCS-900h/TLCS900h_interpret_single.cpp	\
	ngp/TLCS-900h/TLCS900h_interpret.cpp		\
	ngp/TLCS-900h/TLCS900h_registers.cpp		\
	ngp/TLCS-900h/TLCS900h_interpret_reg.cpp	\
	ngp/TLCS-900h/TLCS900h_interpret_src.cpp	\
	ngp/TLCS-900h/TLCS900h_interpret_dst.cpp

libmednafen_marley_a_LIBADD          +=      ngp/libngp_a-bios.o ngp/libngp_a-biosHLE.o ngp/libngp_a-dma.o ngp/libngp_a-flash.o ngp/libngp_a-gfx.o ngp/libngp_a-T6W28_Apu.o ngp/libngp_a-gfx_scanline_mono.o ngp/libngp_a-gfx_scanline_colour.o ngp/libngp_a-interrupt.o ngp/libngp_a-mem.o ngp/libngp_a-neopop.o ngp/libngp_a-rom.o ngp/libngp_a-rtc.o ngp/libngp_a-sound.o ngp/libngp_a-Z80_interface.o ngp/TLCS-900h/libngp_a-TLCS900h_interpret_single.o ngp/TLCS-900h/libngp_a-TLCS900h_interpret.o ngp/TLCS-900h/libngp_a-TLCS900h_registers.o ngp/TLCS-900h/libngp_a-TLCS900h_interpret_reg.o ngp/TLCS-900h/libngp_a-TLCS900h_interpret_src.o ngp/TLCS-900h/libngp_a-TLCS900h_interpret_dst.o
libmednafen_marley_a_DEPENDENCIES   +=      libngp.a

