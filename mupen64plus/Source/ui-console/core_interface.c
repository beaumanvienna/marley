/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-ui-console - core_interface.c                             *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2009 Richard Goedeken                                   *
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

/* This file contains the routines for attaching to the Mupen64Plus core
 * library and pointers to the core functions
 */

#if defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
#endif

#include <stdio.h>

#include "core_interface.h"
#include "m64p_common.h"
#include "m64p_config.h"
#include "m64p_debugger.h"
#include "m64p_frontend.h"
#include "m64p_types.h"
#include "main.h"
#include "osal_dynamiclib.h"
#include "osal_preproc.h"
#include "version.h"


/* global data definitions */
int g_CoreCapabilities;
int g_CoreAPIVersion;

const char * ECoreErrorMessage(m64p_error);
m64p_error ECoreStartup(int, const char *, const char *, void *, ptr_DebugCallback, void *, ptr_StateCallback);
m64p_error ECoreShutdown(void);
void EDebugCallback(void *Context, int level, const char *message);
void EStateCallback(void *Context, m64p_core_param param_type, int new_value);
m64p_error ECoreAttachPlugin(m64p_plugin_type, m64p_dynlib_handle);
m64p_error ECoreDetachPlugin(m64p_plugin_type);
m64p_error ECoreDoCommand(m64p_command, int, void *);
m64p_error ECoreOverrideVidExt(m64p_video_extension_functions *);
m64p_error ECoreAddCheat(const char *, m64p_cheat_code *, int);
m64p_error ECoreCheatEnabled(const char *, int);
m64p_error ECoreGetRomSettings(m64p_rom_settings *, int, int, int);
m64p_error EConfigListSections(void *, void (*)(void *, const char *));
m64p_error EConfigOpenSection(const char *, m64p_handle *);
m64p_error EConfigListParameters(m64p_handle, void *, void (*)(void *, const char *, m64p_type));
m64p_error EConfigSaveFile(void);
m64p_error EConfigSaveSection(const char *);
int EConfigHasUnsavedChanges(const char *);
m64p_error EConfigDeleteSection(const char *SectionName);
m64p_error EConfigRevertChanges(const char *SectionName);
m64p_error EConfigSetParameter(m64p_handle, const char *, m64p_type, const void *);
m64p_error EConfigSetParameterHelp(m64p_handle, const char *, const char *);
m64p_error EConfigGetParameter(m64p_handle, const char *, m64p_type, void *, int);
m64p_error EConfigGetParameterType(m64p_handle, const char *, m64p_type *);
const char * EConfigGetParameterHelp(m64p_handle, const char *);
m64p_error EConfigSetDefaultInt(m64p_handle, const char *, int, const char *);
m64p_error EConfigSetDefaultFloat(m64p_handle, const char *, float, const char *);
m64p_error EConfigSetDefaultBool(m64p_handle, const char *, int, const char *);
m64p_error EConfigSetDefaultString(m64p_handle, const char *, const char *, const char *);
int          EConfigGetParamInt(m64p_handle, const char *);
float        EConfigGetParamFloat(m64p_handle, const char *);
int          EConfigGetParamBool(m64p_handle, const char *);
const char * EConfigGetParamString(m64p_handle, const char *);
const char * EConfigGetSharedDataFilepath(const char *);
const char * EConfigGetUserConfigPath(void);
const char * EConfigGetUserDataPath(void);
const char * EConfigGetUserCachePath(void);
m64p_error EConfigExternalOpen(const char *, m64p_handle *);
m64p_error EConfigExternalClose(m64p_handle);
m64p_error EConfigExternalGetParameter(m64p_handle, const char *, const char *, char *, int);
m64p_error EDebugSetCallbacks(void (*)(void), void (*)(unsigned int), void (*)(void));
m64p_error EDebugSetCoreCompare(void (*)(unsigned int), void (*)(int, void *));
m64p_error EDebugSetRunState(m64p_dbg_runstate);
int EDebugGetState(m64p_dbg_state);
m64p_error EDebugStep(void);
void EDebugDecodeOp(unsigned int, char *, char *, int);
void * EDebugMemGetRecompInfo(m64p_dbg_mem_info, unsigned int, int);
int EDebugMemGetMemInfo(m64p_dbg_mem_info, unsigned int);
void * EDebugMemGetPointer(m64p_dbg_memptr_type);
unsigned long long  EDebugMemRead64(unsigned int);
unsigned int 	    EDebugMemRead32(unsigned int);
unsigned short 	    EDebugMemRead16(unsigned int);
unsigned char 	    EDebugMemRead8(unsigned int);
void EDebugMemWrite64(unsigned int, unsigned long long);
void EDebugMemWrite32(unsigned int, unsigned int);
void EDebugMemWrite16(unsigned int, unsigned short);
void EDebugMemWrite8(unsigned int, unsigned char);
void * EDebugGetCPUDataPtr(m64p_dbg_cpu_data);
int EDebugBreakpointLookup(unsigned int, unsigned int, unsigned int);
int EDebugBreakpointCommand(m64p_dbg_bkp_command, unsigned int, m64p_breakpoint *);
void EDebugBreakpointTriggeredBy(uint32_t *, uint32_t *);
uint32_t EDebugVirtualToPhysical(uint32_t);

