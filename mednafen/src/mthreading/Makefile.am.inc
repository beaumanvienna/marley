if WIN32
mednafen_marley_SOURCES += mthreading/MThreading_Win32.cpp
else
mednafen_marley_SOURCES += mthreading/MThreading_POSIX.cpp
endif

