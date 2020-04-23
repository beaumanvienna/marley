/* Marley Copyright (c) 2020 Marley Development Team 
   https://github.com/beaumanvienna/marley

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <iostream>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

using namespace std;

#ifndef GUI_H
#define GUI_H

    #define NUM_TEXTURES    22
        #define TEX_BACKGROUND          0
        #define TEX_BARREL              1
        #define TEX_PS3                 2
        #define TEX_XBOX360             3
        #define TEX_GENERIC_CTRL        4
        #define TEX_RUDDER              5
        #define TEX_RUDDER_GREY         6
        #define TEX_ICON_PLAY           7
        #define TEX_ICON_PLAY_IN        8
        #define TEX_ICON_SETUP          9
        #define TEX_ICON_SETUP_IN       10
        #define TEX_ICON_OFF            11
        #define TEX_ICON_OFF_IN         12
        #define TEX_ICON_NO_CTRL        13
        #define TEX_ICON_NO_FW_PSX      14
        #define TEX_ICON_NO_GAMES       15
        #define TEX_ICON_GAMES_FLR      16
        #define TEX_ICON_GAMES_FLR_IN   17
        #define TEX_ICON_FW_FLR         18
        #define TEX_ICON_FW_FLR_IN      19
        #define TEX_PS4                 20
        #define TEX_WIIMOTE             21


    #define WINDOW_WIDTH 1280
    #define WINDOW_HEIGHT 750
    
    bool initGUI(void);
    bool loadMedia(void);
    bool closeGUI(void);
    bool freeTextures(void);
    bool renderScreen(void);
    bool setFullscreen(void);
    bool setWindowed(void);
    bool restoreGUI();
    bool createRenderer(void);
    
    SDL_Texture* loadTextureFromFile(string str);
    
    //rendering window 
    extern SDL_Window* gWindow;

    //window renderer
    extern SDL_Renderer* gRenderer;
    
    //textures
    extern SDL_Texture* gTextures[NUM_TEXTURES];
   
    // fullscreen flag
    extern bool gFullscreen;
    extern bool gIgnore;
    
    extern TTF_Font* gFont;
    
#endif