/* definitions of pointers to Core common functions */
ptr_CoreErrorMessage    CoreErrorMessage = ECoreErrorMessage;

/* definitions of pointers to Core front-end functions */
ptr_CoreStartup         CoreStartup = ECoreStartup;
ptr_CoreShutdown        CoreShutdown = ECoreShutdown;
ptr_CoreAttachPlugin    CoreAttachPlugin = ECoreAttachPlugin;
ptr_CoreDetachPlugin    CoreDetachPlugin = ECoreDetachPlugin;
ptr_CoreDoCommand       CoreDoCommand = ECoreDoCommand;
ptr_CoreOverrideVidExt  CoreOverrideVidExt = ECoreOverrideVidExt;
ptr_CoreAddCheat        CoreAddCheat = ECoreAddCheat;
ptr_CoreCheatEnabled    CoreCheatEnabled = ECoreCheatEnabled;

/* definitions of pointers to Core config functions */
ptr_ConfigListSections     ConfigListSections = EConfigListSections;
ptr_ConfigOpenSection      ConfigOpenSection = EConfigOpenSection;
ptr_ConfigDeleteSection    ConfigDeleteSection = EConfigDeleteSection;
ptr_ConfigSaveSection      ConfigSaveSection = EConfigSaveSection;
ptr_ConfigListParameters   ConfigListParameters = EConfigListParameters;
ptr_ConfigSaveFile         ConfigSaveFile = EConfigSaveFile;
ptr_ConfigSetParameter     ConfigSetParameter = EConfigSetParameter;
ptr_ConfigGetParameter     ConfigGetParameter = EConfigGetParameter;
ptr_ConfigGetParameterType ConfigGetParameterType = EConfigGetParameterType;
ptr_ConfigGetParameterHelp ConfigGetParameterHelp = EConfigGetParameterHelp;
ptr_ConfigSetDefaultInt    ConfigSetDefaultInt = EConfigSetDefaultInt;
ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat = EConfigSetDefaultFloat;
ptr_ConfigSetDefaultBool   ConfigSetDefaultBool = EConfigSetDefaultBool;
ptr_ConfigSetDefaultString ConfigSetDefaultString = EConfigSetDefaultString;
ptr_ConfigGetParamInt      ConfigGetParamInt = EConfigGetParamInt;
ptr_ConfigGetParamFloat    ConfigGetParamFloat = EConfigGetParamFloat;
ptr_ConfigGetParamBool     ConfigGetParamBool = EConfigGetParamBool;
ptr_ConfigGetParamString   ConfigGetParamString = EConfigGetParamString;

