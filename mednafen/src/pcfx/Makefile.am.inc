libmednafen_marley_a_SOURCES 	+= 	pcfx/king.cpp pcfx/soundbox.cpp pcfx/pcfx.cpp pcfx/interrupt.cpp pcfx/input.cpp pcfx/timer.cpp pcfx/rainbow.cpp pcfx/idct.cpp pcfx/huc6273.cpp pcfx/fxscsi.cpp
libmednafen_marley_a_SOURCES	+=	pcfx/input/gamepad.cpp pcfx/input/mouse.cpp

if WANT_DEBUGGER
libmednafen_marley_a_SOURCES	+=	pcfx/debug.cpp
endif
