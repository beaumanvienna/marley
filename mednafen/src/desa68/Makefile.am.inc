noinst_LIBRARIES	+=	libdesa68.a
libdesa68_a_CFLAGS	=	@AM_CFLAGS@ -Wno-multichar

libdesa68_a_SOURCES	=	desa68/desa68.c

libmednafen_marley_a_LIBADD		+=	desa68/libdesa68_a-desa68.o
libmednafen_marley_a_DEPENDENCIES	+=	libdesa68.a