ptr_ConfigExternalOpen         ConfigExternalOpen = EConfigExternalOpen;
ptr_ConfigExternalClose        ConfigExternalClose = EConfigExternalClose;
ptr_ConfigExternalGetParameter ConfigExternalGetParameter = EConfigExternalGetParameter;
ptr_ConfigHasUnsavedChanges    ConfigHasUnsavedChanges = EConfigHasUnsavedChanges;

ptr_ConfigGetSharedDataFilepath ConfigGetSharedDataFilepath = EConfigGetSharedDataFilepath;
ptr_ConfigGetUserConfigPath     ConfigGetUserConfigPath = EConfigGetUserConfigPath;
ptr_ConfigGetUserDataPath       ConfigGetUserDataPath = EConfigGetUserDataPath;
ptr_ConfigGetUserCachePath      ConfigGetUserCachePath = EConfigGetUserCachePath;

/* definitions of pointers to Core debugger functions */
ptr_DebugSetCallbacks      DebugSetCallbacks = EDebugSetCallbacks;
ptr_DebugSetCoreCompare    DebugSetCoreCompare = EDebugSetCoreCompare;
ptr_DebugSetRunState       DebugSetRunState = EDebugSetRunState;
ptr_DebugGetState          DebugGetState = EDebugGetState;
ptr_DebugStep              DebugStep = EDebugStep;
ptr_DebugDecodeOp          DebugDecodeOp = EDebugDecodeOp;
ptr_DebugMemGetRecompInfo  DebugMemGetRecompInfo = EDebugMemGetRecompInfo;
ptr_DebugMemGetMemInfo     DebugMemGetMemInfo = EDebugMemGetMemInfo;
ptr_DebugMemGetPointer     DebugMemGetPointer = EDebugMemGetPointer;

ptr_DebugMemRead64         DebugMemRead64 = EDebugMemRead64;
ptr_DebugMemRead32         DebugMemRead32 = EDebugMemRead32;
ptr_DebugMemRead16         DebugMemRead16 = EDebugMemRead16;
ptr_DebugMemRead8          DebugMemRead8 = EDebugMemRead8;

ptr_DebugMemWrite64        DebugMemWrite64 = EDebugMemWrite64;
ptr_DebugMemWrite32        DebugMemWrite32 = EDebugMemWrite32;
ptr_DebugMemWrite16        DebugMemWrite16 = EDebugMemWrite16;
ptr_DebugMemWrite8         DebugMemWrite8 = EDebugMemWrite8;

ptr_DebugGetCPUDataPtr     DebugGetCPUDataPtr = EDebugGetCPUDataPtr;
ptr_DebugBreakpointLookup  DebugBreakpointLookup = EDebugBreakpointLookup;
ptr_DebugBreakpointCommand DebugBreakpointCommand = EDebugBreakpointCommand;

ptr_DebugBreakpointTriggeredBy DebugBreakpointTriggeredBy = EDebugBreakpointTriggeredBy;
ptr_DebugVirtualToPhysical     DebugVirtualToPhysical = EDebugVirtualToPhysical;

// global variables

