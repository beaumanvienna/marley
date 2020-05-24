// Copyright (c) 2012- PPSSPP Project.

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

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <set>

#include "base/display.h"
#include "base/NativeApp.h"
#include "file/ini_file.h"
#include "i18n/i18n.h"
#include "json/json_reader.h"
#include "gfx_es2/gpu_features.h"
#include "net/http_client.h"
#include "util/text/parsers.h"
#include "net/url.h"

#include "Common/CPUDetect.h"
#include "Common/FileUtil.h"
#include "Common/KeyMap.h"
#include "Common/LogManager.h"
#include "Common/OSVersion.h"
#include "Common/StringUtils.h"
#include "Common/Vulkan/VulkanLoader.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include "Core/Loaders.h"
#include "Core/HLE/sceUtility.h"
#include "GPU/Common/FramebufferCommon.h"

// TODO: Find a better place for this.
http::Downloader g_DownloadManager;

Config g_PConfig;

bool jitForcedOff;

#ifdef _DEBUG
static const char *logSectionName = "LogDebug";
#else
static const char *logSectionName = "Log";
#endif

struct ConfigSetting {
	enum Type {
		TYPE_TERMINATOR,
		TYPE_BOOL,
		TYPE_INT,
		TYPE_UINT32,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_TOUCH_POS,
	};
	union Value {
		bool b;
		int i;
		uint32_t u;
		float f;
		const char *s;
		ConfigTouchPos touchPos;
	};
	union SettingPtr {
		bool *b;
		int *i;
		uint32_t *u;
		float *f;
		std::string *s;
		ConfigTouchPos *touchPos;
	};

	typedef bool (*BoolDefaultCallback)();
	typedef int (*IntDefaultCallback)();
	typedef uint32_t (*Uint32DefaultCallback)();
	typedef float (*FloatDefaultCallback)();
	typedef const char *(*StringDefaultCallback)();
	typedef ConfigTouchPos (*TouchPosDefaultCallback)();

	union Callback {
		BoolDefaultCallback b;
		IntDefaultCallback i;
		Uint32DefaultCallback u;
		FloatDefaultCallback f;
		StringDefaultCallback s;
		TouchPosDefaultCallback touchPos;
	};

	ConfigSetting(bool v)
		: ini_(""), type_(TYPE_TERMINATOR), report_(false), save_(false), perGame_(false) {
		ptr_.b = nullptr;
		cb_.b = nullptr;
	}

	ConfigSetting(const char *ini, bool *v, bool def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_BOOL), report_(false), save_(save), perGame_(perGame) {
		ptr_.b = v;
		cb_.b = nullptr;
		default_.b = def;
	}

	ConfigSetting(const char *ini, int *v, int def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_INT), report_(false), save_(save), perGame_(perGame) {
		ptr_.i = v;
		cb_.i = nullptr;
		default_.i = def;
	}

	ConfigSetting(const char *ini, int *v, int def, std::function<std::string(int)> transTo, std::function<int(const std::string &)> transFrom, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_INT), report_(false), save_(save), perGame_(perGame), translateTo_(transTo), translateFrom_(transFrom) {
		ptr_.i = v;
		cb_.i = nullptr;
		default_.i = def;
	}

	ConfigSetting(const char *ini, uint32_t *v, uint32_t def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_UINT32), report_(false), save_(save), perGame_(perGame) {
		ptr_.u = v;
		cb_.u = nullptr;
		default_.u = def;
	}

	ConfigSetting(const char *ini, float *v, float def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_FLOAT), report_(false), save_(save), perGame_(perGame) {
		ptr_.f = v;
		cb_.f = nullptr;
		default_.f = def;
	}

	ConfigSetting(const char *ini, std::string *v, const char *def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_STRING), report_(false), save_(save), perGame_(perGame) {
		ptr_.s = v;
		cb_.s = nullptr;
		default_.s = def;
	}

	ConfigSetting(const char *iniX, const char *iniY, const char *iniScale, const char *iniShow, ConfigTouchPos *v, ConfigTouchPos def, bool save = true, bool perGame = false)
		: ini_(iniX), ini2_(iniY), ini3_(iniScale), ini4_(iniShow), type_(TYPE_TOUCH_POS), report_(false), save_(save), perGame_(perGame) {
		ptr_.touchPos = v;
		cb_.touchPos = nullptr;
		default_.touchPos = def;
	}

	ConfigSetting(const char *ini, bool *v, BoolDefaultCallback def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_BOOL), report_(false), save_(save), perGame_(perGame) {
		ptr_.b = v;
		cb_.b = def;
	}

	ConfigSetting(const char *ini, int *v, IntDefaultCallback def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_INT), report_(false), save_(save), perGame_(perGame) {
		ptr_ .i = v;
		cb_.i = def;
	}

	ConfigSetting(const char *ini, int *v, IntDefaultCallback def, std::function<std::string(int)> transTo, std::function<int(const std::string &)> transFrom, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_INT), report_(false), save_(save), perGame_(perGame), translateTo_(transTo), translateFrom_(transFrom) {
		ptr_.i = v;
		cb_.i = def;
	}

	ConfigSetting(const char *ini, uint32_t *v, Uint32DefaultCallback def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_UINT32), report_(false), save_(save), perGame_(perGame) {
		ptr_ .u = v;
		cb_.u = def;
	}

	ConfigSetting(const char *ini, float *v, FloatDefaultCallback def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_FLOAT), report_(false), save_(save), perGame_(perGame) {
		ptr_.f = v;
		cb_.f = def;
	}

	ConfigSetting(const char *ini, std::string *v, StringDefaultCallback def, bool save = true, bool perGame = false)
		: ini_(ini), type_(TYPE_STRING), report_(false), save_(save), perGame_(perGame) {
		ptr_.s = v;
		cb_.s = def;
	}

	ConfigSetting(const char *iniX, const char *iniY, const char *iniScale, const char *iniShow, ConfigTouchPos *v, TouchPosDefaultCallback def, bool save = true, bool perGame = false)
		: ini_(iniX), ini2_(iniY), ini3_(iniScale), ini4_(iniShow), type_(TYPE_TOUCH_POS), report_(false), save_(save), perGame_(perGame) {
		ptr_.touchPos = v;
		cb_.touchPos = def;
	}

	bool HasMore() const {
		return type_ != TYPE_TERMINATOR;
	}

	bool Get(PIniFile::Section *section) {
		switch (type_) {
		case TYPE_BOOL:
			if (cb_.b) {
				default_.b = cb_.b();
			}
			return section->Get(ini_, ptr_.b, default_.b);
		case TYPE_INT:
			if (cb_.i) {
				default_.i = cb_.i();
			}
			if (translateFrom_) {
				std::string value;
				if (section->Get(ini_, &value, nullptr)) {
					*ptr_.i = translateFrom_(value);
					return true;
				}
			}
			return section->Get(ini_, ptr_.i, default_.i);
		case TYPE_UINT32:
			if (cb_.u) {
				default_.u = cb_.u();
			}
			return section->Get(ini_, ptr_.u, default_.u);
		case TYPE_FLOAT:
			if (cb_.f) {
				default_.f = cb_.f();
			}
			return section->Get(ini_, ptr_.f, default_.f);
		case TYPE_STRING:
			if (cb_.s) {
				default_.s = cb_.s();
			}
			return section->Get(ini_, ptr_.s, default_.s);
		case TYPE_TOUCH_POS:
			if (cb_.touchPos) {
				default_.touchPos = cb_.touchPos();
			}
			section->Get(ini_, &ptr_.touchPos->x, default_.touchPos.x);
			section->Get(ini2_, &ptr_.touchPos->y, default_.touchPos.y);
			section->Get(ini3_, &ptr_.touchPos->scale, default_.touchPos.scale);
			if (ini4_) {
				section->Get(ini4_, &ptr_.touchPos->show, default_.touchPos.show);
			} else {
				ptr_.touchPos->show = default_.touchPos.show;
			}
			return true;
		default:
			_dbg_assert_msg_(LOADER, false, "Unexpected ini setting type");
			return false;
		}
	}

	void Set(PIniFile::Section *section) {
		if (!save_)
			return;

		switch (type_) {
		case TYPE_BOOL:
			return section->Set(ini_, *ptr_.b);
		case TYPE_INT:
			if (translateTo_) {
				std::string value = translateTo_(*ptr_.i);
				return section->Set(ini_, value);
			}
			return section->Set(ini_, *ptr_.i);
		case TYPE_UINT32:
			return section->Set(ini_, *ptr_.u);
		case TYPE_FLOAT:
			return section->Set(ini_, *ptr_.f);
		case TYPE_STRING:
			return section->Set(ini_, *ptr_.s);
		case TYPE_TOUCH_POS:
			section->Set(ini_, ptr_.touchPos->x);
			section->Set(ini2_, ptr_.touchPos->y);
			section->Set(ini3_, ptr_.touchPos->scale);
			if (ini4_) {
				section->Set(ini4_, ptr_.touchPos->show);
			}
			return;
		default:
			_dbg_assert_msg_(LOADER, false, "Unexpected ini setting type");
			return;
		}
	}

