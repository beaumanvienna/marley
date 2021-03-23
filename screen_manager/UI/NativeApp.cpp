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
// in SCREEN_NativeShutdown.

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

#include "UI/MainScreen.h"
#include "UI/MiscScreens.h"
#include "UI/OnScreenDisplay.h"
#include "UI/TextureUtil.h"

#include "../../include/emu.h"
#include "../../include/global.h"

static SCREEN_UI::Theme ui_theme;
extern GlobalUIState globalUIState;
extern int gTheme;
extern bool gUpdateCurrentScreen;
static SCREEN_GPUBackend SCREEN_gpuBackend;
static std::string SCREEN_gpuBackendDevice;

SCREEN_Atlas SCREEN_g_ui_atlas;

SCREEN_ScreenManager *SCREEN_screenManager;
std::string SCREEN_config_filename;

static bool SCREEN_resized = false;
static bool restarting = false;

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

std::thread *SCREEN_graphicsLoadThread;

static SCREEN_LogListener *logger = nullptr;
std::string SCREEN_boot_filename = "";

void SetGPUBackend(SCREEN_GPUBackend type, const std::string &device) {
	SCREEN_gpuBackend = type;
	SCREEN_gpuBackendDevice = device;
}

std::string SCREEN_NativeQueryConfig(std::string query) {
	char temp[128];

	return "";
}

// This is called before NativeInit so we do a little bit of initialization here.
void SCREEN_NativeGetAppInfo(std::string *app_dir_name, std::string *app_nice_name, bool *landscape, std::string *version) {
	*app_nice_name = "MARLEY";
	*app_dir_name = "marley";
	*landscape = true;
	*version = "0.1.9";
}

static void PostLoadConfig() {
}

void SCREEN_NativeInit(int argc, const char *argv[], const char *savegame_dir, const char *external_dir, const char *cache_dir) {
    printf("jc: void SCREEN_NativeInit\n");
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
    
	SCREEN_screenManager = new SCREEN_ScreenManager();
    SCREEN_screenManager->push(new SCREEN_MainScreen());

}

static SCREEN_UI::Style MakeStyle(uint32_t fg, uint32_t bg) {
	SCREEN_UI::Style s;
	s.background = SCREEN_UI::Drawable(bg);
	s.fgColor = fg;

	return s;
}

void SCREEN_UIThemeInit() {
    if (gTheme == THEME_RETRO)
    {
        ui_theme.uiFont = SCREEN_UI::FontStyle(FontID("RETRO24"), "", 15); // only used for tab headers
        ui_theme.uiFontSmall = SCREEN_UI::FontStyle(FontID("RETRO24"), "", 13); // used for file browser
        ui_theme.uiFontSmaller = SCREEN_UI::FontStyle(FontID("RETRO24"), "", 10);
    
        ui_theme.itemStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0x80000000);
        ui_theme.itemFocusedStyle = MakeStyle(0xFFFFFFFF, 0xA0000000); // active icons
        ui_theme.itemDownStyle = MakeStyle(0xFFFFFFFF, 0xB0000000);
        ui_theme.itemDisabledStyle = MakeStyle(0xffEEEEEE, 0x55E0D4AF);
        ui_theme.itemHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55ffffff); //

        ui_theme.buttonStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0x70000000); // inactive button
        ui_theme.buttonFocusedStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0xA0000000); // active button
        ui_theme.buttonDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
        ui_theme.buttonDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
        ui_theme.buttonHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

        ui_theme.headerStyle.fgColor = RETRO_COLOR_FONT_FOREGROUND;
        ui_theme.infoStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0x00000000U);

        ui_theme.popupTitle.fgColor = 0xFFE3BE59;
        ui_theme.popupStyle = MakeStyle(0xFFFFFFFF, 0xFF303030);
    } else
    {
        ui_theme.uiFont = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 20);
        ui_theme.uiFontSmall = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 18);
        ui_theme.uiFontSmaller = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 14);
        
        ui_theme.itemStyle = MakeStyle(0xFFFFFFFF, 0x55000000);
        ui_theme.itemFocusedStyle = MakeStyle(0xFFFFFFFF, 0x70000000);
        ui_theme.itemDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
        ui_theme.itemDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
        ui_theme.itemHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

        ui_theme.buttonStyle = MakeStyle(0xFFFFFFFF, 0x55000000);
        ui_theme.buttonFocusedStyle = MakeStyle(0xFFFFFFFF, 0x70000000);
        ui_theme.buttonDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
        ui_theme.buttonDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
        ui_theme.buttonHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

        ui_theme.headerStyle.fgColor = 0xFFFFFFFF;
        ui_theme.infoStyle = MakeStyle(0xFFFFFFFF, 0x00000000U);

        ui_theme.popupTitle.fgColor = 0xFFE3BE59;
        ui_theme.popupStyle = MakeStyle(0xFFFFFFFF, 0xFF303030);
    }
    
    ui_theme.checkOn = ImageID("I_CHECKEDBOX");
	ui_theme.checkOff = ImageID("I_SQUARE");
	ui_theme.whiteImage = ImageID("I_SOLIDWHITE");
	ui_theme.sliderKnob = ImageID("I_CIRCLE");
	ui_theme.dropShadow4Grid = ImageID("I_DROP_SHADOW");
}

