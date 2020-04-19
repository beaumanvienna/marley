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


#include <string>
#include <SDL.h>

#include "../include/marley.h"
#include "../include/statemachine.h"
#include "../include/gui.h"
#include "../include/controller.h"
#include "../include/emu.h"

int gState = 0;
int gCurrentGame;
std::vector<string> gGame;
bool gQuit=false;
bool gIgnore = false;
bool gSetupIsRunning=false;
bool gTextInput=false;
bool gTextInputForGamingFolder = false;
bool gTextInputForFirmwareFolder = false;
string gText;
string gTextForGamingFolder;
string gTextForFirmwareFolder;
bool gControllerConf = false;
int gControllerConfNum=-1;
int confState;
string gConfText;
int gControllerButton[STATE_CONF_MAX];
int xCount,yCount,xValue,yValue;
int gHat[4],gHatValue[4];
int hatIterator;


bool checkAxis(int cmd);
bool checkTrigger(int cmd);

bool statemachine(int cmd)
{
    string execute;
    bool emuReturn = false;
    
    if (!gControllerConf)
    {
        switch (cmd)
        {
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: 
            case SDLK_DOWN:
            case SDLK_RIGHT:
                switch (gState)
                {
                    case STATE_ZERO:
                        if (gNumDesignatedControllers)
                        {
                            if (gGamesFound)
                            {
                                gState=STATE_PLAY;
                            }
                            else
                            {
                                gState=STATE_SETUP;
                            }
                        }
                        else
                        {
                            gState=STATE_OFF;
                        }
                        break;
                    case STATE_PLAY:
                        gState=STATE_SETUP;
                        break;
                    case STATE_SETUP:
                        gState=STATE_OFF;
                        break;
                    case STATE_OFF:
                        if (gGamesFound)
                        {
                            gState=STATE_LAUNCH;
                        }
                        else
                        {
                            gState=STATE_SETUP;
                        }
                        break;
                     case STATE_CONF0:
                        if (gDesignatedControllers[1].instance != -1)
                        {
                            gState=STATE_CONF1;
                        } 
                        else 
                        {
                            gState=STATE_FLR_GAMES;
                        }
                        break;
                    case STATE_CONF1:
                        gState=STATE_FLR_GAMES;
                        break;
                    case STATE_FLR_GAMES:
                        gState=STATE_FLR_FW;
                        break;
                    case STATE_LAUNCH:
                        if (gCurrentGame == (gGame.size()-1))
                        {
                            gState=STATE_PLAY;
                        }
                        else
                        {
                            gCurrentGame++;
                        }
                        break;
                    default:
                        (void) 0;
                        break;
                }
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            case SDLK_UP:
            case SDLK_LEFT:
                switch (gState)
                {
                    case STATE_ZERO:
                        if (gNumDesignatedControllers)
                        {
                            if (gGamesFound)
                            {
                                gState=STATE_LAUNCH;;
                            }
                            else
                            {
                                gState=STATE_OFF;
                            }
                        }
                        else
                        {
                            gState=STATE_OFF;
                        }
                        break;
                    case STATE_PLAY:
                        gState=STATE_LAUNCH;
                        break;
                    case STATE_SETUP:
                        if (gGamesFound)
                        {
                            gState=STATE_PLAY;
                        }
                        else
                        {
                            gState=STATE_OFF;
                        }
                        break;
                    case STATE_OFF:
                        if (gNumDesignatedControllers)
                        {
                            gState=STATE_SETUP;
                        }
                        break;
                     case STATE_CONF0:
                        gState=STATE_SETUP;
                        gSetupIsRunning=false;
                        break;
                    case STATE_CONF1:
                        if (gDesignatedControllers[0].instance != -1)
                        {
                            gState=STATE_CONF0;
                        } 
                        else 
                        {
                            gSetupIsRunning=false;
                            gState=STATE_SETUP;
                        }
                        break;
                    case STATE_LAUNCH:
                    
                        if (gCurrentGame == 0)
                        {
                            gState=STATE_OFF;
                        }
                        else
                        {
                            gCurrentGame--;
                        }
                        break;
                    case STATE_FLR_GAMES:
                        if (gDesignatedControllers[1].instance != -1)
                        {
                            gState=STATE_CONF1;
                        } 
                        else if (gDesignatedControllers[0].instance != -1)
                        {
                            gState=STATE_CONF0;
                        }
                        else 
                        {
                            gSetupIsRunning=false;
                            gState=STATE_SETUP;
                        }
                        break;
                    case STATE_FLR_FW:
                            gState=STATE_FLR_GAMES;
                        break;
                    default:
                        (void) 0;
                        break;
                }
                break;
            case SDL_CONTROLLER_BUTTON_A:
                switch (gState)
                {
                    case STATE_ZERO:
                        
                        break;
                    case STATE_PLAY:
                        gState=STATE_LAUNCH;
                        break;
                    case STATE_SETUP:
                        if (gDesignatedControllers[0].instance != -1)
                        {
                            gState=STATE_CONF0;
                        } 
                        else if (gDesignatedControllers[1].instance != -1)
                        {
                            gState=STATE_CONF1;
                        }
                        gSetupIsRunning=true;
                        break;
                    case STATE_OFF:
                        gQuit=true;
                        break;
                     case STATE_CONF0:
                     case STATE_CONF1:
                        for (int i=0;i<STATE_CONF_MAX;i++)
                        {
                            gControllerButton[i]=SDL_CONTROLLER_BUTTON_INVALID;
                        }
                        
                        for (int i = 0; i < 4;i++)
                        {
                            gHat[i] = -1;
                            gHatValue[i] = -1;
                        }
                        hatIterator = 0;
                        
                        gControllerConf=true;
                        if (gState==STATE_CONF0)
                        {
                            gControllerConfNum=0;
                        }
                        else
                        {
                            gControllerConfNum=1;
                        }
                        confState = STATE_CONF_BUTTON_DPAD_UP;
                        gConfText = "press up";
                        break;
                    case STATE_FLR_GAMES:
                        if (!gTextInputForGamingFolder)
                        {
                            gTextInputForGamingFolder=true;
                            gTextInput = true;
                            
                            const char *homedir;
                            string slash;
        
                            if ((homedir = getenv("HOME")) != NULL) 
                            {
                                gText = homedir;
                                
                                slash = gText.substr(gText.length()-1,1);
                                if (slash != "/")
                                {
                                    gText += "/";
                                }
                            }
                            else
                            {
                                gText = "";
                            }
                        }
                        else
                        {
                            if (gTextInput) // input exit with RETURN
                            {
                                if (setPathToGames(gText))
                                {
                                    
                                    string setting = "search_dir_games=";
                                    setting += gText;
                                    addSettingToConfigFile(setting);
                                    
                                    gTextForGamingFolder=gText;
                                    buildGameList();
                                }
                            }
                            else // input exit with ESC
                            {
                                gTextForGamingFolder=gText;
                            }
                            gTextInputForGamingFolder=false;
                            gTextInput = false;
                        }
                        break;
                    case STATE_FLR_FW:
                        if (!gTextInputForFirmwareFolder)
                        {
                            gTextInputForFirmwareFolder=true;
                            gTextInput = true;
                            
                            const char *homedir;
                            string slash;
        
                            if ((homedir = getenv("HOME")) != NULL) 
                            {
                                gText = homedir;
                                
                                slash = gText.substr(gText.length()-1,1);
                                if (slash != "/")
                                {
                                    gText += "/";
                                }
                            }
                            else
                            {
                                gText = "";
                            }
                        }
                        else
                        {
                            if (gTextInput) // input exit with RETURN
                            {
                                if (setPathToFirmware(gText))
                                {
                                    
                                    string setting = "search_dir_firmware_PSX=";
                                    setting += gText;
                                    addSettingToConfigFile(setting);
                                    
                                    gTextForFirmwareFolder=gText;
                                    checkFirmwarePSX();
                                }
                            }
                            else // input exit with ESC
                            {
                                gTextForFirmwareFolder=gText;
                            }
                            gTextInputForFirmwareFolder=false;
                            gTextInput = false;
                        }
                        break;
                    case STATE_LAUNCH:
                        if (gGame[gCurrentGame] != "")
                        {
                            #ifdef MEDNAFEN
                                int argc = 2;
                                
                                string str = "mednafen";
                                int n = str.length(); 
                                char arg1[n + 1]; 
                                strcpy(arg1, str.c_str()); 
                                
                                n = gGame[gCurrentGame].length(); 
                                char arg2[n + 1]; 
                                strcpy(arg2, gGame[gCurrentGame].c_str()); 
                                
                                char *argv[] ={arg1,arg2};
                                printf("arg1: %s arg2: %s \n",arg1,arg2);
                                mednafen_main(argc,argv);
                                gIgnore = true;
                                restoreSDL();
                            #else
                                execute = "mednafen \""+gGame[gCurrentGame]+"\"";
                                emuReturn = system(execute.c_str());
                            #endif
                        } else
                        {
                            printf("no valid ROM found\n");
                        }
                        break;
                    default:
                        (void) 0;
                        break;
                }
                break;
            default:
                (void) 0;
                break;
        }
    }
    else // controller conf
    {
        
    }
    return 0;
}


