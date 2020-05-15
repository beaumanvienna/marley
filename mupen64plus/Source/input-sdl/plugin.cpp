/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-input-sdl - plugin.c                                      *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2008-2011 Richard Goedeken                              *
 *   Copyright (C) 2008 Tillin9                                            *
 *   Copyright (C) 2002 Blight                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "config.h"
#include "m64p_common.h"
#include "m64p_config.h"
#include "m64p_plugin.h"
#include "m64p_types.h"
#include "plugin.h"
#include "version.h"

#ifdef __linux__
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/input.h>
#endif /* __linux__ */

#include <errno.h>
#include <string>
//supported number of controllers in GUI
#define MAX_GAMEPADS 2
#define MAX_DEVICES_PER_CONTROLLER 1 

typedef SDL_Joystick* pSDL_Joystick;
typedef SDL_GameController* pSDL_GameController;

typedef struct DesignatedControllers { 
    pSDL_Joystick joy[MAX_DEVICES_PER_CONTROLLER];
    pSDL_GameController gameCtrl[MAX_DEVICES_PER_CONTROLLER];
    int instance[MAX_DEVICES_PER_CONTROLLER];
    int index[MAX_DEVICES_PER_CONTROLLER];
    std::string name[MAX_DEVICES_PER_CONTROLLER];
    std::string nameDB[MAX_DEVICES_PER_CONTROLLER];
    bool mappingOKDevice[MAX_DEVICES_PER_CONTROLLER];
    bool mappingOK;
    int controllerType;
    int numberOfDevices;
} T_DesignatedControllers;

//designated controllers
extern T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];


m64p_error EVidExt_Init(void);
m64p_error EVidExt_Quit(void);
m64p_error EVidExt_ListFullscreenModes(m64p_2d_size *, int *);
m64p_error EVidExt_SetVideoMode(int, int, int, m64p_video_mode, m64p_video_flags);
m64p_error EVidExt_ResizeWindow(int, int);
m64p_error EVidExt_SetCaption(const char *);
m64p_error EVidExt_ToggleFullScreen(void);
m64p_function EVidExt_GL_GetProcAddress(const char *);
m64p_error EVidExt_GL_SetAttribute(m64p_GLattr, int);
m64p_error EVidExt_GL_SwapBuffers(void);
extern "C" m64p_error ECoreGetAPIVersions(int *, int *, int *, int *);
m64p_error EConfigListSections(void *, void (*)(void *, const char *));
extern "C" m64p_error EConfigOpenSection(const char *, m64p_handle *);
extern "C" m64p_error EConfigListParameters(m64p_handle, void *, void (*)(void *, const char *, m64p_type));
m64p_error EConfigSaveFile(void);
m64p_error EConfigSaveSection(const char *);
int EConfigHasUnsavedChanges(const char *);
extern "C" m64p_error EConfigDeleteSection(const char *SectionName);
m64p_error EConfigRevertChanges(const char *SectionName);
extern "C" m64p_error EConfigSetParameter(m64p_handle, const char *, m64p_type, const void *);
m64p_error EConfigSetParameterHelp(m64p_handle, const char *, const char *);
extern "C" m64p_error EConfigGetParameter(m64p_handle, const char *, m64p_type, void *, int);
m64p_error EConfigGetParameterType(m64p_handle, const char *, m64p_type *);
const char * EConfigGetParameterHelp(m64p_handle, const char *);
extern "C" m64p_error EConfigSetDefaultInt(m64p_handle, const char *, int, const char *);
extern "C" m64p_error EConfigSetDefaultFloat(m64p_handle, const char *, float, const char *);
extern "C" m64p_error EConfigSetDefaultBool(m64p_handle, const char *, int, const char *);
extern "C" m64p_error EConfigSetDefaultString(m64p_handle, const char *, const char *, const char *);
extern "C" int          EConfigGetParamInt(m64p_handle, const char *);
extern "C" float        EConfigGetParamFloat(m64p_handle, const char *);
extern "C" int          EConfigGetParamBool(m64p_handle, const char *);
extern "C" const char * EConfigGetParamString(m64p_handle, const char *);
extern "C" const char * EConfigGetSharedDataFilepath(const char *);
extern "C" const char * EConfigGetUserConfigPath(void);
extern "C" const char * EConfigGetUserDataPath(void);
extern "C" const char * EConfigGetUserCachePath(void);
m64p_error EConfigExternalOpen(const char *, m64p_handle *);
m64p_error EConfigExternalClose(m64p_handle);
m64p_error EConfigExternalGetParameter(m64p_handle, const char *, const char *, char *, int);



