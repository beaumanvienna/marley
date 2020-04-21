# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/yo/packaging/dolphin/dolphin-5.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yo/packaging/dolphin/dolphin-5.0/build

# Include any dependencies generated for this target.
include Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/depend.make

# Include the progress variables for this target.
include Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/progress.make

# Include the compile flags for this target's objects.
include Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/flags.make

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/flags.make
Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o: ../Source/UnitTests/Core/MMIOTest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/yo/packaging/dolphin/dolphin-5.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o"
	cd /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o -c /home/yo/packaging/dolphin/dolphin-5.0/Source/UnitTests/Core/MMIOTest.cpp

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MMIOTest.dir/MMIOTest.cpp.i"
	cd /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yo/packaging/dolphin/dolphin-5.0/Source/UnitTests/Core/MMIOTest.cpp > CMakeFiles/MMIOTest.dir/MMIOTest.cpp.i

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MMIOTest.dir/MMIOTest.cpp.s"
	cd /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yo/packaging/dolphin/dolphin-5.0/Source/UnitTests/Core/MMIOTest.cpp -o CMakeFiles/MMIOTest.dir/MMIOTest.cpp.s

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.requires:

.PHONY : Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.requires

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.provides: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.requires
	$(MAKE) -f Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/build.make Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.provides.build
.PHONY : Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.provides

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.provides.build: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o


# Object files for target MMIOTest
MMIOTest_OBJECTS = \
"CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o"

# External object files for target MMIOTest
MMIOTest_EXTERNAL_OBJECTS = \
"/home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/CMakeFiles/unittests_stubhost.dir/StubHost.cpp.o"

Binaries/Tests/MMIOTest: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o
Binaries/Tests/MMIOTest: Source/UnitTests/CMakeFiles/unittests_stubhost.dir/StubHost.cpp.o
Binaries/Tests/MMIOTest: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/build.make
Binaries/Tests/MMIOTest: Source/Core/Core/libcore.a
Binaries/Tests/MMIOTest: Source/Core/UICommon/libuicommon.a
Binaries/Tests/MMIOTest: Externals/gtest/libgtest_main.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/Null/libvideonull.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/OGL/libvideoogl.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/Software/libvideosoftware.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/Vulkan/libvideovulkan.a
Binaries/Tests/MMIOTest: Source/Core/VideoCommon/libvideocommon.a
Binaries/Tests/MMIOTest: Source/Core/Core/libcore.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/Null/libvideonull.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/OGL/libvideoogl.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/Software/libvideosoftware.a
Binaries/Tests/MMIOTest: Source/Core/VideoBackends/Vulkan/libvideovulkan.a
Binaries/Tests/MMIOTest: Source/Core/VideoCommon/libvideocommon.a
Binaries/Tests/MMIOTest: Source/Core/AudioCommon/libaudiocommon.a
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libasound.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libpulse.so
Binaries/Tests/MMIOTest: Externals/soundtouch/libSoundTouch.a
Binaries/Tests/MMIOTest: Externals/FreeSurround/libFreeSurround.a
Binaries/Tests/MMIOTest: Externals/cubeb/libcubeb.a
Binaries/Tests/MMIOTest: Source/Core/DiscIO/libdiscio.a
Binaries/Tests/MMIOTest: Source/Core/InputCommon/libinputcommon.a
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libevdev.so
Binaries/Tests/MMIOTest: /lib/x86_64-linux-gnu/libudev.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/liblzo2.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libz.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libbluetooth.so
Binaries/Tests/MMIOTest: Externals/hidapi/libhidapi.a
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libSM.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libICE.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libX11.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libXext.so
Binaries/Tests/MMIOTest: Externals/glslang/libglslang.a
Binaries/Tests/MMIOTest: Externals/xxhash/libxxhash.a
Binaries/Tests/MMIOTest: Externals/imgui/libimgui.a
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libavformat.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libavcodec.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libswscale.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libavutil.so
Binaries/Tests/MMIOTest: Source/Core/Common/libcommon.a
Binaries/Tests/MMIOTest: Externals/enet/libenet.a
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libmbedtls.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libmbedx509.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libmbedcrypto.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libcurl.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libc.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libEGL.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libOpenGL.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libGLX.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libGLU.so
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libminiupnpc.so
Binaries/Tests/MMIOTest: Externals/pugixml/libpugixml.a
Binaries/Tests/MMIOTest: Externals/fmt/libfmt.a
Binaries/Tests/MMIOTest: Externals/Bochs_disasm/libbdisasm.a
Binaries/Tests/MMIOTest: Externals/cpp-optparse/libcpp-optparse.a
Binaries/Tests/MMIOTest: Externals/minizip/libminizip.a
Binaries/Tests/MMIOTest: /usr/lib/x86_64-linux-gnu/libusb-1.0.so
Binaries/Tests/MMIOTest: Externals/discord-rpc/src/libdiscord-rpc.a
Binaries/Tests/MMIOTest: Externals/gtest/libgtest.a
Binaries/Tests/MMIOTest: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/yo/packaging/dolphin/dolphin-5.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../Binaries/Tests/MMIOTest"
	cd /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MMIOTest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/build: Binaries/Tests/MMIOTest

.PHONY : Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/build

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/requires: Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/MMIOTest.cpp.o.requires

.PHONY : Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/requires

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/clean:
	cd /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core && $(CMAKE_COMMAND) -P CMakeFiles/MMIOTest.dir/cmake_clean.cmake
.PHONY : Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/clean

Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/depend:
	cd /home/yo/packaging/dolphin/dolphin-5.0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yo/packaging/dolphin/dolphin-5.0 /home/yo/packaging/dolphin/dolphin-5.0/Source/UnitTests/Core /home/yo/packaging/dolphin/dolphin-5.0/build /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core /home/yo/packaging/dolphin/dolphin-5.0/build/Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : Source/UnitTests/Core/CMakeFiles/MMIOTest.dir/depend

