libmednafen_marley_a_SOURCES += net/Net.cpp

if HAVE_POSIX_SOCKETS
libmednafen_marley_a_SOURCES += net/Net_POSIX.cpp
endif

if WIN32
libmednafen_marley_a_SOURCES += net/Net_WS2.cpp
endif