	void Report(UrlEncoder &data, const std::string &prefix) {
		if (!report_)
			return;

		switch (type_) {
		case TYPE_BOOL:
			return data.Add(prefix + ini_, *ptr_.b);
		case TYPE_INT:
			return data.Add(prefix + ini_, *ptr_.i);
		case TYPE_UINT32:
			return data.Add(prefix + ini_, *ptr_.u);
		case TYPE_FLOAT:
			return data.Add(prefix + ini_, *ptr_.f);
		case TYPE_STRING:
			return data.Add(prefix + ini_, *ptr_.s);
		case TYPE_TOUCH_POS:
			// Doesn't report.
			return;
		default:
			_dbg_assert_msg_(LOADER, false, "Unexpected ini setting type");
			return;
		}
	}

	const char *ini_;
	const char *ini2_;
	const char *ini3_;
	const char *ini4_;
	Type type_;
	bool report_;
	bool save_;
	bool perGame_;
	SettingPtr ptr_;
	Value default_;
	Callback cb_;

	// We only support transform for ints.
	std::function<std::string(int)> translateTo_;
	std::function<int(const std::string &)> translateFrom_;
};

struct ReportedConfigSetting : public ConfigSetting {
	template <typename T1, typename T2>
	ReportedConfigSetting(const char *ini, T1 *v, T2 def, bool save = true, bool perGame = false)
		: ConfigSetting(ini, v, def, save, perGame) {
		report_ = true;
	}

	template <typename T1, typename T2>
	ReportedConfigSetting(const char *ini, T1 *v, T2 def, std::function<std::string(int)> transTo, std::function<int(const std::string &)> transFrom, bool save = true, bool perGame = false)
		: ConfigSetting(ini, v, def, transTo, transFrom, save, perGame) {
		report_ = true;
	}
};

const char *DefaultLangRegion() {
	// Unfortunate default.  There's no need to use bFirstRun, since this is only a default.
	static std::string defaultLangRegion = "en_US";
	std::string langRegion = System_GetProperty(SYSPROP_LANGREGION);
	if (i18nrepo.IniExists(langRegion)) {
		defaultLangRegion = langRegion;
	} else if (langRegion.length() >= 3) {
		// Don't give up.  Let's try a fuzzy match - so nl_BE can match nl_NL.
		PIniFile mapping;
		mapping.LoadFromVFS("langregion.ini");
		std::vector<std::string> keys;
		mapping.GetKeys("LangRegionNames", keys);

		for (std::string key : keys) {
			if (startsWithNoCase(key, langRegion)) {
				// Exact submatch, or different case.  Let's use it.
				defaultLangRegion = key;
				break;
			} else if (startsWithNoCase(key, langRegion.substr(0, 3))) {
				// Best so far.
				defaultLangRegion = key;
			}
		}
	}

	return defaultLangRegion.c_str();
}

std::string CreateRandMAC() {
	std::stringstream randStream;
	srand(time(nullptr));
	for (int i = 0; i < 6; i++) {
		u32 value = rand() % 256;
		if (value <= 15)
			randStream << '0' << std::hex << value;
		else
			randStream << std::hex << value;
		if (i < 5) {
			randStream << ':'; //we need a : between every octet
		}
	}
	return randStream.str();
}

static int DefaultNumWorkers() {
	return cpu_info.num_cores;
}

static int DefaultCpuCore() {
#if defined(ARM) || defined(ARM64) || defined(_M_IX86) || defined(_M_X64)
	return (int)CPUCore::JIT;
#else
	return (int)CPUCore::INTERPRETER;
#endif
}

static bool DefaultCodeGen() {
#if defined(ARM) || defined(ARM64) || defined(_M_IX86) || defined(_M_X64)
	return true;
#else
	return false;
#endif
}

static bool DefaultEnableStateUndo() {
#ifdef MOBILE_DEVICE
	// Off on mobile to save disk space.
	return false;
#endif
	return true;
}

struct ConfigSectionSettings {
	const char *section;
	ConfigSetting *settings;
};

static ConfigSetting generalSettings[] = {
	ConfigSetting("FirstRun", &g_PConfig.bFirstRun, true),
	ConfigSetting("RunCount", &g_PConfig.iRunCount, 0),
	ConfigSetting("Enable Logging", &g_PConfig.bEnableLogging, false),
	ConfigSetting("AutoRun", &g_PConfig.bAutoRun, true),
	ConfigSetting("Browse", &g_PConfig.bBrowse, false),
	ConfigSetting("IgnoreBadMemAccess", &g_PConfig.bIgnoreBadMemAccess, true, true),
	ConfigSetting("CurrentDirectory", &g_PConfig.currentDirectory, ""),
	ConfigSetting("ShowDebuggerOnLoad", &g_PConfig.bShowDebuggerOnLoad, false),
	ConfigSetting("CheckForNewVersion", &g_PConfig.bCheckForNewVersion, true),
	ConfigSetting("Language", &g_PConfig.sLanguageIni, &DefaultLangRegion),
	ConfigSetting("ForceLagSync2", &g_PConfig.bForceLagSync, false, true, true),
	ConfigSetting("DiscordPresence", &g_PConfig.bDiscordPresence, true, true, false),  // Or maybe it makes sense to have it per-game? Race conditions abound...

	ReportedConfigSetting("NumWorkerThreads", &g_PConfig.iNumWorkerThreads, &DefaultNumWorkers, true, true),
	ConfigSetting("AutoLoadSaveState", &g_PConfig.iAutoLoadSaveState, 0, true, true),
	ReportedConfigSetting("EnableCheats", &g_PConfig.bEnableCheats, false, true, true),
	ConfigSetting("CwCheatRefreshRate", &g_PConfig.iCwCheatRefreshRate, 77, true, true),

	ConfigSetting("ScreenshotsAsPNG", &g_PConfig.bScreenshotsAsPNG, false, true, true),
	ConfigSetting("UseFFV1", &g_PConfig.bUseFFV1, false),
	ConfigSetting("DumpFrames", &g_PConfig.bDumpFrames, false),
	ConfigSetting("DumpVideoOutput", &g_PConfig.bDumpVideoOutput, false),
	ConfigSetting("DumpAudio", &g_PConfig.bDumpAudio, false),
	ConfigSetting("SaveLoadResetsAVdumping", &g_PConfig.bSaveLoadResetsAVdumping, false),
	ConfigSetting("StateSlot", &g_PConfig.iCurrentStateSlot, 0, true, true),
	ConfigSetting("EnableStateUndo", &g_PConfig.bEnableStateUndo, &DefaultEnableStateUndo, true, true),
	ConfigSetting("RewindFlipFrequency", &g_PConfig.iRewindFlipFrequency, 0, true, true),

	ConfigSetting("GridView1", &g_PConfig.bGridView1, true),
	ConfigSetting("GridView2", &g_PConfig.bGridView2, true),
	ConfigSetting("GridView3", &g_PConfig.bGridView3, false),
	ConfigSetting("ComboMode", &g_PConfig.iComboMode, 0),

	// "default" means let emulator decide, "" means disable.
	ConfigSetting("ReportingHost", &g_PConfig.sReportHost, "default"),
	ConfigSetting("AutoSaveSymbolMap", &g_PConfig.bAutoSaveSymbolMap, false, true, true),
	ConfigSetting("CacheFullIsoInRam", &g_PConfig.bCacheFullIsoInRam, false, true, true),
	ConfigSetting("RemoteISOPort", &g_PConfig.iRemoteISOPort, 0, true, false),
	ConfigSetting("LastRemoteISOServer", &g_PConfig.sLastRemoteISOServer, ""),
	ConfigSetting("LastRemoteISOPort", &g_PConfig.iLastRemoteISOPort, 0),
	ConfigSetting("RemoteISOManualConfig", &g_PConfig.bRemoteISOManual, false),
	ConfigSetting("RemoteShareOnStartup", &g_PConfig.bRemoteShareOnStartup, false),
	ConfigSetting("RemoteISOSubdir", &g_PConfig.sRemoteISOSubdir, "/"),
	ConfigSetting("RemoteDebuggerOnStartup", &g_PConfig.bRemoteDebuggerOnStartup, false),

#ifdef __ANDROID__
	ConfigSetting("ScreenRotation", &g_PConfig.iScreenRotation, ROTATION_AUTO_HORIZONTAL),
#endif
	ConfigSetting("InternalScreenRotation", &g_PConfig.iInternalScreenRotation, ROTATION_LOCKED_HORIZONTAL),

#if defined(USING_WIN_UI)
	ConfigSetting("TopMost", &g_PConfig.bTopMost, false),
	ConfigSetting("WindowX", &g_PConfig.iWindowX, -1), // -1 tells us to center the window.
	ConfigSetting("WindowY", &g_PConfig.iWindowY, -1),
	ConfigSetting("WindowWidth", &g_PConfig.iWindowWidth, 0),   // 0 will be automatically reset later (need to do the AdjustWindowRect dance).
	ConfigSetting("WindowHeight", &g_PConfig.iWindowHeight, 0),
	ConfigSetting("PauseOnLostFocus", &g_PConfig.bPauseOnLostFocus, false, true, true),
#endif
	ConfigSetting("PauseWhenMinimized", &g_PConfig.bPauseWhenMinimized, false, true, true),
	ConfigSetting("DumpDecryptedEboots", &g_PConfig.bDumpDecryptedEboot, false, true, true),
	ConfigSetting("FullscreenOnDoubleclick", &g_PConfig.bFullscreenOnDoubleclick, true, false, false),

	ReportedConfigSetting("MemStickInserted", &g_PConfig.bMemStickInserted, true, true, true),

	ConfigSetting(false),
};

static bool DefaultSasThread() {
	return cpu_info.num_cores > 1;
}

