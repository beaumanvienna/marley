noinst_LIBRARIES 	+=	libmpcdec.a
libmpcdec_a_CFLAGS	=	@AM_CFLAGS@ @CFLAG_VISIBILITY@
libmpcdec_a_CPPFLAGS	=	-DMPC_FIXED_POINT=1

libmpcdec_a_SOURCES = mpcdec/huffman.c mpcdec/mpc_decoder.c mpcdec/mpc_reader.c \
        mpcdec/requant.c mpcdec/streaminfo.c mpcdec/synth_filter.c mpcdec/mpc_bits_reader.c mpcdec/mpc_demux.c \
	mpcdec/crc32.c

libmednafen_marley_a_LIBADD          +=	mpcdec/libmpcdec_a-huffman.o mpcdec/libmpcdec_a-mpc_decoder.o mpcdec/libmpcdec_a-mpc_reader.o mpcdec/libmpcdec_a-requant.o mpcdec/libmpcdec_a-streaminfo.o mpcdec/libmpcdec_a-synth_filter.o mpcdec/libmpcdec_a-mpc_bits_reader.o mpcdec/libmpcdec_a-mpc_demux.o mpcdec/libmpcdec_a-crc32.o 
libmednafen_marley_a_DEPENDENCIES   +=	libmpcdec.a