bool statemachineConf(int cmd)
{
    if ( (gControllerConf) && (confState <= STATE_CONF_BUTTON_DPAD_RIGHT) )
    {
        if (gActiveController == gControllerConfNum)
        {
            gControllerButton[confState]=cmd;
            switch (confState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    confState = STATE_CONF_BUTTON_DPAD_DOWN;
                    gConfText = "press dpad down";
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    confState = STATE_CONF_BUTTON_DPAD_LEFT;
                    gConfText = "press dpad left";
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT: 
                    confState = STATE_CONF_BUTTON_DPAD_RIGHT;
                    gConfText = "press dpad right";
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT: 
                    confState = STATE_CONF_BUTTON_A;
                    gConfText = "press button A (lower)";
                    break;
                case STATE_CONF_BUTTON_A:
                    confState = STATE_CONF_BUTTON_B;
                    gConfText = "press button B (right)";
                    break;
                case STATE_CONF_BUTTON_B:
                    confState = STATE_CONF_BUTTON_X;
                    gConfText = "press button X (left)";
                    break;
                case STATE_CONF_BUTTON_X:
                    confState = STATE_CONF_BUTTON_Y;
                    gConfText = "press button Y (upper)";
                    break;
                case STATE_CONF_BUTTON_Y:
                    confState = STATE_CONF_BUTTON_LEFTSTICK;
                    gConfText = "press left stick button";
                    break;
                case STATE_CONF_BUTTON_LEFTSTICK:
                    confState = STATE_CONF_BUTTON_RIGHTSTICK;
                    gConfText = "press right stick button";
                    break;
                case STATE_CONF_BUTTON_RIGHTSTICK:
                    confState = STATE_CONF_BUTTON_LEFTSHOULDER;
                    gConfText = "press left front shoulder";
                    break;
                case STATE_CONF_BUTTON_LEFTSHOULDER:
                    confState = STATE_CONF_BUTTON_RIGHTSHOULDER;
                    gConfText = "press right front shoulder";
                    break;
                case STATE_CONF_BUTTON_RIGHTSHOULDER:
                    confState = STATE_CONF_BUTTON_BACK;
                    gConfText = "press select/back button";
                    break;
                case STATE_CONF_BUTTON_BACK:
                    confState = STATE_CONF_BUTTON_START;
                    gConfText = "press start button";
                    break;
                case STATE_CONF_BUTTON_START:
                    confState = STATE_CONF_BUTTON_GUIDE;
                    gConfText = "press guide button";
                    break;
                case STATE_CONF_BUTTON_GUIDE:
                    confState = STATE_CONF_AXIS_LEFTSTICK_X;
                    gConfText = "twirl left stick";
                    xCount=0;
                    yCount=0;
                    xValue=-1;
                    yValue=-1;
                    break;
                default:
                    (void) 0;
                    break;
            }
        }
    }
}

