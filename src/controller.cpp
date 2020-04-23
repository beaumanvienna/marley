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

#include "../include/controller.h"
#include "../include/gui.h"
#include "../include/statemachine.h"
#include "../include/marley.h"
#include "../include/emu.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

//Gamepad array
SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];

int devicesPerType[] = {CTRL_TYPE_STD_DEVICES,CTRL_TYPE_WIIMOTE_DEVICES};

//designated controllers
T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];
int gNumDesignatedControllers;

bool initJoy(void)
{
    bool ok = true;
    int i;
    
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    string internal = gBaseDir;
    internal += "internaldb.txt";
    
    if ( SDL_GameControllerAddMappingsFromFile(internal.c_str()) == -1 )
    {
    }
    if( SDL_GameControllerAddMappingsFromFile(RESOURCES "gamecontrollerdb.txt") == -1 )
    {
        printf( "Warning: Unable to open gamecontrollerdb.txt\n");
    }
    
    for (i=0; i< MAX_GAMEPADS_PLUGGED; i++)
    {
        gGamepad[i] = NULL;
    }
    
    for (i=0; i< MAX_GAMEPADS; i++)
    {
        for (int j=0; j<MAX_DEVICES_PER_CONTROLLER;j++)
        {
            gDesignatedControllers[i].instance[j] = -1;
            gDesignatedControllers[i].name[j] = "";
            gDesignatedControllers[i].nameDB[j] = "";
            gDesignatedControllers[i].joy[j] = NULL;
            gDesignatedControllers[i].mappingOKDevice[j] = false;
            gDesignatedControllers[i].gameCtrl[j] = NULL;
        }
        gDesignatedControllers[i].mappingOK = false;
        gDesignatedControllers[i].controllerType = -1;
        gDesignatedControllers[i].numberOfDevices = 0;
    }
    gNumDesignatedControllers = 0;
    return ok;
}

bool closeAllJoy(void)
{
    int j = SDL_NumJoysticks();
    string filename;
    
    for (int i=0; i<j; i++)
    {
        closeJoy(i);
    }
    
    // remove temp file
    filename = gBaseDir + "tmpdb.txt";
    remove( filename.c_str() );
    
    return true;
}

bool closeJoy(int instance_id)
{
    int designation, instance, n, num_controller, ctrlType, devPerType;
    printf("closeJoy(int %i)\n",instance_id);
    
    //Close gamepad
    SDL_JoystickClose( gGamepad[instance_id] );
    
    // remove designated controller / from designated controller
    if (instance_id < MAX_GAMEPADS_PLUGGED)
    {
        gGamepad[instance_id] = NULL;
    
        for (designation=0;designation<MAX_GAMEPADS;designation++)
        {   
            ctrlType = gDesignatedControllers[designation].controllerType;
            devPerType = devicesPerType[ctrlType];
            for (int j=0; j < devPerType; j++)
            {
                instance= gDesignatedControllers[designation].instance[j];
                
                if ((instance == instance_id) && (instance != -1))
                {
                    printf("removing from designated controller %i (instance %i)",designation,gDesignatedControllers[designation].instance[j]);
                    gDesignatedControllers[designation].instance[j] = -1;
                    gDesignatedControllers[designation].name[j] = "";
                    gDesignatedControllers[designation].nameDB[j] = "";
                    gDesignatedControllers[designation].joy[j] = NULL;
                    gDesignatedControllers[designation].mappingOKDevice[j] = false;
                    gDesignatedControllers[designation].gameCtrl[j] = NULL;
                    gDesignatedControllers[designation].numberOfDevices--;
                    
                    printf(" number of devices on this controller remaining: %i\n",gDesignatedControllers[designation].numberOfDevices);
                    
                    if (gDesignatedControllers[designation].numberOfDevices == 0) 
                    {
                        gDesignatedControllers[designation].mappingOK = false;
                        gDesignatedControllers[designation].controllerType = -1;
                        gNumDesignatedControllers--;
                    }
                }
            }
        }
           
        printf("remaining devices: %i, remaining designated controllers: %i\n",SDL_NumJoysticks(),gNumDesignatedControllers);
        for (designation=0;designation<MAX_GAMEPADS;designation++)
        {
            for (int j=0;j<devPerType;j++)
            {
                instance= gDesignatedControllers[designation].instance[j];
                if (instance != -1)
                {
                   printf("remaining: instance %i on designated controller %i\n",gDesignatedControllers[designation].instance[j],designation);
                }
            }
        }
        if ( ((gDesignatedControllers[0].numberOfDevices == 0) && (gState == STATE_CONF0)) ||\
                ((gDesignatedControllers[1].numberOfDevices == 0) && (gState == STATE_CONF1)) )
        {
            gState=STATE_ZERO;
            gSetupIsRunning=false;
        }
    }
    return true;
}

