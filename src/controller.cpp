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
#include <gtk/gtk.h>
#include "../resources/res.h"
#include <unistd.h>

//Gamepad array
SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];

int devicesPerType[] = {CTRL_TYPE_STD_DEVICES,CTRL_TYPE_WIIMOTE_DEVICES};

//designated controllers
T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];
int gNumDesignatedControllers;
string sdl_db, internal_db;
extern bool updateSettingsScreen;
extern std::string showTooltipSettingsScreen;
bool initJoy(void)
{
    printf("jc: bool initJoy(void)\n");
    bool ok = true;
    int i;
    
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    
    internal_db = gBaseDir + "internaldb.txt";

    SDL_GameControllerAddMappingsFromFile(internal_db.c_str());
    
    sdl_db = gBaseDir;
    sdl_db += "gamecontrollerdb.txt";
    if (( access( sdl_db.c_str(), F_OK ) == -1 ))
	{
		//file does not exist
		
		GError *error;
		GFile* out_file = g_file_new_for_path(sdl_db.c_str());
		GFile* src_file = g_file_new_for_uri("resource:///database/gamecontrollerdb.txt");
		g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
    
	}

    if( SDL_GameControllerAddMappingsFromFile(sdl_db.c_str()) == -1 )
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
            gDesignatedControllers[i].index[j] = -1;
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
    printf("jc: bool closeAllJoy(void)\n");
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
    printf("jc: bool closeJoy(int instance_id = %d)\n",instance_id);
    int designation, instance, n, num_controller, ctrlType, devPerType;
    
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
                    gDesignatedControllers[designation].index[j] = -1;
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
    printf("jc: bool printJoyInfo(int i=%d)\n",i);
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
    printf("jc: bool checkControllerIsSupported(int i=%d)\n",i);
    // This function is rough draft.
    // There might be controllers that SDL allows but Marley not
    
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    
    bool ok= false;
    string unsupported = "Nintendo Wii";
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
    printf("jc: bool openJoy(int i=%d)\n",i);
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
                                gDesignatedControllers[designation].index[device] = i;
                                
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
    printf("jc: bool findGuidInFile(string filename=%s, string text2match=%s, int length=%d, string* lineRet)\n",filename.c_str(),text.c_str(),length);
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
    printf("jc: bool checkMapping(SDL_JoystickGUID guid, bool* mappingOK, string name=%s)\n",name.c_str());
    char guidStr[1024];
    string line, append, filename;
    
    mappingOK[0] = false;
    
    //set up guidStr
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    filename = internal_db.c_str();
    if (findGuidInFile(filename.c_str(), guidStr,32,&line))
    {
        printf("GUID found in internal db\n");
        mappingOK[0] = true;
    }
    else
    {

        //check public db
        mappingOK[0] = findGuidInFile(sdl_db.c_str(), guidStr,32,&line);
        
        if (mappingOK[0])
        {
            printf("GUID found in public db\n");
        }
        else
        {
            string lineOriginal;
            printf("GUID %s not found in public db\n", guidStr);
            for (int i=27;i>16;i--)
            {
                
                //search in public db for similar
                mappingOK[0] = findGuidInFile(sdl_db.c_str(),guidStr,i,&line);
                
                if (mappingOK[0])
                {
                    mappingOK[0]=false; // found but loading could fail
                    // initialize controller with this line
                    lineOriginal = line;
                    
                    // mapping string after 2nd comma
                    int pos = line.find(",");
                    append = line.substr(pos+1,line.length()-pos-1);
                    pos = append.find(",");
                    append = append.substr(pos+1,append.length()-pos-1);
                    
                    if (name.length()>45) name = name.substr(0,45);
                    
                    //assemble final entry
                    string entry=guidStr;
                    entry += "," + name + "," + append;
                    
                    if (addControllerToInternalDB(entry)) 
                    {
                        printf("added to internal db: %s\n",entry.c_str());
                        removeDuplicatesInDB();

                        int ret = SDL_GameControllerAddMappingsFromFile(internal_db.c_str());
                        if ( ret == -1 )
                        {
                            printf( "Warning: Unable to open '%s' (should be ~/.marley/internaldb.txt)\n",internal_db.c_str());
                        } else
                        {
                            mappingOK[0]=true; // now actually ok
                            // reset SDL 
                            closeAllJoy();
                            SDL_QuitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER);
                            initJoy();
                        }
                    }
                    
                    break;
                }
            }
            if (mappingOK[0])
            { 
                printf("\n%s: trying to load mapping from closest match\n%s\n",guidStr, lineOriginal.c_str());
            }
        }    
    }
    return mappingOK[0];
}


