// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

// NativeApp implementation for platforms that will use that framework, like:
// Android, Linux, MacOSX.
//
// Native is a cross platform framework. It's not very mature and mostly
// just built according to the needs of my own apps.
//
// Windows has its own code that bypasses the framework entirely.

#include "ppsspp_config.h"

// Background worker threads should be spawned in NativeInit and joined
// in NativeShutdown.

#include <locale.h>
#include <algorithm>
#include <memory>
#include <mutex>
#include <thread>

#include "Common/Render/TextureAtlas.h"
#include "Common/Render/Text/draw_text.h"
#include "Common/GPU/OpenGL/GLFeatures.h"
#include "Common/GPU/thin3d.h"
#include "Common/UI/UI.h"
#include "Common/UI/Screen.h"
#include "Common/UI/Context.h"
#include "Common/UI/View.h"

#include "Common/System/Display.h"
#include "Common/System/System.h"
#include "Common/System/NativeApp.h"
#include "Common/Data/Text/I18n.h"
#include "Common/Input/InputState.h"
#include "Common/Math/fast/fast_math.h"
#include "Common/Math/math_util.h"
#include "Common/Math/lin/matrix4x4.h"
#include "Common/Data/Encoding/Utf8.h"
#include "Common/File/VFS/VFS.h"
#include "Common/File/VFS/AssetReader.h"
#include "Common/File/FileUtil.h"
#include "Common/TimeUtil.h"
#include "Common/StringUtils.h"
#include "Common/LogManager.h"
#include "Common/MemArena.h"
#include "Common/GraphicsContext.h"
#include "Common/OSVersion.h"

#include "UI/PauseScreenPCSX2.h"
#include "UI/MiscScreens.h"
#include "UI/OnScreenDisplay.h"
#include "UI/TextureUtil.h"

#include "../../include/emu.h"

static SCREEN_UI::Theme ui_theme;
extern GlobalUIState globalUIState;
static SCREEN_GPUBackend gpuBackend;
static std::string gpuBackendDevice;

SCREEN_Atlas g_ui_atlas;

SCREEN_ScreenManager *screenManager;
std::string config_filename;

bool g_TakeScreenshot;
bool g_ShaderNameListChanged = false;
static bool isOuya;
static bool resized = false;
static bool restarting = false;

static bool askedForStoragePermission = false;
static int renderCounter = 0;

struct PendingMessage {
	std::string msg;
	std::string value;
};

struct PendingInputBox {
	std::function<void(bool, const std::string &)> cb;
	bool result;
	std::string value;
};

static std::mutex pendingMutex;
static std::vector<PendingMessage> pendingMessages;
static std::vector<PendingInputBox> pendingInputBoxes;
static SCREEN_Draw::SCREEN_DrawContext *g_draw;
static SCREEN_Draw::SCREEN_Pipeline *colorPipeline;
static SCREEN_Draw::SCREEN_Pipeline *texColorPipeline;
static SCREEN_UIContext *uiContext;

std::thread *graphicsLoadThread;

static SCREEN_LogListener *logger = nullptr;
std::string boot_filename = "";

void SetGPUBackend(SCREEN_GPUBackend type, const std::string &device) {
	gpuBackend = type;
	gpuBackendDevice = device;
}

std::string NativeQueryConfig(std::string query) {
	char temp[128];

	return "";
}

// This is called before NativeInit so we do a little bit of initialization here.
void NativeGetAppInfo(std::string *app_dir_name, std::string *app_nice_name, bool *landscape, std::string *version) {
	*app_nice_name = "MARLEY";
	*app_dir_name = "marley";
	*landscape = true;
	*version = "0.1.9";
}

static void PostLoadConfig() {
}

void NativeInit(int argc, const char *argv[], const char *savegame_dir, const char *external_dir, const char *cache_dir) {

	globalUIState = UISTATE_MENU;
    
	pendingMessages.clear();
	pendingInputBoxes.clear();

	const char *homedir;
	std::string foldername;

	if ((homedir = getenv("HOME")) != nullptr) 
	{
		foldername = homedir;
        
		// add slash to end if necessary
		if (foldername.substr(foldername.length()-1,1) != "/")
		{
			foldername += "/";
		}		
	}
	else
	{
		foldername = "~/";
	}

	foldername += ".marley/ppsspp/assets/";
	VFSRegister("", new DirectorySCREEN_AssetReader(foldername.c_str()));	
    
	screenManager = new SCREEN_ScreenManager();
    screenManager->push(new GamePauseScreenPCSX2());

}

static SCREEN_UI::Style MakeStyle(uint32_t fg, uint32_t bg) {
	SCREEN_UI::Style s;
	s.background = SCREEN_UI::Drawable(bg);
	s.fgColor = fg;

	return s;
}

