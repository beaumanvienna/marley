libmednafen_marley_a_SOURCES 	+= 	pce/huc6280.cpp pce/pce.cpp pce/vce.cpp pce/input.cpp pce/huc.cpp pce/pcecd.cpp pce/hes.cpp pce/tsushin.cpp pce/mcgenjin.cpp
libmednafen_marley_a_SOURCES	+=	pce/input/gamepad.cpp pce/input/tsushinkb.cpp pce/input/mouse.cpp

if WANT_DEBUGGER
libmednafen_marley_a_SOURCES	+=	pce/dis6280.cpp pce/debug.cpp
endif

