# run: "aclocal && autoconf && automake --add-missing --foreign"
# then "./configure --enable-debug" or simply "./configure"
# and "make"


AC_INIT([marley], 0.1.5)
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AM_EXTRA_RECURSIVE_TARGETS([mednafen_marley])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AM_INIT_AUTOMAKE([subdir-objects])

#reset flags
AM_CXXFLAGS=
AM_LDFLAGS=

#debug option "--enable-debug"
AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

if test x"$debug" = x"true"
then
CMAKE_BUILD_TYPE=-DCMAKE_BUILD_TYPE=Debug
touch tests/.debug_build
else
CMAKE_BUILD_TYPE=-DCMAKE_BUILD_TYPE=Release
fi
echo "CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE"

# GTK2 or GTK3 depending on OS
DETECT_GTK3=`./pcsx2/detect_gtk_flags.sh`
AM_CONDITIONAL([PCSX2_USE_GTK3], test -n "$DETECT_GTK3")
AM_CONDITIONAL([PCSX2_USE_GTK3_GENTOO], test "x$DETECT_GTK3" = "x-DGTK3_API=TRUE_GENTOO")
if test -n "$DETECT_GTK3"
then
PCSX2_USE_GTK3_TRUE=
PCSX2_USE_GTK3_FALSE='#'
GTK_HEADERS_INCLUDE=`pkg-config --cflags gtk+-3.0`
echo "using GTK3 for pcsx2"
else
PCSX2_USE_GTK3_TRUE='#'
PCSX2_USE_GTK3_FALSE=
GTK_HEADERS_INCLUDE=`pkg-config --cflags gtk+-2.0`
echo "using GTK2 for pcsx2"
fi
AC_SUBST([GTK_HEADERS_INCLUDE])

if test "x$DETECT_GTK3" = "x-DGTK3_API=TRUE_GENTOO"
then
PCSX2_USE_GTK3_GENTOO_TRUE=
PCSX2_USE_GTK3_GENTOO_FALSE='#'
echo "using GTK3 for pcsx2 (gentoo)"
else
PCSX2_USE_GTK3_GENTOO_TRUE='#'
PCSX2_USE_GTK3_GENTOO_FALSE=
fi

#resource system
RESOURCE_LIST="\$(shell \$(glib-compile-resources --sourcedir= --generate-dependencies resources/res.xml))"
AC_SUBST([RESOURCE_LIST])

#run all configure scripts
AC_CONFIG_COMMANDS_PRE([cd ppsspp  && mkdir -p build && cd build && cmake $CMAKE_BUILD_TYPE ..&&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd pcsx2  && mkdir -p build && cd build && cmake $CMAKE_BUILD_TYPE -DBUILTIN_PAD=ON -DBUILTIN_GS=ON `../detect_gtk_flags.sh` ..&&cd ../..])
AC_CONFIG_COMMANDS_PRE([cd mednafen && aclocal && autoconf && automake --add-missing --foreign && ./configure&&cd ..])
AC_CONFIG_COMMANDS_PRE([cd dolphin && mkdir -p build && cd build && cmake $CMAKE_BUILD_TYPE ..&&cd ../..])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