/* definitions of pointers to Core config functions */
extern ptr_ConfigOpenSection      ConfigOpenSection;
extern ptr_ConfigDeleteSection    ConfigDeleteSection;
extern ptr_ConfigListParameters   ConfigListParameters;
extern ptr_ConfigSetParameter     ConfigSetParameter;
extern ptr_ConfigGetParameter     ConfigGetParameter;
extern ptr_ConfigGetParameterHelp ConfigGetParameterHelp;
extern ptr_ConfigSetDefaultInt    ConfigSetDefaultInt;
extern ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat;
extern ptr_ConfigSetDefaultBool   ConfigSetDefaultBool;
extern ptr_ConfigSetDefaultString ConfigSetDefaultString;
extern ptr_ConfigGetParamInt      ConfigGetParamInt;
extern ptr_ConfigGetParamFloat    ConfigGetParamFloat;
extern ptr_ConfigGetParamBool     ConfigGetParamBool;
extern ptr_ConfigGetParamString   ConfigGetParamString;

extern ptr_ConfigGetSharedDataFilepath ConfigGetSharedDataFilepath;
extern ptr_ConfigGetUserConfigPath     ConfigGetUserConfigPath;
extern ptr_ConfigGetUserDataPath       ConfigGetUserDataPath;
extern ptr_ConfigGetUserCachePath      ConfigGetUserCachePath;

/* global data definitions */
SController controller[4];   // 4 controllers

/* static data definitions */
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;
static int l_PluginInit = 0;
static int l_joyWasInit = 0;
static int l_hapticWasInit = 0;

static unsigned short button_bits[] = {
    0x0001,  // R_DPAD
    0x0002,  // L_DPAD
    0x0004,  // D_DPAD
    0x0008,  // U_DPAD
    0x0010,  // START_BUTTON
    0x0020,  // Z_TRIG
    0x0040,  // B_BUTTON
    0x0080,  // A_BUTTON
    0x0100,  // R_CBUTTON
    0x0200,  // L_CBUTTON
    0x0400,  // D_CBUTTON
    0x0800,  // U_CBUTTON
    0x1000,  // R_TRIG
    0x2000,  // L_TRIG
    0x4000,  // Mempak switch
    0x8000   // Rumblepak switch
};

static int romopen = 0;         // is a rom opened

static unsigned char myKeyState[SDL_NUM_SCANCODES];


/* Global functions */
void IDebugMessage(int level, const char *message, ...)
{
  char msgbuf[1024];
  va_list args;

  if (l_DebugCallback == NULL)
      return;

  va_start(args, message);
  vsprintf(msgbuf, message, args);

  (*l_DebugCallback)(l_DebugCallContext, level, msgbuf);

  va_end(args);
}

static CONTROL temp_core_controlinfo[4];

