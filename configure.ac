# run: "aclocal && autoconf && automake --add-missing --foreign"
# then "./configure --enable-debug" or simply "./configure"
# and "make"


AC_INIT([marley], 0.1.0)
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AM_EXTRA_RECURSIVE_TARGETS([mednafen_marley])
AC_PROG_CXX
AC_PROG_CC
AC_DEFINE([PICTURES],["pictures/"])
AC_DEFINE([RESOURCES],["resources/"])
AC_PROG_RANLIB
AM_INIT_AUTOMAKE([subdir-objects])

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

AC_ARG_ENABLE(debug,
[case "${enableval}" in
             yes) AC_CONFIG_COMMANDS_PRE([cd pcsx2  && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..&&cd ../..]) ;;
             no)  AC_CONFIG_COMMANDS_PRE([cd pcsx2  && mkdir -p build && cd build && cmake ..&&cd ../..]) ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AC_ARG_ENABLE(debug,
[case "${enableval}" in
             yes) AC_CONFIG_COMMANDS_PRE([cd mednafen && aclocal && autoconf && automake --add-missing --foreign && ./configure --enable-debug&&cd ..]) ;;
             no)  AC_CONFIG_COMMANDS_PRE([cd mednafen && aclocal && autoconf && automake --add-missing --foreign && ./configure &&cd ..]) ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])


AC_ARG_ENABLE(debug,
[case "${enableval}" in
             yes) AC_CONFIG_COMMANDS_PRE([cd dolphin && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..&&cd ../..]) ;;
             no)  AC_CONFIG_COMMANDS_PRE([cd dolphin && mkdir -p build && cd build && cmake ..&&cd ../..]) ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])


AC_ARG_ENABLE(debug,
[case "${enableval}" in
             yes) AC_CONFIG_COMMANDS_PRE([cd ppsspp  && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..&&cd ../..]) ;;
             no)  AC_CONFIG_COMMANDS_PRE([cd ppsspp  && mkdir -p build && cd build && cmake ..&&cd ../..]) ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])



AC_CONFIG_FILES([Makefile])
AC_OUTPUT
