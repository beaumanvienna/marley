// Copyright (C) 2012 PPSSPP Project

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

#include "ppsspp_config.h"

#ifdef _WIN32
#pragma warning(disable:4091)
#include "Common/CommonWindows.h"
#include <ShlObj.h>
#include <string>
#include <codecvt>
#if !PPSSPP_PLATFORM(UWP)
#include "Windows/W32Util/ShellUtil.h"
#endif
#endif

#include <thread>
#include <mutex>
#include <condition_variable>

#include "Common/System/System.h"
#include "Common/Math/math_util.h"
#include "Common/Thread/ThreadUtil.h"
#include "Common/Data/Encoding/Utf8.h"

#include "Common/File/FileUtil.h"
#include "Common/TimeUtil.h"
#include "Common/GraphicsContext.h"
#include "Core/MemFault.h"
#include "Core/HDRemaster.h"
#include "Core/MIPS/MIPS.h"
#include "Core/MIPS/MIPSAnalyst.h"
#include "Core/Debugger/SymbolMap.h"
#include "Core/Host.h"
#include "Core/System.h"
#include "Core/HLE/HLE.h"
#include "Core/HLE/Plugins.h"
#include "Core/HLE/ReplaceTables.h"
#include "Core/HLE/sceKernel.h"
#include "Core/HLE/sceKernelMemory.h"
#include "Core/HLE/sceAudio.h"
#include "Core/Config.h"
#include "Core/Core.h"
#include "Core/CoreTiming.h"
#include "Core/CoreParameter.h"
#include "Core/FileLoaders/RamCachingFileLoader.h"
#include "Core/FileSystems/MetaFileSystem.h"
#include "Core/Loaders.h"
#include "Core/PSPLoaders.h"
#include "Core/ELF/ParamSFO.h"
#include "Core/SaveState.h"
#include "Common/LogManager.h"
#include "Common/ExceptionHandlerSetup.h"
#include "Core/HLE/sceAudiocodec.h"

#include "GPU/GPUState.h"
#include "GPU/GPUInterface.h"
#include "GPU/Debugger/RecordFormat.h"

enum CPUThreadState {
	CPU_THREAD_NOT_RUNNING,
	CPU_THREAD_PENDING,
	CPU_THREAD_STARTING,
	CPU_THREAD_RUNNING,
	CPU_THREAD_SHUTDOWN,
	CPU_THREAD_QUIT,

	CPU_THREAD_EXECUTE,
	CPU_THREAD_RESUME,
};

MetaFileSystem pspFileSystem;
ParamSFOData g_paramSFO;
static GlobalUIState globalUIState;
static CoreParameter coreParameter;
static FileLoader *loadedFile;
// For background loading thread.
static std::mutex loadingLock;
// For loadingReason updates.
static std::mutex loadingReasonLock;
static std::string loadingReason;

bool audioInitialized;

bool coreCollectDebugStats = false;
bool coreCollectDebugStatsForced = false;

// This can be read and written from ANYWHERE.
volatile CoreState coreState = CORE_STEPPING;
// If true, core state has been changed, but JIT has probably not noticed yet.
volatile bool coreStatePending = false;

static volatile CPUThreadState cpuThreadState = CPU_THREAD_NOT_RUNNING;

static GPUBackend gpuBackend;
static std::string gpuBackendDevice;

// Ugly!
static volatile bool pspIsInited = false;
static volatile bool pspIsIniting = false;
static volatile bool pspIsQuitting = false;

void ResetUIState() {
	globalUIState = UISTATE_MENU;
}

void UpdateUIState(GlobalUIState newState) {
	// Never leave the EXIT state.
	if (globalUIState != newState && globalUIState != UISTATE_EXIT) {
		globalUIState = newState;
		host->UpdateDisassembly();
		const char *state = nullptr;
		switch (globalUIState) {
		case UISTATE_EXIT: state = "exit";  break;
		case UISTATE_INGAME: state = "ingame"; break;
		case UISTATE_MENU: state = "menu"; break;
		case UISTATE_PAUSEMENU: state = "pausemenu"; break;
		}
		if (state) {
			System_SendMessage("uistate", state);
		}
	}
}

