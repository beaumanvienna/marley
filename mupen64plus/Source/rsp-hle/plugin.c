/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - plugin.c                                        *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2014 Bobby Smiles                                       *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "hle.h"
#include "hle_internal.h"
#include "hle_external.h"

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_common.h"
#include "m64p_config.h"
#include "m64p_frontend.h"
#include "m64p_plugin.h"
#include "m64p_types.h"

#include "osal_dynamiclib.h"
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
m64p_error ECoreGetAPIVersions(int *, int *, int *, int *);
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
m64p_error ECoreDoCommand(m64p_command, int, void *);

#define CONFIG_API_VERSION       0x020100
#define CONFIG_PARAM_VERSION     1.00

#define RSP_API_VERSION   0x20000
#define RSP_HLE_VERSION        0x020509
#define RSP_PLUGIN_API_VERSION 0x020000

#define RSP_HLE_CONFIG_SECTION "Rsp-HLE"
#define RSP_HLE_CONFIG_VERSION "Version"
#define RSP_HLE_CONFIG_FALLBACK "RspFallback"
#define RSP_HLE_CONFIG_HLE_GFX  "DisplayListToGraphicsPlugin"
#define RSP_HLE_CONFIG_HLE_AUD  "AudioListToAudioPlugin"


#define VERSION_PRINTF_SPLIT(x) (((x) >> 16) & 0xffff), (((x) >> 8) & 0xff), ((x) & 0xff)

/* Handy macro to avoid code bloat when loading symbols */
#define GET_FUNC(type, field, name) \
    ((field = (type)osal_dynlib_getproc(handle, name)) != NULL)

/* local variables */
static struct hle_t g_hle;
static void (*l_CheckInterrupts)(void) = NULL;
static void (*l_ProcessDlistList)(void) = NULL;
static void (*l_ProcessAlistList)(void) = NULL;
static void (*l_ProcessRdpList)(void) = NULL;
static void (*l_ShowCFB)(void) = NULL;
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;
static m64p_dynlib_handle l_CoreHandle = NULL;
static int l_PluginInit = 0;

static m64p_handle l_ConfigRspHle;
static m64p_dynlib_handle l_RspFallback;
static ptr_InitiateRSP l_InitiateRSP = NULL;
static ptr_DoRspCycles l_DoRspCycles = NULL;
static ptr_RomClosed l_RomClosed = NULL;
static ptr_PluginShutdown l_PluginShutdown = NULL;

/* definitions of pointers to Core functions */
extern ptr_ConfigOpenSection      ConfigOpenSection;
extern ptr_ConfigDeleteSection    ConfigDeleteSection;
extern ptr_ConfigSetParameter     ConfigSetParameter;
extern ptr_ConfigGetParameter     ConfigGetParameter;
extern ptr_ConfigSetDefaultInt    ConfigSetDefaultInt;
extern ptr_ConfigSetDefaultFloat  ConfigSetDefaultFloat;
extern ptr_ConfigSetDefaultBool   ConfigSetDefaultBool;
extern ptr_ConfigSetDefaultString ConfigSetDefaultString;
extern ptr_ConfigGetParamInt      ConfigGetParamInt;
extern ptr_ConfigGetParamFloat    ConfigGetParamFloat;
extern ptr_ConfigGetParamBool     ConfigGetParamBool;
extern ptr_ConfigGetParamString   ConfigGetParamString;
extern ptr_CoreDoCommand          CoreDoCommand;

/* local function */
static void teardown_rsp_fallback()
{
    if (l_RspFallback != NULL) {
        (*l_PluginShutdown)();
        osal_dynlib_close(l_RspFallback);
    }

    l_RspFallback = NULL;
    l_DoRspCycles = NULL;
    l_InitiateRSP = NULL;
    l_RomClosed = NULL;
    l_PluginShutdown = NULL;
}