/* Mupen64Plus plugin functions */
extern "C" m64p_error IPluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
                                   void (*DebugCallback)(void *, int, const char *))
{
    ptr_CoreGetAPIVersions CoreAPIVersionFunc;

    int i, ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;

    if (l_PluginInit)
        return M64ERR_ALREADY_INIT;

    /* first thing is to set the callback function for debug info */
    l_DebugCallback = DebugCallback;
    l_DebugCallContext = Context;

    /* attach and call the CoreGetAPIVersions function, check Config API version for compatibility */
    CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) ECoreGetAPIVersions;
    if (CoreAPIVersionFunc == NULL)
    {
        IDebugMessage(M64MSG_ERROR, "Core emulator broken; no CoreAPIVersionFunc() function found.");
        return M64ERR_INCOMPATIBLE;
    }

    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000) || ConfigAPIVersion < CONFIG_API_VERSION)
    {
        IDebugMessage(M64MSG_ERROR, "Emulator core Config API (v%i.%i.%i) incompatible with plugin (v%i.%i.%i)",
                VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        return M64ERR_INCOMPATIBLE;
    }

    /* Get the core config function pointers from the library handle */
    ConfigOpenSection = (ptr_ConfigOpenSection) EConfigOpenSection;
    ConfigDeleteSection = (ptr_ConfigDeleteSection) EConfigDeleteSection;
    ConfigListParameters = (ptr_ConfigListParameters) EConfigListParameters;
    ConfigSetParameter = (ptr_ConfigSetParameter) EConfigSetParameter;
    ConfigGetParameter = (ptr_ConfigGetParameter) EConfigGetParameter;
    ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt) EConfigSetDefaultInt;
    ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat) EConfigSetDefaultFloat;
    ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool) EConfigSetDefaultBool;
    ConfigSetDefaultString = (ptr_ConfigSetDefaultString) EConfigSetDefaultString;
    ConfigGetParamInt = (ptr_ConfigGetParamInt) EConfigGetParamInt;
    ConfigGetParamFloat = (ptr_ConfigGetParamFloat) ConfigGetParamFloat;
    ConfigGetParamBool = (ptr_ConfigGetParamBool) EConfigGetParamBool;
    ConfigGetParamString = (ptr_ConfigGetParamString) EConfigGetParamString;

    ConfigGetSharedDataFilepath = (ptr_ConfigGetSharedDataFilepath) EConfigGetSharedDataFilepath;
    ConfigGetUserConfigPath = (ptr_ConfigGetUserConfigPath) EConfigGetUserConfigPath;
    ConfigGetUserDataPath = (ptr_ConfigGetUserDataPath) EConfigGetUserDataPath;
    ConfigGetUserCachePath = (ptr_ConfigGetUserCachePath) EConfigGetUserCachePath;

    if (!ConfigOpenSection || !ConfigDeleteSection || !ConfigSetParameter || !ConfigGetParameter ||
        !ConfigSetDefaultInt || !ConfigSetDefaultFloat || !ConfigSetDefaultBool || !ConfigSetDefaultString ||
        !ConfigGetParamInt   || !ConfigGetParamFloat   || !ConfigGetParamBool   || !ConfigGetParamString ||
        !ConfigGetSharedDataFilepath || !ConfigGetUserConfigPath || !ConfigGetUserDataPath || !ConfigGetUserCachePath)
    {
        IDebugMessage(M64MSG_ERROR, "Couldn't connect to Core configuration functions");
        return M64ERR_INCOMPATIBLE;
    }

    /* reset controllers */
    memset(controller, 0, sizeof(SController) * 4);
    for (i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        myKeyState[i] = 0;
    }
    /* set CONTROL struct pointers to the temporary static array */
    /* this small struct is used to tell the core whether each controller is plugged in, and what type of pak is connected */
    /* we only need it so that we can call load_configuration below, to auto-config for a GUI front-end */
    for (i = 0; i < 4; i++)
        controller[i].control = temp_core_controlinfo + i;

    /* initialize the joystick subsystem if necessary */
    l_joyWasInit = SDL_WasInit(SDL_INIT_JOYSTICK);
    if (!l_joyWasInit)
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
        {
            IDebugMessage(M64MSG_ERROR, "Couldn't init SDL joystick subsystem: %s", SDL_GetError() );
            return M64ERR_SYSTEM_FAIL;
        }

    /* read plugin config from core config database, auto-config if necessary and update core database */
    load_configuration(1);

    l_PluginInit = 1;
    return M64ERR_SUCCESS;
}

