<img src="pictures/barrel.png" />
Marley : Many Awesome Retro Linux Emulators, Yeah!

https://launchpad.net/~beauman/+archive/ubuntu/marley
https://github.com/beaumanvienna/marley

#*** Installation for Ubuntu 18.04 ***
#=====================================

sudo add-apt-repository ppa:beauman/marley
sudo apt update
sudo apt upgrade

#install as binary version
#-------------------------
sudo apt install marley

#Compile from source
#-------------------

#clone & and check out revision
git clone https://github.com/beaumanvienna/marley
cd marley
#change to the version you like to work with (currently only 'master')
git checkout master

#Install build dependencies specified in debian/control (search for 'Build-Depends'):
# Ubuntu 18.04
sudo apt install libwxgtk3.0-dev debhelper cmake chrpath \
libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
autotools-dev dh-autoreconf libasound2-dev libgl1-mesa-dev \
libjack-dev liblzo2-dev libmpcdec-dev libsamplerate0-dev libsndio-dev \
libsndfile1-dev libtrio-dev libvorbisidec-dev x11proto-core-dev zlib1g-dev \
dpkg-dev pkg-config lsb-release libao-dev \
libavcodec-dev libavformat-dev libbluetooth-dev libcurl4-gnutls-dev \
libegl1-mesa-dev libenet-dev libevdev-dev libgtk2.0-dev \
libminiupnpc-dev libopenal-dev libmbedtls-dev libpulse-dev \
libreadline-dev libsfml-dev libsoil-dev libswscale-dev libudev-dev \
libusb-1.0-0-dev libwxbase3.0-dev  libxext-dev \
libxrandr-dev portaudio19-dev qtbase5-private-dev libsamplerate0-dev libfreetype6-dev libglu1-mesa-dev nasm \
libboost-filesystem-dev libboost-system-dev libswresample-dev libglew-dev libsnappy-dev libavutil-dev \
libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales-all

# Ubuntu 20.04
sudo apt install libwxgtk3.0-gtk3-dev libgtk-3-dev debhelper cmake chrpath \
libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
autotools-dev dh-autoreconf libasound2-dev libgl1-mesa-dev \
libjack-dev liblzo2-dev libmpcdec-dev libsamplerate0-dev libsndio-dev \
libsndfile1-dev libtrio-dev libvorbisidec-dev x11proto-core-dev zlib1g-dev \
dpkg-dev pkg-config lsb-release libao-dev \
libavcodec-dev libavformat-dev libbluetooth-dev libcurl4-gnutls-dev \
libegl1-mesa-dev libenet-dev libevdev-dev libgtk2.0-dev \
libminiupnpc-dev libopenal-dev libmbedtls-dev libpulse-dev \
libreadline-dev libsfml-dev libsoil-dev libswscale-dev libudev-dev \
libusb-1.0-0-dev libwxbase3.0-dev  libxext-dev \
libxrandr-dev portaudio19-dev qtbase5-private-dev libsamplerate0-dev libfreetype6-dev libglu1-mesa-dev nasm \
libboost-filesystem-dev libboost-system-dev libswresample-dev libglew-dev libsnappy-dev libavutil-dev \
libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales-all

#Configure and make
aclocal 
autoconf
automake --add-missing --foreign
./configure
make

sudo mkdir -p /usr/games/Marley/PCSX2
sudo cp pcsx2/build/plugins/libcdvdGigaherz.so pcsx2/build/plugins/libCDVDnull.so pcsx2/build/plugins/libdev9ghzdrk-0.4.so \
pcsx2/build/plugins/libdev9null-0.5.0.so pcsx2/build/plugins/libFWnull-0.7.0.so \
pcsx2/build/plugins/libspu2x-2.0.0.so pcsx2/build/plugins/libUSBnull-0.7.0.so /usr/games/Marley/PCSX2/

#start it
./marley
