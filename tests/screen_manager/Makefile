#standalone diag_screen test version
COMPILER_ARTIFACTS = -I/usr/include/SDL2 -I../../screen_manager/ -I../../screen_manager/Common/

SOURCE_FILES =  main.cpp
SOURCE_FILES += ../../screen_manager/SDL/SDLMain.cpp
SOURCE_FILES += ../../screen_manager/UI/NativeApp.cpp
SOURCE_FILES += ../../screen_manager/SDL/SDLGLGraphicsContext.cpp
SOURCE_FILES += ../../screen_manager/SDL/SDLJoystick.cpp
SOURCE_FILES += ../../screen_manager/Common/System/Display.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/Screen.cpp
SOURCE_FILES += ../../screen_manager/Common/TimeUtil.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/Root.cpp
SOURCE_FILES += ../../screen_manager/Common/File/VFS/AssetReader.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/View.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/Context.cpp
SOURCE_FILES += ../../screen_manager/Common/Render/Text/draw_text.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/UI.cpp
SOURCE_FILES += ../../screen_manager/Common/Render/DrawBuffer.cpp
SOURCE_FILES += ../../screen_manager/Common/Render/TextureAtlas.cpp
SOURCE_FILES += ../../screen_manager/Common/File/VFS/VFS.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/thin3d.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Text/WrapText.cpp
SOURCE_FILES += ../../screen_manager/UI/TextureUtil.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Color/RGBAUtil.cpp
SOURCE_FILES += ../../screen_manager/Common/ColorConv.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Encoding/Utf8.cpp
SOURCE_FILES += ../../screen_manager/Common/File/FileUtil.cpp
SOURCE_FILES += ../../screen_manager/Common/File/DirListing.cpp
SOURCE_FILES += ../../screen_manager/Common/Log.cpp
SOURCE_FILES += ../../screen_manager/Common/SysError.cpp 
SOURCE_FILES += ../../screen_manager/Common/Input/InputState.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Format/PNGLoad.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Format/ZIMLoad.cpp
SOURCE_FILES += ../../screen_manager/UI/MiscScreens.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/UIScreen.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Text/I18n.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Format/IniFile.cpp
SOURCE_FILES += ../../screen_manager/Common/StringUtils.cpp
SOURCE_FILES += ../../screen_manager/Common/Buffer.cpp
SOURCE_FILES += ../../screen_manager/Common/Math/curves.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/ViewGroup.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Text/Parsers.cpp
SOURCE_FILES += ../../screen_manager/Common/UI/Tween.cpp
SOURCE_FILES += ../../screen_manager/Common/File/FileDescriptor.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/OpenGL/GLRenderManager.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/OpenGL/GLQueueRunner.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/OpenGL/GLFeatures.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/OpenGL/thin3d_gl.cpp
SOURCE_FILES += ../../screen_manager/Common/MemoryUtil.cpp
SOURCE_FILES += ../../screen_manager/UI/OnScreenDisplay.cpp
SOURCE_FILES += ../../screen_manager/Common/Data/Convert/SmallDataConvert.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/OpenGL/DataFormatGL.cpp
SOURCE_FILES += ../../screen_manager/Common/Math/lin/matrix4x4.cpp
SOURCE_FILES += ../../screen_manager/Common/GPU/OpenGL/GLDebugLog.cpp
SOURCE_FILES += ../../screen_manager/Common/Math/fast/fast_matrix_sse.c
SOURCE_FILES += ../../screen_manager/Common/File/PathBrowser.cpp
SOURCE_FILES += ../../screen_manager/Common/KeyMap.cpp
SOURCE_FILES += ../../screen_manager/Common/Thread/ThreadUtil.cpp
SOURCE_FILES += ../../screen_manager/UI/SettingsScreen.cpp
SOURCE_FILES += ../../screen_manager/UI/MainScreen.cpp

#screen_manager, static
SCREEN_MAN_LD = \
	../../screen_manager/build/UI/libui.a \
	../../screen_manager/build/Common/libcommon.a \
	../../screen_manager/build/SDL/libsdl.a

LINKER_OBJECTS_STATIC = $(SCREEN_MAN_LD) $(SCREEN_MAN_LD) $(SCREEN_MAN_LD)

LINKER_OBJECTS_DYN 	= -lSDL2
LINKER_OBJECTS_DYN 	+= -lGLEW
LINKER_OBJECTS_DYN 	+= -lpthread
LINKER_OBJECTS_DYN 	+= -lz
LINKER_OBJECTS_DYN  += -ldl
LINKER_OBJECTS_DYN 	+= -lGL
LINKER_OBJECTS_DYN 	+= -lrt
LINKER_OBJECTS_DYN 	+= -lpng
LINKER_OBJECTS_DYN 	+= -lGLU

all: SCREEN_MAN
main.o: main.cpp
	g++ $(COMPILER_ARTIFACTS) main.cpp -c
	
SCREEN_MAN: $(SOURCE_FILES) Makefile main.o
	$(info   *************** tests make screen manager ***************)
	$(MAKE) -C ../../screen_manager/build
	g++ $(COMPILER_ARTIFACTS) -o SCREEN_MAN main.o $(LINKER_OBJECTS_STATIC) $(LINKER_OBJECTS_DYN)

clean:
	$(info   *************** tests screen manager clean ***************)
	rm -rf *.o SCREEN_MAN