static ConfigSetting cpuSettings[] = {
	ReportedConfigSetting("CPUCore", &g_PConfig.iCpuCore, &DefaultCpuCore, true, true),
	ReportedConfigSetting("SeparateSASThread", &g_PConfig.bSeparateSASThread, &DefaultSasThread, true, true),
	ReportedConfigSetting("SeparateIOThread", &g_PConfig.bSeparateIOThread, true, true, true),
	ReportedConfigSetting("IOTimingMethod", &g_PConfig.iIOTimingMethod, IOTIMING_FAST, true, true),
	ConfigSetting("FastMemoryAccess", &g_PConfig.bFastMemory, true, true, true),
	ReportedConfigSetting("FuncReplacements", &g_PConfig.bFuncReplacements, true, true, true),
	ConfigSetting("HideSlowWarnings", &g_PConfig.bHideSlowWarnings, false, true, false),
	ConfigSetting("HideStateWarnings", &g_PConfig.bHideStateWarnings, false, true, false),
	ConfigSetting("PreloadFunctions", &g_PConfig.bPreloadFunctions, false, true, true),
	ConfigSetting("JitDisableFlags", &g_PConfig.uJitDisableFlags, (uint32_t)0, true, true),
	ReportedConfigSetting("CPUSpeed", &g_PConfig.iLockedCPUSpeed, 0, true, true),

	ConfigSetting(false),
};

static int DefaultInternalResolution() {
	// Auto on Windows, 2x on large screens, 1x elsewhere.
#if defined(USING_WIN_UI)
	return 0;
#else
	int longestDisplaySide = std::max(System_GetPropertyInt(SYSPROP_DISPLAY_XRES), System_GetPropertyInt(SYSPROP_DISPLAY_YRES));
	int scale = longestDisplaySide >= 1000 ? 2 : 1;
	ILOG("Longest display side: %d pixels. Choosing scale %d", longestDisplaySide, scale);
	return scale;
#endif
}

static bool DefaultFrameskipUnthrottle() {
#if !PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(UWP)
	return true;
#else
	return false;
#endif
}

static int DefaultZoomType() {
	return (int)SmallDisplayZoom::AUTO;
}

static int DefaultAndroidHwScale() {
#ifdef __ANDROID__
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 19 || System_GetPropertyInt(SYSPROP_DEVICE_TYPE) == DEVICE_TYPE_TV) {
		// Arbitrary cutoff at Kitkat - modern devices are usually powerful enough that hw scaling
		// doesn't really help very much and mostly causes problems. See #11151
		return 0;
	}

	// Get the real resolution as passed in during startup, not dp_xres and stuff
	int xres = System_GetPropertyInt(SYSPROP_DISPLAY_XRES);
	int yres = System_GetPropertyInt(SYSPROP_DISPLAY_YRES);

	if (xres <= 960) {
		// Smaller than the PSP*2, let's go native.
		return 0;
	} else if (xres <= 480 * 3) {  // 720p xres
		// Small-ish screen, we should default to 2x
		return 2 + 1;
	} else {
		// Large or very large screen. Default to 3x psp resolution.
		return 3 + 1;
	}
	return 0;
#else
	return 1;
#endif
}

static int DefaultGPUBackend() {
#if PPSSPP_PLATFORM(WINDOWS)
	// If no Vulkan, use Direct3D 11 on Windows 8+ (most importantly 10.)
	if (DoesVersionMatchWindows(6, 2, 0, 0, true)) {
		return (int)GPUBackend::DIRECT3D11;
	}
#elif PPSSPP_PLATFORM(ANDROID)
	// Default to Vulkan only on Oreo 8.1 (level 27) devices or newer. Drivers before
	// were generally too unreliable to default to (with some exceptions, of course).
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 27) {
		return (int)GPUBackend::VULKAN;
	}
#endif
	// TODO: On some additional Linux platforms, we should also default to Vulkan.
	return (int)GPUBackend::OPENGL;
}

int Config::NextValidBackend() {
	std::vector<std::string> split;
	std::set<GPUBackend> failed;

	PSplitString(sFailedGPUBackends, ',', split);
	for (const auto &str : split) {
		if (!str.empty() && str != "ALL") {
			failed.insert(GPUBackendFromString(str));
		}
	}

	// Count these as "failed" too so we don't pick them.
	PSplitString(sDisabledGPUBackends, ',', split);
	for (const auto &str : split) {
		if (!str.empty()) {
			failed.insert(GPUBackendFromString(str));
		}
	}

	if (failed.count((GPUBackend)iGPUBackend)) {
		ERROR_LOG(LOADER, "Graphics backend failed for %d, trying another", iGPUBackend);

#if (PPSSPP_PLATFORM(WINDOWS) || PPSSPP_PLATFORM(ANDROID)) && !PPSSPP_PLATFORM(UWP)
		if (!failed.count(GPUBackend::VULKAN) && VulkanMayBeAvailable()) {
			return (int)GPUBackend::VULKAN;
		}
#endif
#if PPSSPP_PLATFORM(WINDOWS)
		if (!failed.count(GPUBackend::DIRECT3D11) && DoesVersionMatchWindows(6, 1, 0, 0, true)) {
			return (int)GPUBackend::DIRECT3D11;
		}
#endif
#if PPSSPP_API(ANY_GL)
		if (!failed.count(GPUBackend::OPENGL)) {
			return (int)GPUBackend::OPENGL;
		}
#endif
#if PPSSPP_API(D3D9)
		if (!failed.count(GPUBackend::DIRECT3D9)) {
			return (int)GPUBackend::DIRECT3D9;
		}
#endif

		// They've all failed.  Let them try the default - or on Android, OpenGL.
		sFailedGPUBackends += ",ALL";
		ERROR_LOG(LOADER, "All graphics backends failed");
#if PPSSPP_PLATFORM(ANDROID)
		return (int)GPUBackend::OPENGL;
#else
		return DefaultGPUBackend();
#endif
	}

	return iGPUBackend;
}

bool Config::IsBackendEnabled(GPUBackend backend, bool validate) {
	std::vector<std::string> split;

	PSplitString(sDisabledGPUBackends, ',', split);
	for (const auto &str : split) {
		if (str.empty())
			continue;
		auto match = GPUBackendFromString(str);
		if (match == backend)
			return false;
	}

#if PPSSPP_PLATFORM(IOS)
	if (backend != GPUBackend::OPENGL)
		return false;
#elif PPSSPP_PLATFORM(UWP)
	if (backend != GPUBackend::DIRECT3D11)
		return false;
#elif PPSSPP_PLATFORM(WINDOWS)
	if (validate) {
		if (backend == GPUBackend::DIRECT3D11 && !DoesVersionMatchWindows(6, 0, 0, 0, true))
			return false;
	}
#else
	if (backend == GPUBackend::DIRECT3D11 || backend == GPUBackend::DIRECT3D9)
		return false;
#endif

#if !PPSSPP_API(ANY_GL)
	if (backend == GPUBackend::OPENGL)
		return false;
#endif
#if !PPSSPP_PLATFORM(IOS)
	if (validate) {
		if (backend == GPUBackend::VULKAN && !VulkanMayBeAvailable())
			return false;
	}
#endif

	return true;
}

static bool DefaultVertexCache() {
	return DefaultGPUBackend() == (int)GPUBackend::OPENGL;
}

template <typename T, std::string (*FTo)(T), T (*FFrom)(const std::string &)>
struct ConfigTranslator {
	static std::string To(int v) {
		return StringFromInt(v) + " (" + FTo(T(v)) + ")";
	}

	static int From(const std::string &v) {
		int result;
		if (PTryParse(v, &result)) {
			return result;
		}
		return (int)FFrom(v);
	}
};

typedef ConfigTranslator<GPUBackend, GPUBackendToString, GPUBackendFromString> GPUBackendTranslator;