bool statemachineConfAxis(int cmd)
{
    if ( (gControllerConf) && (confState >= STATE_CONF_AXIS_LEFTSTICK_X) )
    {
        if (gActiveController == gControllerConfNum)
        {
                
            switch (confState)
            {
                case STATE_CONF_AXIS_LEFTSTICK_X:
                case STATE_CONF_AXIS_LEFTSTICK_Y:
                        
                    if (checkAxis(cmd))
                    {
                        printf("***** x axis: %i, y axis: %i\n",xValue,yValue);
                        xCount=0;
                        yCount=0;
                        xValue=-1;
                        yValue=-1;

                        confState = STATE_CONF_AXIS_RIGHTSTICK_X;
                        gConfText = "twirl right stick";
                    }
                    break;
                case STATE_CONF_AXIS_RIGHTSTICK_X:
                case STATE_CONF_AXIS_RIGHTSTICK_Y:
                    if ( (cmd != gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]) &&\
                            (cmd != gControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]))
                    {
                        
                        if (checkAxis(cmd))
                        {
                            printf("***** x axis: %i, y axis: %i\n",xValue,yValue);
                            xCount=0;
                            yCount=0;
                            xValue=-1;
                            yValue=-1;
                            
                            confState = STATE_CONF_AXIS_LEFTTRIGGER;
                            gConfText = "press left rear shoulder";
                        }
                    }
                    break;
                case STATE_CONF_AXIS_LEFTTRIGGER:
                    if ( (cmd != gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]) &&\
                            (cmd != gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]))
                    {
                        if (checkTrigger(cmd))
                        {
                            printf("***** trigger: %i\n",xValue);
                            xCount=0;
                            xValue=-1;
                            
                            confState = STATE_CONF_AXIS_RIGHTTRIGGER;
                            gConfText = "press right rear shoulder";
                        }
                    }
                    
                    break;
                case STATE_CONF_AXIS_RIGHTTRIGGER:
                    if (cmd != gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER]) 
                    {
                        if (checkTrigger(cmd))
                        {
                            printf("***** trigger: %i\n",xValue);
                            xCount=0;
                            xValue=-1;
                            gConfText = "configuration done";
                            gControllerConf = 0;
                            gControllerConfNum = -1;
                            setMapping();
                        }
                    }
                    break;
                default:
                    (void) 0;
                    break;
            }
        }
    }
}