static void UIThemeInit() {

	ui_theme.uiFont = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 20);
	ui_theme.uiFontSmall = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 14);
	ui_theme.uiFontSmaller = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 11);

	ui_theme.checkOn = ImageID("I_CHECKEDBOX");
	ui_theme.checkOff = ImageID("I_SQUARE");
	ui_theme.whiteImage = ImageID("I_SOLIDWHITE");
	ui_theme.sliderKnob = ImageID("I_CIRCLE");
	ui_theme.dropShadow4Grid = ImageID("I_DROP_SHADOW");

	ui_theme.itemStyle = MakeStyle(0xFFFFFFFF, 0x55000000);
	ui_theme.itemFocusedStyle = MakeStyle(0xFFFFFFFF, 0xFFEDC24C);
	ui_theme.itemDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
	ui_theme.itemDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
	ui_theme.itemHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

	ui_theme.buttonStyle = MakeStyle(0xFFFFFFFF, 0x55000000);
	ui_theme.buttonFocusedStyle = MakeStyle(0xFFFFFFFF, 0xFFEDC24C);
	ui_theme.buttonDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
	ui_theme.buttonDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
	ui_theme.buttonHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

	ui_theme.headerStyle.fgColor = 0xFFFFFFFF;
	ui_theme.infoStyle = MakeStyle(0xFFFFFFFF, 0x00000000U);

	ui_theme.popupTitle.fgColor = 0xFFE3BE59;
	ui_theme.popupStyle = MakeStyle(0xFFFFFFFF, 0xFF303030);
}

void RenderOverlays(SCREEN_UIContext *dc, void *userdata);
bool CreateGlobalPipelines();

bool NativeInitGraphics(SCREEN_GraphicsContext *graphicsContext) {
	
	g_draw = graphicsContext->GetSCREEN_DrawContext();

	if (!CreateGlobalPipelines()) {
		printf("Failed to create global pipelines");
		return false;
	}

	// Load the atlas.
	size_t atlas_data_size = 0;
	if (!g_ui_atlas.IsMetadataLoaded()) {
		const uint8_t *atlas_data = VFSReadFile("ui_atlas.meta", &atlas_data_size);
		bool load_success = atlas_data != nullptr && g_ui_atlas.Load(atlas_data, atlas_data_size);
		if (!load_success) {
			printf("Failed to load ui_atlas.meta - graphics will be broken.");
			// Stumble along with broken visuals instead of dying.
		}
		delete[] atlas_data;
	}

	ui_draw2d.SetAtlas(&g_ui_atlas);
	ui_draw2d_front.SetAtlas(&g_ui_atlas);

	UIThemeInit();

	uiContext = new SCREEN_UIContext();
	uiContext->theme = &ui_theme;

	ui_draw2d.Init(g_draw, texColorPipeline);
	ui_draw2d_front.Init(g_draw, texColorPipeline);

	uiContext->Init(g_draw, texColorPipeline, colorPipeline, &ui_draw2d, &ui_draw2d_front);
	if (uiContext->Text())
		uiContext->Text()->SetFont("Tahoma", 20, 0);

	screenManager->setUIContext(uiContext);
	screenManager->setSCREEN_DrawContext(g_draw);

	return true;
}

bool CreateGlobalPipelines() {
	using namespace SCREEN_Draw;

	SCREEN_InputLayout *inputLayout = ui_draw2d.CreateInputLayout(g_draw);
	SCREEN_BlendState *blendNormal = g_draw->CreateBlendState({ true, 0xF, SCREEN_BlendFactor::SRC_ALPHA, SCREEN_BlendFactor::ONE_MINUS_SRC_ALPHA });
	SCREEN_DepthStencilState *depth = g_draw->CreateDepthStencilState({ false, false, SCREEN_Comparison::LESS });
	SCREEN_RasterState *rasterNoCull = g_draw->CreateRasterState({});

	PipelineDesc colorDesc{
		SCREEN_Primitive::TRIANGLE_LIST,
		{ g_draw->GetVshaderPreset(VS_COLOR_2D), g_draw->GetFshaderPreset(FS_COLOR_2D) },
		inputLayout, depth, blendNormal, rasterNoCull, &vsColBufDesc,
	};
	PipelineDesc texColorDesc{
		SCREEN_Primitive::TRIANGLE_LIST,
		{ g_draw->GetVshaderPreset(VS_TEXTURE_COLOR_2D), g_draw->GetFshaderPreset(FS_TEXTURE_COLOR_2D) },
		inputLayout, depth, blendNormal, rasterNoCull, &vsTexColBufDesc,
	};

	colorPipeline = g_draw->CreateGraphicsPipeline(colorDesc);
	if (!colorPipeline) {
		// Something really critical is wrong, don't care much about correct releasing of the states.
		return false;
	}

	texColorPipeline = g_draw->CreateGraphicsPipeline(texColorDesc);
	if (!texColorPipeline) {
		// Something really critical is wrong, don't care much about correct releasing of the states.
		return false;
	}

	// Release these now, reference counting should ensure that they get completely released
	// once we delete both pipelines.
	inputLayout->Release();
	rasterNoCull->Release();
	blendNormal->Release();
	depth->Release();
	return true;
}