extern "C" m64p_error IPluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    /* reset some local variables */
    l_DebugCallback = NULL;
    l_DebugCallContext = NULL;

    /* quit the joystick subsystem if necessary */
    /*if (!l_joyWasInit)
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);*/

    l_PluginInit = 0;
    return M64ERR_SUCCESS;
}

extern "C" m64p_error IPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_INPUT;

    if (PluginVersion != NULL)
        *PluginVersion = PLUGIN_VERSION;

    if (APIVersion != NULL)
        *APIVersion = INPUT_PLUGIN_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = PLUGIN_NAME;

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

/* Helper function to handle the SDL keys */
static void doSdlKeys(const unsigned char* keystate)
{
    int c, b, axis_val, axis_max_val;
    static int grabmouse = 1, grabtoggled = 0;

    axis_max_val = 80;
    if (keystate[SDL_SCANCODE_RCTRL])
        axis_max_val -= 40;
    if (keystate[SDL_SCANCODE_RSHIFT])
        axis_max_val -= 20;

    for( c = 0; c < 4; c++ )
    {
        for( b = 0; b < 16; b++ )
        {
            if( controller[c].button[b].key == SDL_SCANCODE_UNKNOWN || ((int) controller[c].button[b].key) < 0)
                continue;
            if( keystate[controller[c].button[b].key] )
                controller[c].buttons.Value |= button_bits[b];
        }
        for( b = 0; b < 2; b++ )
        {
            // from the N64 func ref: The 3D Stick data is of type signed char and in
            // the range between 80 and -80. (32768 / 409 = ~80.1)
            if( b == 0 )
                axis_val = controller[c].buttons.X_AXIS;
            else
                axis_val = -controller[c].buttons.Y_AXIS;

            if( controller[c].axis[b].key_a != SDL_SCANCODE_UNKNOWN && ((int) controller[c].axis[b].key_a) > 0)
                if( keystate[controller[c].axis[b].key_a] )
                    axis_val = -axis_max_val;
            if( controller[c].axis[b].key_b != SDL_SCANCODE_UNKNOWN && ((int) controller[c].axis[b].key_b) > 0)
                if( keystate[controller[c].axis[b].key_b] )
                    axis_val = axis_max_val;

            if( b == 0 )
                controller[c].buttons.X_AXIS = axis_val;
            else
                controller[c].buttons.Y_AXIS = -axis_val;
        }
        if (controller[c].mouse)
        {
            if (keystate[SDL_SCANCODE_LCTRL] && keystate[SDL_SCANCODE_LALT])
            {
                if (!grabtoggled)
                {
                    grabtoggled = 1;
                    grabmouse = !grabmouse;
                    // grab/ungrab mouse
                    SDL_SetRelativeMouseMode(grabmouse ? SDL_TRUE : SDL_FALSE);
                    SDL_ShowCursor( grabmouse ? 0 : 1 );
                }
            }
            else grabtoggled = 0;
        }
    }
}

static unsigned char DataCRC( unsigned char *Data, int iLenght )
{
    unsigned char Remainder = Data[0];

    int iByte = 1;
    unsigned char bBit = 0;

    while( iByte <= iLenght )
    {
        int HighBit = ((Remainder & 0x80) != 0);
        Remainder = Remainder << 1;

        Remainder += ( iByte < iLenght && Data[iByte] & (0x80 >> bBit )) ? 1 : 0;

        Remainder ^= (HighBit) ? 0x85 : 0;

        bBit++;
        iByte += bBit/8;
        bBit %= 8;
    }

    return Remainder;
}

