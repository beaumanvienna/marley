add_library(sdl
SDLMain.cpp
SDLGLGraphicsContext.cpp
SDLJoystick.cpp
)

target_include_directories(sdl PUBLIC 
../Common
..
/usr/include/SDL2)


