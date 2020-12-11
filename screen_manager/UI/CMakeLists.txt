add_library(ui
NativeApp.cpp
TextureUtil.cpp
MiscScreens.cpp
OnScreenDisplay.cpp
SettingsScreen.cpp
#MainScreen.cpp
)

target_include_directories(ui PUBLIC 
../Common
..
/usr/include/SDL2)
