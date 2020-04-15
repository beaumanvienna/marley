noinst_LIBRARIES	+=	libtrio.a
libtrio_a_CFLAGS	=	@AM_CFLAGS@ @TRIO_CFLAGS@ @CFLAG_VISIBILITY@

libtrio_a_SOURCES 	= 	trio/trio.c trio/trionan.c trio/triostr.c

libmednafen_marley_a_LIBADD 		+=	trio/libtrio_a-trio.o trio/libtrio_a-trionan.o trio/libtrio_a-triostr.o
libmednafen_marley_a_DEPENDENCIES	+=	libtrio.a

