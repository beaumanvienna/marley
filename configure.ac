# run: "aclocal && autoconf && automake --add-missing --foreign"
# then "./configure --enable-debug" or simply "./configure"
# and "make"


AC_INIT([marley], 0.1.5)
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AM_EXTRA_RECURSIVE_TARGETS([mednafen_marley])
AC_PROG_CXX
AC_PROG_CC
AC_DEFINE([PICTURES],["pictures/"])
AC_DEFINE([RESOURCES],["resources/"])
AC_PROG_RANLIB
AM_INIT_AUTOMAKE([subdir-objects])

# GTK2 or GTK3 depending on OS
DETECT_GTK3=`./pcsx2/detect_gtk_flags.sh`
AM_CONDITIONAL([PCSX2_USE_GTK3], test -n "$DETECT_GTK3")
if test -n "$DETECT_GTK3"
then
PCSX2_USE_GTK3_TRUE=
PCSX2_USE_GTK3_FALSE='#'
GTK_HEADERS_INCLUDE=`pkg-config --cflags gtk+-3.0`
else
PCSX2_USE_GTK3_TRUE='#'
PCSX2_USE_GTK3_FALSE=
GTK_HEADERS_INCLUDE=`pkg-config --cflags gtk+-2.0`
fi
AC_SUBST([GTK_HEADERS_INCLUDE])

#resource system
RESOURCE_LIST="\$(shell \$(glib-compile-resources --sourcedir= --generate-dependencies resources/res.xml))"
AC_SUBST([RESOURCE_LIST])

#compile ffmpeg so that the configure run of ppsspp can find it
AC_CONFIG_COMMANDS_PRE([cd ppsspp/ffmpeg  && ./linux_x86-64.sh &&cd ../..])

#run all configure scripts
AC_CONFIG_COMMANDS_PRE([cd ppsspp  && mkdir -p build && cd build && cmake ..&&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd pcsx2  && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DBUILTIN_PAD=ON -DBUILTIN_GS=ON `../detect_gtk_flags.sh` ..&&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd mednafen && aclocal && autoconf && automake --add-missing --foreign && ./configure&&cd ..])
AC_CONFIG_COMMANDS_PRE([cd dolphin && mkdir -p build && cd build && cmake ..&&cd ../..])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