bool printJoyInfo(int i)
{
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    char guidStr[1024];
    const char* name = SDL_JoystickName(joy);
    int num_axes = SDL_JoystickNumAxes(joy);
    int num_buttons = SDL_JoystickNumButtons(joy);
    int num_hats = SDL_JoystickNumHats(joy);
    int num_balls = SDL_JoystickNumBalls(joy);
    int instance = SDL_JoystickInstanceID(joy);
    char *mapping;
    SDL_GameController *gameCtrl;
    
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
    
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    printf("Index: %i  ", i);
    printf("Instance: %i  ", instance);
    printf("Name: %s\n", SDL_JoystickNameForIndex(i));
    printf("Number of Axes: %d  ", SDL_JoystickNumAxes(joy));
    printf("Number of Buttons: %d  ", SDL_JoystickNumButtons(joy));
    printf("Number of Balls: %d,  ", SDL_JoystickNumBalls(joy));
    //printf("GUID: %s", guidStr); //printed later
    
    if (SDL_IsGameController(i)) 
    {
        gameCtrl = SDL_GameControllerOpen(i);
        mapping = SDL_GameControllerMapping(gameCtrl);
        if (mapping) 
        {
            printf(" compatible and mapped as\n\n%s\n\n", mapping);
            SDL_free(mapping);
        }
    }
    else 
    {
        printf("\nIndex \'%i\' is not a compatible controller.", i);
    }
    
    return true;
}
bool checkControllerIsSupported(int i)
{
    // This function is rough draft.
    // There might be controllers that SDL allows but Marley not
    
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    
    bool ok= false;
    string unsupported = "to be defined";
    string name = SDL_JoystickName(joy);
    int str_pos = name.find(unsupported);
    
    // check for unsupported
    if (str_pos>=0)
    {
        printf("not supported, ignoring controller: %s\n",name.c_str());
        ok=false;
    } 
    else
    {
        ok=true;
    }
    return ok;
}

