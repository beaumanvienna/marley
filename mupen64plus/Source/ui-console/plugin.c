/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-ui-console - plugin.c                                     *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2009 Richard42                                          *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
#endif

#include "core_interface.h"
#include "m64p_common.h"
#include "m64p_types.h"
#include "main.h"  // for the debug callback function 
#include "osal_dynamiclib.h"
#include "osal_files.h"
#include "osal_preproc.h"
#include "plugin.h"
#include "version.h"

m64p_error GPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities);
m64p_error GPluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,void (*DebugCallback)(void *, int, const char *));
m64p_error GPluginShutdown(void);

m64p_error APluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities);
m64p_error APluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,void (*DebugCallback)(void *, int, const char *));
m64p_error APluginShutdown(void);

m64p_error IPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities);
m64p_error IPluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,void (*DebugCallback)(void *, int, const char *));
m64p_error IPluginShutdown(void);

m64p_error SPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities);
m64p_error SPluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,void (*DebugCallback)(void *, int, const char *));
m64p_error SPluginShutdown(void);

// global variables 
const char *g_PluginDir = NULL;
const char *g_GfxPlugin = NULL;        // pointer to graphics plugin specified at commandline (if any)
const char *g_AudioPlugin = NULL;      // pointer to audio plugin specified at commandline (if any)
const char *g_InputPlugin = NULL;      // pointer to input plugin specified at commandline (if any)
const char *g_RspPlugin = NULL;        // pointer to rsp plugin specified at commandline (if any)

plugin_map_node g_PluginMap[] = {{M64PLUGIN_GFX,   "Video", NULL, "", NULL, 0 },
                                 {M64PLUGIN_AUDIO, "Audio", NULL, "", NULL, 0 },
                                 {M64PLUGIN_INPUT, "Input", NULL, "", NULL, 0 },
                                 {M64PLUGIN_RSP,   "RSP",   NULL, "", NULL, 0 } };

// local functions 
static m64p_error PluginLoadTry(const char *filepath, int MapIndex)
{
    ptr_PluginGetVersion PluginGetVersion;
    ptr_PluginStartup PluginStartup;
    
    
    m64p_plugin_type PluginType = (m64p_plugin_type) 0;
    int PluginVersion = 0;
    const char *PluginName = NULL;
    m64p_error rval;
    
    #define PLUGIN_VIDEO    0
    #define PLUGIN_AUDIO    1
    #define PLUGIN_INPUT    2
    #define PLUGIN_RSP      3
    if (MapIndex==PLUGIN_VIDEO)
    {
        PluginGetVersion = (ptr_PluginGetVersion) GPluginGetVersion;
        PluginStartup    = (ptr_PluginStartup)    GPluginStartup;
    }
    if (MapIndex==PLUGIN_AUDIO)
    {
        PluginGetVersion = (ptr_PluginGetVersion) APluginGetVersion;
        PluginStartup    = (ptr_PluginStartup)    APluginStartup;
    }
    if (MapIndex==PLUGIN_INPUT)
    {
        PluginGetVersion = (ptr_PluginGetVersion) IPluginGetVersion;
        PluginStartup    = (ptr_PluginStartup)    IPluginStartup;
    }
    if (MapIndex==PLUGIN_RSP)
    {
        PluginGetVersion = (ptr_PluginGetVersion) SPluginGetVersion;
        PluginStartup    = (ptr_PluginStartup)    SPluginStartup;
    }
     
    (*PluginGetVersion)(&PluginType, &PluginVersion, NULL, &PluginName, NULL);
    if (PluginType != g_PluginMap[MapIndex].type)
    {
        // the type of this plugin doesn't match with the type that was requested by the caller 
        return M64ERR_INCOMPATIBLE;
    }
    // the front-end doesn't talk to the plugins, so we don't care about the plugin version or api version 

    // call the plugin's initialization function and make sure it starts okay 
    
    
    if (PluginStartup == NULL)
    {
        UDebugMessage(M64MSG_ERROR, "library '%s' broken.  No PluginStartup() function found.", filepath);
        return M64ERR_INCOMPATIBLE;
    }
    rval = (*PluginStartup)(NULL, g_PluginMap[MapIndex].name, DebugCallback);  // DebugCallback is in main.c 
    if (rval != M64ERR_SUCCESS)
    {
        UDebugMessage(M64MSG_ERROR, "Error: %s plugin library '%s' failed to start.", g_PluginMap[MapIndex].name, filepath);
        return rval;
    }

    // plugin loaded successfully, so set the plugin map's members 
    g_PluginMap[MapIndex].handle = -1;
    strcpy(g_PluginMap[MapIndex].filename, filepath);
    g_PluginMap[MapIndex].libname = PluginName;
    g_PluginMap[MapIndex].libversion = PluginVersion;
    return M64ERR_SUCCESS;
}