/******************************************************************
  Function: ControllerCommand
  Purpose:  To process the raw data that has just been sent to a
            specific controller.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none

  note:     This function is only needed if the DLL is allowing raw
            data, or the plugin is set to raw

            the data that is being processed looks like this:
            initilize controller: 01 03 00 FF FF FF
            read controller:      01 04 01 FF FF FF FF
*******************************************************************/
extern "C" void IControllerCommand(int Control, unsigned char *Command)
{
    unsigned char *Data = &Command[5];

    if (Control == -1)
        return;

    switch (Command[2])
    {
        case RD_GETSTATUS:

            break;
        case RD_READKEYS:

            break;
        case RD_READPAK:

            if (controller[Control].control->Plugin == PLUGIN_RAW)
            {
                unsigned int dwAddress = (Command[3] << 8) + (Command[4] & 0xE0);

                if(( dwAddress >= 0x8000 ) && ( dwAddress < 0x9000 ) )
                    memset( Data, 0x80, 32 );
                else
                    memset( Data, 0x00, 32 );

                Data[32] = DataCRC( Data, 32 );
            }
            break;
        case RD_WRITEPAK:

            if (controller[Control].control->Plugin == PLUGIN_RAW)
            {
                unsigned int dwAddress = (Command[3] << 8) + (Command[4] & 0xE0);
              if (dwAddress == PAK_IO_RUMBLE && *Data)
                    IDebugMessage(M64MSG_VERBOSE, "Triggering rumble pack.");

                if(dwAddress == PAK_IO_RUMBLE && controller[Control].event_joystick) {
                    if (*Data) {
                        SDL_HapticRumblePlay(controller[Control].event_joystick, 1, SDL_HAPTIC_INFINITY);
                    } else {
                        SDL_HapticRumbleStop(controller[Control].event_joystick);
                    }
                }

                Data[32] = DataCRC( Data, 32 );
            }
            break;
        case RD_RESETCONTROLLER:

            break;
        case RD_READEEPROM:

            break;
        case RD_WRITEEPROM:

            break;
        }
}

