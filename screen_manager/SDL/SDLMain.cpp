// SDL/EGL implementation of the framework.
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <pwd.h>

#include "ppsspp_config.h"
#include "SDL.h"
#include "SDL/SDLJoystick.h"
SCREEN_SDLJoystick *SCREEN_joystick = NULL;

#if PPSSPP_PLATFORM(RPI)
#include <bcm_host.h>
#endif

#include <atomic>
#include <algorithm>
#include <cmath>
#include <thread>
#include <locale>

#include "Common/System/Display.h"
#include "Common/System/System.h"
#include "Common/System/NativeApp.h"
#include "Common/Data/Format/PNGLoad.h"
#include "NKCodeFromSDL.h"
#include "Common/Math/math_util.h"
#include "Common/GPU/OpenGL/GLRenderManager.h"

#include "SDL_syswm.h"

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>
#endif

#include "Common/GraphicsContext.h"
#include "Common/TimeUtil.h"
#include "Common/Input/InputState.h"
#include "Common/Input/KeyCodes.h"
#include "Common/Data/Collections/ConstMap.h"
#include "Common/Data/Encoding/Utf8.h"
#include "Common/Thread/ThreadUtil.h"
#include "Common/KeyMap.h"
#include "SDLGLGraphicsContext.h"
#include "../../include/gui.h"
#include "../../include/controller.h"
#include "../include/statemachine.h"
#include "../include/log.h"

void mainLoopWii(void);
void SCREEN_NativeResized(void);
extern bool restart_screen_manager;
extern bool shutdown_now;
extern bool launch_request_from_screen_manager;
extern bool gUpdateCurrentScreen;
extern int window_x,window_y;

GlobalUIState SCREEN_lastUIState = UISTATE_MENU;
GlobalUIState GetUIState();
GlobalUIState globalUIState;

extern bool gFullscreen;

static bool SCREEN_g_ToggleFullScreenNextFrame = false;
static int SCREEN_g_ToggleFullScreenType;
static int SCREEN_g_QuitRequested = 0;

static int SCREEN_g_DesktopWidth = 0;
static int SCREEN_g_DesktopHeight = 0;
static float SCREEN_g_RefreshRate = 60.f;

static SDL_AudioSpec g_retFmt;

int SCREEN_getDisplayNumber(void) {
    int displayNumber = 0;
    char * displayNumberStr;

    //get environment
    displayNumberStr=getenv("SDL_VIDEO_FULLSCREEN_HEAD");

    if (displayNumberStr) {
        displayNumber = atoi(displayNumberStr);
    }

    return displayNumber;
}

void SCREEN_sdl_mixaudio_callback(void *userdata, Uint8 *stream, int len) {
    NativeMix((short *)stream, len / (2 * 2));
}

static SDL_AudioDeviceID SCREEN_audioDev = 0;

