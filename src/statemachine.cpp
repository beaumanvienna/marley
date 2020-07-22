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
#include <algorithm>
#include <X11/Xlib.h>
#include <SDL_syswm.h>

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
int gAxis[4];
bool gAxisValue[4];
int axisIterator;
int secondRun;
int secondRunHat;
int secondRunValue;

extern Display* XDisplay;
extern Window Xwindow;

bool checkAxis(int cmd);
bool checkTrigger(int cmd);

void resetStatemachine(void)
{
    gState=STATE_OFF;
    gSetupIsRunning=false;
    gTextInput=false;
    gControllerConf = false;
    gControllerConfNum=-1;
}

void statemachine(int cmd)
{
    string execute;
    bool emuReturn = false;
    
    if (!gControllerConf)
    {
        switch (cmd)
        {
            case SDL_CONTROLLER_BUTTON_GUIDE:
                if (gState == STATE_OFF)
                {
                    gQuit=true;
                }
                else
                {
                    resetStatemachine();
                }
                break;
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
                        if (gDesignatedControllers[1].numberOfDevices != 0)
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
                                gState=STATE_LAUNCH;
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
                        if (gDesignatedControllers[0].numberOfDevices != 0)
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
                        if (gDesignatedControllers[1].numberOfDevices != 0)
                        {
                            gState=STATE_CONF1;
                        } 
                        else if (gDesignatedControllers[0].numberOfDevices != 0)
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
                        if (gDesignatedControllers[0].numberOfDevices != 0)
                        {
                            gState=STATE_CONF0;
                        } 
                        else if (gDesignatedControllers[1].numberOfDevices != 0)
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
                            gControllerButton[i]=STATE_CONF_SKIP_ITEM;
                        }
                        
                        for (int i = 0; i < 4;i++)
                        {
                            gHat[i] = -1;
                            gHatValue[i] = -1;
                            gAxis[i] = -1;
                            gAxisValue[i] = false;
                        }
                        hatIterator = 0;
                        axisIterator = 0;
                        secondRun = -1;
                        secondRunHat = -1;
                        secondRunValue = -1;
                        
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
                        gConfText = "press dpad up";
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
                            int argc;
                            char *argv[10]; 
                            
                            char arg1[1024]; 
                            char arg2[1024];
                            char arg3[1024]; 
                            char arg4[1024]; 
                            char arg5[1024]; 
                            char arg6[1024]; 
                            char arg7[1024]; 
                            char arg8[1024]; 
                            char arg9[1024]; 
                            char arg10[1024]; 
                            
                            int n;
                            string str, ext;
                            
                            argc = 2;
                            str = gGame[gCurrentGame];
                            ext = str.substr(str.find_last_of(".") + 1);
                            std::transform(ext.begin(), ext.end(), ext.begin(),
                                [](unsigned char c){ return std::tolower(c); });
                            std::transform(str.begin(), str.end(), str.begin(),
                                [](unsigned char c){ return std::tolower(c); });

#ifdef PCSX2
                            
                            if ((ext == "iso") && (str.find("ps2") != string::npos))
                            {
                                str = "pcsx2";
                                n = str.length(); 
                                strcpy(arg1, str.c_str()); 
                                
                                str = "--spu2=/usr/games/Marley/PCSX2/libspu2x-2.0.0.so";
                                n = str.length(); 
                                strcpy(arg2, str.c_str()); 
                                
                                str = "--cdvd=/usr/games/Marley/PCSX2/libCDVDnull.so";
                                n = str.length(); 
                                strcpy(arg3, str.c_str()); 

                                str = "--usb=/usr/games/Marley/PCSX2/libUSBnull-0.7.0.so";
                                n = str.length(); 
                                strcpy(arg4, str.c_str()); 

                                str = "--fw=/usr/games/Marley/PCSX2/libFWnull-0.7.0.so";
                                n = str.length(); 
                                strcpy(arg5, str.c_str()); 

                                str = "--dev9=/usr/games/Marley/PCSX2/libdev9null-0.5.0.so";
                                n = str.length(); 
                                strcpy(arg6, str.c_str()); 
                                
                                str = "--fullboot";
                                n = str.length(); 
                                strcpy(arg7, str.c_str()); 

                                str = gGame[gCurrentGame];
                                n = str.length(); 
                                strcpy(arg8, str.c_str());
                                
                                str = "--nogui";
                                n = str.length(); 
                                strcpy(arg9, str.c_str());
                                
                                /*str = "--fullscreen";
                                n = str.length(); 
                                strcpy(arg10, str.c_str()); */

                                argv[0] = arg1;
                                argv[1] = arg2;
                                argv[2] = arg3;
                                argv[3] = arg4;
                                argv[4] = arg5;
                                argv[5] = arg6;
                                argv[6] = arg7;
                                argv[7] = arg8;
                                argv[8] = arg9;
                                //argv[9] = arg10;
                                
                                //argc = 10; // fullscreen
                                argc = 9;
                                
                                SDL_SysWMinfo sdlWindowInfo;
                                SDL_VERSION(&sdlWindowInfo.version);
                                if(SDL_GetWindowWMInfo(gWindow, &sdlWindowInfo))
                                {
                                    if(sdlWindowInfo.subsystem == SDL_SYSWM_X11) 
                                    {
                                        Xwindow      = sdlWindowInfo.info.x11.window;
                                        XDisplay     = sdlWindowInfo.info.x11.display;
                                        pcsx2_main(argc,argv);   
                                        
                                        if (gWindow)
                                        {
                                            SDL_GL_ResetAttributes();
      
                                            SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
                                            SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
                                            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

                                            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
                                            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
                                            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
                                            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
                                            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
                                            SDL_GL_SetAttribute(SDL_GL_STEREO, 0);                                        
                                            
                                            SDL_GL_MakeCurrent(gWindow,SDL_GL_CreateContext(gWindow));

                                            restoreSDL();

                                        } 
                                        else
                                        {
                                            printf("gWindow == NULL, aborting\n");
                                            exit (0);
                                        }
                                        
                                    }
                                } 
                                else
                                {
                                    printf("jc SDL_GetWindowWMInfo(gWindow, &sdlWindowInfo) failed\n");
                                }
                            }
#endif


#ifdef MUPEN64PLUS
                            
                            if (ext == "z64")
                            {
                                
                                
                                str = "mupen64plus";
                                n = str.length(); 
                                strcpy(arg1, str.c_str()); 
                                
                                n = gGame[gCurrentGame].length(); 
                                strcpy(arg2, gGame[gCurrentGame].c_str()); 
                                
                                argv[0] = arg1;
                                argv[1] = arg2;
                                printf("arg1: %s arg2: %s \n",arg1,arg2);
                                mupen64plus_main(argc,argv);
                                restoreSDL();
                            }
#endif

#ifdef PPSSPP
                            
                            if ((ext == "iso") && (str.find("psp") != string::npos))
                            {
                                
                                
                                str = "ppsspp";
                                n = str.length(); 
                                strcpy(arg1, str.c_str()); 
                                
                                n = gGame[gCurrentGame].length(); 
                                strcpy(arg2, gGame[gCurrentGame].c_str()); 
                                
                                argv[0] = arg1;
                                argv[1] = arg2;
                                printf("arg1: %s arg2: %s \n",arg1,arg2);
                                ppsspp_main(argc,argv);
                                
                                restoreSDL();
                            }
#endif

#ifdef DOLPHIN
                            
                            if ((ext == "iso") && ((str.find("wii") != string::npos)||(str.find("gamecube") != string::npos)))
                            {
                                
                                
                                str = "dolphin-emu";
                                n = str.length(); 
                                strcpy(arg1, str.c_str()); 
                                
                                n = gGame[gCurrentGame].length(); 
                                strcpy(arg2, gGame[gCurrentGame].c_str()); 
                                
                                argv[0] = arg1;
                                argv[1] = arg2;
                                printf("arg1: %s arg2: %s \n",arg1,arg2);
                                dolphin_main(argc,argv);
                                gIgnore = true;
                                restoreSDL();
                            }
#endif
                            
                            
#ifdef MEDNAFEN
                            if ((ext != "iso") && (ext != "z64"))
                            {
                                str = "mednafen";
                                n = str.length(); 
                                strcpy(arg1, str.c_str()); 
                                
                                n = gGame[gCurrentGame].length(); 
                                strcpy(arg2, gGame[gCurrentGame].c_str()); 
                                
                                argv[0] = arg1;
                                argv[1] = arg2;
                                printf("arg1: %s arg2: %s \n",arg1,arg2);
                                mednafen_main(argc,argv);
                                gIgnore = true;
                                restoreSDL();
                            }
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
}


void statemachineConf(int cmd)
{
    if ((cmd==STATE_CONF_SKIP_ITEM) && (confState > STATE_CONF_BUTTON_RIGHTSHOULDER)) statemachineConfAxis(STATE_CONF_SKIP_ITEM,false);
    if ( (gControllerConf) && (confState <= STATE_CONF_BUTTON_RIGHTSHOULDER) )
    {
        if ((gActiveController == gControllerConfNum) || (cmd==STATE_CONF_SKIP_ITEM))
        {
            switch (confState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    if (secondRun == -1)
                    {
                        gConfText = "press dpad up";
                        secondRun = cmd;
                    }
                    else if (secondRun == cmd)
                    {
                        gControllerButton[confState]=cmd;
                        confState = STATE_CONF_BUTTON_DPAD_DOWN;
                        gConfText = "press dpad down";
                        secondRun = -1;
                    }
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_DPAD_LEFT;
                    gConfText = "press dpad left";
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_DPAD_RIGHT;
                    gConfText = "press dpad right";
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_A;
                    gConfText = "press button A (lower)";
                    break;
                case STATE_CONF_BUTTON_A:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_B;
                    gConfText = "press button B (right)";
                    break;
                case STATE_CONF_BUTTON_B:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_X;
                    gConfText = "press button X (left)";
                    break;
                case STATE_CONF_BUTTON_X:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_Y;
                    gConfText = "press button Y (upper)";
                    break;
                case STATE_CONF_BUTTON_Y:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_LEFTSTICK;
                    gConfText = "press left stick button";
                    break;
                case STATE_CONF_BUTTON_LEFTSTICK:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_RIGHTSTICK;
                    gConfText = "press right stick button";
                    break;
                case STATE_CONF_BUTTON_RIGHTSTICK:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_LEFTSHOULDER;
                    gConfText = "press left front shoulder";
                    break;
                case STATE_CONF_BUTTON_LEFTSHOULDER:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_RIGHTSHOULDER;
                    gConfText = "press right front shoulder";
                    break;
                case STATE_CONF_BUTTON_RIGHTSHOULDER:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_BACK;
                    gConfText = "press select/back button";
                    break;
                case STATE_CONF_BUTTON_BACK:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_START;
                    gConfText = "press start button";
                    break;
                case STATE_CONF_BUTTON_START:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_GUIDE;
                    gConfText = "press guide button";
                    break;
                case STATE_CONF_BUTTON_GUIDE:
                    gControllerButton[confState]=cmd;
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

void statemachineConfAxis(int cmd, bool negative)
{
    if ( (gControllerConf) && (confState >= STATE_CONF_AXIS_LEFTSTICK_X) )
    {
        if ((gActiveController == gControllerConfNum)  || (cmd==STATE_CONF_SKIP_ITEM))
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
                    if (cmd == STATE_CONF_SKIP_ITEM)
                    {
                        xCount=0;
                        yCount=0;
                        xValue=-1;
                        yValue=-1;
                        
                        confState = STATE_CONF_AXIS_LEFTTRIGGER;
                        gConfText = "press left rear shoulder";
                    }
                    else if ( (cmd != gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]) &&\
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
                    if (cmd == STATE_CONF_SKIP_ITEM)
                    {
                        xCount=0;
                        xValue=-1;
                          
                        confState = STATE_CONF_AXIS_RIGHTTRIGGER;
                        gConfText = "press right rear shoulder";
                    }
                    else if ( (cmd != gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]) &&\
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
                    if (cmd == STATE_CONF_SKIP_ITEM)
                    {
                        xCount=0;
                        xValue=-1;
                          
                        gConfText = "configuration done";
                        setMapping();
                    }
                    else if (cmd != gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER]) 
                    {
                        if (checkTrigger(cmd))
                        {
                            printf("***** trigger: %i\n",xValue);
                            xCount=0;
                            xValue=-1;
                            gConfText = "configuration done";
                            setMapping();
                        }
                    }
                    break;
                default:
                    (void) 0;
                    break;
            }
        }
    } else if ( (gControllerConf) && (confState <= STATE_CONF_BUTTON_DPAD_RIGHT) )
    {
        if ((gActiveController == gControllerConfNum)  || (cmd==STATE_CONF_SKIP_ITEM))
        {
            gAxis[axisIterator] = cmd;
            gAxisValue[axisIterator] = negative;
            switch (confState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    gConfText = "press dpad down";
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    gConfText = "press dpad left";
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT:
                    gConfText = "press dpad right";
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT:
                    gConfText = "press button A (lower)";
                    break;
                default:
                    (void) 0;
                    break;
            }
            confState++;
            axisIterator++;
        }
    }
}

bool checkAxis(int cmd)
{
    if (cmd==STATE_CONF_SKIP_ITEM) return true;
    
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
    if (cmd==STATE_CONF_SKIP_ITEM) return true;
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


void statemachineConfHat(int hat, int value)
{    
    printf("ConfHat hat: %i, value: %i\n",hat,value);
    
    gHat[hatIterator] = hat;
    gHatValue[hatIterator] = value;
    
    if (gActiveController == gControllerConfNum)
    {
        
        switch (confState)
        {
            case STATE_CONF_BUTTON_DPAD_UP:            
                if ( (secondRunHat == -1) && (secondRunValue == -1) )
                {
                    gConfText = "press dpad up again";
                    secondRunHat = hat;
                    secondRunValue = value;
                }
                else if ( (secondRunHat == hat) && (secondRunValue == value) )
                {
                    hatIterator++;
                    confState = STATE_CONF_BUTTON_DPAD_DOWN;
                    gConfText = "press dpad down";
                    secondRunHat = -1;
                    secondRunValue = -1;
                }
                break;
            case STATE_CONF_BUTTON_DPAD_DOWN:
                confState = STATE_CONF_BUTTON_DPAD_LEFT;
                gConfText = "press dpad left";
                hatIterator++;
                break;
            case STATE_CONF_BUTTON_DPAD_LEFT: 
                confState = STATE_CONF_BUTTON_DPAD_RIGHT;
                gConfText = "press dpad right";
                hatIterator++;
                break;
            case STATE_CONF_BUTTON_DPAD_RIGHT: 
                confState = STATE_CONF_BUTTON_A;
                gConfText = "press button A (lower)";
                hatIterator++;
                break;
            default:
                (void) 0;
                break;
        }
    }
}