/******************************************************************
  Function: GetKeys
  Purpose:  To get the current state of the controllers buttons.
  input:    - Controller Number (0 to 3)
            - A pointer to a BUTTONS structure to be filled with
            the controller state.
  output:   none
*******************************************************************/
extern "C" void IGetKeys( int Control, BUTTONS *Keys )
{
    static int mousex_residual = 0;
    static int mousey_residual = 0;
    int b, axis_val;
    SDL_Event event;
    unsigned char mstate;

    SDL_PumpEvents();

    // Handle keyboard input first
    doSdlKeys(SDL_GetKeyboardState(NULL));
    doSdlKeys(myKeyState);


    #warning "JC: modified"
    /*int slot = 0;
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gDesignatedControllers[i].gameCtrl[0] != NULL)
        {
            controller[b].joystick = gDesignatedControllers[slot].gameCtrl[0];
            slot++;
        }
    }*/
    
    /*for ( b = 0; b < 4; ++b )
    {
        if (controller[b].device >= 0)
        {
            if (!SDL_JoystickGetAttached(controller[b].joystick))
                controller[b].joystick = SDL_JoystickOpen(controller[b].device);
        }
    }*/

    // read joystick state
    SDL_JoystickUpdate();

    if( controller[Control].device >= 0 )
    {
        for( b = 0; b < 16; b++ )
        {
            if( controller[Control].button[b].button >= 0 )
                if( SDL_GameControllerGetButton( controller[Control].joystick, (SDL_GameControllerButton)(controller[Control].button[b].button )) )
                    controller[Control].buttons.Value |= button_bits[b];

            if( controller[Control].button[b].axis >= 0 )
            {
                int deadzone = controller[Control].button[b].axis_deadzone;
                axis_val = SDL_GameControllerGetAxis( controller[Control].joystick, (SDL_GameControllerAxis)(controller[Control].button[b].axis));
                if (deadzone < 0)
                    deadzone = 16384; /* default */
                if( (controller[Control].button[b].axis_dir < 0) && (axis_val <= -deadzone) )
                    controller[Control].buttons.Value |= button_bits[b];
                else if( (controller[Control].button[b].axis_dir > 0) && (axis_val >= deadzone) )
                    controller[Control].buttons.Value |= button_bits[b];
            }
        }
        int iX = controller[Control].buttons.X_AXIS;
        int iY = controller[Control].buttons.Y_AXIS;
        for( b = 0; b < 2; b++ )
        {
            /* from the N64 func ref: The 3D Stick data is of type signed char and in the range between -80 and +80 */
            int deadzone = controller[Control].axis_deadzone[b];
            int range = controller[Control].axis_peak[b] - controller[Control].axis_deadzone[b];
            /* skip this axis if the deadzone/peak values are invalid */
            if (deadzone < 0 || range < 1)
                continue;

            if( b == 0 )
                axis_val = iX;
            else
                axis_val = -iY;

            if( controller[Control].axis[b].axis_a >= 0 )  /* up and left for N64 */
            {
                int joy_val = SDL_GameControllerGetAxis(controller[Control].joystick, (SDL_GameControllerAxis)(controller[Control].axis[b].axis_a));
                int axis_dir = controller[Control].axis[b].axis_dir_a;
                if (joy_val * axis_dir > deadzone)
                    axis_val = -((abs(joy_val) - deadzone) * 80 / range);
            }
            if( controller[Control].axis[b].axis_b >= 0 ) /* down and right for N64 */
            {
                int joy_val = SDL_GameControllerGetAxis(controller[Control].joystick, (SDL_GameControllerAxis)(controller[Control].axis[b].axis_b));
                int axis_dir = controller[Control].axis[b].axis_dir_b;
                if (joy_val * axis_dir > deadzone)
                    axis_val = ((abs(joy_val) - deadzone) * 80 / range);
            }
            

            if( controller[Control].axis[b].button_a >= 0 )
                if( SDL_GameControllerGetButton( controller[Control].joystick, (SDL_GameControllerButton)(controller[Control].axis[b].button_a )) )
                    axis_val = -80;
            if( controller[Control].axis[b].button_b >= 0 )
                if( SDL_GameControllerGetButton( controller[Control].joystick, (SDL_GameControllerButton)(controller[Control].axis[b].button_b) ) )
                    axis_val = 80;

            if( b == 0 )
                iX = axis_val;
            else
                iY = -axis_val;
        }
        /* store the result */
        if (iX < -80) iX = -80;
        if (iX >  80) iX =  80;
        if (iY < -80) iY = -80;
        if (iY >  80) iY =  80;
        controller[Control].buttons.X_AXIS = iX;
        controller[Control].buttons.Y_AXIS = iY;
    }

    // process mouse events
    mstate = SDL_GetMouseState( NULL, NULL );
    for( b = 0; b < 16; b++ )
    {
        if( controller[Control].button[b].mouse < 1 )
            continue;
        if( mstate & SDL_BUTTON(controller[Control].button[b].mouse) )
            controller[Control].buttons.Value |= button_bits[b];
    }

    if (controller[Control].mouse)
    {
        if (SDL_GetRelativeMouseMode())

        {

            while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) == 1)

            {
                int w, h;
                SDL_Window *focus;

                if (event.motion.xrel)
                {
                    mousex_residual += (int) (event.motion.xrel * controller[Control].mouse_sens[0]);
                }
                if (event.motion.yrel)
                {
                    mousey_residual += (int) (event.motion.yrel * controller[Control].mouse_sens[1]);
                }

                focus = SDL_GetKeyboardFocus();
                if (focus) {
                    SDL_GetWindowSize(focus, &w, &h);
                    SDL_WarpMouseInWindow(focus, w / 2, h / 2);
                } else {
                    mousex_residual = 0;
                    mousey_residual = 0;
                }
            }

            /* store the result */
            int iX = mousex_residual;
            int iY = -mousey_residual;
            if (iX < -80) iX = -80;
            if (iX >  80) iX =  80;
            if (iY < -80) iY = -80;
            if (iY >  80) iY =  80;
            controller[Control].buttons.X_AXIS = iX;
            controller[Control].buttons.Y_AXIS = iY;

            /* the mouse x/y values decay exponentially (returns to center), unless the left "Windows" key is held down */
            if (!myKeyState[SDL_SCANCODE_LGUI])
            {
                mousex_residual = (mousex_residual * 224) / 256;
                mousey_residual = (mousey_residual * 224) / 256;
            }
        }
        else
        {
            mousex_residual = 0;
            mousey_residual = 0;
        }
    }

    *Keys = controller[Control].buttons;

    /* handle mempack / rumblepak switching (only if rumble is active on joystick) */

    if (controller[Control].event_joystick) {
        static unsigned int SwitchPackTime[4] = {0, 0, 0, 0}, SwitchPackType[4] = {0, 0, 0, 0};
        if (controller[Control].buttons.Value & button_bits[14]) {
            SwitchPackTime[Control] = SDL_GetTicks();         // time at which the 'switch pack' command was given
            SwitchPackType[Control] = PLUGIN_MEMPAK;          // type of new pack to insert
            controller[Control].control->Plugin = PLUGIN_NONE;// remove old pack
            SDL_HapticRumblePlay(controller[Control].event_joystick, 0.5, 500);
        }
        if (controller[Control].buttons.Value & button_bits[15]) {
            SwitchPackTime[Control] = SDL_GetTicks();         // time at which the 'switch pack' command was given
            SwitchPackType[Control] = PLUGIN_RAW;             // type of new pack to insert
            controller[Control].control->Plugin = PLUGIN_NONE;// remove old pack
            SDL_HapticRumblePlay(controller[Control].event_joystick, 1, 500);
        }
        // handle inserting new pack if the time has arrived
        if (SwitchPackTime[Control] != 0 && (SDL_GetTicks() - SwitchPackTime[Control]) >= 1000)
        {
            SDL_HapticRumbleStop(controller[Control].event_joystick);
            controller[Control].control->Plugin = SwitchPackType[Control];
            SwitchPackTime[Control] = 0;
        }
    }

    controller[Control].buttons.Value = 0;
}