void RenderOverlays(SCREEN_UIContext *dc, void *userdata);
bool SCREEN_CreateGlobalPipelines();

bool SCREEN_NativeInitGraphics(SCREEN_GraphicsContext *graphicsContext) {
	
	g_draw = graphicsContext->GetSCREEN_DrawContext();

	if (!SCREEN_CreateGlobalPipelines()) {
		printf("Failed to create global pipelines");
		return false;
	}

	// Load the atlas.
	size_t atlas_data_size = 0;
	if (!SCREEN_g_ui_atlas.IsMetadataLoaded()) {
		const uint8_t *atlas_data = SCREEN_VFSReadFile("ui_atlas.meta", &atlas_data_size);
		bool load_success = atlas_data != nullptr && SCREEN_g_ui_atlas.Load(atlas_data, atlas_data_size);
		if (!load_success) {
			printf("Failed to load ui_atlas.meta - graphics will be broken.");
			// Stumble along with broken visuals instead of dying.
		}
		delete[] atlas_data;
	}

	SCREEN_ui_draw2d.SetAtlas(&SCREEN_g_ui_atlas);
	SCREEN_ui_draw2d_front.SetAtlas(&SCREEN_g_ui_atlas);

	SCREEN_UIThemeInit();

	uiContext = new SCREEN_UIContext();
	uiContext->theme = &ui_theme;

	SCREEN_ui_draw2d.Init(g_draw, texColorPipeline);
	SCREEN_ui_draw2d_front.Init(g_draw, texColorPipeline);

	uiContext->Init(g_draw, texColorPipeline, colorPipeline, &SCREEN_ui_draw2d, &SCREEN_ui_draw2d_front);
	if (uiContext->Text())
		uiContext->Text()->SetFont("Tahoma", 20, 0);

	SCREEN_screenManager->setUIContext(uiContext);
	SCREEN_screenManager->setSCREEN_DrawContext(g_draw);

	return true;
}

bool SCREEN_CreateGlobalPipelines() {
	using namespace SCREEN_Draw;

	SCREEN_InputLayout *inputLayout = SCREEN_ui_draw2d.CreateInputLayout(g_draw);
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

void SCREEN_NativeShutdownGraphics() {
	SCREEN_screenManager->deviceLost();
	
	SCREEN_UIBackgroundShutdown();

	delete uiContext;
	uiContext = nullptr;

	SCREEN_ui_draw2d.Shutdown();
	SCREEN_ui_draw2d_front.Shutdown();

	if (colorPipeline) {
		colorPipeline->Release();
		colorPipeline = nullptr;
	}
	if (texColorPipeline) {
		texColorPipeline->Release();
		texColorPipeline = nullptr;
	}
}


void SCREEN_NativeRender(SCREEN_GraphicsContext *graphicsContext) {

	float xres = dp_xres;
	float yres = dp_yres;

	// Apply the UIContext bounds as a 2D transformation matrix.
	SCREEN_Matrix4x4 ortho;
    ortho.setOrtho(0.0f, xres, yres, 0.0f, -1.0f, 1.0f);
    
	SCREEN_ui_draw2d.PushDrawMatrix(ortho);
	SCREEN_ui_draw2d_front.PushDrawMatrix(ortho);

	// All actual rendering happens in here
	SCREEN_screenManager->render();
	if (SCREEN_screenManager->getUIContext()->Text()) {
		SCREEN_screenManager->getUIContext()->Text()->OncePerFrame();
	}
    
	if (SCREEN_resized) {
        printf("jc: if (SCREEN_resized)  \n");
		SCREEN_resized = false;

		if (uiContext) {
			// Modifying the bounds here can be used to "inset" the whole image to gain borders for TV overscan etc.
			// The UI now supports any offset but not the EmuScreen yet.
			uiContext->SetBounds(Bounds(0, 0, dp_xres, dp_yres));
		}
		graphicsContext->Resize();
		SCREEN_screenManager->resized();
        gUpdateCurrentScreen = true;
	}

	SCREEN_ui_draw2d.PopDrawMatrix();
	SCREEN_ui_draw2d_front.PopDrawMatrix();
    
}

void SCREEN_HandleGlobalMessage(const std::string &msg, const std::string &value) {
	
}

void SCREEN_NativeUpdate() {

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
		SCREEN_HandleGlobalMessage(item.msg, item.value);
		SCREEN_screenManager->sendMessage(item.msg.c_str(), item.value.c_str());
	}
	for (const auto &item : inputToProcess) {
		item.cb(item.result, item.value);
	}

	SCREEN_screenManager->update();

}