void restoreController(void)
{
    printf("jc: void restoreController(void)\n");
    SDL_JoystickEventState(SDL_ENABLE);
}


void setMapping(void)
{
    printf("jc: void setMapping(void)\n");

    char guidStr[1024];
    SDL_JoystickGUID guid;
    string name, entry;
    SDL_Joystick *joy;
    
    name = gDesignatedControllers[gControllerConfNum].name[0];
    int pos;
    while ((pos = name.find(",")) != string::npos)
    {
        name = name.erase(pos,1);
    }
    if (name.length() > 45) name = name.substr(0,45);
    joy = gDesignatedControllers[gControllerConfNum].joy[0];
    guid = SDL_JoystickGetGUID(joy);
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    entry = guidStr;
    entry = entry + "," + name;
    
    if (gControllerButton[STATE_CONF_BUTTON_A] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",a:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_A]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_B] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",b:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_B]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_BACK] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",back:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_BACK]);
    }
    
    if (gControllerButton[STATE_CONF_BUTTON_DPAD_DOWN] != STATE_CONF_SKIP_ITEM)
    {
        entry = entry + ",dpdown:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_DOWN]);
    }
    else if ((gHat[1] != -1) && (gHatValue[1] != -1))
    {
        entry = entry + ",dpdown:h" + to_string(gHat[1]) + "." + to_string(gHatValue[1]);
    }
    else if (gAxis[1] != -1)
    {
        if (gAxisValue[1])
        {
            entry = entry + ",dpdown:-a" + to_string(gAxis[1]);
        }
        else
        {
            entry = entry + ",dpdown:+a" + to_string(gAxis[1]);
        }
    }

    if (gControllerButton[STATE_CONF_BUTTON_DPAD_LEFT] != STATE_CONF_SKIP_ITEM)
    {
        entry = entry + ",dpleft:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_LEFT]);
    }
    else if ((gHat[2] != -1) && (gHatValue[2] != -1))
    {
        entry = entry + ",dpleft:h" + to_string(gHat[2]) + "." + to_string(gHatValue[2]);
    }
    else if (gAxis[2] != -1)
    {
        if (gAxisValue[2])
        {
            entry = entry + ",dpleft:-a" + to_string(gAxis[2]);
        }
        else
        {
            entry = entry + ",dpleft:+a" + to_string(gAxis[2]);
        }
    }
    
    if ( gControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT] != STATE_CONF_SKIP_ITEM)
    {
        entry = entry + ",dpright:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT]);
    }
    else if ((gHat[3] != -1) && (gHatValue[3] != -1))
    {
        entry = entry + ",dpright:h" + to_string(gHat[3]) + "." + to_string(gHatValue[3]);
    }
    else if (gAxis[3] != -1)
    {
        if (gAxisValue[3])
        {
            entry = entry + ",dpright:-a" + to_string(gAxis[3]);
        }
        else
        {
            entry = entry + ",dpright:+a" + to_string(gAxis[3]);
        }
    }
    
    if (gControllerButton[STATE_CONF_BUTTON_DPAD_UP] != STATE_CONF_SKIP_ITEM)
    {
        entry = entry + ",dpup:b" + to_string(gControllerButton[STATE_CONF_BUTTON_DPAD_UP]);
    }
    else if ((gHat[0] != -1) && (gHatValue[0] != -1))
    {
        entry = entry + ",dpup:h" + to_string(gHat[0]) + "." + to_string(gHatValue[0]);
    }
    else if (gAxis[0] != -1)
    {
        if (gAxisValue[0])
        {
            entry = entry + ",dpup:-a" + to_string(gAxis[0]);
        }
        else
        {
            entry = entry + ",dpup:+a" + to_string(gAxis[0]);
        }
    }
    
    if (gControllerButton[STATE_CONF_BUTTON_GUIDE] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",guide:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_GUIDE]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_LEFTSHOULDER] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",leftshoulder:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_LEFTSHOULDER]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_LEFTSTICK] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",leftstick:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_LEFTSTICK]);
    }
    
    if (gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",lefttrigger:a";
        entry += to_string(gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER]);
    }    
    if (gControllerButton[STATE_CONF_BUTTON_LEFTTRIGGER] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",lefttrigger:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_LEFTTRIGGER]);
    }
    if (gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",leftx:a";
        entry += to_string(gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_LEFTSTICK] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",lefty:a";
        entry += to_string(gControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]);
    }
    
    
    if (gControllerButton[STATE_CONF_BUTTON_RIGHTSHOULDER] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",rightshoulder:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_RIGHTSHOULDER]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_RIGHTSTICK] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",rightstick:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_RIGHTSTICK]);
    }
    if (gControllerButton[STATE_CONF_AXIS_RIGHTTRIGGER] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",righttrigger:a";
        entry += to_string(gControllerButton[STATE_CONF_AXIS_RIGHTTRIGGER]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_RIGHTTRIGGER] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",righttrigger:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_RIGHTTRIGGER]);
    }
    if (gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",rightx:a";
        entry += to_string(gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]);
    }
    if (gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",righty:a";
        entry += to_string(gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]);
    }
    
    
    if (gControllerButton[STATE_CONF_BUTTON_START] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",start:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_START]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_X] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",x:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_X]);
    }
    if (gControllerButton[STATE_CONF_BUTTON_Y] != STATE_CONF_SKIP_ITEM)
    {
        entry += ",y:b";
        entry += to_string(gControllerButton[STATE_CONF_BUTTON_Y]);
    }
    entry +=  ",platform:Linux,";
    
    if (addControllerToInternalDB(entry)) 
    {
        printf("added to internal db: %s\n",entry.c_str());
    }
    
    removeDuplicatesInDB();

    int ret = SDL_GameControllerAddMappingsFromFile(internal_db.c_str());
    if ( ret == -1 )
    {
        printf( "Warning: Unable to open '%s' (should be ~/.marley/internaldb.txt)\n",internal_db.c_str());
    }
    showTooltipSettingsScreen = "Added as '" + name + "'";
    updateSettingsScreen = true;
    
    // we're done here: reset everything
    resetStatemachine();
    closeAllJoy();
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER);
    initJoy();
}

