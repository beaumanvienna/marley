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
ptr_CoreErrorMessage    CoreErrorMessage;

/* definitions of pointers to Core front-end functions */
ptr_CoreStartup         CoreStartup;
ptr_CoreShutdown        CoreShutdown;
ptr_CoreAttachPlugin    CoreAttachPlugin;
ptr_CoreDetachPlugin    CoreDetachPlugin;
ptr_CoreDoCommand       CoreDoCommand;
ptr_CoreOverrideVidExt  CoreOverrideVidExt;
ptr_CoreAddCheat        CoreAddCheat;
ptr_CoreCheatEnabled    CoreCheatEnabled;

/* definitions of pointers to Core config functions */
ptr_ConfigListSections     ConfigListSections;
ptr_ConfigOpenSection      ConfigOpenSection;
ptr_ConfigDeleteSection    ConfigDeleteSection;
ptr_ConfigSaveSection      ConfigSaveSection;
ptr_ConfigListParameters   ConfigListParameters;
ptr_ConfigSaveFile         ConfigSaveFile;
ptr_ConfigSetParameter     ConfigSetParameter;
ptr_ConfigGetParameter     ConfigGetParameter;
ptr_ConfigGetParameterType ConfigGetParameterType;
ptr_ConfigGetParameterHelp ConfigGetParameterHelp;
ptr_ConfigSetDefaultInt    ConfigSetDefaultInt;
ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat;
ptr_ConfigSetDefaultBool   ConfigSetDefaultBool;
ptr_ConfigSetDefaultString ConfigSetDefaultString;
ptr_ConfigGetParamInt      ConfigGetParamInt;
ptr_ConfigGetParamFloat    ConfigGetParamFloat;
ptr_ConfigGetParamBool     ConfigGetParamBool;
ptr_ConfigGetParamString   ConfigGetParamString;

ptr_ConfigExternalOpen         ConfigExternalOpen;
ptr_ConfigExternalClose        ConfigExternalClose;
ptr_ConfigExternalGetParameter ConfigExternalGetParameter;
ptr_ConfigHasUnsavedChanges    ConfigHasUnsavedChanges;

ptr_ConfigGetSharedDataFilepath ConfigGetSharedDataFilepath;
ptr_ConfigGetUserConfigPath     ConfigGetUserConfigPath;
ptr_ConfigGetUserDataPath       ConfigGetUserDataPath;
ptr_ConfigGetUserCachePath      ConfigGetUserCachePath;

/* definitions of pointers to Core debugger functions */
ptr_DebugSetCallbacks      DebugSetCallbacks;
ptr_DebugSetCoreCompare    DebugSetCoreCompare;
ptr_DebugSetRunState       DebugSetRunState;
ptr_DebugGetState          DebugGetState;
ptr_DebugStep              DebugStep;
ptr_DebugDecodeOp          DebugDecodeOp;
ptr_DebugMemGetRecompInfo  DebugMemGetRecompInfo;
ptr_DebugMemGetMemInfo     DebugMemGetMemInfo;
ptr_DebugMemGetPointer     DebugMemGetPointer;

ptr_DebugMemRead64         DebugMemRead64;
ptr_DebugMemRead32         DebugMemRead32;
ptr_DebugMemRead16         DebugMemRead16;
ptr_DebugMemRead8          DebugMemRead8;

ptr_DebugMemWrite64        DebugMemWrite64;
ptr_DebugMemWrite32        DebugMemWrite32;
ptr_DebugMemWrite16        DebugMemWrite16;
ptr_DebugMemWrite8         DebugMemWrite8;

ptr_DebugGetCPUDataPtr     DebugGetCPUDataPtr;
ptr_DebugBreakpointLookup  DebugBreakpointLookup;
ptr_DebugBreakpointCommand DebugBreakpointCommand;

ptr_DebugBreakpointTriggeredBy DebugBreakpointTriggeredBy;
ptr_DebugVirtualToPhysical     DebugVirtualToPhysical;

// global variables

m64p_error AttachCoreLib(const char *CoreLibFilepath)
{
    
    // Core common functions
    CoreErrorMessage = ECoreErrorMessage;

    // Core front-end functions
    CoreStartup = ECoreStartup;
    CoreShutdown = ECoreShutdown;
    CoreAttachPlugin = ECoreAttachPlugin;
    CoreDetachPlugin = ECoreDetachPlugin;
    CoreDoCommand = ECoreDoCommand;
    CoreOverrideVidExt = ECoreOverrideVidExt;
    CoreAddCheat = ECoreAddCheat;
    CoreCheatEnabled = ECoreCheatEnabled;

    // Core config functions
    ConfigListSections = EConfigListSections;
    ConfigOpenSection = EConfigOpenSection;
    ConfigDeleteSection = EConfigDeleteSection;
    ConfigSaveSection = EConfigSaveSection;
    ConfigListParameters = EConfigListParameters;
    ConfigSaveFile = EConfigSaveFile;
    ConfigSetParameter = EConfigSetParameter;
    ConfigGetParameter = EConfigGetParameter;
    ConfigGetParameterType = EConfigGetParameterType;
    ConfigGetParameterHelp = EConfigGetParameterHelp;
    ConfigSetDefaultInt = EConfigSetDefaultInt;
    ConfigSetDefaultFloat = EConfigSetDefaultFloat;
    ConfigSetDefaultBool = EConfigSetDefaultBool;
    ConfigSetDefaultString = EConfigSetDefaultString;
    ConfigGetParamInt = EConfigGetParamInt;
    ConfigGetParamFloat = EConfigGetParamFloat;
    ConfigGetParamBool = EConfigGetParamBool;
    ConfigGetParamString = EConfigGetParamString;

    ConfigExternalOpen = EConfigExternalOpen;
    ConfigExternalClose = EConfigExternalClose;
    ConfigExternalGetParameter = EConfigExternalGetParameter;
    ConfigHasUnsavedChanges = EConfigHasUnsavedChanges;

    ConfigGetSharedDataFilepath = EConfigGetSharedDataFilepath;
    ConfigGetUserConfigPath = EConfigGetUserConfigPath;
    ConfigGetUserDataPath = EConfigGetUserDataPath;
    ConfigGetUserCachePath = EConfigGetUserCachePath;

    // Core debugger functions
    DebugSetCallbacks = EDebugSetCallbacks;
    DebugSetCoreCompare = EDebugSetCoreCompare;
    DebugSetRunState = EDebugSetRunState;
    DebugGetState = EDebugGetState;
    DebugStep = EDebugStep;
    DebugDecodeOp = EDebugDecodeOp;
    DebugMemGetRecompInfo = EDebugMemGetRecompInfo;
    DebugMemGetMemInfo = EDebugMemGetMemInfo;
    DebugMemGetPointer = EDebugMemGetPointer;

    DebugMemRead64 = EDebugMemRead64;
    DebugMemRead32 = EDebugMemRead32;
    DebugMemRead16 = EDebugMemRead16;
    DebugMemRead8 = EDebugMemRead8;

    DebugMemWrite64 = EDebugMemWrite64;
    DebugMemWrite32 = EDebugMemWrite32;
    DebugMemWrite16 = EDebugMemWrite16;
    DebugMemWrite8 = EDebugMemWrite8;

    DebugGetCPUDataPtr = EDebugGetCPUDataPtr;
    DebugBreakpointLookup = EDebugBreakpointLookup;
    DebugBreakpointCommand = EDebugBreakpointCommand;

    DebugBreakpointTriggeredBy = EDebugBreakpointTriggeredBy;
    DebugVirtualToPhysical = EDebugVirtualToPhysical;
    
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