static ConfigSetting graphicsSettings[] = {
	ConfigSetting("ShowFPSCounter", &g_PConfig.iShowFPSCounter, 0, true, true),
	ReportedConfigSetting("GraphicsBackend", &g_PConfig.iGPUBackend, &DefaultGPUBackend, &GPUBackendTranslator::To, &GPUBackendTranslator::From, true, false),
	ConfigSetting("FailedGraphicsBackends", &g_PConfig.sFailedGPUBackends, ""),
	ConfigSetting("DisabledGraphicsBackends", &g_PConfig.sDisabledGPUBackends, ""),
	ConfigSetting("VulkanDevice", &g_PConfig.sVulkanDevice, "", true, false),
#ifdef _WIN32
	ConfigSetting("D3D11Device", &g_PConfig.sD3D11Device, "", true, false),
#endif
	ConfigSetting("VendorBugChecksEnabled", &g_PConfig.bVendorBugChecksEnabled, true, false, false),
	ReportedConfigSetting("RenderingMode", &g_PConfig.iRenderingMode, 1, true, true),
	ConfigSetting("SoftwareRenderer", &g_PConfig.bSoftwareRendering, false, true, true),
	ReportedConfigSetting("HardwareTransform", &g_PConfig.bHardwareTransform, true, true, true),
	ReportedConfigSetting("SoftwareSkinning", &g_PConfig.bSoftwareSkinning, true, true, true),
	ReportedConfigSetting("TextureFiltering", &g_PConfig.iTexFiltering, 1, true, true),
	ReportedConfigSetting("BufferFiltering", &g_PConfig.iBufFilter, SCALE_LINEAR, true, true),
	ReportedConfigSetting("InternalResolution", &g_PConfig.iInternalResolution, &DefaultInternalResolution, true, true),
	ReportedConfigSetting("AndroidHwScale", &g_PConfig.iAndroidHwScale, &DefaultAndroidHwScale),
	ReportedConfigSetting("HighQualityDepth", &g_PConfig.bHighQualityDepth, true, true, true),
	ReportedConfigSetting("FrameSkip", &g_PConfig.iFrameSkip, 0, true, true),
	ReportedConfigSetting("FrameSkipType", &g_PConfig.iFrameSkipType, 0, true, true),
	ReportedConfigSetting("AutoFrameSkip", &g_PConfig.bAutoFrameSkip, false, true, true),
	ConfigSetting("FrameRate", &g_PConfig.iFpsLimit1, 0, true, true),
	ConfigSetting("FrameRate2", &g_PConfig.iFpsLimit2, -1, true, true),
	ConfigSetting("FrameSkipUnthrottle", &g_PConfig.bFrameSkipUnthrottle, &DefaultFrameskipUnthrottle, true, false),
#if defined(USING_WIN_UI)
	ConfigSetting("RestartRequired", &g_PConfig.bRestartRequired, false, false),
#endif

	// Most low-performance (and many high performance) mobile GPUs do not support aniso anyway so defaulting to 4 is fine.
	ConfigSetting("AnisotropyLevel", &g_PConfig.iAnisotropyLevel, 4, true, true),

	ReportedConfigSetting("VertexDecCache", &g_PConfig.bVertexCache, &DefaultVertexCache, true, true),
	ReportedConfigSetting("TextureBackoffCache", &g_PConfig.bTextureBackoffCache, false, true, true),
	ReportedConfigSetting("TextureSecondaryCache", &g_PConfig.bTextureSecondaryCache, false, true, true),
	ReportedConfigSetting("VertexDecJit", &g_PConfig.bVertexDecoderJit, &DefaultCodeGen, false),

#ifndef MOBILE_DEVICE
	ConfigSetting("FullScreen", &g_PConfig.bFullScreen, false),
	ConfigSetting("FullScreenMulti", &g_PConfig.bFullScreenMulti, false),
#endif

	ConfigSetting("SmallDisplayZoomType", &g_PConfig.iSmallDisplayZoomType, &DefaultZoomType, true, true),
	ConfigSetting("SmallDisplayOffsetX", &g_PConfig.fSmallDisplayOffsetX, 0.5f, true, true),
	ConfigSetting("SmallDisplayOffsetY", &g_PConfig.fSmallDisplayOffsetY, 0.5f, true, true),
	ConfigSetting("SmallDisplayZoomLevel", &g_PConfig.fSmallDisplayZoomLevel, 1.0f, true, true),
	ConfigSetting("ImmersiveMode", &g_PConfig.bImmersiveMode, false, true, true),
	ConfigSetting("SustainedPerformanceMode", &g_PConfig.bSustainedPerformanceMode, false, true, true),

	ReportedConfigSetting("ReplaceTextures", &g_PConfig.bReplaceTextures, true, true, true),
	ReportedConfigSetting("SaveNewTextures", &g_PConfig.bSaveNewTextures, false, true, true),
	ConfigSetting("IgnoreTextureFilenames", &g_PConfig.bIgnoreTextureFilenames, false, true, true),

	ReportedConfigSetting("TexScalingLevel", &g_PConfig.iTexScalingLevel, 1, true, true),
	ReportedConfigSetting("TexScalingType", &g_PConfig.iTexScalingType, 0, true, true),
	ReportedConfigSetting("TexDeposterize", &g_PConfig.bTexDeposterize, false, true, true),
	ConfigSetting("VSyncInterval", &g_PConfig.bVSync, false, true, true),
	ReportedConfigSetting("BloomHack", &g_PConfig.iBloomHack, 0, true, true),

	// Not really a graphics setting...
	ReportedConfigSetting("SplineBezierQuality", &g_PConfig.iSplineBezierQuality, 2, true, true),
	ReportedConfigSetting("HardwareTessellation", &g_PConfig.bHardwareTessellation, false, true, true),
	ReportedConfigSetting("PostShader", &g_PConfig.sPostShaderName, "Off", true, true),

	ReportedConfigSetting("MemBlockTransferGPU", &g_PConfig.bBlockTransferGPU, true, true, true),
	ReportedConfigSetting("DisableSlowFramebufEffects", &g_PConfig.bDisableSlowFramebufEffects, false, true, true),
	ReportedConfigSetting("FragmentTestCache", &g_PConfig.bFragmentTestCache, true, true, true),

	ConfigSetting("GfxDebugOutput", &g_PConfig.bGfxDebugOutput, false, false, false),
	ConfigSetting("GfxDebugSplitSubmit", &g_PConfig.bGfxDebugSplitSubmit, false, false, false),
	ConfigSetting("LogFrameDrops", &g_PConfig.bLogFrameDrops, false, true, false),

	ConfigSetting(false),
};

static ConfigSetting soundSettings[] = {
	ConfigSetting("Enable", &g_PConfig.bEnableSound, true, true, true),
	ConfigSetting("AudioBackend", &g_PConfig.iAudioBackend, 0, true, true),
	ConfigSetting("AudioLatency", &g_PConfig.iAudioLatency, 1, true, true),
	ConfigSetting("ExtraAudioBuffering", &g_PConfig.bExtraAudioBuffering, false, true, false),
	ConfigSetting("AudioResampler", &g_PConfig.bAudioResampler, true, true, true),
	ConfigSetting("GlobalVolume", &g_PConfig.iGlobalVolume, VOLUME_MAX, true, true),
	ConfigSetting("AltSpeedVolume", &g_PConfig.iAltSpeedVolume, -1, true, true),

	ConfigSetting(false),
};

static bool DefaultShowTouchControls() {
	int deviceType = System_GetPropertyInt(SYSPROP_DEVICE_TYPE);
	if (deviceType == DEVICE_TYPE_MOBILE) {
		std::string name = System_GetProperty(SYSPROP_NAME);
		if (KeyMap::HasBuiltinController(name)) {
			return false;
		} else {
			return true;
		}
	} else if (deviceType == DEVICE_TYPE_TV) {
		return false;
	} else if (deviceType == DEVICE_TYPE_DESKTOP) {
		return false;
	} else {
		return false;
	}
}

static const float defaultControlScale = 1.15f;
static const ConfigTouchPos defaultTouchPosShow = { -1.0f, -1.0f, defaultControlScale, true };
static const ConfigTouchPos defaultTouchPosHide = { -1.0f, -1.0f, defaultControlScale, false };

