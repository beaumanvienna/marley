#!/bin/sh

set -ex

linux_64_before_install() {
	# Compilers
	if [ "${CXX}" = "g++" ]; then
		sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		COMPILER_PACKAGE="g++-${VERSION}"
	fi

	sudo apt-get -qq update

	sudo apt-get -y install \
		debhelper cmake chrpath libsdl2-dev libsdl2-image-dev \
		libsdl2-ttf-dev \
		autotools-dev dh-autoreconf libasound2-dev libgl1-mesa-dev \
		libjack-dev liblzo2-dev libmpcdec-dev libsamplerate0-dev libsndio-dev \
		libsndfile1-dev libtrio-dev libvorbisidec-dev x11proto-core-dev zlib1g-dev \
		dpkg-dev pkg-config lsb-release libao-dev \
		libavcodec-dev libavformat-dev libbluetooth-dev libcurl4-gnutls-dev \
		libegl1-mesa-dev libenet-dev libevdev-dev libgtk2.0-dev \
		libminiupnpc-dev libopenal-dev libmbedtls-dev libpulse-dev \
		libreadline-dev libsfml-dev libsoil-dev libswscale-dev libudev-dev \
		libusb-1.0-0-dev libwxbase3.0-dev libwxgtk3.0-dev libxext-dev \
		libxrandr-dev portaudio19-dev qtbase5-private-dev libsamplerate0-dev libfreetype6-dev libglu1-mesa-dev nasm \
		libboost-filesystem-dev libboost-system-dev libswresample-dev libglew-dev libsnappy-dev libavutil-dev \
		libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales-all \
		${COMPILER_PACKAGE}
}


linux_64_script() {
	aclocal 
	autoconf
	automake --add-missing --foreign
	./configure
	make
}

linux_after_success() {
	ccache -s
}

case "${1}" in
before_install|script)
	${TRAVIS_OS_NAME}_${BITS}_${1}
	;;
before_script)
    ;;
after_success)
	${TRAVIS_OS_NAME}_${1}
	;;
*)
	echo "Unknown command" && false
	;;
esac