GlobalUIState GetUIState() {
	return globalUIState;
}

void SetGPUBackend(GPUBackend type, const std::string &device) {
	gpuBackend = type;
	gpuBackendDevice = device;
}

GPUBackend GetGPUBackend() {
	return gpuBackend;
}

std::string GetGPUBackendDevice() {
	return gpuBackendDevice;
}

bool IsAudioInitialised() {
	return audioInitialized;
}

void Audio_Init() {
	if (!audioInitialized) {
		audioInitialized = true;
		host->InitSound();
	}
}

void Audio_Shutdown() {
	if (audioInitialized) {
		audioInitialized = false;
		host->ShutdownSound();
	}
}

bool CPU_IsReady() {
	if (coreState == CORE_POWERUP)
		return false;
	return cpuThreadState == CPU_THREAD_RUNNING || cpuThreadState == CPU_THREAD_NOT_RUNNING;
}

bool CPU_IsShutdown() {
	return cpuThreadState == CPU_THREAD_NOT_RUNNING;
}

bool CPU_HasPendingAction() {
	return cpuThreadState != CPU_THREAD_RUNNING;
}

void CPU_Shutdown();

bool DiscIDFromGEDumpPath(const std::string &path, FileLoader *fileLoader, std::string *id) {
	using namespace GPURecord;

	// For newer files, it's stored in the dump.
	Header header;
	if (fileLoader->ReadAt(0, sizeof(header), &header) == sizeof(header)) {
		const bool magicMatch = memcmp(header.magic, HEADER_MAGIC, sizeof(header.magic)) == 0;
		if (magicMatch && header.version <= VERSION && header.version >= 4) {
			size_t gameIDLength = strnlen(header.gameID, sizeof(header.gameID));
			if (gameIDLength != 0) {
				*id = std::string(header.gameID, gameIDLength);
				return true;
			}
		}
	}

	// Fall back to using the filename.
	std::string filename = PFile::GetFilename(path);
	// Could be more discerning, but hey..
	if (filename.size() > 10 && filename[0] == 'U' && filename[9] == '_') {
		*id = filename.substr(0, 9);
		return true;
	} else {
		return false;
	}
}