static ConfigSetting controlSettings[] = {
	ConfigSetting("HapticFeedback", &g_PConfig.bHapticFeedback, false, true, true),
	ConfigSetting("ShowTouchCross", &g_PConfig.bShowTouchCross, true, true, true),
	ConfigSetting("ShowTouchCircle", &g_PConfig.bShowTouchCircle, true, true, true),
	ConfigSetting("ShowTouchSquare", &g_PConfig.bShowTouchSquare, true, true, true),
	ConfigSetting("ShowTouchTriangle", &g_PConfig.bShowTouchTriangle, true, true, true),

	ConfigSetting("ComboKey0Mapping", &g_PConfig.iCombokey0, 0, true, true),
	ConfigSetting("ComboKey1Mapping", &g_PConfig.iCombokey1, 0, true, true),
	ConfigSetting("ComboKey2Mapping", &g_PConfig.iCombokey2, 0, true, true),
	ConfigSetting("ComboKey3Mapping", &g_PConfig.iCombokey3, 0, true, true),
	ConfigSetting("ComboKey4Mapping", &g_PConfig.iCombokey4, 0, true, true),

#if defined(_WIN32)
	// A win32 user seeing touch controls is likely using PPSSPP on a tablet. There it makes
	// sense to default this to on.
	ConfigSetting("ShowTouchPause", &g_PConfig.bShowTouchPause, true, true, false),
#else
	ConfigSetting("ShowTouchPause", &g_PConfig.bShowTouchPause, false, true, false),
#endif
#if defined(USING_WIN_UI)
	ConfigSetting("IgnoreWindowsKey", &g_PConfig.bIgnoreWindowsKey, false, true, true),
#endif
	ConfigSetting("ShowTouchControls", &g_PConfig.bShowTouchControls, &DefaultShowTouchControls, true, true),
	// ConfigSetting("KeyMapping", &g_PConfig.iMappingMap, 0),

#ifdef MOBILE_DEVICE
	ConfigSetting("TiltBaseX", &g_PConfig.fTiltBaseX, 0.0f, true, true),
	ConfigSetting("TiltBaseY", &g_PConfig.fTiltBaseY, 0.0f, true, true),
	ConfigSetting("InvertTiltX", &g_PConfig.bInvertTiltX, false, true, true),
	ConfigSetting("InvertTiltY", &g_PConfig.bInvertTiltY, true, true, true),
	ConfigSetting("TiltSensitivityX", &g_PConfig.iTiltSensitivityX, 100, true, true),
	ConfigSetting("TiltSensitivityY", &g_PConfig.iTiltSensitivityY, 100, true, true),
	ConfigSetting("DeadzoneRadius", &g_PConfig.fDeadzoneRadius, 0.2f, true, true),
	ConfigSetting("TiltInputType", &g_PConfig.iTiltInputType, 0, true, true),
#endif

	ConfigSetting("DisableDpadDiagonals", &g_PConfig.bDisableDpadDiagonals, false, true, true),
	ConfigSetting("GamepadOnlyFocused", &g_PConfig.bGamepadOnlyFocused, false, true, true),
	ConfigSetting("TouchButtonStyle", &g_PConfig.iTouchButtonStyle, 1, true, true),
	ConfigSetting("TouchButtonOpacity", &g_PConfig.iTouchButtonOpacity, 65, true, true),
	ConfigSetting("TouchButtonHideSeconds", &g_PConfig.iTouchButtonHideSeconds, 20, true, true),
	ConfigSetting("AutoCenterTouchAnalog", &g_PConfig.bAutoCenterTouchAnalog, false, true, true),

	// -1.0f means uninitialized, set in GamepadEmu::CreatePadLayout().
	ConfigSetting("ActionButtonSpacing2", &g_PConfig.fActionButtonSpacing, 1.0f, true, true),
	ConfigSetting("ActionButtonCenterX", "ActionButtonCenterY", "ActionButtonScale", nullptr, &g_PConfig.touchActionButtonCenter, defaultTouchPosShow, true, true),
	ConfigSetting("DPadX", "DPadY", "DPadScale", "ShowTouchDpad", &g_PConfig.touchDpad, defaultTouchPosShow, true, true),

	// Note: these will be overwritten if DPadRadius is set.
	ConfigSetting("DPadSpacing", &g_PConfig.fDpadSpacing, 1.0f, true, true),
	ConfigSetting("StartKeyX", "StartKeyY", "StartKeyScale", "ShowTouchStart", &g_PConfig.touchStartKey, defaultTouchPosShow, true, true),
	ConfigSetting("SelectKeyX", "SelectKeyY", "SelectKeyScale", "ShowTouchSelect", &g_PConfig.touchSelectKey, defaultTouchPosShow, true, true),
	ConfigSetting("UnthrottleKeyX", "UnthrottleKeyY", "UnthrottleKeyScale", "ShowTouchUnthrottle", &g_PConfig.touchUnthrottleKey, defaultTouchPosShow, true, true),
	ConfigSetting("LKeyX", "LKeyY", "LKeyScale", "ShowTouchLTrigger", &g_PConfig.touchLKey, defaultTouchPosShow, true, true),
	ConfigSetting("RKeyX", "RKeyY", "RKeyScale", "ShowTouchRTrigger", &g_PConfig.touchRKey, defaultTouchPosShow, true, true),
	ConfigSetting("AnalogStickX", "AnalogStickY", "AnalogStickScale", "ShowAnalogStick", &g_PConfig.touchAnalogStick, defaultTouchPosShow, true, true),
	ConfigSetting("RightAnalogStickX", "RightAnalogStickY", "RightAnalogStickScale", "ShowRightAnalogStick", &g_PConfig.touchRightAnalogStick, defaultTouchPosHide, true, true),

	ConfigSetting("fcombo0X", "fcombo0Y", "comboKeyScale0", "ShowComboKey0", &g_PConfig.touchCombo0, defaultTouchPosHide, true, true),
	ConfigSetting("fcombo1X", "fcombo1Y", "comboKeyScale1", "ShowComboKey1", &g_PConfig.touchCombo1, defaultTouchPosHide, true, true),
	ConfigSetting("fcombo2X", "fcombo2Y", "comboKeyScale2", "ShowComboKey2", &g_PConfig.touchCombo2, defaultTouchPosHide, true, true),
	ConfigSetting("fcombo3X", "fcombo3Y", "comboKeyScale3", "ShowComboKey3", &g_PConfig.touchCombo3, defaultTouchPosHide, true, true),
	ConfigSetting("fcombo4X", "fcombo4Y", "comboKeyScale4", "ShowComboKey4", &g_PConfig.touchCombo4, defaultTouchPosHide, true, true),
	ConfigSetting("Speed1KeyX", "Speed1KeyY", "Speed1KeyScale", "ShowSpeed1Key", &g_PConfig.touchSpeed1Key, defaultTouchPosHide, true, true),
	ConfigSetting("Speed2KeyX", "Speed2KeyY", "Speed2KeyScale", "ShowSpeed2Key", &g_PConfig.touchSpeed2Key, defaultTouchPosHide, true, true),

#ifdef _WIN32
	ConfigSetting("DInputAnalogDeadzone", &g_PConfig.fDInputAnalogDeadzone, 0.1f, true, true),
	ConfigSetting("DInputAnalogInverseMode", &g_PConfig.iDInputAnalogInverseMode, 0, true, true),
	ConfigSetting("DInputAnalogInverseDeadzone", &g_PConfig.fDInputAnalogInverseDeadzone, 0.0f, true, true),
	ConfigSetting("DInputAnalogSensitivity", &g_PConfig.fDInputAnalogSensitivity, 1.0f, true, true),

	ConfigSetting("XInputAnalogDeadzone", &g_PConfig.fXInputAnalogDeadzone, 0.24f, true, true),
	ConfigSetting("XInputAnalogInverseMode", &g_PConfig.iXInputAnalogInverseMode, 0, true, true),
	ConfigSetting("XInputAnalogInverseDeadzone", &g_PConfig.fXInputAnalogInverseDeadzone, 0.0f, true, true),
#endif
	// Also reused as generic analog sensitivity
	ConfigSetting("XInputAnalogSensitivity", &g_PConfig.fXInputAnalogSensitivity, 1.0f, true, true),
	ConfigSetting("AnalogLimiterDeadzone", &g_PConfig.fAnalogLimiterDeadzone, 0.6f, true, true),

	ConfigSetting("UseMouse", &g_PConfig.bMouseControl, false, true, true),
	ConfigSetting("MapMouse", &g_PConfig.bMapMouse, false, true, true),
	ConfigSetting("ConfineMap", &g_PConfig.bMouseConfine, false, true, true),
	ConfigSetting("MouseSensitivity", &g_PConfig.fMouseSensitivity, 0.1f, true, true),
	ConfigSetting("MouseSmoothing", &g_PConfig.fMouseSmoothing, 0.9f, true, true),

	ConfigSetting(false),
};

static ConfigSetting networkSettings[] = {
	ConfigSetting("EnableWlan", &g_PConfig.bEnableWlan, false, true, true),
	ConfigSetting("EnableAdhocServer", &g_PConfig.bEnableAdhocServer, false, true, true),

	ConfigSetting(false),
};

static int DefaultPSPModel() {
	// TODO: Can probably default this on, but not sure about its memory differences.
#if !defined(_M_X64) && !defined(_WIN32)
	return PSP_MODEL_FAT;
#else
	return PSP_MODEL_SLIM;
#endif
}

static int DefaultSystemParamLanguage() {
	int defaultLang = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	if (g_PConfig.bFirstRun) {
		// TODO: Be smart about same language, different country
		auto langValuesMapping = GetLangValuesMapping();
		if (langValuesMapping.find(g_PConfig.sLanguageIni) != langValuesMapping.end()) {
			defaultLang = langValuesMapping[g_PConfig.sLanguageIni].second;
		}
	}
	return defaultLang;
}

static ConfigSetting systemParamSettings[] = {
	ReportedConfigSetting("PSPModel", &g_PConfig.iPSPModel, &DefaultPSPModel, true, true),
	ReportedConfigSetting("PSPFirmwareVersion", &g_PConfig.iFirmwareVersion, PSP_DEFAULT_FIRMWARE, true, true),
	ConfigSetting("NickName", &g_PConfig.sNickName, "PPSSPP", true, true),
	ConfigSetting("proAdhocServer", &g_PConfig.proAdhocServer, "myneighborsushicat.com", true, true),
	ConfigSetting("MacAddress", &g_PConfig.sMACAddress, "", true, true),
	ConfigSetting("PortOffset", &g_PConfig.iPortOffset, 0, true, true),
	ReportedConfigSetting("Language", &g_PConfig.iLanguage, &DefaultSystemParamLanguage, true, true),
	ConfigSetting("TimeFormat", &g_PConfig.iTimeFormat, PSP_SYSTEMPARAM_TIME_FORMAT_24HR, true, true),
	ConfigSetting("DateFormat", &g_PConfig.iDateFormat, PSP_SYSTEMPARAM_DATE_FORMAT_YYYYMMDD, true, true),
	ConfigSetting("TimeZone", &g_PConfig.iTimeZone, 0, true, true),
	ConfigSetting("DayLightSavings", &g_PConfig.bDayLightSavings, (bool) PSP_SYSTEMPARAM_DAYLIGHTSAVINGS_STD, true, true),
	ReportedConfigSetting("ButtonPreference", &g_PConfig.iButtonPreference, PSP_SYSTEMPARAM_BUTTON_CROSS, true, true),
	ConfigSetting("LockParentalLevel", &g_PConfig.iLockParentalLevel, 0, true, true),
	ConfigSetting("WlanAdhocChannel", &g_PConfig.iWlanAdhocChannel, PSP_SYSTEMPARAM_ADHOC_CHANNEL_AUTOMATIC, true, true),
#if defined(USING_WIN_UI)
	ConfigSetting("BypassOSKWithKeyboard", &g_PConfig.bBypassOSKWithKeyboard, false, true, true),
#endif
	ConfigSetting("WlanPowerSave", &g_PConfig.bWlanPowerSave, (bool) PSP_SYSTEMPARAM_WLAN_POWERSAVE_OFF, true, true),
	ReportedConfigSetting("EncryptSave", &g_PConfig.bEncryptSave, true, true, true),
	ConfigSetting("SavedataUpgradeVersion", &g_PConfig.bSavedataUpgrade, true, true, false),

	ConfigSetting(false),
};

