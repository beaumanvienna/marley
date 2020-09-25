<p align="center">
  <img width="80" src="pictures/barrel.bmp">
</p>

# Marley: Many Awesome Retro Linux Emulators, Yay!

https://launchpad.net/~beauman/+archive/ubuntu/marley <br />
https://github.com/beaumanvienna/marley 

- [User Manual](#user-manual)
- [Controls](#controls)
- [Developer information](#developer-information)
- [Installation for Ubuntu 18.04 and 20.04](#installation-for-ubuntu-1804-and-2004)
- [Compile from source](#compile-from-source)

## User Manual

Marley is a bundle of console emulators for Linux.
The project is comprised of Mednafen, Dolphin, 
Mupen64plus, PPSSPP, and PCSX2.
Marley's launcher interface is designed to be used with 
gamepads. Gamepads can be hotplugged and automatically detected
in the launcher interface. Marley also allows to configure 
the button assignment of a controller manually. The controller settings 
are shared with all emulators. Marley allows browsing
ROM collections and launching games for the GBA, GBC, NES/SNES, Sega Genesis, N64, PS1, PS2, PSP, Gamecube, and Wii. <br /> 
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
There is a setup screen to configure Marley. The folder path to the ROM collections needs to be entered with a keyboard, as well as a path to the firmware of the PS1 and PS2. All other settings are configured automatically.<br />
<br />
To contact us, open a ticket on the issue tab. Pull requests are welcome! <br />
<br />
A big thanks to the teams of Mednafen, Dolphin and PPSSPP, Mupen64Plus, and PCSX2. Thanks for all the work you put into your projects and the awesome emulators you provide. All credits go to you guys, while this project here is just a small front end.<br />
<br />
Happy retro gaming! <br />
<br />
Team Marley
<br />
## Controls
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
Use "p" to print the current gamepad mapping(s) while in the front end.<br />
<br />
Use "l" to print a list of detected controllers to the command line while in the front end.<br />
<br />
command line options: <br />
<br />
  --version             : print version <br />
  --fullscreen, -f      : start in fullscreen mode<br />
  --keepX11pointer, -k  : do not switch off X11 mouse pointer<br />
<br />

## Developer information
Marley is using five core modules that are linked as static libraries. This way, it is ensured that the core modules are always available, compiled with the same compiler / compiler version, and against the same dependencies. Resources are shared among the front end and the emulators. For the most part, these are the SDL game controller instances, the SDL main window, and the Open GL settings. <br />
<br />
Pull requests are welcome! The project needs Open GL and game programmers to bring more life into the front end. It should feel like an retro game or console eventually. A retro art designer would also be great.  Other programming tasks include integrating more core modules such as Scumm VM or RPCS3, porting the ROM collection browser from Kodi to C++/Marley, or simply testing and bug fixing.<br />
<br />
Marley knows only big-ṕicture mode. This is to resemble a gaming console. It is designed to be a "sofa" application or could be used for a DIY arcade machine. Marley does not have mouse support or allow pop-up windows. All core modules render into an SDL Open GL context in the main window. While Mednafen, PPSSPP, and Mupen64Plus were doing this already and were easy to integrate, Dolphin was changed from an X11 Open GL context and PCSX2 was changed from a wxWidgets context. <br />
<br />
Difficulties arise when changing the previously standalone emulators into libraries that can get called multiple times. For all five core modules, the initialization was reworked to remove any dependencies from globally initialized signals. <br />
<br />
The mapping of the game controllers happens entirely in the front end. Unlike the old SDL_Joystick, the SDL_Gamecontroller has a fixed mapping. Marley is taking advantage of this. The default settings in the core modules are hard-coded to the SDL_Gamecontroller. This way, the very first run of a core module automatically generates the correct ini files. Marley is using the SDL game controller database and a nifty little trick to find similar controllers in the database when there is no exact match. Marley supports standard PS/X-Box controllers, as well as less-than 15-button controllers such as the NES gamepads. It also supports the Nintendo Wiimote by using Dolphin's built-in drivers in the front end.<br />
<br />
Needless to say, Marley has a configuration folder in which the core modules save their settings. Marley is completely isolated from other emulator installations. <br />
<br />
The project build system is autoconf, however, most emulator modules use cmake. The build processes are chained together. Marley can be compiled with gcc or clang. We tried integrating Travis into the Github repository. Unfortunately, the build was too long (or their build server too slow) and it got canceled.  Github CI has yet to be looked into. There are no intentions to set this project up for Windows. Since most people prefer gaming under Windows, pull requests in this regard are welcome, though. The emulators should work fine under Windows, except for PCSX2, for which the latest x64 version supporting both Linux and Windows did not work. This PCSX2 version is trying to allocate memory close to the program code, which unfortunately fails when the other core modules are present. This issue could be solved with a few defines. <br />
<br />
Currently, development takes place under Ubuntu and Arch Linux. Testing is happening under Bionic and Focal, under Arch Linux, and soon also under Fedora and Gentoo. <br />
<br />
There is a unit test for each core emulator available to help isolate faults, improve debugging, and decrease compile time during development. Adding a core module begins with creating a new unit test. When the unit test is up and running, the core module can be added to the main application. To build the unit tests, run ./configure in the project root folder and say 'make unit_tests' in tests/.<br />
<br />
To debug marley, run "configure --enable-debug" and "make" in the root directory. Start gdb and run "./marley". Create the unit tests as described above. To debug PCSX2, run the unit test version "./tests/pcsx2/PCSX2" without any additional arguments in the terminal (not gdb). The normally-dormant GUI of PCSX2 should come up. React with "no" to any assert messages. Select the interpreter instead of the recompiler for the Emotion Engine under Emulation Settings / EE and IOP and close PCSX2. Launch gdb in tests/pcsx2 and run "./PCSX2 myGame.iso". To debug dolphin, set "Fastmem" to "False" in ~/.marley/dolphin-emu/Config/Dolphin.ini. Start gdb in tests/dolphin and and run "./DOLPHIN myGame.iso". A good front end for gdb is gdbgui, see https://www.gdbgui.com/.<br />
<br />



## Installation for Ubuntu 18.04, 20.04, and Ubuntu derivatives

sudo add-apt-repository ppa:beauman/marley <br />
sudo apt update <br />
sudo apt install marley <br />

##  Compile from source

#Install build dependencies specified in debian/control (search for 'Build-Depends'):  <br />

| Ubuntu 18.04, Elementary OS 5.1, Linux Mint 19 | 
| ------------ | 
| sudo apt install git libwxgtk3.0-dev debhelper cmake chrpath \ |
libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \ |
autotools-dev dh-autoreconf libasound2-dev libgl1-mesa-dev \ |
libjack-dev liblzo2-dev libmpcdec-dev libsamplerate0-dev libsndio-dev \ |
libsndfile1-dev libtrio-dev libvorbisidec-dev x11proto-core-dev zlib1g-dev \ |
dpkg-dev pkg-config lsb-release libao-dev \ |
libavcodec-dev libavformat-dev libbluetooth-dev libcurl4-gnutls-dev \ |
libegl1-mesa-dev libenet-dev libevdev-dev libgtk2.0-dev \ |
libminiupnpc-dev libopenal-dev libmbedtls-dev libpulse-dev \ |
libreadline-dev libsfml-dev libsoil-dev libswscale-dev libudev-dev \ |
libusb-1.0-0-dev libwxbase3.0-dev  libxext-dev libx11-xcb-dev \ |
libxrandr-dev portaudio19-dev qtbase5-private-dev libsamplerate0-dev libfreetype6-dev libglu1-mesa-dev nasm \ |
libboost-filesystem-dev libboost-system-dev libswresample-dev libglew-dev libsnappy-dev libavutil-dev \ |
libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales zip libzstd1-dev |


| Ubuntu 20.04, Linux Mint 20 | 
| ------------ | 
| sudo apt install git libwxgtk3.0-gtk3-dev libgtk-3-dev debhelper cmake chrpath \ |
| libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \ |
| autotools-dev dh-autoreconf libasound2-dev libgl1-mesa-dev \ |
| libjack-dev liblzo2-dev libmpcdec-dev libsamplerate0-dev libsndio-dev \ |
| libsndfile1-dev libtrio-dev libvorbisidec-dev x11proto-core-dev zlib1g-dev \ |
| dpkg-dev pkg-config lsb-release libao-dev \ |
| libavcodec-dev libavformat-dev libbluetooth-dev libcurl4-gnutls-dev \ |
| libegl1-mesa-dev libenet-dev libevdev-dev libgtk2.0-dev \ |
| libminiupnpc-dev libopenal-dev libmbedtls-dev libpulse-dev \ |
| libreadline-dev libsfml-dev libsoil-dev libswscale-dev libudev-dev \ |
| libusb-1.0-0-dev libwxbase3.0-dev  libxext-dev libx11-xcb-dev \ |
| libxrandr-dev portaudio19-dev qtbase5-private-dev libsamplerate0-dev libfreetype6-dev libglu1-mesa-dev nasm \ |
| libboost-filesystem-dev libboost-system-dev libswresample-dev libglew-dev libsnappy-dev libavutil-dev \ |
| libaio-dev liblzma-dev libpcap0.8-dev libpng-dev libsoundtouch-dev libxml2-dev libx11-dev locales zip build-essential libzstd-dev |


| Arch, Manjaro | 
| ---- | 
| sudo pacman -S libaio libjpeg-turbo libpcap libpulse portaudio sdl2 soundtouch boost-libs \ |
| alsa-lib bluez-libs enet libevdev libx11 libxi libxrandr lzo mbedtls libsndfile mesa libsamplerate \ | 
| libudev.so libusb-1.0.so libgl glew glibc zlib glu cmake git libglvnd python qt5-tools  freetype2 \ |
| qt5-base sfml libavcodec.so libavformat.so libavutil.so libcurl.so libminiupnpc.so libswscale.so \ |
| sdl2_image sdl2_ttf nasm boost libpng wxgtk2 libzip sndio aom zip bluez bluez-plugins bluez-utils |

### clone and check out branch <br />
git clone https://github.com/beaumanvienna/marley  <br />
cd marley <br />
#change to the branch you like to work with (currently only 'master')  <br />
git checkout master <br />

### Define the number of CPU cores to be used for compiling
#"-j1" for one core, "-j2" for two cores, etc. <br />
#To use all available CPU cores, say:<br />
export MAKEFLAGS=-j$(nproc)

#Check, if MAKEFLAGS is as expected:<br />
echo $MAKEFLAGS

### Configure and make
aclocal && autoconf && automake --add-missing --foreign<br /> 
./configure --prefix=/usr MAKEFLAGS=$MAKEFLAGS<br />
make<br />


### Start it
./marley <br />

### Install system-wide
sudo make install <br />

### Remark: using clang instead of gcc<br />
#From where "Configure and make" is described above, say<br />
export CXX=clang++<br />
export CC=clang<br />
aclocal && autoconf && automake --add-missing --foreign && ./configure MAKEFLAGS=$MAKEFLAGS CXX=$CXX CC=$CC<br />
make<br />
