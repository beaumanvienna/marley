<p align="center">
  <img width="80" src="pictures/barrel.png">
</p>

# Marley : Many Awesome Retro Linux Emulators, Yeah!

https://launchpad.net/~beauman/+archive/ubuntu/marley <br />
https://github.com/beaumanvienna/marley 


Marley is a bundle of console emulators plus a launcher app for Linux.
The project is comprised of Mednafen, Dolphin, 
Mupen64plus, PPSSPP, and PCSX2.
Marley's launcher interface is designed to be used with 
gamepads. Gamepads can be hotplugged and automatically detetcted
in the launcher interface. Marley also allows to configure 
the button assignment of a controller maunally. The controller settings 
are shared with all emulators. Marley allows to browse 
one's ROM collection and launch games for the NES, SNES, N64, PS1, PS2, PSP,
Sega Genesis, Gamecube, and Wii. <br /> 
<br />
Please make sure to install an
OpenGl driver for your graphics card. <br /> 
<br />
If you have a Bluethooth gamepad, 
you need to pair it with your computer first before it can be used in Marley. 
The Wiimote is an exception and needs to be directly paired with Marley. On newer 
Wiimotes, you need to press 1 and 2 at the same time. For older Wiimote models, 
you need to press a small button in the battery compartment. <br />
<br />
To contact us, open a ticket on the issue tab. Pull requests are welcome! <br />
<br />
Happy retro gaming! <br />
<br />
Team Marley
<br />
## *** Controls ***
<br />
Use your controller or arrow keys/enter/escape on your keyboard to navigate.<br />
<br />
Use "f" to toggle fullscreen in-game or in the front end.<br />
<br />
Use "F5" to save and "F7" to load game states while playing.<br />
<br />
Use the guide button to exit a game with no questions asked. The guide button is the big one in the middle.<br />
<br />
Use "ESC" to exit. Same as guide button.<br />
<br />
Use "p" to print the current gamepad mapping(s) while in the front-end.<br />
<br />
Use "l" to print a list of detected controllers to the command line while in the front-end.<br />
<br />
command line options: <br />
<br />
  --version             : print version <br />
  --fullscreen, -f      : start in fullscreen mode<br />
<br />

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