static ConfigSetting debuggerSettings[] = {
	ConfigSetting("DisasmWindowX", &g_PConfig.iDisasmWindowX, -1),
	ConfigSetting("DisasmWindowY", &g_PConfig.iDisasmWindowY, -1),
	ConfigSetting("DisasmWindowW", &g_PConfig.iDisasmWindowW, -1),
	ConfigSetting("DisasmWindowH", &g_PConfig.iDisasmWindowH, -1),
	ConfigSetting("GEWindowX", &g_PConfig.iGEWindowX, -1),
	ConfigSetting("GEWindowY", &g_PConfig.iGEWindowY, -1),
	ConfigSetting("GEWindowW", &g_PConfig.iGEWindowW, -1),
	ConfigSetting("GEWindowH", &g_PConfig.iGEWindowH, -1),
	ConfigSetting("ConsoleWindowX", &g_PConfig.iConsoleWindowX, -1),
	ConfigSetting("ConsoleWindowY", &g_PConfig.iConsoleWindowY, -1),
	ConfigSetting("FontWidth", &g_PConfig.iFontWidth, 8),
	ConfigSetting("FontHeight", &g_PConfig.iFontHeight, 12),
	ConfigSetting("DisplayStatusBar", &g_PConfig.bDisplayStatusBar, true),
	ConfigSetting("ShowBottomTabTitles",&g_PConfig.bShowBottomTabTitles, true),
	ConfigSetting("ShowDeveloperMenu", &g_PConfig.bShowDeveloperMenu, false),
	ConfigSetting("ShowAllocatorDebug", &g_PConfig.bShowAllocatorDebug, false, false),
	ConfigSetting("ShowGpuProfile", &g_PConfig.bShowGpuProfile, false, false),
	ConfigSetting("SkipDeadbeefFilling", &g_PConfig.bSkipDeadbeefFilling, false),
	ConfigSetting("FuncHashMap", &g_PConfig.bFuncHashMap, false),

	ConfigSetting(false),
};

static ConfigSetting jitSettings[] = {
	ReportedConfigSetting("DiscardRegsOnJRRA", &g_PConfig.bDiscardRegsOnJRRA, false, false),

	ConfigSetting(false),
};

static ConfigSetting upgradeSettings[] = {
	ConfigSetting("UpgradeMessage", &g_PConfig.upgradeMessage, ""),
	ConfigSetting("UpgradeVersion", &g_PConfig.upgradeVersion, ""),
	ConfigSetting("DismissedVersion", &g_PConfig.dismissedVersion, ""),

	ConfigSetting(false),
};

