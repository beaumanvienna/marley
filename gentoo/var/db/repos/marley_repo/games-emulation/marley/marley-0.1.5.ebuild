# Maintainer: beaumanvienna (JC) <beaumanvienna@gmail.com>
# Distributed under the terms of the GNU General Public License v3
# File: https://github.com/beaumanvienna/marley/gentoo/marley.ebuild

EAPI=7

inherit autotools git-r3

DESCRIPTION="A bundle of console emulators for Linux (git)"
HOMEPAGE="https://github.com/beaumanvienna/marley#readme"

SRC_URI="https://github.com/beaumanvienna/marley/archive/master.zip"
EGIT_REPO_URI="https://github.com/beaumanvienna/marley.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64"
IUSE="alsa bluetooth discord-presence doc +evdev ffmpeg log lto profile pulseaudio +qt5 systemd upnp"

RDEPEND="
	app-arch/bzip2
	app-arch/xz-utils
	app-arch/zip
	dev-libs/libaio
	dev-libs/libxml2
	dev-libs/boost
	dev-lang/python
	media-libs/alsa-lib
	media-libs/libpng
	media-libs/libsdl2
	media-libs/sdl2-image
	media-libs/sdl2-ttf
	media-libs/sdl2-mixer
	media-libs/glew
	media-libs/glu
	media-libs/libglvnd
	media-libs/libaom
	media-libs/freetype
	media-libs/libsamplerate
	media-libs/libsndfile
	media-libs/libjpeg-turbo
	media-libs/libsoundtouch
	media-video/ffmpeg
	net-libs/libpcap
	sys-libs/zlib
	sys-libs/glibc
	dev-libs/libzip
	virtual/libudev
	virtual/opengl
	x11-libs/gtk+
	x11-libs/libICE
	x11-libs/libX11
	x11-libs/libXext
	>=x11-libs/wxGTK-3.0.4-r302
	dev-libs/lzo:2=
	dev-libs/pugixml:0=
	media-libs/libpng:0=
	media-libs/libsfml
	media-libs/mesa[egl]
	media-sound/jack2
	net-libs/mbedtls:0=
	sys-libs/readline:0=
	sys-libs/zlib:0=
	x11-libs/libXext
	x11-libs/libXi
	x11-libs/libXrandr
	virtual/libusb:1
	virtual/opengl
	alsa? ( media-libs/alsa-lib )
	bluetooth? ( net-wireless/bluez )
	evdev? (
		dev-libs/libevdev
		virtual/udev
	)
	profile? ( dev-util/oprofile )
	pulseaudio? ( media-sound/pulseaudio )
	qt5? (
		dev-qt/qtcore:5
		dev-qt/qtgui:5
		dev-qt/qtwidgets:5
	)
	systemd? ( sys-apps/systemd:0= )
	upnp? ( net-libs/miniupnpc )
"

DEPEND="
	${RDEPEND}
	dev-cpp/pngpp
	dev-cpp/sparsehash
"
    
BDEPEND="
	sys-devel/gettext
	virtual/pkgconfig
"

src_prepare() {
	echo MAKEOPTS $MAKEOPTS
	default
	aclocal
	autoconf
	automake --add-missing --foreign
	./configure --prefix=/usr
	make $MAKEOPTS
}

src_configure() {
	econf
}

src_install() {
	default
}
