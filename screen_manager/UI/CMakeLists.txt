add_library(ui
NativeApp.cpp
TextureUtil.cpp
MiscScreens.cpp
OnScreenDisplay.cpp
PauseScreenPCSX2.cpp
)

target_include_directories(ui PUBLIC 
../Common
..
/usr/include/SDL2)
