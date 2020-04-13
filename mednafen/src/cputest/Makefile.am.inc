mednafen_marley_SOURCES	+= cputest/cputest.c

if ARCH_X86
mednafen_marley_SOURCES	+= cputest/x86_cpu.c
endif

if ARCH_POWERPC
mednafen_marley_SOURCES	+= cputest/ppc_cpu.c
endif

