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

#ifndef CONTROLLER_H
#define CONTROLLER_H

    //supported number of controllers in GUI
    #define MAX_GAMEPADS 2

    //SDL increases the instance for a controller every 
    //time a controller reconnects
    #define MAX_GAMEPADS_PLUGGED 128

    // suppress controller noise
    const int ANALOG_DEAD_ZONE = 2000;

    // controllers detected by SDL 
    // will be assigned a slot
    // (designated controller 0, controller 1)
    typedef struct DesignatedControllers { 
        SDL_Joystick* joy; 
        SDL_GameController* gameCtrl;
        int instance; 
        string name;
        bool mappingOK; 
    } T_DesignatedControllers;
    
    bool initJoy(void);
    bool openJoy(int i);
    bool checkControllerIsSupported(int i);
    bool checkMapping(SDL_JoystickGUID guid, bool* mappingOK,string name);
    bool printJoyInfo(int i);
    bool closeJoy(int instance_id);
    bool closeAllJoy(void);
    bool restoreController(void);

    //Gamepad array for all instances
    extern SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];

    //designated controllers
    extern T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];

#endif