// Must be called after NativeInit().
static void SCREEN_InitSDLAudioDevice(const std::string &name = "") {
    SDL_AudioSpec fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.freq = 44100;
    fmt.format = AUDIO_S16;
    fmt.channels = 2;
    fmt.samples = 1024;
    fmt.callback = &SCREEN_sdl_mixaudio_callback;
    fmt.userdata = nullptr;

    std::string startDevice = name;

    SCREEN_audioDev = 0;

    SDL_Init( SDL_INIT_AUDIO );

    if (!startDevice.empty()) {
        SCREEN_audioDev = SDL_OpenAudioDevice(startDevice.c_str(), 0, &fmt, &g_retFmt, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        if (SCREEN_audioDev <= 0) {
            printf("Failed to open audio device: %s\n", startDevice.c_str());
        }
    }
    if (SCREEN_audioDev <= 0) {
        if (SCREEN_audioDev < 0) printf("SDL: Trying a different audio device\n");
        SCREEN_audioDev = SDL_OpenAudioDevice(nullptr, 0, &fmt, &g_retFmt, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    }
    if (SCREEN_audioDev <= 0) {
        printf("Failed to open audio device: %s\n", SDL_GetError());
    } else {
        if (g_retFmt.samples != fmt.samples) // Notify, but still use it
            printf("Output audio samples: %d (requested: %d)\n", g_retFmt.samples, fmt.samples);
        if (g_retFmt.format != fmt.format || g_retFmt.channels != fmt.channels) {
            printf("Sound buffer format does not match requested format.\n");
            printf("Output audio freq: %d (requested: %d)\n", g_retFmt.freq, fmt.freq);
            printf("Output audio format: %d (requested: %d)\n", g_retFmt.format, fmt.format);
            printf("Output audio channels: %d (requested: %d)\n", g_retFmt.channels, fmt.channels);
            printf("Provided output format does not match requirement, turning audio off\n");
            SDL_CloseAudioDevice(SCREEN_audioDev);
        }
        SDL_PauseAudioDevice(SCREEN_audioDev, 0);
    }
}

static void SCREEN_StopSDLAudioDevice() {
    if (SCREEN_audioDev > 0) {
        SDL_PauseAudioDevice(SCREEN_audioDev, 1);
        SDL_CloseAudioDevice(SCREEN_audioDev);
    }
}

void SCREEN_SystemToast(const char *text) {
    puts(text);
}

void SCREEN_ShowKeyboard() {
    // Irrelevant on PC
}

void SCREEN_Vibrate(int length_ms) {
    // Ignore on PC
}

void SCREEN_System_SendMessage(const char *command, const char *parameter) {
    if (!strcmp(command, "toggle_fullscreen")) {
        SCREEN_g_ToggleFullScreenNextFrame = true;
        if (strcmp(parameter, "1") == 0) {
            SCREEN_g_ToggleFullScreenType = 1;
        } else if (strcmp(parameter, "0") == 0) {
            SCREEN_g_ToggleFullScreenType = 0;
        } else {
            // Just toggle.
            SCREEN_g_ToggleFullScreenType = -1;
        }
    } else if (!strcmp(command, "finish")) {
        // Do a clean exit
        SCREEN_g_QuitRequested = true;
    } else if (!strcmp(command, "graphics_restart")) {
        // Not sure how we best do this, but do a clean exit, better than being stuck in a bad state.
        SCREEN_g_QuitRequested = true;
    } else if (!strcmp(command, "setclipboardtext")) {
        SDL_SetClipboardText(parameter);
    } else if (!strcmp(command, "audio_resetDevice")) {
        SCREEN_StopSDLAudioDevice();
        SCREEN_InitSDLAudioDevice();
    }
}

void SCREEN_System_AskForPermission(SystemPermission permission) {}
PermissionStatus SCREEN_System_GetPermissionStatus(SystemPermission permission) { return PERMISSION_STATUS_GRANTED; }

void SCREEN_OpenDirectory(const char *path) {

}

std::string SCREEN_System_GetProperty(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_NAME:
#ifdef _WIN32
        return "SDL:Windows";
#elif __linux__
        return "SDL:Linux";
#elif __APPLE__
        return "SDL:macOS";
#elif PPSSPP_PLATFORM(SWITCH)
        return "SDL:Horizon";
#else
        return "SDL:";
#endif
    case SYSPROP_LANGREGION: {
        // Get user-preferred locale from OS
        setlocale(LC_ALL, "");
        std::string locale(setlocale(LC_ALL, NULL));
        // Set c and c++ strings back to POSIX
        std::locale::global(std::locale("POSIX"));
        if (!locale.empty()) {
            // Technically, this is an opaque string, but try to find the locale code.
            size_t messagesPos = locale.find("LC_MESSAGES=");
            if (messagesPos != std::string::npos) {
                messagesPos += strlen("LC_MESSAGES=");
                size_t semi = locale.find(';', messagesPos);
                locale = locale.substr(messagesPos, semi - messagesPos);
            }

            if (locale.find("_", 0) != std::string::npos) {
                if (locale.find(".", 0) != std::string::npos) {
                    return locale.substr(0, locale.find(".",0));
                }
                return locale;
            }
        }
        return "en_US";
    }
    case SYSPROP_CLIPBOARD_TEXT:
        return SDL_HasClipboardText() ? SDL_GetClipboardText() : "";
    case SYSPROP_AUDIO_DEVICE_LIST:
        {
            std::string result;
            int availableDevices = 0;

            for (int i = 0; i < SDL_GetNumAudioDevices(0); ++i) {
                const char *name = SDL_GetAudioDeviceName(i, 0);

                if (!name) {
                    continue;
                }

                std::string command = "pacmd list-cards"; 
                std::string data;
                FILE * stream;
                const int MAX_BUFFER = 65535;
                char buffer[MAX_BUFFER];

                stream = popen(command.c_str(), "r");
                if (stream) {
                    while (!feof(stream))
                        if (fgets(buffer, MAX_BUFFER, stream) != nullptr) data.append(buffer);
                    pclose(stream);
                }
                
                std::istringstream iss(data);

                for (std::string line; std::getline(iss, line); )
                {
                    
                    if (line.find("output:") != std::string::npos) 
                    {

                        line = line.substr(line.find_first_not_of(" \t"));
                        if ( (line.find("output:") == 0) && (line.find("input") == std::string::npos) && (line.find("extra") == std::string::npos) )
                        {
                            line = line.substr(7);
                            line = line.substr(0,line.find(":"));
                            
                            availableDevices++;
                            line = std::string(name) + " (" + line + ")";
                            if (availableDevices == 1) {
                                result = line;
                            } else {
                                result.append(1, '\0');
                                result.append(line);
                            }
                        }
                    }
                }
            }
            return result;
        }
    default:
        return "";
    }
}

int SCREEN_System_GetPropertyInt(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_AUDIO_SAMPLE_RATE:
        return g_retFmt.freq;
    case SYSPROP_AUDIO_FRAMES_PER_BUFFER:
        return g_retFmt.samples;
    case SYSPROP_DEVICE_TYPE:
        return DEVICE_TYPE_DESKTOP;
    case SYSPROP_DISPLAY_COUNT:
        return SDL_GetNumVideoDisplays();
    default:
        return -1;
    }
}

