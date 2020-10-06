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

#include <vector>
using namespace std;

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

    #define STATE_ZERO      0
    #define STATE_CONF0     1
    #define STATE_CONF1     2
    #define STATE_PLAY      3
    #define STATE_SETUP     4
    #define STATE_OFF       5
    #define STATE_LAUNCH    6
    #define STATE_FLR_GAMES 7
    #define STATE_FLR_FW    8
    #define STATE_SHUTDOWN  9
    
    typedef enum
    {
        STATE_CONF_BUTTON_DPAD_UP=0,
        STATE_CONF_BUTTON_DPAD_DOWN,
        STATE_CONF_BUTTON_DPAD_LEFT,
        STATE_CONF_BUTTON_DPAD_RIGHT,
        STATE_CONF_BUTTON_A,
        STATE_CONF_BUTTON_B,
        STATE_CONF_BUTTON_X,
        STATE_CONF_BUTTON_Y,
        STATE_CONF_BUTTON_BACK,
        STATE_CONF_BUTTON_GUIDE,
        STATE_CONF_BUTTON_START,
        STATE_CONF_BUTTON_LEFTSTICK,
        STATE_CONF_BUTTON_RIGHTSTICK,
        STATE_CONF_BUTTON_LEFTSHOULDER,
        STATE_CONF_BUTTON_RIGHTSHOULDER,
        STATE_CONF_AXIS_LEFTSTICK_X,
        STATE_CONF_AXIS_LEFTSTICK_Y,
        STATE_CONF_AXIS_RIGHTSTICK_X,
        STATE_CONF_AXIS_RIGHTSTICK_Y,
        STATE_CONF_AXIS_LEFTTRIGGER,
        STATE_CONF_AXIS_RIGHTTRIGGER,
        STATE_CONF_SKIP_ITEM,
        STATE_CONF_MAX
    } configStates;

    void statemachine(int cmd);
    void statemachineConf(int cmd);
    void statemachineConfAxis(int cmd, bool negative);
    void statemachineConfHat(int hat, int value);
    void resetStatemachine(void);
    
     // statemachine
    extern int gState;
    
    extern int gCurrentGame;
    extern std::vector<string> gGame;
    
    extern bool gQuit;
    extern bool gSetupIsRunning;
    extern bool gTextInput;
    extern string gText;
    extern bool gTextInputForGamingFolder;
    extern string gTextForGamingFolder;
    extern bool gTextInputForFirmwareFolder;
    extern string gTextForFirmwareFolder;
    extern int gControllerButton[STATE_CONF_MAX];
    extern int gHat[4],gHatValue[4];
    extern int gAxis[4];
    extern bool gAxisValue[4];
    
    extern bool gControllerConf;
    extern int gControllerConfNum;
    extern string gConfText;
    
#endif