bool openJoy(int i)
{
    int designation, instance, devPerType;
    int device, numberOfDevices, ctrlType;
    bool mappingOK;
    char *mapping;
    
    if (i<MAX_GAMEPADS_PLUGGED)
    {
        SDL_Joystick *joy;
        joy = SDL_JoystickOpen(i);
        if (joy) {
            
            //Load gamepad
            gGamepad[i] = joy;
            if( gGamepad[i] == NULL )
            {
                printf( "Warning: Unable to open game gamepad! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                printJoyInfo(i);
                printf("opened %i  ",i);
                printf("active controllers: %i\n",SDL_NumJoysticks());
                if ( checkControllerIsSupported(i))
                {
                    //search for 1st empty slot
                    for (designation=0;designation< MAX_GAMEPADS; designation++)
                    {
                        numberOfDevices = gDesignatedControllers[designation].numberOfDevices;
                        ctrlType = gDesignatedControllers[designation].controllerType;
                        if (numberOfDevices != 0)
                        {
                            devPerType = devicesPerType[ctrlType];
                        }
                        else
                        {
                            devPerType = 0;
                        }
                        
                        if ( (numberOfDevices == 0) || ( (devPerType - numberOfDevices) > 0) )
                        {
                            if (numberOfDevices < MAX_DEVICES_PER_CONTROLLER)
                            {
                                device = gDesignatedControllers[designation].numberOfDevices;
                                gDesignatedControllers[designation].numberOfDevices++;
                                
                                instance = SDL_JoystickInstanceID(gGamepad[i]);
                                gDesignatedControllers[designation].instance[device] = instance;
                                
                                
                                string jName = SDL_JoystickNameForIndex(i);
                                transform(jName.begin(), jName.end(), jName.begin(),
                                    [](unsigned char c){ return tolower(c); });
                                gDesignatedControllers[designation].name[device] = jName;
                                
                                if (jName.find("nintendo wii remote") == 0)
                                {
                                    gDesignatedControllers[designation].controllerType = CTRL_TYPE_WIIMOTE;
                                }
                                else
                                {
                                    gDesignatedControllers[designation].controllerType = CTRL_TYPE_STD;
                                }
                                gDesignatedControllers[designation].joy[device] = joy;
                                gDesignatedControllers[designation].gameCtrl[device] = SDL_GameControllerFromInstanceID(instance);
                                SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
                                checkMapping(guid, &mappingOK,gDesignatedControllers[designation].name[device]);
                                gDesignatedControllers[designation].mappingOK = mappingOK;
                                
                                ctrlType = gDesignatedControllers[designation].controllerType;
                                if (gDesignatedControllers[designation].numberOfDevices == devicesPerType[ctrlType])
                                {
                                    gNumDesignatedControllers++;
                                }
                                
                                mapping = SDL_GameControllerMapping(gDesignatedControllers[designation].gameCtrl[device]);
                                if (mapping) 
                                {
                                    string str = mapping;
                                    SDL_free(mapping);
                                    //remove guid
                                    str = str.substr(str.find(",")+1,str.length()-(str.find(",")+1));
                                    // extract name from db
                                    str = str.substr(0,str.find(","));
                                    
                                    transform(str.begin(), str.end(), str.begin(),
                                        [](unsigned char c){ return tolower(c); });
                                    
                                    gDesignatedControllers[designation].nameDB[device] = str;
                                }
                                
                                printf("adding to designated controller %i (instance %i)\n",designation,gDesignatedControllers[designation].instance[device]);
                                //cancel loop
                                designation = MAX_GAMEPADS;
                            }
                        }
                    }
                    //print all designated controllers to terminal
                    for (designation=0;designation<MAX_GAMEPADS;designation++)
                    {        
                        for (int j=0;j < gDesignatedControllers[designation].numberOfDevices;j++)
                        {
                            instance= gDesignatedControllers[designation].instance[j];
                            if (instance != -1)
                            {
                               printf("active: instance %i on designated controller %i\n",gDesignatedControllers[designation].instance[j],designation);
                            }
                        }
                    }
                }
            }
            
        } else {
            printf("Couldn't open Joystick %i\n",i);
        }

        // Close if opened
        if (SDL_JoystickGetAttached(joy)) {
            SDL_JoystickClose(joy);
        }            
        return true;
    } else
    {   
        return false;
    }
}



bool findGuidInFile(string filename, string text2match, int length, string* lineRet)
{
    const char* file = filename.c_str();
    bool ok = false;
    string line;
    string text = text2match.substr(0,length);
    
    lineRet[0] = "";
    
    ifstream fileHandle (file);
    if (!fileHandle.is_open())
    {
        printf("Could not open file: findGuidInFile(%s,%s,%i)\n",filename.c_str(),text2match.c_str(),length);
    }
    else 
    {
        while ( getline (fileHandle,line) && !ok)
        {
            if (line.find(text.c_str()) == 0)
            {
                //printf("found!\n");
                ok = true;
                lineRet[0]=line;
            }
        }
        fileHandle.close();
    }
   
    return ok;
}


bool checkMapping(SDL_JoystickGUID guid, bool* mappingOK, string name)
{
    char guidStr[1024];
    string line, append, filename;
    
    mappingOK[0] = false;
    
    //set up guidStr
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    filename = gBaseDir + "internaldb.txt";
    if (findGuidInFile(filename.c_str(), guidStr,32,&line))
    {
        printf("GUID found in internal db\n");
        mappingOK[0] = true;
    }
    else
    {

        //check public db
        mappingOK[0] = findGuidInFile(RESOURCES "gamecontrollerdb.txt", guidStr,32,&line);
        
        if (mappingOK[0])
        {
            printf("GUID found in public db\n");
        }
        else
        {
            string lineOriginal;
            printf("GUID %s not found in public db", guidStr);
            for (int i=27;i>18;i--)
            {
                
                //check in public db
                mappingOK[0] = findGuidInFile(RESOURCES "gamecontrollerdb.txt",guidStr,i,&line);
                
                if (mappingOK[0])
                {
                    // initialize controller with this line
                    lineOriginal = line;
                    int pos = line.find(",");
                    append = line.substr(pos+1,line.length()-pos-1);
                    
                    pos = append.find(",");
                    append = append.substr(pos+1,append.length()-pos-1);
                    
                    line = guidStr;
                    append=line+","+name+","+append;
                    
                    
                    std::ofstream outfile;
                    filename = gBaseDir + "tmpdb.txt";

                    outfile.open(filename.c_str(), std::ios_base::app); 
                    if(outfile) 
                    {
                        outfile << append + "\n"; 
                        outfile.close();
                        SDL_GameControllerAddMappingsFromFile(filename.c_str());
                        mappingOK[0]=true;
                    }
                    
                    break;
                }
            }
            if (mappingOK[0]) printf("\n%s: trying to load mapping from closest match\n%s\n",guidStr, lineOriginal.c_str());
            printf("\n");
        }    
    }
    return mappingOK[0];
}


bool restoreController(void)
{
    SDL_JoystickEventState(SDL_ENABLE);
}


void setMapping(void)
{
    
/*
    char guidStr[1024];
    SDL_JoystickGUID guid;
    string name, entry;
    SDL_Joystick *joy;
    
    name = gDesignatedControllers[gActiveController].name;
    joy = gDesignatedControllers[gActiveController].joy;
    guid = SDL_JoystickGetGUID(joy);
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    //printf("\n\n");
    //printf("%s,%s,",guidStr,name.c_str());
    entry = guidStr;
    entry = entry + "," + name + ",";
    
    //printf("a:b%i,b:b%i,back:b%i,",gControllerButton[STATE_CONF_BUTTON_A],gControllerButton[STATE_CONF_BUTTON_B],\
        gControllerButton[STATE_CONF_BUTTON_BACK]);
    entry += "a:b";
    entry += to_string(gControllerButton[STATE_CONF_BUTTON_A]);
    entry += ",b:b";
    entry += to_string(gControllerButton[STATE_CONF_BUTTON_B]);
    entry += ",back:b";
    entry += to_string(gControllerButton[STATE_CONF_BUTTON_BACK]);
    entry += ",";
    
    if (gControllerButton[STATE_CONF_BUTTON_DPAD_DOWN] != -1)
    {
        //printf("dpdown:b%i,",gControllerButton[STATE_CONF_BUTTON_DPAD_DOWN]);
        entry = entry + "dpdown:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_DOWN]) + ",";
    }
    else
    {
        //printf("dpdown:h%i.%i,",gHat[1],gHatValue[1]);
        entry = entry + "dpdown:h" + to_string(gHat[1]) + "." + to_string(gHatValue[1]) + ",";
    }
    
    if (gControllerButton[STATE_CONF_BUTTON_DPAD_LEFT] != -1)
    {
        //printf("dpleft:b%i,",gControllerButton[STATE_CONF_BUTTON_DPAD_LEFT]);
        entry = entry + "dpleft:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_LEFT]) + ",";
    }
    else
    {
        //printf("dpleft:h%i.%i,",gHat[2],gHatValue[2]);
        entry = entry + "dpleft:h" + to_string(gHat[2]) + "." + to_string(gHatValue[2]) + ",";
    }
    
    if ( gControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT] != -1)
    {
        //printf("dpright:b%i,",gControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT]);
        entry = entry + "dpright:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT]) + ",";
    }
    else
    {
        //printf("dpright:h%i.%i,",gHat[3],gHatValue[3]);
        entry = entry + "dpright:h" + to_string(gHat[3]) + "." + to_string(gHatValue[3]) + ",";
    }
    
    if (gControllerButton[STATE_CONF_BUTTON_DPAD_UP] != -1)
    {
        //printf("dpup:b%i,",gControllerButton[STATE_CONF_BUTTON_DPAD_UP]);
        entry = entry + "dpup:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_UP]) + ",";
    }
    else
    {
        //printf("dpup:h%i.%i,",gHat[0],gHatValue[0]);
        entry = entry + "dpup:h" + to_string(gHat[0]) + "." + to_string(gHatValue[0]) + ",";
    }
    
    //printf("guide:b%i,leftshoulder:b%i,leftstick:b%i,",gControllerButton[STATE_CONF_BUTTON_GUIDE],\
        gControllerButton[STATE_CONF_BUTTON_LEFTSHOULDER],gControllerButton[STATE_CONF_BUTTON_LEFTSTICK]);
    entry = entry + "guide:b" + to_string(gControllerButton[STATE_CONF_BUTTON_GUIDE]) +\
            ",leftshoulder:b" + to_string(gControllerButton[STATE_CONF_BUTTON_LEFTSHOULDER]) +\
            ",leftstick:b" + to_string(gControllerButton[STATE_CONF_BUTTON_LEFTSTICK]) + ",";
    
    //printf("lefttrigger:a%i,leftx:a%i,lefty:a%i,",gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER],\
        gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X],gControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]);
    entry = entry + "lefttrigger:a" + to_string(gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER]) +\
            ",leftx:a" + to_string(gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]) +\
            ",lefty:a" + to_string(gControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]) + ",";
    
    //printf("rightshoulder:b%i,rightstick:b%i,",\
        gControllerButton[STATE_CONF_BUTTON_RIGHTSHOULDER],gControllerButton[STATE_CONF_BUTTON_RIGHTSTICK]);
    entry = entry + "rightshoulder:b" + to_string(gControllerButton[STATE_CONF_BUTTON_RIGHTSHOULDER]) +\
            ",rightstick:b" + to_string(gControllerButton[STATE_CONF_BUTTON_RIGHTSTICK]) + ",";
    
    //printf("righttrigger:a%i,rightx:a%i,righty:a%i,",gControllerButton[STATE_CONF_AXIS_RIGHTTRIGGER],\
        gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X],gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]);
    entry = entry + "righttrigger:a" + to_string(gControllerButton[STATE_CONF_AXIS_RIGHTTRIGGER]) +\
        ",rightx:a" + to_string(gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]) +\
        ",righty:a" + to_string(gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]) + ",";
    
    //printf("start:b%i,x:b%i,y:b%i,platform:Linux,",gControllerButton[STATE_CONF_BUTTON_START],\
        gControllerButton[STATE_CONF_BUTTON_X],gControllerButton[STATE_CONF_BUTTON_Y]);
    entry = entry + "start:b" + to_string(gControllerButton[STATE_CONF_BUTTON_START]) +\
        ",x:b" + to_string(gControllerButton[STATE_CONF_BUTTON_X]) +\
        ",y:b" + to_string(gControllerButton[STATE_CONF_BUTTON_Y]) + ",platform:Linux,";
    
    //printf("\n\n");
    
    if (addControllerToInternalDB(entry)) 
    {
        printf("added to internal db: %s\n",entry.c_str());
    }
    
    removeDuplicatesInDB();
    
    string internal = gBaseDir;
    internal += "internaldb.txt";
    if ( SDL_GameControllerAddMappingsFromFile(internal.c_str()) == -1 )
    {
        printf( "Warning: Unable to open internaldb.txt\n");
    }
*/
}

int checkType(string name, string nameDB)
{
    int ctrlTex = TEX_GENERIC_CTRL;

    //check if PS3
    int str_pos  =   name.find("sony playstation(r)3");
    int str_pos2 = nameDB.find("ps3");
    if ( (str_pos>=0) || ((str_pos2>=0)) )
    {
        ctrlTex = TEX_PS3;
    } 
    
    //check if XBOX
    str_pos  =   name.find("box");
    str_pos2 = nameDB.find("box");
    if ( (str_pos>=0) || ((str_pos2>=0)) )
    {
        ctrlTex = TEX_XBOX360;
    }
    
    //check if PS4
    str_pos  =   name.find("ps4");
    str_pos2 = nameDB.find("ps4");
    if ( (str_pos>=0) || ((str_pos2>=0)) )
    {
        ctrlTex = TEX_PS4;
    }
    
    //check if Wiimote
    str_pos  =   name.find("nintendo wii");
    str_pos2 = nameDB.find("nintendo wii");
    if ( (str_pos>=0) || ((str_pos2>=0)) )
    {
        ctrlTex = TEX_WIIMOTE;
    }
    
    return ctrlTex;
}