bool checkAxis(int cmd)
{
    bool ok = false;
    
    if ( (xCount > 20) && (yCount>20) )
    {
        gControllerButton[confState]=xValue;
        gControllerButton[confState+1]=yValue;
        ok = true;
    }
    
    if ( (xValue!=-1) && (yValue!=-1) )
    {
        if (xValue == cmd) xCount++;
        if (yValue == cmd) yCount++;
    }
    
    if ( (xValue!=-1) && (yValue==-1) )
    {
        if (xValue > cmd)
        {
            yValue = xValue;
            xValue = cmd;
        }
        else if (xValue != cmd)
        {
            yValue = cmd;
        }
    }
    
    if ( (xValue==-1) && (yValue==-1) )
    {
        xValue=cmd;
    }   
    
    printf("axis confState: %i, cmd: %i, xValue: %i, yValue: %i, xCount: %i, yCount: %i \n",confState,cmd,xValue,yValue,xCount,yCount); 
    return ok;
}

bool checkTrigger(int cmd)
{
    bool ok = false;
    
    if ( (xCount > 20)  )
    {
        gControllerButton[confState]=xValue;
        ok = true;
    }
    
    if (xValue!=-1)
    {
        if (xValue == cmd) xCount++;
    }
    
    if (xValue==-1)
    {
        xValue=cmd;
    }   
    
    printf("axis confState: %i, cmd: %i, xValue: %i, xCount: %i\n",confState,cmd,xValue,xCount); 
    return ok;
}


bool statemachineConfHat(int hat, int value)
{    
    printf("ConfHat hat: %i, value: %i\n",hat,value);
    
    gHat[hatIterator] = hat;
    gHatValue[hatIterator] = value;
    
    hatIterator++;
    
    if (gActiveController == gControllerConfNum)
    {
        
        switch (confState)
        {
            case STATE_CONF_BUTTON_DPAD_UP:
                confState = STATE_CONF_BUTTON_DPAD_DOWN;
                gConfText = "press dpad down";
                break;
            case STATE_CONF_BUTTON_DPAD_DOWN:
                confState = STATE_CONF_BUTTON_DPAD_LEFT;
                gConfText = "press dpad left";
                break;
            case STATE_CONF_BUTTON_DPAD_LEFT: 
                confState = STATE_CONF_BUTTON_DPAD_RIGHT;
                gConfText = "press dpad right";
                break;
            case STATE_CONF_BUTTON_DPAD_RIGHT: 
                confState = STATE_CONF_BUTTON_A;
                gConfText = "press button A (lower)";
                break;
            default:
                (void) 0;
                break;
        }
    }
}
