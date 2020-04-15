libmednafen_marley_a_SOURCES	+= cputest/cputest.c

if ARCH_X86
libmednafen_marley_a_SOURCES	+= cputest/x86_cpu.c
endif

if ARCH_POWERPC
libmednafen_marley_a_SOURCES	+= cputest/ppc_cpu.c
endif