// global functions 
m64p_error PluginSearchLoad(m64p_handle ConfigUI)
{
    for (int i = 0; i < 4; i++)
    {
        PluginLoadTry("built-in", i);
    }
    /*
            
    osal_lib_search *lib_filelist = NULL;
    int i;

    // start by checking the directory given on the command line 
    if (g_PluginDir != NULL)
    {
        lib_filelist = osal_library_search(g_PluginDir);
        if (lib_filelist == NULL)
        {
            UDebugMessage(M64MSG_ERROR, "No plugins found in --plugindir path: %s", g_PluginDir);
            return M64ERR_INPUT_NOT_FOUND;
        }
    }

    // if no plugins found, search the PluginDir in the UI-console section of the config file 
    if (lib_filelist == NULL)
    {
        const char *plugindir = (*ConfigGetParamString)(ConfigUI, "PluginDir");
        lib_filelist = osal_library_search(plugindir);
    }

    // if still no plugins found, search some common system folders 
    if (lib_filelist == NULL)
    {
        for (i = 0; i < osal_libsearchdirs; i++)
        {
            lib_filelist = osal_library_search(osal_libsearchpath[i]);
            if (lib_filelist != NULL)
                break;
        }
    }

    // try to load one of each type of plugin 
    for (i = 0; i < 4; i++)
    {
        m64p_plugin_type type = g_PluginMap[i].type;
        const char      *cmdline_path = NULL;
        const char      *config_var = NULL;
        int              use_dummy = 0;
        switch (type)
        {
            case M64PLUGIN_RSP:    cmdline_path = g_RspPlugin;    config_var = "RspPlugin";   break;
            case M64PLUGIN_GFX:    cmdline_path = g_GfxPlugin;    config_var = "VideoPlugin"; break;
            case M64PLUGIN_AUDIO:  cmdline_path = g_AudioPlugin;  config_var = "AudioPlugin"; break;
            case M64PLUGIN_INPUT:  cmdline_path = g_InputPlugin;  config_var = "InputPlugin"; break;
            default:               cmdline_path = NULL;           config_var = "";
        }

        // first search for a plugin matching what was given on the command line 
        if (cmdline_path != NULL)
        {
            // if full path was given, try loading exactly this file 
            if (strchr(cmdline_path, OSAL_DIR_SEPARATOR) != NULL)
            {
                PluginLoadTry(cmdline_path, i);
            }
            else if (strcmp(cmdline_path, "dummy") == 0)
            {
                use_dummy = 1;
            }
            else // otherwise search through the plugin directory to find a match with this name 
            {
                osal_lib_search *curr = lib_filelist;
                while (curr != NULL && g_PluginMap[i].handle == NULL)
                {
                    if (strncmp(curr->filename, cmdline_path, strlen(cmdline_path)) == 0)
                    {
                        PluginLoadTry(curr->filepath, i);
                    }
                    curr = curr->next;
                }
            }
            // exit with error if we couldn't find the specified plugin 
            if (!use_dummy && g_PluginMap[i].handle == NULL)
            {
                UDebugMessage(M64MSG_ERROR, "Specified %s plugin not found: %s", g_PluginMap[i].name, cmdline_path);
                osal_free_lib_list(lib_filelist);
                return M64ERR_INPUT_NOT_FOUND;
            }
        }
        else // otherwise search for a plugin specified in the config file 
        {
            const char *config_path = (*ConfigGetParamString)(ConfigUI, config_var);
            if (config_path != NULL && strlen(config_path) > 0)
            {
                // if full path was given, try loading exactly this file 
                if (strchr(config_path, OSAL_DIR_SEPARATOR) != NULL)
                {
                    PluginLoadTry(config_path, i);
                }
                else if (strcmp(config_path, "dummy") == 0)
                {
                    use_dummy = 1;
                }
                else // otherwise search through the plugin directory to find a match with this name 
                {
                    osal_lib_search *curr = lib_filelist;
                    while (curr != NULL && g_PluginMap[i].handle == NULL)
                    {
                        if (strncmp(curr->filename, config_path, strlen(config_path)) == 0)
                        {
                            PluginLoadTry(curr->filepath, i);
                        }
                        curr = curr->next;
                    }
                }
            }
        }
        // As a last resort, search for any appropriate plugin in search directory 
        if (!use_dummy && g_PluginMap[i].handle == NULL)
        {
            osal_lib_search *curr = lib_filelist;
            while (curr != NULL && g_PluginMap[i].handle == NULL)
            {
                PluginLoadTry(curr->filepath, i);
                curr = curr->next;
            }
        }
        // print out the particular plugin used 
        if (g_PluginMap[i].handle == NULL)
        {
            UDebugMessage(M64MSG_INFO, "using %s plugin: <dummy>", g_PluginMap[i].name);
        }
        else
        {
            UDebugMessage(M64MSG_INFO, "using %s plugin: '%s' v%i.%i.%i", g_PluginMap[i].name,
                   g_PluginMap[i].libname, VERSION_PRINTF_SPLIT(g_PluginMap[i].libversion));
            UDebugMessage(M64MSG_VERBOSE, "%s plugin library: %s", g_PluginMap[i].name, g_PluginMap[i].filename);
        }
    }

    // free up the list of library files in the plugin search directory 
    osal_free_lib_list(lib_filelist);*/
    return M64ERR_SUCCESS;
}

m64p_error PluginUnload(void)
{
    typedef m64p_error (*ptr_PluginShutdown)(void);
    ptr_PluginShutdown PluginShutdown;
    int i;

    // shutdown each type of plugin 
    for (i = 0; i < 4; i++)
    {
        if (g_PluginMap[i].handle == NULL)
            continue;
        
        if (i==PLUGIN_VIDEO)
        {
            PluginShutdown = (ptr_PluginShutdown) GPluginShutdown;
        }
        if (i==PLUGIN_AUDIO)
        {
            PluginShutdown = (ptr_PluginShutdown) APluginShutdown;
        }
        if (i==PLUGIN_INPUT)
        {
            PluginShutdown = (ptr_PluginShutdown) IPluginShutdown;
        }
        if (i==PLUGIN_RSP)
        {
            PluginShutdown = (ptr_PluginShutdown) SPluginShutdown;
        }
        
        
        if (PluginShutdown != NULL)
            (*PluginShutdown)();
        
        // clear out the plugin map's members 
        g_PluginMap[i].handle = NULL;
        g_PluginMap[i].filename[0] = 0;
        g_PluginMap[i].libname = NULL;
        g_PluginMap[i].libversion = 0;
    }

    return M64ERR_SUCCESS;
}