m64p_error AttachCoreLib(const char *CoreLibFilepath)
{
    /*
    // check if Core DLL is already attached 
    if (CoreHandle != NULL)
        return M64ERR_INVALID_STATE;

    // load the DLL
    m64p_error rval = M64ERR_INTERNAL;
    / first, try a library path+name that was given on the command-line
    if (CoreLibFilepath != NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, CoreLibFilepath);
    }
    // then try a library path that was given at compile time
#if defined(COREDIR)
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, COREDIR OSAL_DEFAULT_DYNLIB_FILENAME);
    }
#endif
    // for MacOS, look for the library in the Frameworks folder of the app bundle
#if defined(__APPLE__)
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle != NULL)
    {
        CFURLRef frameworksURL = CFBundleCopyPrivateFrameworksURL(mainBundle);
        if (frameworksURL != NULL)
        {
            char libPath[1024 + 32];
            if (CFURLGetFileSystemRepresentation(frameworksURL, TRUE, (uint8_t *) libPath, 1024))
            {
                strcat(libPath, "/" OSAL_DEFAULT_DYNLIB_FILENAME);
                rval = osal_dynlib_open(&CoreHandle, libPath);
            }
            CFRelease(frameworksURL);
        }
    }
#endif
    // then try just the filename of the shared library, to let dlopen() look through the system lib dirs
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, OSAL_DEFAULT_DYNLIB_FILENAME);
    }
    // as a last-ditch effort, try loading library in current directory
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        rval = osal_dynlib_open(&CoreHandle, OSAL_CURRENT_DIR OSAL_DEFAULT_DYNLIB_FILENAME);
    }
    // if we haven't found a good core library by now, then we're screwed
    if (rval != M64ERR_SUCCESS || CoreHandle == NULL)
    {
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: failed to find Mupen64Plus Core library");
        CoreHandle = NULL;
        return M64ERR_INPUT_NOT_FOUND;
    }

    // attach and call the PluginGetVersion function, check the Core and API versions for compatibility with this front-end
    ptr_PluginGetVersion CoreVersionFunc;
    CoreVersionFunc = (ptr_PluginGetVersion) osal_dynlib_getproc(CoreHandle, "PluginGetVersion");
    if (CoreVersionFunc == NULL)
    {
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: Shared library '%s' invalid; no PluginGetVersion() function found.", CoreLibFilepath);
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INPUT_INVALID;
    }
    m64p_plugin_type PluginType = (m64p_plugin_type) 0;
    int Compatible = 0;
    int CoreVersion = 0;
    const char *CoreName = NULL;
    (*CoreVersionFunc)(&PluginType, &CoreVersion, &g_CoreAPIVersion, &CoreName, &g_CoreCapabilities);
    if (PluginType != M64PLUGIN_CORE)
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: Shared library '%s' invalid; this is not the emulator core.", CoreLibFilepath);
    else if (CoreVersion < MINIMUM_CORE_VERSION)
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: Shared library '%s' incompatible; core version %i.%i.%i is below minimum supported %i.%i.%i",
                CoreLibFilepath, VERSION_PRINTF_SPLIT(CoreVersion), VERSION_PRINTF_SPLIT(MINIMUM_CORE_VERSION));
    else if ((g_CoreAPIVersion & 0xffff0000) != (CORE_API_VERSION & 0xffff0000))
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: Shared library '%s' incompatible; core API major version %i.%i.%i doesn't match with this application (%i.%i.%i)",
                CoreLibFilepath, VERSION_PRINTF_SPLIT(g_CoreAPIVersion), VERSION_PRINTF_SPLIT(CORE_API_VERSION));
    else
        Compatible = 1;
    // exit if not compatible
    if (Compatible == 0)
    {
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INCOMPATIBLE;
    }

    // attach and call the CoreGetAPIVersion function, check Config API version for compatibility
    ptr_CoreGetAPIVersions CoreAPIVersionFunc;
    CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) osal_dynlib_getproc(CoreHandle, "CoreGetAPIVersions");
    if (CoreAPIVersionFunc == NULL)
    {
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: Library '%s' broken; no CoreAPIVersionFunc() function found.", CoreLibFilepath);
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INPUT_INVALID;
    }
    int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;
    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000) || ConfigAPIVersion < CONFIG_API_VERSION)
    {
        UDebugMessage(M64MSG_ERROR, "AttachCoreLib() Error: Emulator core '%s' incompatible; Config API version %i.%i.%i doesn't match application: %i.%i.%i",
                CoreLibFilepath, VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        osal_dynlib_close(CoreHandle);
        CoreHandle = NULL;
        return M64ERR_INCOMPATIBLE;
    }

    // print some information about the core library
    UDebugMessage(M64MSG_INFO, "attached to core library '%s' version %i.%i.%i", CoreName, VERSION_PRINTF_SPLIT(CoreVersion));
    if (g_CoreCapabilities & M64CAPS_DYNAREC)
        UDebugMessage(M64MSG_INFO, "            Includes support for Dynamic Recompiler.");
    if (g_CoreCapabilities & M64CAPS_DEBUGGER)
        UDebugMessage(M64MSG_INFO, "            Includes support for MIPS r4300 Debugger.");
    if (g_CoreCapabilities & M64CAPS_CORE_COMPARE)
        UDebugMessage(M64MSG_INFO, "            Includes support for r4300 Core Comparison.");
    
    // get function pointers to the common and front-end functions
    CoreErrorMessage = (ptr_CoreErrorMessage) osal_dynlib_getproc(CoreHandle, "CoreErrorMessage");
    CoreStartup = (ptr_CoreStartup) osal_dynlib_getproc(CoreHandle, "CoreStartup");
    CoreShutdown = (ptr_CoreShutdown) osal_dynlib_getproc(CoreHandle, "CoreShutdown");
    CoreAttachPlugin = (ptr_CoreAttachPlugin) osal_dynlib_getproc(CoreHandle, "CoreAttachPlugin");
    CoreDetachPlugin = (ptr_CoreDetachPlugin) osal_dynlib_getproc(CoreHandle, "CoreDetachPlugin");
    CoreDoCommand = (ptr_CoreDoCommand) osal_dynlib_getproc(CoreHandle, "CoreDoCommand");
    CoreOverrideVidExt = (ptr_CoreOverrideVidExt) osal_dynlib_getproc(CoreHandle, "CoreOverrideVidExt");
    CoreAddCheat = (ptr_CoreAddCheat) osal_dynlib_getproc(CoreHandle, "CoreAddCheat");
    CoreCheatEnabled = (ptr_CoreCheatEnabled) osal_dynlib_getproc(CoreHandle, "CoreCheatEnabled");

    // get function pointers to the configuration functions
    ConfigListSections = (ptr_ConfigListSections) osal_dynlib_getproc(CoreHandle, "ConfigListSections");
    ConfigOpenSection = (ptr_ConfigOpenSection) osal_dynlib_getproc(CoreHandle, "ConfigOpenSection");
    ConfigDeleteSection = (ptr_ConfigDeleteSection) osal_dynlib_getproc(CoreHandle, "ConfigDeleteSection");
    ConfigSaveSection = (ptr_ConfigSaveSection) osal_dynlib_getproc(CoreHandle, "ConfigSaveSection");
    ConfigListParameters = (ptr_ConfigListParameters) osal_dynlib_getproc(CoreHandle, "ConfigListParameters");
    ConfigSaveFile = (ptr_ConfigSaveFile) osal_dynlib_getproc(CoreHandle, "ConfigSaveFile");
    ConfigSetParameter = (ptr_ConfigSetParameter) osal_dynlib_getproc(CoreHandle, "ConfigSetParameter");
    ConfigGetParameter = (ptr_ConfigGetParameter) osal_dynlib_getproc(CoreHandle, "ConfigGetParameter");
    ConfigGetParameterType = (ptr_ConfigGetParameterType) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterType");
    ConfigGetParameterHelp = (ptr_ConfigGetParameterHelp) osal_dynlib_getproc(CoreHandle, "ConfigGetParameterHelp");
    ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultInt");
    ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultFloat");
    ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultBool");
    ConfigSetDefaultString = (ptr_ConfigSetDefaultString) osal_dynlib_getproc(CoreHandle, "ConfigSetDefaultString");
    ConfigGetParamInt = (ptr_ConfigGetParamInt) osal_dynlib_getproc(CoreHandle, "ConfigGetParamInt");
    ConfigGetParamFloat = (ptr_ConfigGetParamFloat) osal_dynlib_getproc(CoreHandle, "ConfigGetParamFloat");
    ConfigGetParamBool = (ptr_ConfigGetParamBool) osal_dynlib_getproc(CoreHandle, "ConfigGetParamBool");
    ConfigGetParamString = (ptr_ConfigGetParamString) osal_dynlib_getproc(CoreHandle, "ConfigGetParamString");

    ConfigExternalOpen = (ptr_ConfigExternalOpen) osal_dynlib_getproc(CoreHandle, "ConfigExternalOpen");
    ConfigExternalClose = (ptr_ConfigExternalClose) osal_dynlib_getproc(CoreHandle, "ConfigExternalClose");
    ConfigExternalGetParameter = (ptr_ConfigExternalGetParameter) osal_dynlib_getproc(CoreHandle, "ConfigExternalGetParameter");
    ConfigHasUnsavedChanges = (ptr_ConfigHasUnsavedChanges) osal_dynlib_getproc(CoreHandle, "ConfigHasUnsavedChanges");

    ConfigGetSharedDataFilepath = (ptr_ConfigGetSharedDataFilepath) osal_dynlib_getproc(CoreHandle, "ConfigGetSharedDataFilepath");
    ConfigGetUserConfigPath = (ptr_ConfigGetUserConfigPath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserConfigPath");
    ConfigGetUserDataPath = (ptr_ConfigGetUserDataPath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserDataPath");
    ConfigGetUserCachePath = (ptr_ConfigGetUserCachePath) osal_dynlib_getproc(CoreHandle, "ConfigGetUserCachePath");

    // get function pointers to the debugger functions
    DebugSetCallbacks = (ptr_DebugSetCallbacks) osal_dynlib_getproc(CoreHandle, "DebugSetCallbacks");
    DebugSetCoreCompare = (ptr_DebugSetCoreCompare) osal_dynlib_getproc(CoreHandle, "DebugSetCoreCompare");
    DebugSetRunState = (ptr_DebugSetRunState) osal_dynlib_getproc(CoreHandle, "DebugSetRunState");
    DebugGetState = (ptr_DebugGetState) osal_dynlib_getproc(CoreHandle, "DebugGetState");
    DebugStep = (ptr_DebugStep) osal_dynlib_getproc(CoreHandle, "DebugStep");
    DebugDecodeOp = (ptr_DebugDecodeOp) osal_dynlib_getproc(CoreHandle, "DebugDecodeOp");
    DebugMemGetRecompInfo = (ptr_DebugMemGetRecompInfo) osal_dynlib_getproc(CoreHandle, "DebugMemGetRecompInfo");
    DebugMemGetMemInfo = (ptr_DebugMemGetMemInfo) osal_dynlib_getproc(CoreHandle, "DebugMemGetMemInfo");
    DebugMemGetPointer = (ptr_DebugMemGetPointer) osal_dynlib_getproc(CoreHandle, "DebugMemGetPointer");

    DebugMemRead64 = (ptr_DebugMemRead64) osal_dynlib_getproc(CoreHandle, "DebugMemRead64");
    DebugMemRead32 = (ptr_DebugMemRead32) osal_dynlib_getproc(CoreHandle, "DebugMemRead32");
    DebugMemRead16 = (ptr_DebugMemRead16) osal_dynlib_getproc(CoreHandle, "DebugMemRead16");
    DebugMemRead8 = (ptr_DebugMemRead8) osal_dynlib_getproc(CoreHandle, "DebugMemRead8");

    DebugMemWrite64 = (ptr_DebugMemWrite64) osal_dynlib_getproc(CoreHandle, "DebugMemWrite64");
    DebugMemWrite32 = (ptr_DebugMemWrite32) osal_dynlib_getproc(CoreHandle, "DebugMemWrite32");
    DebugMemWrite16 = (ptr_DebugMemWrite16) osal_dynlib_getproc(CoreHandle, "DebugMemWrite16");
    DebugMemWrite8 = (ptr_DebugMemWrite8) osal_dynlib_getproc(CoreHandle, "DebugMemWrite8");

    DebugGetCPUDataPtr = (ptr_DebugGetCPUDataPtr) osal_dynlib_getproc(CoreHandle, "DebugGetCPUDataPtr");
    DebugBreakpointLookup = (ptr_DebugBreakpointLookup) osal_dynlib_getproc(CoreHandle, "DebugBreakpointLookup");
    DebugBreakpointCommand = (ptr_DebugBreakpointCommand) osal_dynlib_getproc(CoreHandle, "DebugBreakpointCommand");

    DebugBreakpointTriggeredBy = (ptr_DebugBreakpointTriggeredBy) osal_dynlib_getproc(CoreHandle, "DebugBreakpointTriggeredBy");
    DebugVirtualToPhysical = (ptr_DebugVirtualToPhysical) osal_dynlib_getproc(CoreHandle, "DebugVirtualToPhysical");
    */
    return M64ERR_SUCCESS;
}