bool SCREEN_NativeIsAtTopLevel() {
	// This might need some synchronization?
	if (!SCREEN_screenManager) {
		printf("No screen manager active");
		return false;
	}
	SCREEN_Screen *currentScreen = SCREEN_screenManager->topScreen();
	if (currentScreen) {
		bool top = currentScreen->isTopLevel();
		printf("Screen toplevel: %i", (int)top);
		return currentScreen->isTopLevel();
	} else {
		printf("No current screen");
		return false;
	}
}

bool SCREEN_NativeTouch(const TouchInput &touch) {
	if (SCREEN_screenManager) {
		// Brute force prevent NaNs from getting into the UI system
		if (my_isnan(touch.x) || my_isnan(touch.y)) {
			return false;
		}
		SCREEN_screenManager->touch(touch);
		return true;
	} else {
		return false;
	}
}

bool SCREEN_NativeKey(const KeyInput &key) {
	bool retval = false;
	if (SCREEN_screenManager)
		retval = SCREEN_screenManager->key(key);
	return retval;
}
KeyInput key;
void SCREEN_wiimoteInput(int button)
{
    key.deviceId = DEVICE_ID_KEYBOARD;
    if (button)
    {
        key.flags = KEY_DOWN;
        switch(button)
        {
            case BUTTON_DPAD_UP:
                key.keyCode = 19;
                break;
            case BUTTON_DPAD_DOWN:
                key.keyCode = 20;
                break;
            case BUTTON_DPAD_LEFT:
                key.keyCode = 21;
                break;
            case BUTTON_DPAD_RIGHT:
                key.keyCode = 22;
                break;
            case BUTTON_A:
            case BUTTON_B:
                key.keyCode = 66;
                break;
            case BUTTON_GUIDE:
                key.keyCode = 111;
                break;
            default:
                break;
        }
    } else
    {
        key.flags = KEY_UP;
    }
    SCREEN_NativeKey(key);
}

bool SCREEN_NativeAxis(const AxisInput &axis) {
	
    return false;
}

void SCREEN_NativeMessageReceived(const char *message, const char *value) {
	// We can only have one message queued.
	std::lock_guard<std::mutex> lock(pendingMutex);
	PendingMessage pendingMessage;
	pendingMessage.msg = message;
	pendingMessage.value = value;
	pendingMessages.push_back(pendingMessage);
}

void SCREEN_NativeInputBoxReceived(std::function<void(bool, const std::string &)> cb, bool result, const std::string &value) {
	std::lock_guard<std::mutex> lock(pendingMutex);
	PendingInputBox pendingMessage;
	pendingMessage.cb = cb;
	pendingMessage.result = result;
	pendingMessage.value = value;
	pendingInputBoxes.push_back(pendingMessage);
}

void SCREEN_NativeResized(void) {
	printf("jc: void SCREEN_NativeResized(void) \n");
	SCREEN_resized = true;
}

void SCREEN_NativeSetRestarting() {
	restarting = true;
}

bool SCREEN_NativeIsRestarting() {
	return restarting;
}

void SCREEN_NativeShutdown() {
	if (SCREEN_screenManager)
		SCREEN_screenManager->shutdown();
	delete SCREEN_screenManager;
	SCREEN_screenManager = nullptr;

	SCREEN_System_SendMessage("finish", "");

	if (logger) {
		delete logger;
		logger = nullptr;
	}
}