static void setup_rsp_fallback(const char* rsp_fallback_path)
{
    return;
}

static void SDebugMessage(int level, const char *message, va_list args)
{
    char msgbuf[1024];

    if (l_DebugCallback == NULL)
        return;

    vsprintf(msgbuf, message, args);

    (*l_DebugCallback)(l_DebugCallContext, level, msgbuf);
}

/* Global functions needed by HLE core */
void HleVerboseMessage(void* UNUSED(user_defined), const char *message, ...)
{
    va_list args;
    va_start(args, message);
    SDebugMessage(M64MSG_VERBOSE, message, args);
    va_end(args);
}

void HleInfoMessage(void* UNUSED(user_defined), const char *message, ...)
{
    va_list args;
    va_start(args, message);
    SDebugMessage(M64MSG_INFO, message, args);
    va_end(args);
}

void HleErrorMessage(void* UNUSED(user_defined), const char *message, ...)
{
    va_list args;
    va_start(args, message);
    SDebugMessage(M64MSG_ERROR, message, args);
    va_end(args);
}

void HleWarnMessage(void* UNUSED(user_defined), const char *message, ...)
{
    va_list args;
    va_start(args, message);
    SDebugMessage(M64MSG_WARNING, message, args);
    va_end(args);
}

void HleCheckInterrupts(void* UNUSED(user_defined))
{
    if (l_CheckInterrupts == NULL)
        return;

    (*l_CheckInterrupts)();
}

void HleProcessDlistList(void* UNUSED(user_defined))
{
    if (l_ProcessDlistList == NULL)
        return;

    (*l_ProcessDlistList)();
}

void HleProcessAlistList(void* UNUSED(user_defined))
{
    if (l_ProcessAlistList == NULL)
        return;

    (*l_ProcessAlistList)();
}

void HleProcessRdpList(void* UNUSED(user_defined))
{
    if (l_ProcessRdpList == NULL)
        return;

    (*l_ProcessRdpList)();
}

void HleShowCFB(void* UNUSED(user_defined))
{
    if (l_ShowCFB == NULL)
        return;

    (*l_ShowCFB)();
}


int HleForwardTask(void* user_defined)
{
    if (l_DoRspCycles == NULL)
        return -1;

    (*l_DoRspCycles)(-1);
    return 0;
}