bool CPU_Init() {
	coreState = CORE_POWERUP;
	currentMIPS = &mipsr4k;

	g_symbolMap = new SymbolMap();

	// Default memory settings
	// Seems to be the safest place currently..
	PMemory::g_MemorySize = PMemory::RAM_NORMAL_SIZE; // 32 MB of ram by default

	g_RemasterMode = false;
	g_DoubleTextureCoordinates = false;
	PMemory::g_PSPModel = g_PConfig.iPSPModel;

	std::string filename = coreParameter.fileToStart;
	loadedFile = ResolveFileLoaderTarget(ConstructFileLoader(filename));
#ifdef _M_X64
	if (g_PConfig.bCacheFullIsoInRam) {
		loadedFile = new RamCachingFileLoader(loadedFile);
	}
#endif
	IdentifiedFileType type = Identify_File(loadedFile);

	// TODO: Put this somewhere better?
	if (coreParameter.mountIso != "") {
		coreParameter.mountIsoLoader = ConstructFileLoader(coreParameter.mountIso);
	}

	MIPSAnalyst::Reset();
	Replacement_Init();

	std::string discID;

	switch (type) {
	case IdentifiedFileType::PSP_ISO:
	case IdentifiedFileType::PSP_ISO_NP:
	case IdentifiedFileType::PSP_DISC_DIRECTORY:
		InitMemoryForGameISO(loadedFile);
		discID = g_paramSFO.GetDiscID();
		break;
	case IdentifiedFileType::PSP_PBP:
	case IdentifiedFileType::PSP_PBP_DIRECTORY:
		// This is normal for homebrew.
		// ERROR_LOG(LOADER, "PBP directory resolution failed.");
		InitMemoryForGamePBP(loadedFile);
		discID = g_paramSFO.GetDiscID();
		break;
	case IdentifiedFileType::PSP_ELF:
		if (PMemory::g_PSPModel != PSP_MODEL_FAT) {
			INFO_LOG(LOADER, "ELF, using full PSP-2000 memory access");
			PMemory::g_MemorySize = PMemory::RAM_DOUBLE_SIZE;
		}
		discID = g_paramSFO.GetDiscID();
		break;
	case IdentifiedFileType::PPSSPP_GE_DUMP:
		// Try to grab the disc ID from the filename, since unfortunately, we don't store
		// it in the GE dump. This should probably be fixed, but as long as you don't rename the dumps,
		// this will do the trick.
		if (!DiscIDFromGEDumpPath(filename, loadedFile, &discID)) {
			// Failed? Let the param SFO autogen a fake disc ID.
			discID = g_paramSFO.GetDiscID();
		}
		break;
	default:
		discID = g_paramSFO.GetDiscID();
		break;
	}

	// Here we have read the PARAM.SFO, let's see if we need any compatibility overrides.
	// Homebrew usually has an empty discID, and even if they do have a disc id, it's not
	// likely to collide with any commercial ones.
	coreParameter.compat.Load(discID);

	HLEPlugins::Init();
	if (!PMemory::Init()) {
		// We're screwed.
		return false;
	}
	mipsr4k.Reset();

	host->AttemptLoadSymbolMap();

	if (coreParameter.enableSound) {
		Audio_Init();
	}

	PCoreTiming::Init();

	// Init all the HLE modules
	HLEInit();

	// TODO: Check Game INI here for settings, patches and cheats, and modify coreParameter accordingly

	// If they shut down early, we'll catch it when load completes.
	// Note: this may return before init is complete, which is checked if CPU_IsReady().
	if (!LoadFile(&loadedFile, &coreParameter.errorString)) {
		CPU_Shutdown();
		coreParameter.fileToStart = "";
		return false;
	}

	if (coreParameter.updateRecent) {
		g_PConfig.AddRecent(filename);
	}

	InstallExceptionHandler(&PMemory::HandleFault);
	return true;
}

PSP_LoadingLock::PSP_LoadingLock() {
	loadingLock.lock();
}

PSP_LoadingLock::~PSP_LoadingLock() {
	loadingLock.unlock();
}

void CPU_Shutdown() {
	UninstallExceptionHandler();

	// Since we load on a background thread, wait for startup to complete.
	PSP_LoadingLock lock;
	PSPLoaders_Shutdown();

	if (g_PConfig.bAutoSaveSymbolMap) {
		host->SaveSymbolMap();
	}

	Replacement_Shutdown();

	PCoreTiming::Shutdown();
	__KernelShutdown();
	HLEShutdown();
	if (coreParameter.enableSound) {
		Audio_Shutdown();
	}
	pspFileSystem.Shutdown();
	mipsr4k.Shutdown();
	PMemory::Shutdown();
	HLEPlugins::Shutdown();

	delete loadedFile;
	loadedFile = nullptr;

	delete coreParameter.mountIsoLoader;
	delete g_symbolMap;
	g_symbolMap = nullptr;

	coreParameter.mountIsoLoader = nullptr;
}

// TODO: Maybe loadedFile doesn't even belong here...
void UpdateLoadedFile(FileLoader *fileLoader) {
	delete loadedFile;
	loadedFile = fileLoader;
}

void Core_UpdateState(CoreState newState) {
	if ((coreState == CORE_RUNNING || coreState == CORE_NEXTFRAME) && newState != CORE_RUNNING)
		coreStatePending = true;
	coreState = newState;
	Core_UpdateSingleStep();
}

void Core_UpdateDebugStats(bool collectStats) {
	if (coreCollectDebugStats != collectStats) {
		coreCollectDebugStats = collectStats;
		mipsr4k.ClearJitCache();
	}

	kernelStats.ResetFrame();
	gpuStats.ResetFrame();
}