void NativeShutdownGraphics() {
	screenManager->deviceLost();
	
	UIBackgroundShutdown();

	delete uiContext;
	uiContext = nullptr;

	ui_draw2d.Shutdown();
	ui_draw2d_front.Shutdown();

	if (colorPipeline) {
		colorPipeline->Release();
		colorPipeline = nullptr;
	}
	if (texColorPipeline) {
		texColorPipeline->Release();
		texColorPipeline = nullptr;
	}
}


void NativeRender(SCREEN_GraphicsContext *graphicsContext) {

	float xres = dp_xres;
	float yres = dp_yres;

	// Apply the UIContext bounds as a 2D transformation matrix.
	SCREEN_Matrix4x4 ortho;
    ortho.setOrtho(0.0f, xres, yres, 0.0f, -1.0f, 1.0f);
    
	ui_draw2d.PushDrawMatrix(ortho);
	ui_draw2d_front.PushDrawMatrix(ortho);

	// All actual rendering happens in here
	screenManager->render();
	if (screenManager->getUIContext()->Text()) {
		screenManager->getUIContext()->Text()->OncePerFrame();
	}

	ui_draw2d.PopDrawMatrix();
	ui_draw2d_front.PopDrawMatrix();
    
}

void HandleGlobalMessage(const std::string &msg, const std::string &value) {
	
}

void NativeUpdate() {

	std::vector<PendingMessage> toProcess;
	std::vector<PendingInputBox> inputToProcess;
	{
		std::lock_guard<std::mutex> lock(pendingMutex);
		toProcess = std::move(pendingMessages);
		inputToProcess = std::move(pendingInputBoxes);
		pendingMessages.clear();
		pendingInputBoxes.clear();
	}

	for (const auto &item : toProcess) {
		HandleGlobalMessage(item.msg, item.value);
		screenManager->sendMessage(item.msg.c_str(), item.value.c_str());
	}
	for (const auto &item : inputToProcess) {
		item.cb(item.result, item.value);
	}

	screenManager->update();

}

bool NativeIsAtTopLevel() {
	// This might need some synchronization?
	if (!screenManager) {
		printf("No screen manager active");
		return false;
	}
	SCREEN_Screen *currentScreen = screenManager->topScreen();
	if (currentScreen) {
		bool top = currentScreen->isTopLevel();
		printf("Screen toplevel: %i", (int)top);
		return currentScreen->isTopLevel();
	} else {
		printf("No current screen");
		return false;
	}
}

bool NativeTouch(const TouchInput &touch) {
	if (screenManager) {
		// Brute force prevent NaNs from getting into the UI system
		if (my_isnan(touch.x) || my_isnan(touch.y)) {
			return false;
		}
		screenManager->touch(touch);
		return true;
	} else {
		return false;
	}
}

bool NativeKey(const KeyInput &key) {
	bool retval = false;
	if (screenManager)
		retval = screenManager->key(key);
	return retval;
}

bool NativeAxis(const AxisInput &axis) {
	
    return false;
}

void NativeMessageReceived(const char *message, const char *value) {
	// We can only have one message queued.
	std::lock_guard<std::mutex> lock(pendingMutex);
	PendingMessage pendingMessage;
	pendingMessage.msg = message;
	pendingMessage.value = value;
	pendingMessages.push_back(pendingMessage);
}

void NativeInputBoxReceived(std::function<void(bool, const std::string &)> cb, bool result, const std::string &value) {
	std::lock_guard<std::mutex> lock(pendingMutex);
	PendingInputBox pendingMessage;
	pendingMessage.cb = cb;
	pendingMessage.result = result;
	pendingMessage.value = value;
	pendingInputBoxes.push_back(pendingMessage);
}

void NativeResized() {
	// NativeResized can come from any thread so we just set a flag, then process it later.
	printf("NativeResized - setting flag");
	resized = true;
}

void NativeSetRestarting() {
	restarting = true;
}

bool NativeIsRestarting() {
	return restarting;
}

void NativeShutdown() {
	if (screenManager)
		screenManager->shutdown();
	delete screenManager;
	screenManager = nullptr;

	System_SendMessage("finish", "");

	if (logger) {
		delete logger;
		logger = nullptr;
	}
}
