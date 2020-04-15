noinst_LIBRARIES	+=	libvorbisidec.a
libvorbisidec_a_CFLAGS	=	@AM_CFLAGS@ @CFLAG_VISIBILITY@ -Wno-shadow -Wno-unused
libvorbisidec_a_CPPFLAGS=	-I$(srcdir)/tremor

libvorbisidec_a_SOURCES = tremor/mdct.c tremor/block.c tremor/window.c \
                        tremor/synthesis.c tremor/info.c \
                        tremor/floor1.c tremor/floor0.c tremor/vorbisfile.c \
                        tremor/res012.c tremor/mapping0.c tremor/registry.c tremor/codebook.c \
			tremor/sharedbook.c tremor/framing.c tremor/bitwise.c

libmednafen_marley_a_LIBADD          +=      tremor/libvorbisidec_a-mdct.o tremor/libvorbisidec_a-block.o tremor/libvorbisidec_a-window.o tremor/libvorbisidec_a-synthesis.o tremor/libvorbisidec_a-info.o tremor/libvorbisidec_a-floor1.o tremor/libvorbisidec_a-floor0.o tremor/libvorbisidec_a-vorbisfile.o tremor/libvorbisidec_a-res012.o tremor/libvorbisidec_a-mapping0.o tremor/libvorbisidec_a-registry.o tremor/libvorbisidec_a-codebook.o tremor/libvorbisidec_a-sharedbook.o tremor/libvorbisidec_a-framing.o tremor/libvorbisidec_a-bitwise.o 
libmednafen_marley_a_DEPENDENCIES   +=      libvorbisidec.a