bool PSP_InitStart(const CoreParameter &coreParam, std::string *error_string) {
	if (pspIsIniting || pspIsQuitting) {
		return false;
	}

#if defined(_WIN32) && defined(_M_X64)
	INFO_LOG(BOOT, "PPSSPP %s Windows 64 bit", PPSSPP_GIT_VERSION);
#elif defined(_WIN32) && !defined(_M_X64)
	INFO_LOG(BOOT, "PPSSPP %s Windows 32 bit", PPSSPP_GIT_VERSION);
#else
	INFO_LOG(BOOT, "PPSSPP %s", PPSSPP_GIT_VERSION);
#endif

	Core_NotifyLifecycle(CoreLifecycle::STARTING);
	GraphicsContext *temp = coreParameter.graphicsContext;
	coreParameter = coreParam;
	if (coreParameter.graphicsContext == nullptr) {
		coreParameter.graphicsContext = temp;
	}
	coreParameter.errorString = "";
	pspIsIniting = true;
	PSP_SetLoading("Loading game...");

	if (!CPU_Init()) {
		*error_string = "Failed initializing CPU/Memory";
		return false;
	}

	// Compat flags get loaded in CPU_Init (which is a bit of a misnomer) so we check for SW renderer here.
	if (g_PConfig.bSoftwareRendering || PSP_CoreParameter().compat.flags().ForceSoftwareRenderer) {
		coreParameter.gpuCore = GPUCORE_SOFTWARE;
	}

	*error_string = coreParameter.errorString;
	bool success = coreParameter.fileToStart != "";
	if (!success) {
		Core_NotifyLifecycle(CoreLifecycle::START_COMPLETE);
		pspIsIniting = false;
	}
	return success;
}

bool PSP_InitUpdate(std::string *error_string) {
	if (pspIsInited || !pspIsIniting) {
		return true;
	}

	if (!CPU_IsReady()) {
		return false;
	}

	bool success = coreParameter.fileToStart != "";
	*error_string = coreParameter.errorString;
	if (success && gpu == nullptr) {
		PSP_SetLoading("Starting graphics...");
		Draw::DrawContext *draw = coreParameter.graphicsContext ? coreParameter.graphicsContext->GetDrawContext() : nullptr;
		success = GPU_Init(coreParameter.graphicsContext, draw);
		if (!success) {
			*error_string = "Unable to initialize rendering engine.";
		}
	}
	if (!success) {
		PSP_Shutdown();
		return true;
	}

	pspIsInited = GPU_IsReady();
	pspIsIniting = !pspIsInited;
	if (pspIsInited) {
		Core_NotifyLifecycle(CoreLifecycle::START_COMPLETE);
	}
	return pspIsInited;
}

bool PSP_Init(const CoreParameter &coreParam, std::string *error_string) {
	PSP_InitStart(coreParam, error_string);

	while (!PSP_InitUpdate(error_string))
		sleep_ms(10);
	return pspIsInited;
}

bool PSP_IsIniting() {
	return pspIsIniting;
}

bool PSP_IsInited() {
	return pspIsInited && !pspIsQuitting;
}

bool PSP_IsQuitting() {
	return pspIsQuitting;
}

void PSP_Shutdown() {
	// Do nothing if we never inited.
	if (!pspIsInited && !pspIsIniting && !pspIsQuitting) {
		return;
	}

	// Make sure things know right away that PSP memory, etc. is going away.
	pspIsQuitting = true;
	if (coreState == CORE_RUNNING)
		Core_UpdateState(CORE_POWERDOWN);

#ifndef MOBILE_DEVICE
	if (g_PConfig.bFuncHashMap) {
		MIPSAnalyst::StoreHashMap();
	}
#endif

	if (pspIsIniting)
		Core_NotifyLifecycle(CoreLifecycle::START_COMPLETE);
	Core_NotifyLifecycle(CoreLifecycle::STOPPING);
	CPU_Shutdown();
	GPU_Shutdown();
	g_paramSFO.Clear();
	host->SetWindowTitle(0);
	currentMIPS = 0;
	pspIsInited = false;
	pspIsIniting = false;
	pspIsQuitting = false;
	g_PConfig.unloadGameConfig();
	Core_NotifyLifecycle(CoreLifecycle::STOPPED);
}

