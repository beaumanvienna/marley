/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-sdl-audio - main.c                                        *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2007-2009 Richard Goedeken                              *
 *   Copyright (C) 2007-2008 Ebenblues                                     *
 *   Copyright (C) 2003 JttL                                               *
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

#include <SDL.h>
#include <SDL_audio.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"
#include "sdl_backend.h"
#include "volume.h"
#include "resamplers/resamplers.h"

#ifdef USE_AUDIORESOURCE
#include <audioresource.h>
#include <glib.h>
#endif

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_common.h"
#include "m64p_config.h"
#include "m64p_plugin.h"
#include "m64p_types.h"

/* version info */
#define SDL_AUDIO_PLUGIN_VERSION 0x020509
#define AUDIO_PLUGIN_API_VERSION 0x020000
#define CONFIG_API_VERSION       0x020100
#define CONFIG_PARAM_VERSION     1.00

#define VERSION_PRINTF_SPLIT(x) (((x) >> 16) & 0xffff), (((x) >> 8) & 0xff), ((x) & 0xff)

/* Default start-time size of primary buffer (in equivalent output samples).
   This is the buffer where audio is loaded after it's extracted from n64's memory.
   This value must be larger than PRIMARY_BUFFER_TARGET */
#define PRIMARY_BUFFER_SIZE 16384

/* this is the buffer fullness level (in equivalent output samples) which is targeted
   for the primary audio buffer (by inserting delays) each time data is received from
   the running N64 program.  This value must be larger than the SECONDARY_BUFFER_SIZE.
   Decreasing this value will reduce audio latency but requires a faster PC to avoid
   choppiness. Increasing this will increase audio latency but reduce the chance of
   drop-outs. The default value 2048 gives a 46ms maximum A/V delay at 44.1khz */
#define PRIMARY_BUFFER_TARGET 2048

/* Size of secondary buffer, in output samples. This is the requested size of SDL's
   hardware buffer, and the size of the mix buffer for doing SDL volume control. The
   SDL documentation states that this should be a power of two between 512 and 8192. */
#define SECONDARY_BUFFER_SIZE 1024

/* This sets default frequency what is used if rom doesn't want to change it.
   Probably only game that needs this is Zelda: Ocarina Of Time Master Quest
   *NOTICE* We should try to find out why Demos' frequencies are always wrong
   They tend to rely on a default frequency, apparently, never the same one ;)*/
#define DEFAULT_FREQUENCY 33600

#if SDL_VERSION_ATLEAST(2,0,0)
#define SDL_MixAudio(A, B, C, D) SDL_MixAudioFormat(A, B, AUDIO_S16SYS, C, D)
#endif

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

/* local variables */
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;
static int l_PluginInit = 0;
static m64p_handle l_ConfigAudio;
#ifdef USE_AUDIORESOURCE
static audioresource_t *l_audioresource = NULL;
static int l_audioresource_acquired = 0;
#endif

static struct sdl_backend* l_sdl_backend = NULL;

/* Read header for type definition */
static AUDIO_INFO AudioInfo;
// volume to scale the audio by, range of 0..100
// if muted, this holds the volume when not muted
static int VolPercent = 80;
// how much percent to increment/decrement volume by
static int VolDelta = 5;
// the actual volume passed into SDL, range of 0..SDL_MIX_MAXVOLUME
static int VolSDL = SDL_MIX_MAXVOLUME;
// Muted or not
static int VolIsMuted = 0;
//which type of volume control to use
static int VolumeControlType = VOLUME_TYPE_SDL;

/* definitions of pointers to Core config functions */
extern ptr_ConfigOpenSection      ConfigOpenSection;
extern ptr_ConfigDeleteSection    ConfigDeleteSection;
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

/* Global functions */
void ADebugMessage(int level, const char *message, ...)
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

#ifdef USE_AUDIORESOURCE
void on_audioresource_acquired(audioresource_t *audioresource, bool acquired, void *user_data)
{
    ADebugMessage(M64MSG_VERBOSE, "audioresource acquired: %d", acquired);
    l_audioresource_acquired = acquired;
}
#endif