//this function should only be called on a new controller instance
int checkType(string name, string nameDB)
{
    int ctrlTex = TEX_GENERIC_CTRL;
    
    int str_pos;
    int str_pos2;
    
    //check if SNES
    str_pos  =   name.find("snes");
    str_pos2 = nameDB.find("snes");
    if ( (str_pos>=0) || ((str_pos2>=0)) )
    {
        ctrlTex = TEX_SNES;
    } 

    //check if PS3
    str_pos  =   name.find("sony playstation(r)3");
    str_pos2 = nameDB.find("ps3");
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
    str_pos  =   name.find("wiimote");
    str_pos2 = nameDB.find("wiimote");
    if ( (str_pos>=0) || ((str_pos2>=0)) )
    {
        ctrlTex = TEX_WIIMOTE;
    }
    
    return ctrlTex;
}


void openWiimote(int nb)
{
    printf("jc: void openWiimote(int nb)\n");
    //search for 1st empty slot
    for (int designation=0;designation< MAX_GAMEPADS; designation++)
    {
        
        if (gDesignatedControllers[designation].numberOfDevices == 0)
        {
            
            gDesignatedControllers[designation].numberOfDevices=1;
            gDesignatedControllers[designation].instance[0] = -1;
            gDesignatedControllers[designation].index[0] = -1;
            gDesignatedControllers[designation].name[0] = "wiimote";
            gDesignatedControllers[designation].nameDB[0] = "wiimote";
            gDesignatedControllers[designation].controllerType = CTRL_TYPE_WIIMOTE;
            gDesignatedControllers[designation].joy[0] = NULL;
            gDesignatedControllers[designation].gameCtrl[0] = NULL;
            gDesignatedControllers[designation].mappingOK = true;
            
            gNumDesignatedControllers++;
            
            break;
        }
    }
}
void closeWiimote(int nb)
{
    printf("jc: void closeWiimote(int nb)\n");
    //search for 1st empty slot
    for (int designation=0;designation< MAX_GAMEPADS; designation++)
    {
        if (gDesignatedControllers[designation].controllerType == CTRL_TYPE_WIIMOTE)
        {
            
            gDesignatedControllers[designation].numberOfDevices=0;
            gDesignatedControllers[designation].instance[0] = -1;
            gDesignatedControllers[designation].index[0] = -1;
            gDesignatedControllers[designation].name[0] = "";
            gDesignatedControllers[designation].nameDB[0] = "";
            gDesignatedControllers[designation].controllerType = -1;
            gDesignatedControllers[designation].joy[0] = NULL;
            gDesignatedControllers[designation].gameCtrl[0] = NULL;
            gDesignatedControllers[designation].mappingOK = false;
            
            gNumDesignatedControllers--;
            
            break;
        }
    }
}