static void InitiateJoysticks(int cntrl)
{
    if ((controller[cntrl].device >= 0) && (cntrl < MAX_GAMEPADS))
    {
        controller[cntrl].joystick = gDesignatedControllers[cntrl].gameCtrl[0];
        if (!controller[cntrl].joystick)
            IDebugMessage(M64MSG_WARNING, "Couldn't open joystick for controller #%d: %s", cntrl + 1, SDL_GetError());
    } else {
        controller[cntrl].joystick = NULL;
    }
}

static void DeinitJoystick(int cntrl)
{
    if (controller[cntrl].joystick) 
    {
        //SDL_JoystickClose(controller[cntrl].joystick);
        controller[cntrl].joystick = NULL;
    }
}

static void InitiateRumble(int cntrl)
{
/*
    l_hapticWasInit = SDL_WasInit(SDL_INIT_HAPTIC);
    if (!l_hapticWasInit) {
        if (SDL_InitSubSystem(SDL_INIT_HAPTIC) == -1) {
            IDebugMessage(M64MSG_ERROR, "Couldn't init SDL haptic subsystem: %s", SDL_GetError() );
            return;
        }
    }

    controller[cntrl].event_joystick = SDL_HapticOpenFromJoystick(controller[cntrl].joystick);
    if (!controller[cntrl].event_joystick) {
        IDebugMessage(M64MSG_WARNING, "Couldn't open rumble support for joystick #%i", cntrl + 1);
        return;
    }

    if (SDL_HapticRumbleSupported(controller[cntrl].event_joystick) == SDL_FALSE) {
        SDL_HapticClose(controller[cntrl].event_joystick);
        controller[cntrl].event_joystick = NULL;
        IDebugMessage(M64MSG_WARNING, "Joystick #%i doesn't support rumble effect", cntrl + 1);
        return;
    }

    if (SDL_HapticRumbleInit(controller[cntrl].event_joystick) != 0) {
        SDL_HapticClose(controller[cntrl].event_joystick);
        controller[cntrl].event_joystick = NULL;
        IDebugMessage(M64MSG_WARNING, "Rumble initialization failed for Joystick #%i", cntrl + 1);
        return;
    }

    IDebugMessage(M64MSG_INFO, "Rumble activated on N64 joystick #%i", cntrl + 1);
    */
}