/* Mupen64Plus plugin functions */
m64p_error APluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
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
        ADebugMessage(M64MSG_ERROR, "Core emulator broken; no CoreAPIVersionFunc() function found.");
        return M64ERR_INCOMPATIBLE;
    }

    (*CoreAPIVersionFunc)(&ConfigAPIVersion, &DebugAPIVersion, &VidextAPIVersion, NULL);
    if ((ConfigAPIVersion & 0xffff0000) != (CONFIG_API_VERSION & 0xffff0000))
    {
        ADebugMessage(M64MSG_ERROR, "Emulator core Config API (v%i.%i.%i) incompatible with plugin (v%i.%i.%i)",
                VERSION_PRINTF_SPLIT(ConfigAPIVersion), VERSION_PRINTF_SPLIT(CONFIG_API_VERSION));
        return M64ERR_INCOMPATIBLE;
    }

    /* Get the core config function pointers from the library handle */
    ConfigOpenSection = (ptr_ConfigOpenSection) EConfigOpenSection;
    ConfigDeleteSection = (ptr_ConfigDeleteSection) EConfigDeleteSection;
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

    /* get a configuration section handle */
    if (ConfigOpenSection("Audio-SDL", &l_ConfigAudio) != M64ERR_SUCCESS)
    {
        ADebugMessage(M64MSG_ERROR, "Couldn't open config section 'Audio-SDL'");
        return M64ERR_INPUT_NOT_FOUND;
    }

    /* check the section version number */
    if (ConfigGetParameter(l_ConfigAudio, "Version", M64TYPE_FLOAT, &fConfigParamsVersion, sizeof(float)) != M64ERR_SUCCESS)
    {
        ADebugMessage(M64MSG_WARNING, "No version number in 'Audio-SDL' config section. Setting defaults.");
        ConfigDeleteSection("Audio-SDL");
        ConfigOpenSection("Audio-SDL", &l_ConfigAudio);
    }
    else if (((int) fConfigParamsVersion) != ((int) CONFIG_PARAM_VERSION))
    {
        ADebugMessage(M64MSG_WARNING, "Incompatible version %.2f in 'Audio-SDL' config section: current is %.2f. Setting defaults.", fConfigParamsVersion, (float) CONFIG_PARAM_VERSION);
        ConfigDeleteSection("Audio-SDL");
        ConfigOpenSection("Audio-SDL", &l_ConfigAudio);
    }
    else if ((CONFIG_PARAM_VERSION - fConfigParamsVersion) >= 0.0001f)
    {
        /* handle upgrades */
        float fVersion = CONFIG_PARAM_VERSION;
        ConfigSetParameter(l_ConfigAudio, "Version", M64TYPE_FLOAT, &fVersion);
        ADebugMessage(M64MSG_INFO, "Updating parameter set version in 'Audio-SDL' config section to %.2f", fVersion);
    }

    /* set the default values for this plugin */
    ConfigSetDefaultFloat(l_ConfigAudio, "Version",             CONFIG_PARAM_VERSION,  "Mupen64Plus SDL Audio Plugin config parameter version number");
    ConfigSetDefaultInt(l_ConfigAudio, "DEFAULT_FREQUENCY",     DEFAULT_FREQUENCY,     "Frequency which is used if rom doesn't want to change it");
    ConfigSetDefaultBool(l_ConfigAudio, "SWAP_CHANNELS",        0,                     "Swaps left and right channels");
    ConfigSetDefaultInt(l_ConfigAudio, "PRIMARY_BUFFER_SIZE",   PRIMARY_BUFFER_SIZE,   "Size of primary buffer in output samples. This is where audio is loaded after it's extracted from n64's memory.");
    ConfigSetDefaultInt(l_ConfigAudio, "PRIMARY_BUFFER_TARGET", PRIMARY_BUFFER_TARGET, "Fullness level target for Primary audio buffer, in equivalent output samples. This value must be larger than the SECONDARY_BUFFER_SIZE. Decreasing this value will reduce audio latency but requires a faster PC to avoid choppiness. Increasing this will increase audio latency but reduce the chance of drop-outs.");
    ConfigSetDefaultInt(l_ConfigAudio, "SECONDARY_BUFFER_SIZE", SECONDARY_BUFFER_SIZE, "Size of secondary buffer in output samples. This is SDL's hardware buffer. The SDL documentation states that this should be a power of two between 512 and 8192.");
    ConfigSetDefaultString(l_ConfigAudio, "RESAMPLE",           DEFAULT_RESAMPLER,             "Audio resampling algorithm. src-sinc-best-quality, src-sinc-medium-quality, src-sinc-fastest, src-zero-order-hold, src-linear, speex-fixed-{10-0}, trivial");
    ConfigSetDefaultInt(l_ConfigAudio, "VOLUME_CONTROL_TYPE",   VOLUME_TYPE_SDL,       "Volume control type: 1 = SDL (only affects Mupen64Plus output)  2 = OSS mixer (adjusts master PC volume)");
    ConfigSetDefaultInt(l_ConfigAudio, "VOLUME_ADJUST",         5,                     "Percentage change each time the volume is increased or decreased");
    ConfigSetDefaultInt(l_ConfigAudio, "VOLUME_DEFAULT",        80,                    "Default volume when a game is started.  Only used if VOLUME_CONTROL_TYPE is 1");
    ConfigSetDefaultBool(l_ConfigAudio, "AUDIO_SYNC",           1,                     "Synchronize Video/Audio");