/* DLL-exported functions */
m64p_error SPluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
                                     void (*DebugCallback)(void *, int, const char *))
{
    ptr_CoreGetAPIVersions CoreAPIVersionFunc;
    int ConfigAPIVersion, DebugAPIVersion, VidextAPIVersion;
    float fConfigParamsVersion = 0.0f;

    if (l_PluginInit)
        return M64ERR_ALREADY_INIT;

    /* first thing is to set the callback function for debug info */
    l_DebugCallback = DebugCallback;
    l_DebugCallContext = Context;

    /* attach and call the CoreGetAPIVersions function, check Config API version for compatibility */
    CoreAPIVersionFunc = (ptr_CoreGetAPIVersions) ECoreGetAPIVersions;
    if (CoreAPIVersionFunc == NULL)
    {
        HleErrorMessage(NULL, "Core emulator broken; no CoreAPIVersionFunc() function found.");
        return M64ERR_INCOMPATIBLE;
    }

    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000))
    {
        HleErrorMessage(NULL, "Emulator core Config API (v%i.%i.%i) incompatible with plugin (v%i.%i.%i)",
                VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        return M64ERR_INCOMPATIBLE;
    }

    /* Get the core config function pointers from the library handle */
    ConfigOpenSection = (ptr_ConfigOpenSection) EConfigOpenSection;
    ConfigDeleteSection = (ptr_ConfigDeleteSection) ConfigDeleteSection;
    ConfigSetParameter = (ptr_ConfigSetParameter) EConfigSetParameter;
    ConfigGetParameter = (ptr_ConfigGetParameter) EConfigGetParameter;
    ConfigSetDefaultInt = (ptr_ConfigSetDefaultInt) EConfigSetDefaultInt;
    ConfigSetDefaultFloat = (ptr_ConfigSetDefaultFloat) EConfigSetDefaultFloat;
    ConfigSetDefaultBool = (ptr_ConfigSetDefaultBool) EConfigSetDefaultBool;
    ConfigSetDefaultString = (ptr_ConfigSetDefaultString) EConfigSetDefaultString;
    ConfigGetParamInt = (ptr_ConfigGetParamInt) EConfigGetParamInt;
    ConfigGetParamFloat = (ptr_ConfigGetParamFloat) EConfigGetParamFloat;
    ConfigGetParamBool = (ptr_ConfigGetParamBool) EConfigGetParamBool;
    ConfigGetParamString = (ptr_ConfigGetParamString) EConfigGetParamString;

    if (!ConfigOpenSection || !ConfigDeleteSection || !ConfigSetParameter || !ConfigGetParameter ||
        !ConfigSetDefaultInt || !ConfigSetDefaultFloat || !ConfigSetDefaultBool || !ConfigSetDefaultString ||
        !ConfigGetParamInt   || !ConfigGetParamFloat   || !ConfigGetParamBool   || !ConfigGetParamString)
        return M64ERR_INCOMPATIBLE;

    /* Get core DoCommand function */
    CoreDoCommand = (ptr_CoreDoCommand) ECoreDoCommand;
    if (!CoreDoCommand) {
        return M64ERR_INCOMPATIBLE;
    }

    /* get a configuration section handle */
    if (ConfigOpenSection(RSP_HLE_CONFIG_SECTION, &l_ConfigRspHle) != M64ERR_SUCCESS)
    {
        HleErrorMessage(NULL, "Couldn't open config section '" RSP_HLE_CONFIG_SECTION "'");
        return M64ERR_INPUT_NOT_FOUND;
    }

    /* check the section version number */
    if (ConfigGetParameter(l_ConfigRspHle, RSP_HLE_CONFIG_VERSION, M64TYPE_FLOAT, &fConfigParamsVersion, sizeof(float)) != M64ERR_SUCCESS)
    {
        HleWarnMessage(NULL, "No version number in '" RSP_HLE_CONFIG_SECTION "' config section. Setting defaults.");
        ConfigDeleteSection(RSP_HLE_CONFIG_SECTION);
        ConfigOpenSection(RSP_HLE_CONFIG_SECTION, &l_ConfigRspHle);
    }
    else if (((int) fConfigParamsVersion) != ((int) CONFIG_PARAM_VERSION))
    {
        HleWarnMessage(NULL, "Incompatible version %.2f in '" RSP_HLE_CONFIG_SECTION "' config section: current is %.2f. Setting defaults.", fConfigParamsVersion, (float) CONFIG_PARAM_VERSION);
        ConfigDeleteSection(RSP_HLE_CONFIG_SECTION);
        ConfigOpenSection(RSP_HLE_CONFIG_SECTION, &l_ConfigRspHle);
    }
    else if ((CONFIG_PARAM_VERSION - fConfigParamsVersion) >= 0.0001f)
    {
        /* handle upgrades */
        float fVersion = CONFIG_PARAM_VERSION;
        ConfigSetParameter(l_ConfigRspHle, "Version", M64TYPE_FLOAT, &fVersion);
        HleInfoMessage(NULL, "Updating parameter set version in '" RSP_HLE_CONFIG_SECTION "' config section to %.2f", fVersion);
    }

    /* set the default values for this plugin */
    ConfigSetDefaultFloat(l_ConfigRspHle, RSP_HLE_CONFIG_VERSION, CONFIG_PARAM_VERSION,
        "Mupen64Plus RSP HLE Plugin config parameter version number");
    ConfigSetDefaultString(l_ConfigRspHle, RSP_HLE_CONFIG_FALLBACK, "",
        "Path to a RSP plugin which will be used when encountering an unknown ucode."
        "You can disable this by letting an empty string.");
    ConfigSetDefaultBool(l_ConfigRspHle, RSP_HLE_CONFIG_HLE_GFX, 1,
        "Send display lists to the graphics plugin");
    ConfigSetDefaultBool(l_ConfigRspHle, RSP_HLE_CONFIG_HLE_AUD, 0,
        "Send audio lists to the audio plugin");

    l_CoreHandle = CoreLibHandle;

    l_PluginInit = 1;
    return M64ERR_SUCCESS;
}

