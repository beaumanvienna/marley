// Copyright (c) 2013- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>
#include "Common/Input/InputState.h" // KeyDef
#include "Common/Input/KeyCodes.h"     // keyboard keys
#include "Common/System/System.h"

#define KEYMAP_ERROR_KEY_ALREADY_USED -1
#define KEYMAP_ERROR_UNKNOWN_KEY 0

#define CTRL_SQUARE     0x8000
#define CTRL_TRIANGLE   0x1000
#define CTRL_CIRCLE     0x2000
#define CTRL_CROSS      0x4000
#define CTRL_UP         0x0010
#define CTRL_DOWN       0x0040
#define CTRL_LEFT       0x0080
#define CTRL_RIGHT      0x0020
#define CTRL_START      0x0008
#define CTRL_SELECT     0x0001
#define CTRL_LTRIGGER   0x0100
#define CTRL_RTRIGGER   0x0200

#define CTRL_HOME         0x00010000
#define CTRL_HOLD         0x00020000
#define CTRL_WLAN         0x00040000
#define CTRL_REMOTE_HOLD  0x00080000
#define CTRL_VOL_UP       0x00100000
#define CTRL_VOL_DOWN     0x00200000
#define CTRL_SCREEN       0x00400000
#define CTRL_NOTE         0x00800000
#define CTRL_DISC         0x01000000
#define CTRL_MEMSTICK     0x02000000
#define CTRL_FORWARD      0x10000000
#define CTRL_BACK         0x20000000
#define CTRL_PLAYPAUSE    0x40000000

enum DefaultMaps {
	DEFAULT_MAPPING_KEYBOARD,
	DEFAULT_MAPPING_PAD,
	DEFAULT_MAPPING_X360,
	DEFAULT_MAPPING_SHIELD,
	DEFAULT_MAPPING_OUYA,
	DEFAULT_MAPPING_XPERIA_PLAY,
	DEFAULT_MAPPING_MOQI_I7S,
};

const float AXIS_BIND_THRESHOLD = 0.75f;
const float AXIS_BIND_THRESHOLD_MOUSE = 0.01f;

typedef std::map<int, std::vector<SCREEN_KeyDef>> KeyMapping;

// SCREEN_KeyMap
// A translation layer for key assignment. Provides
// integration with Core's config state.
//
// Does not handle input state managment.
//
// Platform ports should map their platform's keys to SCREEN_KeyMap's keys (NKCODE_*).
//
// Then have SCREEN_KeyMap transform those into psp buttons.

class SCREEN_IniFile;

namespace SCREEN_KeyMap {
	extern KeyMapping g_controllerMap;
	extern int g_controllerMapGeneration;

	// Key & Button names
	struct KeyMap_IntStrPair {
		int key;
		const char *name;
	};

	// Use if you need to display the textual name
	std::string GetKeyName(int keyCode);
	std::string GetKeyOrAxisName(int keyCode);
	std::string GetAxisName(int axisId);
	std::string GetPspButtonName(int btn);

	std::vector<KeyMap_IntStrPair> GetMappableKeys();

	// Use to translate SCREEN_KeyMap Keys to PSP
	// buttons. You should have already translated
	// your platform's keys to SCREEN_KeyMap keys.
	bool KeyToPspButton(int deviceId, int key, std::vector<int> *pspKeys);
	bool KeyFromPspButton(int btn, std::vector<SCREEN_KeyDef> *keys, bool ignoreMouse);

	int TranslateKeyCodeToAxis(int keyCode, int &direction);
	int TranslateKeyCodeFromAxis(int axisId, int direction);

	// Configure the key mapping.
	// Any configuration will be saved to the Core config.
	void SetKeyMapping(int psp_key, SCREEN_KeyDef key, bool replace);

	// Configure an axis mapping, saves the configuration.
	// Direction is negative or positive.
	void SetAxisMapping(int btn, int deviceId, int axisId, int direction, bool replace);

	bool AxisToPspButton(int deviceId, int axisId, int direction, std::vector<int> *pspKeys);
	bool AxisFromPspButton(int btn, int *deviceId, int *axisId, int *direction);
	std::string NamePspButtonFromAxis(int deviceId, int axisId, int direction);

	void LoadFromIni(SCREEN_IniFile &iniFile);
	void SaveToIni(SCREEN_IniFile &iniFile);

	void SetDefaultKeyMap(DefaultMaps dmap, bool replace);

	void RestoreDefault();

	void SwapAxis();
	void UpdateNativeMenuKeys();

	void NotifyPadConnected(const std::string &name);
	bool IsNvidiaShield(const std::string &name);
	bool IsNvidiaShieldTV(const std::string &name);
	bool IsXperiaPlay(const std::string &name);
	bool IsOuya(const std::string &name);
	bool IsMOQII7S(const std::string &name);
	bool HasBuiltinController(const std::string &name);

	const std::set<std::string> &GetSeenPads();
	void AutoConfForPad(const std::string &name);
}