#ifdef USE_AUDIORESOURCE
    setenv("PULSE_PROP_media.role", "x-maemo", 1);

    l_audioresource = audioresource_init(AUDIO_RESOURCE_GAME, on_audioresource_acquired, NULL);

    audioresource_acquire(l_audioresource);

    while(!l_audioresource_acquired)
    {
        ADebugMessage(M64MSG_INFO, "Waiting for audio resource...");
        g_main_context_iteration(NULL, false);
    }
#endif

    l_PluginInit = 1;
    return M64ERR_SUCCESS;
}

m64p_error APluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    /* reset some local variables */
    l_DebugCallback = NULL;
    l_DebugCallContext = NULL;

#ifdef USE_AUDIORESOURCE
    audioresource_release(l_audioresource);
    audioresource_free(l_audioresource);
#endif

    l_PluginInit = 0;
    return M64ERR_SUCCESS;
}

m64p_error APluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_AUDIO;

    if (PluginVersion != NULL)
        *PluginVersion = SDL_AUDIO_PLUGIN_VERSION;

    if (APIVersion != NULL)
        *APIVersion = AUDIO_PLUGIN_API_VERSION;

    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Mupen64Plus SDL Audio Plugin";

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }

    return M64ERR_SUCCESS;
}

/* ----------- Audio Functions ------------- */
static unsigned int vi_clock_from_system_type(int system_type)
{
    switch (system_type)
    {
    default:
        ADebugMessage(M64MSG_WARNING, "Invalid system_type %d. Assuming NTSC", system_type);
        /* fallback */
    case SYSTEM_NTSC: return 48681812;
    case SYSTEM_PAL:  return 49656530;
    case SYSTEM_MPAL: return 48628316;
    }
}

static unsigned int dacrate2freq(unsigned int vi_clock, uint32_t dacrate)
{
    return vi_clock / (dacrate + 1);
}

void AAiDacrateChanged(int SystemType)
{
    if (!l_PluginInit || l_sdl_backend == NULL)
        return;

    unsigned int frequency = dacrate2freq(vi_clock_from_system_type(SystemType), *AudioInfo.AI_DACRATE_REG);
    unsigned int bits = 1 + (*AudioInfo.AI_BITRATE_REG);

    sdl_set_format(l_sdl_backend, frequency, bits);
}

void AAiLenChanged(void)
{
    if (!l_PluginInit || l_sdl_backend == NULL)
        return;

    sdl_push_samples(l_sdl_backend, AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0xffffff), *AudioInfo.AI_LEN_REG);

    sdl_synchronize_audio(l_sdl_backend);
}

int AInitiateAudio(AUDIO_INFO Audio_Info)
{
    if (!l_PluginInit)
        return 0;

    AudioInfo = Audio_Info;

    return 1;
}

