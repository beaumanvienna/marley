# run: "aclocal && autoconf && automake --add-missing --foreign"
# then "./configure --enable-debug" or simply "./configure"
# and "make"


AC_INIT([marley], 0.1.2)
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AM_EXTRA_RECURSIVE_TARGETS([mednafen_marley])
AC_PROG_CXX
AC_PROG_CC
AC_DEFINE([PICTURES],["pictures/"])
AC_DEFINE([RESOURCES],["resources/"])
AC_PROG_RANLIB
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_COMMANDS_PRE([cd ppsspp/ffmpeg  && ./linux_x86-64.sh &&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd ppsspp  && mkdir -p build && cd build && cmake ..&&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd pcsx2  && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DBUILTIN_PAD=ON -DBUILTIN_GS=ON -DGTK3_API=ON ..&&cd ../..])
#AC_CONFIG_COMMANDS_PRE([cd pcsx2  && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DBUILTIN_PAD=ON -DBUILTIN_GS=ON ..&&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd mednafen && aclocal && autoconf && automake --add-missing --foreign && ./configure&&cd ..])
AC_CONFIG_COMMANDS_PRE([cd dolphin && mkdir -p build && cd build && cmake ..&&cd ../..])



AC_CONFIG_FILES([Makefile])
AC_OUTPUT
