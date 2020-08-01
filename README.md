<p align="center">
  <img width="80" src="pictures/barrel.png">
</p>

# Marley : Many Awesome Retro Linux Emulators, Yeah!

https://launchpad.net/~beauman/+archive/ubuntu/marley <br />
https://github.com/beaumanvienna/marley 

## *** Installation for Ubuntu 18.04 and 20.04 ***

sudo add-apt-repository ppa:beauman/marley <br />
sudo apt update <br />
sudo apt install marley <br />

## Compile from source

#clone & and check out revision <br />
git clone https://github.com/beaumanvienna/marley  <br />
cd marley <br />
#change to the version you like to work with (currently only 'master')  <br />
git checkout master <br />

#Install build dependencies specified in debian/control (search for 'Build-Depends'):  <br />

| Ubuntu 18.04 | 
| ------------ | 
| sudo apt install libwxgtk3.0-dev debhelper cmake chrpath \
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
libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales-all |


| Ubuntu 20.04 | 
| ------------ | 
| sudo apt install libwxgtk3.0-gtk3-dev libgtk-3-dev debhelper cmake chrpath \ |
| libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \ |
| autotools-dev dh-autoreconf libasound2-dev libgl1-mesa-dev \ |
| libjack-dev liblzo2-dev libmpcdec-dev libsamplerate0-dev libsndio-dev \ |
| libsndfile1-dev libtrio-dev libvorbisidec-dev x11proto-core-dev zlib1g-dev \ |
| dpkg-dev pkg-config lsb-release libao-dev \ |
| libavcodec-dev libavformat-dev libbluetooth-dev libcurl4-gnutls-dev \ |
| libegl1-mesa-dev libenet-dev libevdev-dev libgtk2.0-dev \ |
| libminiupnpc-dev libopenal-dev libmbedtls-dev libpulse-dev \ |
| libreadline-dev libsfml-dev libsoil-dev libswscale-dev libudev-dev \ |
| libusb-1.0-0-dev libwxbase3.0-dev  libxext-dev \ |
| libxrandr-dev portaudio19-dev qtbase5-private-dev libsamplerate0-dev libfreetype6-dev libglu1-mesa-dev nasm \ |
| libboost-filesystem-dev libboost-system-dev libswresample-dev libglew-dev libsnappy-dev libavutil-dev \ |
| libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales-all |

### Configure and make
aclocal <br />
autoconf <br />
automake --add-missing --foreign <br />
./configure <br />
make <br />


### start it
./marley <br />