int ARomOpen(void)
{
    if (!l_PluginInit || l_sdl_backend != NULL)
        return 0;

    VolDelta = ConfigGetParamInt(l_ConfigAudio, "VOLUME_ADJUST");
    VolumeControlType = ConfigGetParamInt(l_ConfigAudio, "VOLUME_CONTROL_TYPE");
    VolPercent = ConfigGetParamInt(l_ConfigAudio, "VOLUME_DEFAULT");

    l_sdl_backend = init_sdl_backend_from_config(l_ConfigAudio);

    return 1;
}

void ARomClosed(void)
{
    if (!l_PluginInit)
        return;

    release_sdl_backend(l_sdl_backend);
    l_sdl_backend = NULL;
}

void AProcessAList(void)
{
}

void ASetSpeedFactor(int percentage)
{
    if (!l_PluginInit || l_sdl_backend == NULL)
        return;

    sdl_set_speed_factor(l_sdl_backend, percentage);
}

size_t ResampleAndMix(void* resampler, const struct resampler_interface* iresampler,
        void* mix_buffer,
        const void* src, size_t src_size, unsigned int src_freq,
        void* dst, size_t dst_size, unsigned int dst_freq)
{
    size_t consumed;

#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        consumed = iresampler->resample(resampler, src, src_size, src_freq, dst, dst_size, dst_freq);
    }
    else
#endif
    {
        consumed = iresampler->resample(resampler, src, src_size, src_freq, mix_buffer, dst_size, dst_freq);
        memset(dst, 0, dst_size);
        SDL_MixAudio(dst, mix_buffer, dst_size, VolSDL);
    }

    return consumed;
}

void SetPlaybackVolume(void)
{
#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        VolPercent = volGet();
    }
    else
#endif
    {
        VolSDL = SDL_MIX_MAXVOLUME * VolPercent / 100;
    }
}


// Returns the most recent ummuted volume level.
static int VolumeGetUnmutedLevel(void)
{
#if defined(HAS_OSS_SUPPORT)
    // reload volume if we're using OSS
    if (!VolIsMuted && VolumeControlType == VOLUME_TYPE_OSS)
    {
        return volGet();
    }
#endif

    return VolPercent;
}

// Sets the volume level based on the contents of VolPercent and VolIsMuted
static void VolumeCommit(void)
{
    int levelToCommit = VolIsMuted ? 0 : VolPercent;

#if defined(HAS_OSS_SUPPORT)
    if (VolumeControlType == VOLUME_TYPE_OSS)
    {
        //OSS mixer volume
        volSet(levelToCommit);
    }
    else
#endif
    {
        VolSDL = SDL_MIX_MAXVOLUME * levelToCommit / 100;
    }
}

void AVolumeMute(void)
{
    if (!l_PluginInit)
        return;

    // Store the volume level in order to restore it later
    if (!VolIsMuted)
        VolPercent = VolumeGetUnmutedLevel();

    // Toogle mute
    VolIsMuted = !VolIsMuted;
    VolumeCommit();
}
void AVolumeSetLevel(int level);
void AVolumeUp(void)
{
    if (!l_PluginInit)
        return;

    AVolumeSetLevel(VolumeGetUnmutedLevel() + VolDelta);
}

void AVolumeDown(void)
{
    if (!l_PluginInit)
        return;

    AVolumeSetLevel(VolumeGetUnmutedLevel() - VolDelta);
}

int AVolumeGetLevel(void)
{
    return VolIsMuted ? 0 : VolumeGetUnmutedLevel();
}

void AVolumeSetLevel(int level)
{
    if (!l_PluginInit)
        return;

    //if muted, unmute first
    VolIsMuted = 0;

    // adjust volume
    VolPercent = level;
    if (VolPercent < 0)
        VolPercent = 0;
    else if (VolPercent > 100)
        VolPercent = 100;

    VolumeCommit();
}

const char * AVolumeGetString(void)
{
    static char VolumeString[32];

    if (VolIsMuted)
    {
        strcpy(VolumeString, "Mute");
    }
    else
    {
        sprintf(VolumeString, "%i%%", VolPercent);
    }

    return VolumeString;
}

