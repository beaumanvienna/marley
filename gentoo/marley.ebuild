# Maintainer: beaumanvienna (JC) <beaumanvienna@gmail.com>
# Distributed under the terms of the GNU General Public License v3
# File: https://github.com/beaumanvienna/marley/gentoo/marley.ebuild

EAPI=7

inherit cmake git-r3 wxwidgets

DESCRIPTION="A bundle of console emulators for Linux (git)"
HOMEPAGE="https://github.com/beaumanvienna/marley#readme"
EGIT_REPO_URI="https://github.com/beaumanvienna/${PN}.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS=""
IUSE=""

#from PKGBUILD (Arch Linux)
#depends=(sdl2\                     ok
#         sdl2_image\               ok
#         sdl2_ttf\                 ok
#         qt5-base\                 ok
#         wxgtk2\                   will be wxgtk3 (ok)
#         libaio\                   ok
#         sndio\
#         libpulse\                 ok
#         alsa-lib\                 ok
#         libsndfile\               ok
#         libsamplerate\            ok
#         libpng\                   ok
#         libjpeg-turbo\            ok
#         libavcodec.so\
#         boost\                    ok
#         boost-libs\               included in boost?
#         libudev.so\               ok
#         libevdev\                 ok
#         libusb-1.0.so\            ok
#         bluez-libs\               ok
#         libx11\                   ok
#         libxrandr\                ok
#         mesa\                     ok
#         libgl\                    (mesa?)
#         glew\                     ok
#         glibc\                    ok
#         glu\                      ok
#         libglvnd\                 ok
#         libxi\                    ok
#         lzo\                      ok
#         mbedtls\                  ok
#         libpcap\                  ok
#         zlib\                     ok
#         sfml\                     ok
#         libminiupnpc.so\          ok
#         aom\                      ok
#         python\                   ok
#         freetype2\                ok         
#         zip                       ok


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
	media-libs/libsdl2_image
	media-libs/libsdl2_ttf
	media-libs/glew
	media-libs/glu
	media-libs/libglvnd
	media-libs/libaom
	media-libs/freetype
	media-libs/libsamplerate
	media-libs/libsndfile
	media-libs/libjpeg-turbo
	net-libs/libpcap
	sys-libs/zlib
	sys-libs/glibc
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
DEPEND="${RDEPEND}
	dev-cpp/pngpp
	dev-cpp/sparsehash
"

pkg_setup() {
}

src_configure() {

	#

}

src_install() {
	# 
}