static ConfigSetting themeSettings[] = {
	ConfigSetting("ItemStyleFg", &g_PConfig.uItemStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ItemStyleBg", &g_PConfig.uItemStyleBg, 0x55000000, true, false),
	ConfigSetting("ItemFocusedStyleFg", &g_PConfig.uItemFocusedStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ItemFocusedStyleBg", &g_PConfig.uItemFocusedStyleBg, 0xFFEDC24C, true, false),
	ConfigSetting("ItemDownStyleFg", &g_PConfig.uItemDownStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ItemDownStyleBg", &g_PConfig.uItemDownStyleBg, 0xFFBD9939, true, false),
	ConfigSetting("ItemDisabledStyleFg", &g_PConfig.uItemDisabledStyleFg, 0x80EEEEEE, true, false),
	ConfigSetting("ItemDisabledStyleBg", &g_PConfig.uItemDisabledStyleBg, 0x55E0D4AF, true, false),
	ConfigSetting("ItemHighlightedStyleFg", &g_PConfig.uItemHighlightedStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ItemHighlightedStyleBg", &g_PConfig.uItemHighlightedStyleBg, 0x55BDBB39, true, false),

	ConfigSetting("ButtonStyleFg", &g_PConfig.uButtonStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ButtonStyleBg", &g_PConfig.uButtonStyleBg, 0x55000000, true, false),
	ConfigSetting("ButtonFocusedStyleFg", &g_PConfig.uButtonFocusedStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ButtonFocusedStyleBg", &g_PConfig.uButtonFocusedStyleBg, 0xFFEDC24C, true, false),
	ConfigSetting("ButtonDownStyleFg", &g_PConfig.uButtonDownStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ButtonDownStyleBg", &g_PConfig.uButtonDownStyleBg, 0xFFBD9939, true, false),
	ConfigSetting("ButtonDisabledStyleFg", &g_PConfig.uButtonDisabledStyleFg, 0x80EEEEEE, true, false),
	ConfigSetting("ButtonDisabledStyleBg", &g_PConfig.uButtonDisabledStyleBg, 0x55E0D4AF, true, false),
	ConfigSetting("ButtonHighlightedStyleFg", &g_PConfig.uButtonHighlightedStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("ButtonHighlightedStyleBg", &g_PConfig.uButtonHighlightedStyleBg, 0x55BDBB39, true, false),

	ConfigSetting("HeaderStyleFg", &g_PConfig.uHeaderStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("InfoStyleFg", &g_PConfig.uInfoStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("InfoStyleBg", &g_PConfig.uInfoStyleBg, 0x00000000U, true, false),
	ConfigSetting("PopupTitleStyleFg", &g_PConfig.uPopupTitleStyleFg, 0xFFE3BE59, true, false),
	ConfigSetting("PopupStyleFg", &g_PConfig.uPopupStyleFg, 0xFFFFFFFF, true, false),
	ConfigSetting("PopupStyleBg", &g_PConfig.uPopupStyleBg, 0xFF303030, true, false),

	ConfigSetting(false),
};

static ConfigSectionSettings sections[] = {
	{"General", generalSettings},
	{"CPU", cpuSettings},
	{"Graphics", graphicsSettings},
	{"Sound", soundSettings},
	{"Control", controlSettings},
	{"Network", networkSettings},
	{"SystemParam", systemParamSettings},
	{"Debugger", debuggerSettings},
	{"JIT", jitSettings},
	{"Upgrade", upgradeSettings},
	{"Theme", themeSettings},
};

static void IterateSettings(PIniFile &iniFile, std::function<void(PIniFile::Section *section, ConfigSetting *setting)> func) {
	for (size_t i = 0; i < ARRAY_SIZE(sections); ++i) {
		PIniFile::Section *section = iniFile.GetOrCreateSection(sections[i].section);
		for (auto setting = sections[i].settings; setting->HasMore(); ++setting) {
			func(section, setting);
		}
	}
}

Config::Config() : bGameSpecific(false) { }
Config::~Config() { }

std::map<std::string, std::pair<std::string, int>> GetLangValuesMapping() {
	std::map<std::string, std::pair<std::string, int>> langValuesMapping;
	PIniFile mapping;
	mapping.LoadFromVFS("langregion.ini");
	std::vector<std::string> keys;
	mapping.GetKeys("LangRegionNames", keys);


	std::map<std::string, int> langCodeMapping;
	langCodeMapping["JAPANESE"] = PSP_SYSTEMPARAM_LANGUAGE_JAPANESE;
	langCodeMapping["ENGLISH"] = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	langCodeMapping["FRENCH"] = PSP_SYSTEMPARAM_LANGUAGE_FRENCH;
	langCodeMapping["SPANISH"] = PSP_SYSTEMPARAM_LANGUAGE_SPANISH;
	langCodeMapping["GERMAN"] = PSP_SYSTEMPARAM_LANGUAGE_GERMAN;
	langCodeMapping["ITALIAN"] = PSP_SYSTEMPARAM_LANGUAGE_ITALIAN;
	langCodeMapping["DUTCH"] = PSP_SYSTEMPARAM_LANGUAGE_DUTCH;
	langCodeMapping["PORTUGUESE"] = PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE;
	langCodeMapping["RUSSIAN"] = PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN;
	langCodeMapping["KOREAN"] = PSP_SYSTEMPARAM_LANGUAGE_KOREAN;
	langCodeMapping["CHINESE_TRADITIONAL"] = PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL;
	langCodeMapping["CHINESE_SIMPLIFIED"] = PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED;

	PIniFile::Section *langRegionNames = mapping.GetOrCreateSection("LangRegionNames");
	PIniFile::Section *systemLanguage = mapping.GetOrCreateSection("SystemLanguage");

	for (size_t i = 0; i < keys.size(); i++) {
		std::string langName;
		langRegionNames->Get(keys[i].c_str(), &langName, "ERROR");
		std::string langCode;
		systemLanguage->Get(keys[i].c_str(), &langCode, "ENGLISH");
		int iLangCode = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
		if (langCodeMapping.find(langCode) != langCodeMapping.end())
			iLangCode = langCodeMapping[langCode];
		langValuesMapping[keys[i]] = std::make_pair(langName, iLangCode);
	}
	return langValuesMapping;
}

void Config::Load(const char *iniFileName, const char *controllerPIniFilename) {
	const bool usePIniFilename = iniFileName != nullptr && strlen(iniFileName) > 0;
	iniFilename_ = FindConfigFile(usePIniFilename ? iniFileName : "ppsspp.ini");

	const bool useControllerPIniFilename = controllerPIniFilename != nullptr && strlen(controllerPIniFilename) > 0;
	controllerPIniFilename_ = FindConfigFile(useControllerPIniFilename ? controllerPIniFilename : "controls.ini");

	INFO_LOG(LOADER, "Loading config: %s", iniFilename_.c_str());
	bSaveSettings = true;

	bShowFrameProfiler = true;

	PIniFile iniFile;
	if (!iniFile.Load(iniFilename_)) {
		ERROR_LOG(LOADER, "Failed to read '%s'. Setting config to default.", iniFilename_.c_str());
		// Continue anyway to initialize the config.
	}

	IterateSettings(iniFile, [](PIniFile::Section *section, ConfigSetting *setting) {
		setting->Get(section);
	});

	iRunCount++;
	if (!PFile::Exists(currentDirectory))
		currentDirectory = "";

	PIniFile::Section *log = iniFile.GetOrCreateSection(logSectionName);

	bool debugDefaults = false;
#ifdef _DEBUG
	debugDefaults = true;
#endif
	LogManager::GetInstance()->LoadConfig(log, debugDefaults);

	PIniFile::Section *recent = iniFile.GetOrCreateSection("Recent");
	recent->Get("MaxRecent", &iMaxRecent, 30);

	// Fix issue from switching from uint (hex in .ini) to int (dec)
	// -1 is okay, though. We'll just ignore recent stuff if it is.
	if (iMaxRecent == 0)
		iMaxRecent = 30;

	if (iMaxRecent > 0) {
		recentIsos.clear();
		for (int i = 0; i < iMaxRecent; i++) {
			char keyName[64];
			std::string fileName;

			snprintf(keyName, sizeof(keyName), "FileName%d", i);
			if (recent->Get(keyName, &fileName, "") && !fileName.empty()) {
				recentIsos.push_back(fileName);
			}
		}
	}

	auto pinnedPaths = iniFile.GetOrCreateSection("PinnedPaths")->ToMap();
	vPinnedPaths.clear();
	for (auto it = pinnedPaths.begin(), end = pinnedPaths.end(); it != end; ++it) {
		// Unpin paths that are deleted automatically.
		if (PFile::Exists(it->second)) {
			vPinnedPaths.push_back(PFile::ResolvePath(it->second));
		}
	}

	// This caps the exponent 4 (so 16x.)
	if (iAnisotropyLevel > 4) {
		iAnisotropyLevel = 4;
	}
	if (iRenderingMode != FB_NON_BUFFERED_MODE && iRenderingMode != FB_BUFFERED_MODE) {
		g_PConfig.iRenderingMode = FB_BUFFERED_MODE;
	}

	// Check for an old dpad setting
	PIniFile::Section *control = iniFile.GetOrCreateSection("Control");
	float f;
	control->Get("DPadRadius", &f, 0.0f);
	if (f > 0.0f) {
		ResetControlLayout();
	}

	const char *gitVer = PPSSPP_GIT_VERSION;
	Version installed(gitVer);
	Version upgrade(upgradeVersion);
	const bool versionsValid = installed.IsValid() && upgrade.IsValid();

	// Do this regardless of iRunCount to prevent a silly bug where one might use an older
	// build of PPSSPP, receive an upgrade notice, then start a newer version, and still receive the upgrade notice,
	// even if said newer version is >= the upgrade found online.
	if ((dismissedVersion == upgradeVersion) || (versionsValid && (installed >= upgrade))) {
		upgradeMessage = "";
	}

	// Check for new version on every 10 runs.
	// Sometimes the download may not be finished when the main screen shows (if the user dismisses the
	// splash screen quickly), but then we'll just show the notification next time instead, we store the
	// upgrade number in the ini.
	/*if (iRunCount % 10 == 0 && bCheckForNewVersion) {
		std::shared_ptr<http::Download> dl = g_DownloadManager.StartDownloadWithCallback(
			"http://www.ppsspp.org/version.json", "", &DownloadCompletedCallback);
		dl->SetHidden(true);
	}*/

	INFO_LOG(LOADER, "Loading controller config: %s", controllerPIniFilename_.c_str());
	bSaveSettings = true;

	LoadStandardControllerIni();

	//so this is all the way down here to overwrite the controller settings
	//sadly it won't benefit from all the "version conversion" going on up-above
	//but these configs shouldn't contain older versions anyhow
	if (bGameSpecific) {
		loadGameConfig(gameId_, gameIdTitle_);
	}

	CleanRecent();

	// Set a default MAC, and correct if it's an old format.
	if (sMACAddress.length() != 17)
		sMACAddress = CreateRandMAC();

	if (g_PConfig.bAutoFrameSkip && g_PConfig.iRenderingMode == FB_NON_BUFFERED_MODE) {
		g_PConfig.iRenderingMode = FB_BUFFERED_MODE;
	}

	// Override ppsspp.ini JIT value to prevent crashing
	if (DefaultCpuCore() != (int)CPUCore::JIT && g_PConfig.iCpuCore == (int)CPUCore::JIT) {
		jitForcedOff = true;
		g_PConfig.iCpuCore = (int)CPUCore::INTERPRETER;
	}
}

void Config::Save(const char *saveReason) {
	if (jitForcedOff) {
		// if JIT has been forced off, we don't want to screw up the user's ppsspp.ini
		g_PConfig.iCpuCore = (int)CPUCore::JIT;
	}
	if (iniFilename_.size() && g_PConfig.bSaveSettings) {
		saveGameConfig(gameId_, gameIdTitle_);

		CleanRecent();
		PIniFile iniFile;
		if (!iniFile.Load(iniFilename_.c_str())) {
			ERROR_LOG(LOADER, "Error saving config - can't read ini '%s'", iniFilename_.c_str());
		}

		// Need to do this somewhere...
		bFirstRun = false;

		IterateSettings(iniFile, [&](PIniFile::Section *section, ConfigSetting *setting) {
			if (!bGameSpecific || !setting->perGame_) {
				setting->Set(section);
			}
		});

		PIniFile::Section *recent = iniFile.GetOrCreateSection("Recent");
		recent->Set("MaxRecent", iMaxRecent);

		for (int i = 0; i < iMaxRecent; i++) {
			char keyName[64];
			snprintf(keyName, sizeof(keyName), "FileName%d", i);
			if (i < (int)recentIsos.size()) {
				recent->Set(keyName, recentIsos[i]);
			} else {
				recent->Delete(keyName); // delete the nonexisting FileName
			}
		}

		PIniFile::Section *pinnedPaths = iniFile.GetOrCreateSection("PinnedPaths");
		pinnedPaths->Clear();
		for (size_t i = 0; i < vPinnedPaths.size(); ++i) {
			char keyName[64];
			snprintf(keyName, sizeof(keyName), "Path%d", (int)i);
			pinnedPaths->Set(keyName, vPinnedPaths[i]);
		}

		PIniFile::Section *control = iniFile.GetOrCreateSection("Control");
		control->Delete("DPadRadius");

		PIniFile::Section *log = iniFile.GetOrCreateSection(logSectionName);
		if (LogManager::GetInstance())
			LogManager::GetInstance()->SaveConfig(log);

		if (!iniFile.Save(iniFilename_.c_str())) {
			ERROR_LOG(LOADER, "Error saving config (%s)- can't write ini '%s'", saveReason, iniFilename_.c_str());
			System_SendMessage("toast", "Failed to save settings!\nCheck permissions, or try to restart the device.");
			return;
		}
		INFO_LOG(LOADER, "Config saved (%s): '%s'", saveReason, iniFilename_.c_str());

		if (!bGameSpecific) //otherwise we already did this in saveGameConfig()
		{
			PIniFile controllerPIniFile;
			if (!controllerPIniFile.Load(controllerPIniFilename_.c_str())) {
				ERROR_LOG(LOADER, "Error saving config - can't read ini '%s'", controllerPIniFilename_.c_str());
			}
			KeyMap::SaveToIni(controllerPIniFile);
			if (!controllerPIniFile.Save(controllerPIniFilename_.c_str())) {
				ERROR_LOG(LOADER, "Error saving config - can't write ini '%s'", controllerPIniFilename_.c_str());
				return;
			}
			INFO_LOG(LOADER, "Controller config saved: %s", controllerPIniFilename_.c_str());
		}
	} else {
		INFO_LOG(LOADER, "Not saving config");
	}
	if (jitForcedOff) {
		// force JIT off again just in case Config::Save() is called without exiting PPSSPP
		g_PConfig.iCpuCore = (int)CPUCore::INTERPRETER;
	}
}

// Use for debugging the version check without messing with the server
#if 0
#define PPSSPP_GIT_VERSION "v0.0.1-gaaaaaaaaa"
#endif

void Config::DownloadCompletedCallback(http::Download &download) {
	if (download.ResultCode() != 200) {
		ERROR_LOG(LOADER, "Failed to download %s: %d", download.url().c_str(), download.ResultCode());
		return;
	}
	std::string data;
	download.buffer().TakeAll(&data);
	if (data.empty()) {
		ERROR_LOG(LOADER, "Version check: Empty data from server!");
		return;
	}

	json::JsonReader reader(data.c_str(), data.size());
	const json::JsonGet root = reader.root();
	if (!root) {
		ERROR_LOG(LOADER, "Failed to parse json");
		return;
	}

	std::string version = root.getString("version", "");

	const char *gitVer = PPSSPP_GIT_VERSION;
	Version installed(gitVer);
	Version upgrade(version);
	Version dismissed(g_PConfig.dismissedVersion);

	if (!installed.IsValid()) {
		ERROR_LOG(LOADER, "Version check: Local version string invalid. Build problems? %s", PPSSPP_GIT_VERSION);
		return;
	}
	if (!upgrade.IsValid()) {
		ERROR_LOG(LOADER, "Version check: Invalid server version: %s", version.c_str());
		return;
	}

	if (installed >= upgrade) {
		INFO_LOG(LOADER, "Version check: Already up to date, erasing any upgrade message");
		g_PConfig.upgradeMessage = "";
		g_PConfig.upgradeVersion = upgrade.ToString();
		g_PConfig.dismissedVersion = "";
		return;
	}

	if (installed < upgrade && dismissed != upgrade) {
		g_PConfig.upgradeMessage = "New version of PPSSPP available!";
		g_PConfig.upgradeVersion = upgrade.ToString();
		g_PConfig.dismissedVersion = "";
	}
}

void Config::DismissUpgrade() {
	g_PConfig.dismissedVersion = g_PConfig.upgradeVersion;
}

void Config::AddRecent(const std::string &file) {
	// Don't bother with this if the user disabled recents (it's -1).
	if (iMaxRecent <= 0)
		return;

	// We'll add it back below.  This makes sure it's at the front, and only once.
	RemoveRecent(file);

	const std::string filename = PFile::ResolvePath(file);
	recentIsos.insert(recentIsos.begin(), filename);
	if ((int)recentIsos.size() > iMaxRecent)
		recentIsos.resize(iMaxRecent);
}

void Config::RemoveRecent(const std::string &file) {
	// Don't bother with this if the user disabled recents (it's -1).
	if (iMaxRecent <= 0)
		return;

	const std::string filename = PFile::ResolvePath(file);
	for (auto iter = recentIsos.begin(); iter != recentIsos.end();) {
		const std::string recent = PFile::ResolvePath(*iter);
		if (filename == recent) {
			// Note that the increment-erase idiom doesn't work with vectors.
			iter = recentIsos.erase(iter);
		} else {
			iter++;
		}
	}
}

void Config::CleanRecent() {
	std::vector<std::string> cleanedRecent;
	for (size_t i = 0; i < recentIsos.size(); i++) {
		FileLoader *loader = ConstructFileLoader(recentIsos[i]);
		if (loader->ExistsFast()) {
			// Make sure we don't have any redundant items.
			auto duplicate = std::find(cleanedRecent.begin(), cleanedRecent.end(), recentIsos[i]);
			if (duplicate == cleanedRecent.end()) {
				cleanedRecent.push_back(recentIsos[i]);
			}
		}
		delete loader;
	}
	recentIsos = cleanedRecent;
}

void Config::SetDefaultPath(const std::string &defaultPath) {
	defaultPath_ = defaultPath;
}

void Config::AddSearchPath(const std::string &path) {
	searchPath_.push_back(path);
}

const std::string Config::FindConfigFile(const std::string &baseFilename) {
	// Don't search for an absolute path.
	if (baseFilename.size() > 1 && baseFilename[0] == '/') {
		return baseFilename;
	}
#ifdef _WIN32
	if (baseFilename.size() > 3 && baseFilename[1] == ':' && (baseFilename[2] == '/' || baseFilename[2] == '\\')) {
		return baseFilename;
	}
#endif

	for (size_t i = 0; i < searchPath_.size(); ++i) {
		std::string filename = searchPath_[i] + baseFilename;
		if (PFile::Exists(filename)) {
			return filename;
		}
	}

	const std::string filename = defaultPath_.empty() ? baseFilename : defaultPath_ + baseFilename;
	if (!PFile::Exists(filename)) {
		std::string path;
		PSplitPath(filename, &path, NULL, NULL);
		if (createdPath_ != path) {
			PFile::CreateFullPath(path);
			createdPath_ = path;
		}
	}
	return filename;
}

void Config::RestoreDefaults() {
	if (bGameSpecific) {
		deleteGameConfig(gameId_);
		createGameConfig(gameId_);
	} else {
		if (PFile::Exists(iniFilename_))
			PFile::Delete(iniFilename_);
		recentIsos.clear();
		currentDirectory = "";
	}
	Load();
}

bool Config::hasGameConfig(const std::string &pGameId) {
	std::string fullPIniFilePath = getGameConfigFile(pGameId);
	return PFile::Exists(fullPIniFilePath);
}

void Config::changeGameSpecific(const std::string &pGameId, const std::string &title) {
	Save("changeGameSpecific");
	gameId_ = pGameId;
	gameIdTitle_ = title;
	bGameSpecific = !pGameId.empty();
}

bool Config::createGameConfig(const std::string &pGameId) {
	std::string fullPIniFilePath = getGameConfigFile(pGameId);

	if (hasGameConfig(pGameId)) {
		return false;
	}

	PFile::CreateEmptyFile(fullPIniFilePath);

	return true;
}

bool Config::deleteGameConfig(const std::string& pGameId) {
	std::string fullPIniFilePath = getGameConfigFile(pGameId);

	PFile::Delete(fullPIniFilePath);
	return true;
}

std::string Config::getGameConfigFile(const std::string &pGameId) {
	std::string iniFileName = pGameId + "_ppsspp.ini";
	std::string iniFileNameFull = FindConfigFile(iniFileName);

	return iniFileNameFull;
}

bool Config::saveGameConfig(const std::string &pGameId, const std::string &title) {
	if (pGameId.empty()) {
		return false;
	}

	std::string fullPIniFilePath = getGameConfigFile(pGameId);

	PIniFile iniFile;

	PIniFile::Section *top = iniFile.GetOrCreateSection("");
	top->AddComment(PStringFromFormat("Game config for %s - %s", pGameId.c_str(), title.c_str()));

	IterateSettings(iniFile, [](PIniFile::Section *section, ConfigSetting *setting) {
		if (setting->perGame_) {
			setting->Set(section);
		}
	});

	KeyMap::SaveToIni(iniFile);
	iniFile.Save(fullPIniFilePath);

	return true;
}

bool Config::loadGameConfig(const std::string &pGameId, const std::string &title) {
	std::string iniFileNameFull = getGameConfigFile(pGameId);

	if (!hasGameConfig(pGameId)) {
		INFO_LOG(LOADER, "Failed to read %s. No game-specific settings found, using global defaults.", iniFileNameFull.c_str());
		return false;
	}

	changeGameSpecific(pGameId, title);
	PIniFile iniFile;
	iniFile.Load(iniFileNameFull);

	IterateSettings(iniFile, [](PIniFile::Section *section, ConfigSetting *setting) {
		if (setting->perGame_) {
			setting->Get(section);
		}
	});

	KeyMap::LoadFromIni(iniFile);
	return true;
}

void Config::unloadGameConfig() {
	if (bGameSpecific){
		changeGameSpecific();

		PIniFile iniFile;
		iniFile.Load(iniFilename_);

		// Reload game specific settings back to standard.
		IterateSettings(iniFile, [](PIniFile::Section *section, ConfigSetting *setting) {
			if (setting->perGame_) {
				setting->Get(section);
			}
		});

		LoadStandardControllerIni();
	}
}

void Config::LoadStandardControllerIni() {
	PIniFile controllerPIniFile;
	if (!controllerPIniFile.Load(controllerPIniFilename_)) {
		ERROR_LOG(LOADER, "Failed to read %s. Setting controller config to default.", controllerPIniFilename_.c_str());
		KeyMap::RestoreDefault();
	} else {
		// Continue anyway to initialize the config. It will just restore the defaults.
		KeyMap::LoadFromIni(controllerPIniFile);
	}
}

void Config::ResetControlLayout() {
	auto reset = [](ConfigTouchPos &pos) {
		pos.x = defaultTouchPosShow.x;
		pos.y = defaultTouchPosShow.y;
		pos.scale = defaultTouchPosShow.scale;
	};
	reset(g_PConfig.touchActionButtonCenter);
	g_PConfig.fActionButtonSpacing = 1.0f;
	reset(g_PConfig.touchDpad);
	g_PConfig.fDpadSpacing = 1.0f;
	reset(g_PConfig.touchStartKey);
	reset(g_PConfig.touchSelectKey);
	reset(g_PConfig.touchUnthrottleKey);
	reset(g_PConfig.touchLKey);
	reset(g_PConfig.touchRKey);
	reset(g_PConfig.touchAnalogStick);
	reset(g_PConfig.touchRightAnalogStick);
	reset(g_PConfig.touchCombo0);
	reset(g_PConfig.touchCombo1);
	reset(g_PConfig.touchCombo2);
	reset(g_PConfig.touchCombo3);
	reset(g_PConfig.touchCombo4);
	reset(g_PConfig.touchSpeed1Key);
	reset(g_PConfig.touchSpeed2Key);
}

void Config::GetReportingInfo(UrlEncoder &data) {
	for (size_t i = 0; i < ARRAY_SIZE(sections); ++i) {
		const std::string prefix = std::string("config.") + sections[i].section;
		for (auto setting = sections[i].settings; setting->HasMore(); ++setting) {
			setting->Report(data, prefix);
		}
	}
}

bool Config::IsPortrait() const {
	return (iInternalScreenRotation == ROTATION_LOCKED_VERTICAL || iInternalScreenRotation == ROTATION_LOCKED_VERTICAL180) && iRenderingMode != FB_NON_BUFFERED_MODE;
}