void PSP_BeginHostFrame() {
	// Reapply the graphics state of the PSP
	if (gpu) {
		gpu->BeginHostFrame();
	}
}

void PSP_EndHostFrame() {
	if (gpu) {
		gpu->EndHostFrame();
	}
}

void PSP_RunLoopWhileState() {
	// We just run the CPU until we get to vblank. This will quickly sync up pretty nicely.
	// The actual number of cycles doesn't matter so much here as we will break due to CORE_NEXTFRAME, most of the time hopefully...
	int blockTicks = usToCycles(1000000 / 10);

	// Run until CORE_NEXTFRAME
	while (coreState == CORE_RUNNING || coreState == CORE_STEPPING) {
		PSP_RunLoopFor(blockTicks);
		if (coreState == CORE_STEPPING) {
			// Keep the UI responsive.
			break;
		}
	}
}

void PSP_RunLoopUntil(u64 globalticks) {
	SaveState::Process();
	if (coreState == CORE_POWERDOWN || coreState == CORE_BOOT_ERROR || coreState == CORE_RUNTIME_ERROR) {
		return;
	} else if (coreState == CORE_STEPPING) {
		Core_ProcessStepping();
		return;
	}

	mipsr4k.RunLoopUntil(globalticks);
	gpu->CleanupBeforeUI();
}

void PSP_RunLoopFor(int cycles) {
	PSP_RunLoopUntil(PCoreTiming::GetTicks() + cycles);
}

void PSP_SetLoading(const std::string &reason) {
	std::lock_guard<std::mutex> guard(loadingReasonLock);
	loadingReason = reason;
}

std::string PSP_GetLoading() {
	std::lock_guard<std::mutex> guard(loadingReasonLock);
	return loadingReason;
}

CoreParameter &PSP_CoreParameter() {
	return coreParameter;
}

std::string GetSysDirectory(PSPDirectories directoryType) {
	switch (directoryType) {
	case DIRECTORY_CHEATS:
		return g_PConfig.memStickDirectory + "PSP/Cheats/";
	case DIRECTORY_GAME:
		return g_PConfig.memStickDirectory + "PSP/GAME/";
	case DIRECTORY_SAVEDATA:
		return g_PConfig.memStickDirectory + "PSP/SAVEDATA/";
	case DIRECTORY_SCREENSHOT:
		return g_PConfig.memStickDirectory + "PSP/SCREENSHOT/";
	case DIRECTORY_SYSTEM:
		return g_PConfig.memStickDirectory + "PSP/SYSTEM/";
	case DIRECTORY_PAUTH:
		return g_PConfig.memStickDirectory + "PAUTH/";
	case DIRECTORY_DUMP:
		return g_PConfig.memStickDirectory + "PSP/SYSTEM/DUMP/";
	case DIRECTORY_SAVESTATE:
		return g_PConfig.memStickDirectory + "PSP/PPSSPP_STATE/";
	case DIRECTORY_CACHE:
		return g_PConfig.memStickDirectory + "PSP/SYSTEM/CACHE/";
	case DIRECTORY_TEXTURES:
		return g_PConfig.memStickDirectory + "PSP/TEXTURES/";
	case DIRECTORY_PLUGINS:
		return g_PConfig.memStickDirectory + "PSP/PLUGINS/";
	case DIRECTORY_APP_CACHE:
		if (!g_PConfig.appCacheDirectory.empty()) {
			return g_PConfig.appCacheDirectory;
		}
		return g_PConfig.memStickDirectory + "PSP/SYSTEM/CACHE/";
	case DIRECTORY_VIDEO:
		return g_PConfig.memStickDirectory + "PSP/VIDEO/";
	case DIRECTORY_AUDIO:
		return g_PConfig.memStickDirectory + "PSP/AUDIO/";
	// Just return the memory stick root if we run into some sort of problem.
	default:
		ERROR_LOG(FILESYS, "Unknown directory type.");
		return g_PConfig.memStickDirectory;
	}
}

