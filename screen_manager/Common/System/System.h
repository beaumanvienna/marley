#pragma once

#include <string>
#include <vector>
#include <functional>

// To synchronize the two UIs, we need to know which state we're in.
enum GlobalUIState {
	UISTATE_MENU,
	UISTATE_PAUSEMENU,
	UISTATE_INGAME,
	UISTATE_EXIT,
};

enum {
	VIRTKEY_FIRST = 0x40000001,
	VIRTKEY_AXIS_X_MIN = 0x40000001,
	VIRTKEY_AXIS_Y_MIN = 0x40000002,
	VIRTKEY_AXIS_X_MAX = 0x40000003,
	VIRTKEY_AXIS_Y_MAX = 0x40000004,
	VIRTKEY_RAPID_FIRE = 0x40000005,
	VIRTKEY_UNTHROTTLE = 0x40000006,
	VIRTKEY_PAUSE = 0x40000007,
	VIRTKEY_SPEED_TOGGLE = 0x40000008,
	VIRTKEY_AXIS_RIGHT_X_MIN = 0x40000009,
	VIRTKEY_AXIS_RIGHT_Y_MIN = 0x4000000a,
	VIRTKEY_AXIS_RIGHT_X_MAX = 0x4000000b,
	VIRTKEY_AXIS_RIGHT_Y_MAX = 0x4000000c,
	VIRTKEY_REWIND = 0x4000000d,
	VIRTKEY_SAVE_STATE = 0x4000000e,
	VIRTKEY_LOAD_STATE = 0x4000000f,
	VIRTKEY_NEXT_SLOT = 0x40000010,
	VIRTKEY_TOGGLE_FULLSCREEN = 0x40000011,
	VIRTKEY_ANALOG_LIGHTLY = 0x40000012,
	VIRTKEY_AXIS_SWAP = 0x40000013,
	VIRTKEY_DEVMENU = 0x40000014,
	VIRTKEY_FRAME_ADVANCE = 0x40000015,
	VIRTKEY_RECORD = 0x40000016,
	VIRTKEY_SPEED_CUSTOM1 = 0x40000017,
	VIRTKEY_SPEED_CUSTOM2 = 0x40000018,
	VIRTKEY_TEXTURE_DUMP = 0x40000019,
	VIRTKEY_TEXTURE_REPLACE = 0x4000001A,
	VIRTKEY_SCREENSHOT = 0x4000001B,
	VIRTKEY_MUTE_TOGGLE = 0x4000001C,
	VIRTKEY_OPENCHAT = 0x4000001D,
	VIRTKEY_ANALOG_ROTATE_CW = 0x4000001E,
	VIRTKEY_ANALOG_ROTATE_CCW = 0x4000001F,
	VIRTKEY_LAST,
	VIRTKEY_COUNT = VIRTKEY_LAST - VIRTKEY_FIRST
};

enum class SCREEN_GPUBackend {
	OPENGL = 0,
	DIRECT3D9 = 1,
	DIRECT3D11 = 2,
	VULKAN = 3,
};
void SetGPUBackend(SCREEN_GPUBackend type, const std::string &device = "");

enum SystemPermission {
	SYSTEM_PERMISSION_STORAGE,
};

enum PermissionStatus {
	PERMISSION_STATUS_UNKNOWN,
	PERMISSION_STATUS_DENIED,
	PERMISSION_STATUS_PENDING,
	PERMISSION_STATUS_GRANTED,
};

// These APIs must be implemented by every port (for example app-android.cpp, SDLMain.cpp).
// Ideally these should be safe to call from any thread.
void SystemToast(const char *text);
void ShowKeyboard();

// Vibrate either takes a number of milliseconds to vibrate unconditionally,
// or you can specify these constants for "standard" feedback. On Android,
// these will only be performed if haptic feedback is enabled globally.
// Also, on Android, these will work even if you don't have the VIBRATE permission,
// while generic vibration will not if you don't have it.
enum {
	HAPTIC_SOFT_KEYBOARD = -1,
	HAPTIC_VIRTUAL_KEY = -2,
	HAPTIC_LONG_PRESS_ACTIVATED = -3,
};
void Vibrate(int length_ms);
void OpenDirectory(const char *path);
void LaunchBrowser(const char *url);
void LaunchMarket(const char *url);
void LaunchEmail(const char *email_address);
void System_InputBoxGetString(const std::string &title, const std::string &defaultValue, std::function<void(bool, const std::string &)> cb);
void System_SendMessage(const char *command, const char *parameter);
PermissionStatus System_GetPermissionStatus(SystemPermission permission);
void System_AskForPermission(SystemPermission permission);

// This will get muddy with multi-screen support :/ But this will always be the type of the main device.
enum SystemDeviceType {
	DEVICE_TYPE_MOBILE = 0,  // phones and pads
	DEVICE_TYPE_TV = 1,  // Android TV and similar
	DEVICE_TYPE_DESKTOP = 2,  // Desktop computer
};

enum SystemProperty {
	SYSPROP_NAME,
	SYSPROP_LANGREGION,
	SYSPROP_CPUINFO,
	SYSPROP_BOARDNAME,
	SYSPROP_CLIPBOARD_TEXT,
	SYSPROP_GPUDRIVER_VERSION,

	SYSPROP_HAS_FILE_BROWSER,
	SYSPROP_HAS_IMAGE_BROWSER,
	SYSPROP_HAS_BACK_BUTTON,

	// Available as Int:
	SYSPROP_SYSTEMVERSION,
	SYSPROP_DISPLAY_XRES,
	SYSPROP_DISPLAY_YRES,
	SYSPROP_DISPLAY_REFRESH_RATE,
	SYSPROP_DISPLAY_LOGICAL_DPI,
	SYSPROP_DISPLAY_DPI,
	SYSPROP_DISPLAY_COUNT,
	SYSPROP_MOGA_VERSION,

	// Float only:
	SYSPROP_DISPLAY_SAFE_INSET_LEFT,
	SYSPROP_DISPLAY_SAFE_INSET_RIGHT,
	SYSPROP_DISPLAY_SAFE_INSET_TOP,
	SYSPROP_DISPLAY_SAFE_INSET_BOTTOM,

	SYSPROP_DEVICE_TYPE,
	SYSPROP_APP_GOLD,  // To avoid having #ifdef GOLD other than in main.cpp and similar.

	// Exposed on Android. Choosing the optimal sample rate for audio
	// will result in lower latencies. Buffer size is automatically matched
	// by the OpenSL audio backend, only exposed here for debugging/info.
	SYSPROP_AUDIO_SAMPLE_RATE,
	SYSPROP_AUDIO_FRAMES_PER_BUFFER,
	SYSPROP_AUDIO_OPTIMAL_SAMPLE_RATE,
	SYSPROP_AUDIO_OPTIMAL_FRAMES_PER_BUFFER,

	// Exposed on SDL.
	SYSPROP_AUDIO_DEVICE_LIST,

	SYSPROP_SUPPORTS_PERMISSIONS,
	SYSPROP_SUPPORTS_SUSTAINED_PERF_MODE,
};

std::string System_GetProperty(SystemProperty prop);
int System_GetPropertyInt(SystemProperty prop);
float System_GetPropertyFloat(SystemProperty prop);
bool System_GetPropertyBool(SystemProperty prop);

std::vector<std::string> __cameraGetDeviceList();