m64p_error DetachCoreLib(void)
{
    /*
    if (CoreHandle == NULL)
        return M64ERR_INVALID_STATE;

    // set the core function pointers to NULL
    CoreErrorMessage = NULL;
    CoreStartup = NULL;
    CoreShutdown = NULL;
    CoreAttachPlugin = NULL;
    CoreDetachPlugin = NULL;
    CoreDoCommand = NULL;
    CoreOverrideVidExt = NULL;
    CoreAddCheat = NULL;
    CoreCheatEnabled = NULL;

    ConfigListSections = NULL;
    ConfigOpenSection = NULL;
    ConfigDeleteSection = NULL;
    ConfigSaveSection = NULL;
    ConfigListParameters = NULL;
    ConfigSetParameter = NULL;
    ConfigGetParameter = NULL;
    ConfigGetParameterType = NULL;
    ConfigGetParameterHelp = NULL;
    ConfigSetDefaultInt = NULL;
    ConfigSetDefaultBool = NULL;
    ConfigSetDefaultString = NULL;
    ConfigGetParamInt = NULL;
    ConfigGetParamBool = NULL;
    ConfigGetParamString = NULL;

    ConfigExternalOpen = NULL;
    ConfigExternalClose = NULL;
    ConfigExternalGetParameter = NULL;
    ConfigHasUnsavedChanges = NULL;

    ConfigGetSharedDataFilepath = NULL;
    ConfigGetUserDataPath = NULL;
    ConfigGetUserCachePath = NULL;

    DebugSetCallbacks = NULL;
    DebugSetCoreCompare = NULL;
    DebugSetRunState = NULL;
    DebugGetState = NULL;
    DebugStep = NULL;
    DebugDecodeOp = NULL;
    DebugMemGetRecompInfo = NULL;
    DebugMemGetMemInfo = NULL;
    DebugMemGetPointer = NULL;

    DebugMemRead64 = NULL;
    DebugMemRead32 = NULL;
    DebugMemRead16 = NULL;
    DebugMemRead8 = NULL;

    DebugMemWrite64 = NULL;
    DebugMemWrite32 = NULL;
    DebugMemWrite16 = NULL;
    DebugMemWrite8 = NULL;

    DebugGetCPUDataPtr = NULL;
    DebugBreakpointLookup = NULL;
    DebugBreakpointCommand = NULL;

    DebugBreakpointTriggeredBy = NULL;
    DebugVirtualToPhysical = NULL;

    // detach the shared library
    osal_dynlib_close(CoreHandle);
    CoreHandle = NULL;
    */
    return M64ERR_SUCCESS;
}