float SCREEN_System_GetPropertyFloat(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_DISPLAY_REFRESH_RATE:
        return SCREEN_g_RefreshRate;
    case SYSPROP_DISPLAY_SAFE_INSET_LEFT:
    case SYSPROP_DISPLAY_SAFE_INSET_RIGHT:
    case SYSPROP_DISPLAY_SAFE_INSET_TOP:
    case SYSPROP_DISPLAY_SAFE_INSET_BOTTOM:
        return 0.0f;
    default:
        return -1;
    }
}

bool SCREEN_System_GetPropertyBool(SystemProperty prop) {
    switch (prop) {
    case SYSPROP_HAS_BACK_BUTTON:
        return true;
    default:
        return false;
    }
}

// returns -1 on failure
static int parseInt(const char *str) {
    int val;
    int retval = sscanf(str, "%d", &val);
    printf("%i = scanf %s\n", retval, str);
    if (retval != 1) {
        return -1;
    } else {
        return val;
    }
}

static float parseFloat(const char *str) {
    float val;
    int retval = sscanf(str, "%f", &val);
    printf("%i = sscanf %s\n", retval, str);
    if (retval != 1) {
        return -1.0f;
    } else {
        return val;
    }
}

void SCREEN_UpdateScreenScale(int width, int height) {
    dp_xres = width;
    dp_yres = height;
    pixel_xres = width;
    pixel_yres = height;
}


void SCREEN_ToggleFullScreen(void) {
    DEBUG_PRINTF("   void SCREEN_ToggleFullScreen(void)\n");

    Uint32 window_flags = SDL_GetWindowFlags(gWindow);
    
    // save position if not in fullscreen
    if (!(window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) && !(window_flags & SDL_WINDOW_FULLSCREEN))
    {
        SDL_GetWindowSize(gWindow,&window_width,&window_height);
        SDL_GetWindowPosition(gWindow,&window_x,&window_y);
    }
    // toggle
    window_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
    
    SDL_SetWindowFullscreen(gWindow, window_flags);
    if (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) 
    {
        SCREEN_UpdateScreenScale(SCREEN_g_DesktopWidth,SCREEN_g_DesktopHeight);
    } else 
    {
        SDL_SetWindowPosition(gWindow,window_x,window_y-37);
        SCREEN_UpdateScreenScale(WINDOW_WIDTH,WINDOW_HEIGHT);
    }
    SCREEN_NativeResized();
}