static void DeinitRumble(int cntrl)
{
    /*
	// quit the haptic subsystem if necessary
    if (!l_hapticWasInit)
        SDL_QuitSubSystem(SDL_INIT_HAPTIC);

    if (controller[cntrl].event_joystick) {
        SDL_HapticClose(controller[cntrl].event_joystick);
        controller[cntrl].event_joystick = NULL;
    }*/
}

/******************************************************************
  Function: InitiateControllers
  Purpose:  This function initialises how each of the controllers
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for
              the emulator to know how to handle each controller.
  output:   none
*******************************************************************/
extern "C" void IInitiateControllers(CONTROL_INFO ControlInfo)
{
    int i;

    // reset controllers
    memset( controller, 0, sizeof( SController ) * 4 );
    for ( i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        myKeyState[i] = 0;
    }
    // set our CONTROL struct pointers to the array that was passed in to this function from the core
    // this small struct tells the core whether each controller is plugged in, and what type of pak is connected
    for (i = 0; i < 4; i++)
        controller[i].control = ControlInfo.Controls + i;

    // read configuration
    load_configuration(0);

    for( i = 0; i < 4; i++ )
    {
        // test for rumble support for this joystick
        InitiateJoysticks(i);
        InitiateRumble(i);
        // if rumble not supported, switch to mempack
        if (controller[i].control->Plugin == PLUGIN_RAW && controller[i].event_joystick == 0)
            controller[i].control->Plugin = PLUGIN_MEMPAK;
        DeinitRumble(i);
        DeinitJoystick(i);
    }

    IDebugMessage(M64MSG_INFO, "%s version %i.%i.%i initialized.", PLUGIN_NAME, VERSION_PRINTF_SPLIT(PLUGIN_VERSION));
}

/******************************************************************
  Function: ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
extern "C" void IReadController(int Control, unsigned char *Command)
{
#ifdef _DEBUG
    if (Command != NULL)
        IDebugMessage(M64MSG_INFO, "Raw Read (cont=%d):  %02X %02X %02X %02X %02X %02X", Control,
                     Command[0], Command[1], Command[2], Command[3], Command[4], Command[5]);
#endif
}

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/
extern "C" void IRomClosed(void)
{
    int i;

    // close joysticks
    for( i = 0; i < 4; i++ ) {
        DeinitRumble(i);
        DeinitJoystick(i);
    }

    // release/ungrab mouse
    SDL_SetRelativeMouseMode(SDL_FALSE);

    SDL_ShowCursor( 1 );

    romopen = 0;
}

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the
            emulation thread)
  input:    none
  output:   none
*******************************************************************/
extern "C" int IRomOpen(void)
{
    int i;

    // open joysticks
    for (i = 0; i < 4; i++) {
        InitiateJoysticks(i);
        InitiateRumble(i);
    }

    // grab mouse
    if (controller[0].mouse || controller[1].mouse || controller[2].mouse || controller[3].mouse)
    {
        SDL_ShowCursor( 0 );
        if (SDL_SetRelativeMouseMode(SDL_TRUE) < 0) {
            IDebugMessage(M64MSG_WARNING, "Couldn't grab input! Mouse support won't work!");
        }
    }

    romopen = 1;
    return 1;
}

/******************************************************************
  Function: SDL_KeyDown
  Purpose:  To pass the SDL_KeyDown message from the emulator to the
            plugin.
  input:    keymod and keysym of the SDL_KEYDOWN message.
  output:   none
*******************************************************************/
extern "C" void ISDL_KeyDown(int keymod, int keysym)
{
    myKeyState[keysym] = 1;
}

/******************************************************************
  Function: SDL_KeyUp
  Purpose:  To pass the SDL_KeyUp message from the emulator to the
            plugin.
  input:    keymod and keysym of the SDL_KEYUP message.
  output:   none
*******************************************************************/
extern "C" void ISDL_KeyUp(int keymod, int keysym)
{
    myKeyState[keysym] = 0;
}