#if defined(_WIN32)
// Run this at startup time. Please use GetSysDirectory if you need to query where folders are.
void InitSysDirectories() {
	if (!g_PConfig.memStickDirectory.empty() && !g_PConfig.flash0Directory.empty())
		return;

	const std::string path = PFile::GetExeDirectory();

	// Mount a filesystem
	g_PConfig.flash0Directory = path + "assets/flash0/";

	// Detect the "My Documents"(XP) or "Documents"(on Vista/7/8) folder.
#if PPSSPP_PLATFORM(UWP)
	// We set g_PConfig.memStickDirectory outside.

#else
	// Caller sets this to the Documents folder.
	const std::string rootMyDocsPath = g_PConfig.internalDataDirectory;
	const std::string myDocsPath = rootMyDocsPath + "/PPSSPP/";
	const std::string installedFile = path + "installed.txt";
	const bool installed = PFile::Exists(installedFile);

	// If installed.txt exists(and we can determine the Documents directory)
	if (installed && rootMyDocsPath.size() > 0) {
#if defined(_WIN32) && defined(__MINGW32__)
		std::ifstream inputFile(installedFile);
#else
		std::ifstream inputFile(ConvertUTF8ToWString(installedFile));
#endif

		if (!inputFile.fail() && inputFile.is_open()) {
			std::string tempString;

			std::getline(inputFile, tempString);

			// Skip UTF-8 encoding bytes if there are any. There are 3 of them.
			if (tempString.substr(0, 3) == "\xEF\xBB\xBF")
				tempString = tempString.substr(3);

			g_PConfig.memStickDirectory = tempString;
		}
		inputFile.close();

		// Check if the file is empty first, before appending the slash.
		if (g_PConfig.memStickDirectory.empty())
			g_PConfig.memStickDirectory = myDocsPath;

		size_t lastSlash = g_PConfig.memStickDirectory.find_last_of("/");
		if (lastSlash != (g_PConfig.memStickDirectory.length() - 1))
			g_PConfig.memStickDirectory.append("/");
	} else {
		g_PConfig.memStickDirectory = path + "memstick/";
	}

	// Create the memstickpath before trying to write to it, and fall back on Documents yet again
	// if we can't make it.
	if (!PFile::Exists(g_PConfig.memStickDirectory)) {
		if (!PFile::CreateDir(g_PConfig.memStickDirectory))
			g_PConfig.memStickDirectory = myDocsPath;
		INFO_LOG(COMMON, "Memstick directory not present, creating at '%s'", g_PConfig.memStickDirectory.c_str());
	}

	const std::string testFile = g_PConfig.memStickDirectory + "_writable_test.$$$";

	// If any directory is read-only, fall back to the Documents directory.
	// We're screwed anyway if we can't write to Documents, or can't detect it.
	if (!PFile::CreateEmptyFile(testFile))
		g_PConfig.memStickDirectory = myDocsPath;

	// Clean up our mess.
	if (PFile::Exists(testFile))
		PFile::Delete(testFile);
#endif

	// Create the default directories that a real PSP creates. Good for homebrew so they can
	// expect a standard environment. Skipping THEME though, that's pointless.
	PFile::CreateDir(g_PConfig.memStickDirectory + "PSP");
	PFile::CreateDir(g_PConfig.memStickDirectory + "PSP/COMMON");
	PFile::CreateDir(GetSysDirectory(DIRECTORY_GAME));
	PFile::CreateDir(GetSysDirectory(DIRECTORY_SAVEDATA));
	PFile::CreateDir(GetSysDirectory(DIRECTORY_SAVESTATE));

	if (g_PConfig.currentDirectory.empty()) {
		g_PConfig.currentDirectory = GetSysDirectory(DIRECTORY_GAME);
	}
}
#endif
