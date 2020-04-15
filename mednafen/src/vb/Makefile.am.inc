libmednafen_marley_a_SOURCES 	+= 	vb/vb.cpp vb/timer.cpp vb/input.cpp vb/vip.cpp vb/vsu.cpp

if WANT_DEBUGGER
libmednafen_marley_a_SOURCES	+=	vb/debug.cpp
endif
