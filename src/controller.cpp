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

//Gamepad array
SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];

//designated controllers
T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];

SDL_JoystickGUID guid;
char guid_str[1024];

SDL_GameController *ctrl;

bool closeJoy(int instance_id)
{
    int designation, designation_instance, n, num_controller;
    printf("closeJoy(int %i)\n",instance_id);
    //Close gamepad
    SDL_JoystickClose( gGamepad[instance_id] );
    if (instance_id < MAX_GAMEPADS_PLUGGED)
    {
        gGamepad[instance_id] = NULL;
    
        for (designation=0;designation<MAX_GAMEPADS;designation++)
        {        
            designation_instance= gDesignatedControllers[designation].instance;
            //printf(" *** designation %i designation_instance %i instance %i\n",designation,designation_instance,instance_id);
            if ((designation_instance == instance_id) && (designation_instance != -1))
            {
                printf("removing designated controller %i (instance %i)\n",designation,gDesignatedControllers[designation].instance);
                gDesignatedControllers[designation].instance = -1;
                gDesignatedControllers[designation].name = "NULL";
                gDesignatedControllers[designation].joy = NULL;
            }
        }
           
        printf("remaining controllers: %i\n",SDL_NumJoysticks());
        for (designation=0;designation<MAX_GAMEPADS;designation++)
        {        
            designation_instance= gDesignatedControllers[designation].instance;
            if (designation_instance != -1)
            {
               printf("remaining: instance %i designated as %i\n",gDesignatedControllers[designation].instance,designation);
            }
        }
    }
    printf("closeJoy() end \n");
    return true;
}

bool printJoyInfo(int i)
{
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    char guid_str[1024];
    const char* name = SDL_JoystickName(joy);
    int num_axes = SDL_JoystickNumAxes(joy);
    int num_buttons = SDL_JoystickNumButtons(joy);
    int num_hats = SDL_JoystickNumHats(joy);
    int num_balls = SDL_JoystickNumBalls(joy);
    char *mapping;
    
    printf("printJoyInfo(int %i)",i);
    printf("  *************************************************************************\n");
    

    
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
    
    SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
    
    
    printf("Name: %s  ", SDL_JoystickNameForIndex(i));
    printf("Number of Axes: %d  ", SDL_JoystickNumAxes(joy));
    printf("Number of Buttons: %d  ", SDL_JoystickNumButtons(joy));
    printf("Number of Balls: %d  ", SDL_JoystickNumBalls(joy));
    printf("%s \"%s\" axes:%d buttons:%d hats:%d balls:%d\n", guid_str, name, num_axes, num_buttons, num_hats, num_balls);

    
    SDL_Log("Index \'%i\' is a compatible gamepad, named \'%s\' mapped as \"%s\".", i, SDL_GameControllerNameForIndex(i),mapping);
    ctrl = SDL_GameControllerOpen(i);
    mapping = SDL_GameControllerMapping(ctrl);
    
    SDL_free(mapping);
    
}
bool checkControllerIsSupported(int i)
{
    // This function is rough draft.
    // There might be controllers that SDL allows but Marley not
    // The Wii Mote is such an example because it is detected with five instances. 
    // For the time being, instances of the Wii Mote will not be designated as marley controllers.
    
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    
    bool ok= false;
    string unsupported = "Nintendo Wii";
    string name = SDL_JoystickName(joy);
    int str_pos = name.find(unsupported);
    
    // check for unsupported
    if (str_pos>=0)
    {
        printf("checkControllerIsSupported: not supported, ignoring controller: %s\n",name.c_str());
        ok=false;
    } 
    else
    {
        printf("checkControllerIsSupported: supported controller: %s  ",name.c_str());
        ok=true;
    }
    return ok;
}

bool openJoy(int i)
{
    int designation, designation_instance;
    printf("openJoy(int %i)   ",i);
    if (i<MAX_GAMEPADS_PLUGGED)
    {
        SDL_Joystick *joy;
        joy = SDL_JoystickOpen(i);
        if (joy) {
            
            //Load gamepad
            gGamepad[i] = joy;
            if( gGamepad[i] == NULL )
            {
                printf( "Warning: Unable to open 1st game gamepad! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                printJoyInfo(i);
                printf("opened %i  ",i);
                printf("active controllers: %i\n",SDL_NumJoysticks());
                if ( checkControllerIsSupported(i))
                {
                    for (designation=0;designation< MAX_GAMEPADS; designation++)
                    {
                        if (gDesignatedControllers[designation].instance == -1)
                        {
                            // instance is what e.jaxis.which returns
                            gDesignatedControllers[designation].instance = SDL_JoystickInstanceID(gGamepad[i]);
                            gDesignatedControllers[designation].name = SDL_JoystickNameForIndex(i);
                            printf("adding designated controller %i (instance %i)\n",designation,gDesignatedControllers[designation].instance);
                            designation = MAX_GAMEPADS;
                        }
                    }
                    
                    for (designation=0;designation<MAX_GAMEPADS;designation++)
                    {        
                        designation_instance= gDesignatedControllers[designation].instance;
                        if (designation_instance != -1)
                        {
                           printf("active: instance %i designated as %i\n",gDesignatedControllers[designation].instance,designation);
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