m64p_error SPluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    /* reset some local variable */
    l_DebugCallback = NULL;
    l_DebugCallContext = NULL;
    l_CoreHandle = NULL;

    teardown_rsp_fallback();

    l_PluginInit = 0;
    return M64ERR_SUCCESS;
}

m64p_error SPluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_RSP;

    if (PluginVersion != NULL)
        *PluginVersion = RSP_HLE_VERSION;

    if (APIVersion != NULL)
        *APIVersion = RSP_PLUGIN_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Hacktarux/Azimer High-Level Emulation RSP Plugin";

    if (Capabilities != NULL)
        *Capabilities = 0;

    return M64ERR_SUCCESS;
}

unsigned int SDoRspCycles(unsigned int Cycles)
{
    hle_execute(&g_hle);
    return Cycles;
}

void SInitiateRSP(RSP_INFO Rsp_Info, unsigned int* CycleCount)
{
    hle_init(&g_hle,
             Rsp_Info.RDRAM,
             Rsp_Info.DMEM,
             Rsp_Info.IMEM,
             Rsp_Info.MI_INTR_REG,
             Rsp_Info.SP_MEM_ADDR_REG,
             Rsp_Info.SP_DRAM_ADDR_REG,
             Rsp_Info.SP_RD_LEN_REG,
             Rsp_Info.SP_WR_LEN_REG,
             Rsp_Info.SP_STATUS_REG,
             Rsp_Info.SP_DMA_FULL_REG,
             Rsp_Info.SP_DMA_BUSY_REG,
             Rsp_Info.SP_PC_REG,
             Rsp_Info.SP_SEMAPHORE_REG,
             Rsp_Info.DPC_START_REG,
             Rsp_Info.DPC_END_REG,
             Rsp_Info.DPC_CURRENT_REG,
             Rsp_Info.DPC_STATUS_REG,
             Rsp_Info.DPC_CLOCK_REG,
             Rsp_Info.DPC_BUFBUSY_REG,
             Rsp_Info.DPC_PIPEBUSY_REG,
             Rsp_Info.DPC_TMEM_REG,
             NULL);

    l_CheckInterrupts = Rsp_Info.CheckInterrupts;
    l_ProcessDlistList = Rsp_Info.ProcessDlistList;
    l_ProcessAlistList = Rsp_Info.ProcessAlistList;
    l_ProcessRdpList = Rsp_Info.ProcessRdpList;
    l_ShowCFB = Rsp_Info.ShowCFB;

    setup_rsp_fallback(ConfigGetParamString(l_ConfigRspHle, RSP_HLE_CONFIG_FALLBACK));

    g_hle.hle_gfx = ConfigGetParamBool(l_ConfigRspHle, RSP_HLE_CONFIG_HLE_GFX);
    g_hle.hle_aud = ConfigGetParamBool(l_ConfigRspHle, RSP_HLE_CONFIG_HLE_AUD);

    /* notify fallback plugin */
    if (l_InitiateRSP) {
        l_InitiateRSP(Rsp_Info, CycleCount);
    }
}

void SRomClosed(void)
{
    g_hle.cached_ucodes.count = 0;

    /* notify fallback plugin */
    if (l_RomClosed) {
        l_RomClosed();
    }
}
