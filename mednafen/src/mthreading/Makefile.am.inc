if WIN32
libmednafen_marley_a_SOURCES += mthreading/MThreading_Win32.cpp
else
libmednafen_marley_a_SOURCES += mthreading/MThreading_POSIX.cpp
endif

