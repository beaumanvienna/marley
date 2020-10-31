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

using namespace std;

#ifndef MARLEY_H
#define MARLEY_H

    #define MEDNAFEN    1
    #define DOLPHIN     1
    #define MUPEN64PLUS 1
    #define PPSSPP      1
    #define PCSX2       1
    
    #ifdef MEDNAFEN
        int mednafen_main(int argc, char* argv[]);
    #endif
    
    #ifdef DOLPHIN
        int dolphin_main(int argc, char* argv[]);
    #endif
    
    #ifdef MUPEN64PLUS
        extern "C" int mupen64plus_main(int argc, char* argv[]);
    #endif
    
    #ifdef PPSSPP
        int ppsspp_main(int argc, char* argv[]);
    #endif
    
    #ifdef PCSX2
        int pcsx2_main(int argc, char* argv[]);
    #endif
    
    void restoreSDL(void);
    bool setPathToGames(string filename);
    bool setPathToFirmware(string str);
    bool addSettingToConfigFile(string setting);
    bool addControllerToInternalDB(string entry);
    void removeDuplicatesInDB(void);
    
    extern double angle0L;
    extern double angle1L;
    extern double angle0R;
    extern double angle1R;
    extern double amplitude0L;
    extern double amplitude1L;
    extern double amplitude0R;
    extern double amplitude1R;
    
    extern int gActiveController;


#endif