void SCREEN_ToggleFullScreenIfFlagSet(SDL_Window *window) {
    if (SCREEN_g_ToggleFullScreenNextFrame) {
        SCREEN_g_ToggleFullScreenNextFrame = false;

        Uint32 window_flags = SDL_GetWindowFlags(window);
        if (SCREEN_g_ToggleFullScreenType == -1) {
            window_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
        } else if (SCREEN_g_ToggleFullScreenType == 1) {
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        } else {
            window_flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        SDL_SetWindowFullscreen(window, window_flags);
    }
}
void gpu_features_reset(void);
int screen_manager_main(int argc, char *argv[]) {
    gpu_features_reset();
    SDL_GL_ResetAttributes();
    SCREEN_lastUIState = UISTATE_MENU;
    SCREEN_g_ToggleFullScreenNextFrame = false;
    SCREEN_g_QuitRequested = 0;
    SCREEN_g_DesktopWidth  = 0;
    SCREEN_g_DesktopHeight = 0;
    SCREEN_g_RefreshRate = 60.f;

    SDL_version compiled;
    SDL_version linked;
    int set_xres = -1;
    int set_yres = -1;
    int w = 0, h = 0;
    bool portrait = false;
    float set_dpi = 1.0f;
    float set_scale = 1.0f;

    // Produce a new set of arguments with the ones we skip.
    int remain_argc = 1;
    const char *remain_argv[256] = { argv[0] };

    Uint32 mode = 0;

    std::string app_name;
    std::string app_name_nice;
    std::string version;

    // Get the video info before doing anything else, so we don't get skewed resolution results.
    // TODO: support multiple displays correctly
    SDL_DisplayMode displayMode;
    int should_be_zero = SDL_GetCurrentDisplayMode(0, &displayMode);
    if (should_be_zero != 0) {
        fprintf(stderr, "Could not get display mode: %s\n", SDL_GetError());
        return 1;
    }
    SCREEN_g_DesktopWidth = displayMode.w;
    SCREEN_g_DesktopHeight = displayMode.h;
    SCREEN_g_RefreshRate = displayMode.refresh_rate;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    mode = SDL_GetWindowFlags(gWindow);
    if (mode & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        pixel_xres = SCREEN_g_DesktopWidth;
        pixel_yres = SCREEN_g_DesktopHeight;
    } else {
        pixel_xres = WINDOW_WIDTH;
        pixel_yres = WINDOW_HEIGHT;
    }

    set_dpi = 1.0f / set_dpi;

    if (set_xres > 0) {
        pixel_xres = set_xres;
    }
    if (set_yres > 0) {
        pixel_yres = set_yres;
    }
    float dpi_scale = 1.0f;
    if (set_dpi > 0) {
        dpi_scale = set_dpi;
    }

    dp_xres = (float)pixel_xres * dpi_scale;
    dp_yres = (float)pixel_yres * dpi_scale;

    char path[2048];
    const char *the_path = getenv("HOME");
    if (!the_path) {
        struct passwd *pwd = getpwuid(getuid());
        if (pwd)
            the_path = pwd->pw_dir;
    }
    if (the_path)
        strcpy(path, the_path);

    if (strlen(path) > 0 && path[strlen(path) - 1] != '/')
        strcat(path, "/");

    SCREEN_NativeInit(remain_argc, (const char **)remain_argv, path, "/tmp", nullptr);

    if (SDL_GetWindowFlags(gWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP)
        mode |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    int x,y;
    SDL_GetWindowPosition(gWindow,&x,&y);

    SCREEN_GraphicsContext *graphicsContext = nullptr;
    SDL_Window *window = gWindow;

    std::string error_message;

    SDLGLSCREEN_GraphicsContext *ctx = new SDLGLSCREEN_GraphicsContext();
    if (ctx->Init(window, x, y, mode, &error_message) != 0) {
        printf("GL init error '%s'\n", error_message.c_str());
    }
    graphicsContext = ctx;

    // Since we render from the main thread, there's nothing done here, but we call it to avoid confusion.
    if (!graphicsContext->InitFromRenderThread(&error_message)) {
        printf("Init from thread error: '%s'\n", error_message.c_str());
    }

    SCREEN_joystick = new SCREEN_SDLJoystick();
    //SCREEN_KeyMap::UpdateNativeMenuKeys();
    SCREEN_KeyMap::RestoreDefault();
    
    SCREEN_InitSDLAudioDevice();

    SCREEN_NativeInitGraphics(graphicsContext);
        
    graphicsContext->ThreadStart();

    while (true) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                SCREEN_g_QuitRequested = 1;
                break;
            case SDL_KEYDOWN:
                {
                    if (event.key.repeat > 0) 
                    { 
                        break;
                    } else
                    if (event.key.keysym.sym == SDLK_p) 
                    {
                        printJoyInfoAll();
                        break;
                    } else
                    {
                        int k = event.key.keysym.sym;
                        KeyInput key;
                        key.flags = KEY_DOWN;
                        auto mapped = KeyMapRawSDLtoNative.find(k);
                        if (mapped == KeyMapRawSDLtoNative.end() || mapped->second == NKCODE_UNKNOWN) {
                            break;
                        }
                        key.keyCode = mapped->second;
                        key.deviceId = DEVICE_ID_KEYBOARD;
                        SCREEN_NativeKey(key);
                    }
                    break;
                }
            case SDL_KEYUP:
                {
                    if (event.key.repeat > 0) 
                    {
                        break;
                    } else
                    if (event.key.keysym.sym == SDLK_f) 
                    {
                        gFullscreen = !gFullscreen;
                        SCREEN_ToggleFullScreen();
                        break;
                    } else
                    {
                        int k = event.key.keysym.sym;
                        KeyInput key;
                        key.flags = KEY_UP;
                        auto mapped = KeyMapRawSDLtoNative.find(k);
                        if (mapped == KeyMapRawSDLtoNative.end() || mapped->second == NKCODE_UNKNOWN) {
                            break;
                        }
                        key.keyCode = mapped->second;
                        key.deviceId = DEVICE_ID_KEYBOARD;
                        SCREEN_NativeKey(key);
                    }
                    break;
                }
            case SDL_JOYDEVICEADDED:
            case SDL_CONTROLLERDEVICEADDED: 
                printf("\n+++ Found new controller ");
                openJoy(event.jdevice.which);
                gUpdateCurrentScreen = true;
                break;
            case SDL_JOYDEVICEREMOVED: 
            case SDL_CONTROLLERDEVICEREMOVED:
                printf("+++ controller removed\n");
                closeJoy(event.jdevice.which);
                gUpdateCurrentScreen = true;
                break;
            case SDL_JOYHATMOTION: 
                if (gControllerConf)
                {
                    if ( (event.jhat.value == SDL_HAT_UP) || (event.jhat.value == SDL_HAT_DOWN) || \
                            (event.jhat.value == SDL_HAT_LEFT) || (event.jhat.value == SDL_HAT_RIGHT) )
                    {
                        if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                        {
                            gActiveController=0;  
                            statemachineConfHat(event.jhat.hat,event.jhat.value);
                        }
                        else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                        {
                            gActiveController=1;  
                            statemachineConfHat(event.jhat.hat,event.jhat.value);
                        }
                    }
                    break;
                }
                if (SCREEN_joystick) {
                    SCREEN_joystick->ProcessInput(event);
                }
                break;
            case SDL_JOYAXISMOTION: 
                if (gControllerConf)
                {
                    if (abs(event.jaxis.value) > 16384)
                    {
                        if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                        {
                            gActiveController=0; 
                            statemachineConfAxis(event.jaxis.axis,(event.jaxis.value < 0));
                        }
                        else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                        {
                            gActiveController=1;  
                            statemachineConfAxis(event.jaxis.axis,(event.jaxis.value < 0));
                        }
                    }
                    break;
                }
                if (SCREEN_joystick) {
                    SCREEN_joystick->ProcessInput(event);
                }
                break;
            case SDL_JOYBUTTONDOWN: 
                if (gControllerConf)
                {
                    if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                    {
                        gActiveController=0;  
                        statemachineConf(event.jbutton.button);
                    }
                    else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                    {
                        gActiveController=1;  
                        statemachineConf(event.jbutton.button);
                    }
                    break;
                }
                if (SCREEN_joystick) {
                    SCREEN_joystick->ProcessInput(event);
                }
                break;
            default:
                if (SCREEN_joystick) {
                    SCREEN_joystick->ProcessInput(event);
                }
                break;
            }
        }
        
        SCREEN_NativeUpdate();
        SCREEN_NativeRender(graphicsContext);
        
        graphicsContext->ThreadFrame();

        graphicsContext->SwapBuffers();

        if (SCREEN_g_QuitRequested) break;
        SDL_Delay(10);
        mainLoopWii();
    }

    SCREEN_StopSDLAudioDevice();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
    SCREEN_NativeShutdownGraphics();

    delete SCREEN_joystick;

    graphicsContext->ThreadEnd();

    SCREEN_NativeShutdown();

    // Destroys Draw, which is used in SCREEN_NativeShutdown to shutdown.
    graphicsContext->ShutdownFromRenderThread();

    delete graphicsContext;

    return 0;
}
