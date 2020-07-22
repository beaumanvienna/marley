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
    #define MAX_DEVICES_PER_CONTROLLER 1 
    
    #define CTRL_TYPE_STD       0 // ps3, ps4, x-box, ...
    #define CTRL_TYPE_WIIMOTE   1 
    
    #define CTRL_TYPE_STD_DEVICES       1 
    #define CTRL_TYPE_WIIMOTE_DEVICES   1 

    // suppress controller noise
    const int ANALOG_DEAD_ZONE = 2000;
    
    typedef SDL_Joystick* pSDL_Joystick;
    typedef SDL_GameController* pSDL_GameController;
    // controllers detected by SDL 
    // will be assigned a slot
    // (designated controller 0, controller 1)
    typedef struct DesignatedControllers { 
        pSDL_Joystick joy[MAX_DEVICES_PER_CONTROLLER];
        pSDL_GameController gameCtrl[MAX_DEVICES_PER_CONTROLLER];
        int instance[MAX_DEVICES_PER_CONTROLLER];
        int index[MAX_DEVICES_PER_CONTROLLER];
        string name[MAX_DEVICES_PER_CONTROLLER];
        string nameDB[MAX_DEVICES_PER_CONTROLLER];
        bool mappingOKDevice[MAX_DEVICES_PER_CONTROLLER];
        bool mappingOK;
        int controllerType;
        int numberOfDevices;
    } T_DesignatedControllers;
    
    bool initJoy(void);
    bool openJoy(int i);
    bool checkControllerIsSupported(int i);
    bool checkMapping(SDL_JoystickGUID guid, bool* mappingOK,string name);
    bool printJoyInfo(int i);
    bool closeJoy(int instance_id);
    bool closeAllJoy(void);
    void restoreController(void);
    void setMapping(void);
    int checkType(string name, string nameDB);
    void openWiimote(int nb);
    void closeWiimote(int nb);

    //Gamepad array for all instances
    extern SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];

    //designated controllers
    extern T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];
    extern int gNumDesignatedControllers;

#endif
