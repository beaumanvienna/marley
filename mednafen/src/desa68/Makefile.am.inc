noinst_LIBRARIES	+=	libdesa68.a
libdesa68_a_CFLAGS	=	@AM_CFLAGS@ -Wno-multichar

libdesa68_a_SOURCES	=	desa68/desa68.c

mednafen_marley_LDADD		+=	libdesa68.a
mednafen_marley_DEPENDENCIES	+=	libdesa68.a

