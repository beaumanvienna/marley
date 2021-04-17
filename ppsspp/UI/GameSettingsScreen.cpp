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

#include "ppsspp_config.h"

#include <algorithm>
#include <iostream>
#include "../screen_manager/UI/Scale.h"
#include "Common/Net/Resolve.h"
#include "Common/GPU/OpenGL/GLFeatures.h"
#include "Common/Render/DrawBuffer.h"
#include "Common/UI/Root.h"
#include "Common/UI/View.h"
#include "Common/UI/ViewGroup.h"
#include "Common/UI/Context.h"

#include "Common/System/Display.h"  // Only to check screen aspect ratio with pixel_yres/pixel_xres
#include "Common/System/System.h"
#include "Common/System/NativeApp.h"
#include "Common/Data/Color/RGBAUtil.h"
#include "Common/Math/curves.h"
#include "Common/Data/Text/I18n.h"
#include "Common/Data/Encoding/Utf8.h"
#include "UI/EmuScreen.h"
#include "UI/GameSettingsScreen.h"
#include "UI/GameInfoCache.h"
#include "UI/GamepadEmu.h"
#include "UI/MiscScreens.h"
#include "UI/ControlMappingScreen.h"
#include "UI/DevScreens.h"
#include "UI/DisplayLayoutScreen.h"
#include "UI/RemoteISOScreen.h"
#include "UI/SavedataScreen.h"
#include "UI/TouchControlLayoutScreen.h"
#include "UI/TouchControlVisibilityScreen.h"
#include "UI/TiltAnalogSettingsScreen.h"
#include "UI/TiltEventProcessor.h"
#include "UI/ComboKeyMappingScreen.h"
#include "UI/GPUDriverTestScreen.h"

#include "Common/File/FileUtil.h"
#include "Common/OSVersion.h"
#include "Common/TimeUtil.h"
#include "Common/StringUtils.h"
#include "Core/Config.h"
#include "Core/ConfigValues.h"
#include "Core/Host.h"
#include "Core/KeyMap.h"
#include "Core/Instance.h"
#include "Core/System.h"
#include "Core/Reporting.h"
#include "Core/TextureReplacer.h"
#include "Core/WebServer.h"
#include "Core/HLE/sceUsbCam.h"
#include "Core/HLE/sceUsbMic.h"
#include "GPU/Common/PostShader.h"
#if PPSSPP_PLATFORM(ANDROID)
#include "android/jni/TestRunner.h"
#endif
#include "GPU/GPUInterface.h"
#include "GPU/Common/FramebufferManagerCommon.h"

#if defined(_WIN32) && !PPSSPP_PLATFORM(UWP)
#pragma warning(disable:4091)  // workaround bug in VS2015 headers
#include "Windows/MainWindow.h"
#include <shlobj.h>
#include "Windows/W32Util/ShellUtil.h"
#endif

extern bool g_ShaderNameListChanged;

GameSettingsScreen::GameSettingsScreen(std::string gamePath, std::string gameID, bool editThenRestore)
	: UIDialogScreenWithGameBackground(gamePath), gameID_(gameID), enableReports_(false), editThenRestore_(editThenRestore) {
	lastVertical_ = UseVerticalLayout();
	prevInflightFrames_ = g_PConfig.iInflightFrames;
}

bool GameSettingsScreen::UseVerticalLayout() const {
	return dp_yres > dp_xres * 1.1f;
}

// This needs before run CheckGPUFeatures()
// TODO: Remove this if fix the issue
bool CheckSupportShaderTessellationGLES() {
#if PPSSPP_PLATFORM(UWP)
	return true;
#else
	// TODO: Make work with non-GL backends
	int maxVertexTextureImageUnits = gl_extensions.maxVertexTextureUnits;
	bool vertexTexture = maxVertexTextureImageUnits >= 3; // At least 3 for hardware tessellation

	bool textureFloat = gl_extensions.ARB_texture_float || gl_extensions.OES_texture_float;
	bool hasTexelFetch = gl_extensions.GLES3 || (!gl_extensions.IsGLES && gl_extensions.VersionGEThan(3, 3, 0)) || gl_extensions.EXT_gpu_shader4;

	return vertexTexture && textureFloat && hasTexelFetch;
#endif
}

bool DoesBackendSupportHWTess() {
	switch (GetGPUBackend()) {
	case GPUBackend::OPENGL:
		return CheckSupportShaderTessellationGLES();
	case GPUBackend::VULKAN:
	case GPUBackend::DIRECT3D11:
		return true;
	default:
		return false;
	}
}

static bool UsingHardwareTextureScaling() {
	// For now, Vulkan only.
	return g_PConfig.bTexHardwareScaling && GetGPUBackend() == GPUBackend::VULKAN && !g_PConfig.bSoftwareRendering;
}

static std::string TextureTranslateName(const char *value) {
	auto ps = GetI18NCategory("TextureShaders");
	const TextureShaderInfo *info = GetTextureShaderInfo(value);
	if (info) {
		return ps->T(value, info ? info->name.c_str() : value);
	} else {
		return value;
	}
}

static std::string PostShaderTranslateName(const char *value) {
	auto ps = GetI18NCategory("PostShaders");
	const ShaderInfo *info = GetPostShaderInfo(value);
	if (info) {
		return ps->T(value, info ? info->name.c_str() : value);
	} else {
		return value;
	}
}

static std::string *GPUDeviceNameSetting() {
	if (g_PConfig.iGPUBackend == (int)GPUBackend::VULKAN) {
		return &g_PConfig.sVulkanDevice;
	}
#ifdef _WIN32
	if (g_PConfig.iGPUBackend == (int)GPUBackend::DIRECT3D11) {
		return &g_PConfig.sD3D11Device;
	}
#endif
	return nullptr;
}

void GameSettingsScreen::CreateViews() {
	ReloadAllPostShaderInfo();

	if (editThenRestore_) {
		std::shared_ptr<GameInfo> info = g_gameInfoCache->GetInfo(nullptr, gamePath_, 0);
		g_PConfig.loadGameConfig(gameID_, info->GetTitle());
	}

	iAlternateSpeedPercent1_ = g_PConfig.iFpsLimit1 < 0 ? -1 : (g_PConfig.iFpsLimit1 * 100) / 60;
	iAlternateSpeedPercent2_ = g_PConfig.iFpsLimit2 < 0 ? -1 : (g_PConfig.iFpsLimit2 * 100) / 60;

	bool vertical = UseVerticalLayout();

	// Information in the top left.
	// Back button to the bottom left.
	// Scrolling action menu to the right.
	using namespace UI;

	auto di = GetI18NCategory("Dialog");
	auto gr = GetI18NCategory("Graphics");
	auto co = GetI18NCategory("Controls");
	auto a = GetI18NCategory("Audio");
	auto sa = GetI18NCategory("Savedata");
	auto sy = GetI18NCategory("System");
	auto n = GetI18NCategory("Networking");
	auto ms = GetI18NCategory("MainSettings");
	auto dev = GetI18NCategory("Developer");
	auto ri = GetI18NCategory("RemoteISO");
	auto ps = GetI18NCategory("PostShaders");

	root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
	TabHolder *tabHolder;
	if (vertical) {
		LinearLayout *verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
		tabHolder = new TabHolder(ORIENT_HORIZONTAL, f200, new LinearLayoutParams(1.0f));
		verticalLayout->Add(tabHolder);
		verticalLayout->Add(new Choice(di->T("Back"), "", false, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 0.0f, Margins(0))))->OnClick.Handle<UIScreen>(this, &UIScreen::OnBack);
		root_->Add(verticalLayout);
	} else {
		tabHolder = new TabHolder(ORIENT_VERTICAL, f300, new AnchorLayoutParams(10, 0, 10, 0, false));
		root_->Add(tabHolder);
		AddStandardBack(root_);
	}
	tabHolder->SetTag("GameSettings");
	root_->SetDefaultFocusView(tabHolder);

	float leftSide = 40.0f;
	if (!vertical) {
		leftSide += 200.0f;
	}
	settingInfo_ = new SettingInfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new AnchorLayoutParams(dp_xres - leftSide - f40, WRAP_CONTENT, leftSide, dp_yres - f80 - f40, NONE, NONE));
	settingInfo_->SetBottomCutoff(dp_yres - f200);
	root_->Add(settingInfo_);

	// TODO: These currently point to global settings, not game specific ones.

	// Graphics
	ViewGroup *graphicsSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	graphicsSettingsScroll->SetTag("GameSettingsGraphics");
	LinearLayout *graphicsSettings = new LinearLayout(ORIENT_VERTICAL);
	graphicsSettings->SetSpacing(0);
	graphicsSettingsScroll->Add(graphicsSettings);
	tabHolder->AddTab(ms->T("Graphics"), graphicsSettingsScroll);

	graphicsSettings->Add(new ItemHeader(gr->T("Rendering Mode")));

#if !PPSSPP_PLATFORM(UWP)
	static const char *renderingBackend[] = { "OpenGL", "Direct3D 9", "Direct3D 11", "Vulkan" };
	PopupMultiChoice *renderingBackendChoice = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iGPUBackend, gr->T("Backend"), renderingBackend, (int)GPUBackend::OPENGL, ARRAY_SIZE(renderingBackend), gr->GetName(), screenManager()));
	renderingBackendChoice->OnChoice.Handle(this, &GameSettingsScreen::OnRenderingBackend);

	if (!g_PConfig.IsBackendEnabled(GPUBackend::OPENGL))
		renderingBackendChoice->HideChoice((int)GPUBackend::OPENGL);
	if (!g_PConfig.IsBackendEnabled(GPUBackend::DIRECT3D9))
		renderingBackendChoice->HideChoice((int)GPUBackend::DIRECT3D9);
	if (!g_PConfig.IsBackendEnabled(GPUBackend::DIRECT3D11))
		renderingBackendChoice->HideChoice((int)GPUBackend::DIRECT3D11);
	if (!g_PConfig.IsBackendEnabled(GPUBackend::VULKAN))
		renderingBackendChoice->HideChoice((int)GPUBackend::VULKAN);

	if (!IsFirstInstance()) {
		// If we're not the first instance, can't save the setting, and it requires a restart, so...
		renderingBackendChoice->SetEnabled(false);
	}
#endif

	Draw::DrawContext *draw = screenManager()->getDrawContext();

	// Backends that don't allow a device choice will only expose one device.
	if (draw->GetDeviceList().size() > 1) {
		std::string *deviceNameSetting = GPUDeviceNameSetting();
		if (deviceNameSetting) {
			PopupMultiChoiceDynamic *deviceChoice = graphicsSettings->Add(new PopupMultiChoiceDynamic(deviceNameSetting, gr->T("Device"), draw->GetDeviceList(), nullptr, screenManager()));
			deviceChoice->OnChoice.Handle(this, &GameSettingsScreen::OnRenderingDevice);
		}
	}

	static const char *renderingMode[] = { "Non-Buffered Rendering", "Buffered Rendering" };
	PopupMultiChoice *renderingModeChoice = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iRenderingMode, gr->T("Mode"), renderingMode, 0, ARRAY_SIZE(renderingMode), gr->GetName(), screenManager()));
	renderingModeChoice->OnChoice.Add([=](EventParams &e) {
		switch (g_PConfig.iRenderingMode) {
		case FB_NON_BUFFERED_MODE:
			settingInfo_->Show(gr->T("RenderingMode NonBuffered Tip", "Faster, but graphics may be missing in some games"), e.v);
			break;
		case FB_BUFFERED_MODE:
			break;
		}
		return UI::EVENT_CONTINUE;
	});
	renderingModeChoice->OnChoice.Handle(this, &GameSettingsScreen::OnRenderingMode);
	renderingModeChoice->SetDisabledPtr(&g_PConfig.bSoftwareRendering);
	CheckBox *blockTransfer = graphicsSettings->Add(new CheckBox(&g_PConfig.bBlockTransferGPU, gr->T("Simulate Block Transfer", "Simulate Block Transfer")));
	blockTransfer->OnClick.Add([=](EventParams &e) {
		if (!g_PConfig.bBlockTransferGPU)
			settingInfo_->Show(gr->T("BlockTransfer Tip", "Some games require this to be On for correct graphics"), e.v);
		return UI::EVENT_CONTINUE;
	});
	blockTransfer->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	bool showSoftGPU = true;
#ifdef MOBILE_DEVICE
	// On Android, only show the software rendering setting if it's already enabled.
	// Can still be turned on through INI file editing.
	showSoftGPU = g_PConfig.bSoftwareRendering;
#endif
	if (showSoftGPU) {
		CheckBox *softwareGPU = graphicsSettings->Add(new CheckBox(&g_PConfig.bSoftwareRendering, gr->T("Software Rendering", "Software Rendering (slow)")));
		softwareGPU->OnClick.Add([=](EventParams &e) {
			if (g_PConfig.bSoftwareRendering)
				settingInfo_->Show(gr->T("SoftGPU Tip", "Currently VERY slow"), e.v);
			return UI::EVENT_CONTINUE;
		});
		softwareGPU->OnClick.Handle(this, &GameSettingsScreen::OnSoftwareRendering);
		softwareGPU->SetEnabled(!PSP_IsInited());
	}

	graphicsSettings->Add(new ItemHeader(gr->T("Frame Rate Control")));
	static const char *frameSkip[] = {"Off", "1", "2", "3", "4", "5", "6", "7", "8"};
	graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iFrameSkip, gr->T("Frame Skipping"), frameSkip, 0, ARRAY_SIZE(frameSkip), gr->GetName(), screenManager()));
	static const char *frameSkipType[] = {"Number of Frames", "Percent of FPS"};
	graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iFrameSkipType, gr->T("Frame Skipping Type"), frameSkipType, 0, ARRAY_SIZE(frameSkipType), gr->GetName(), screenManager()));
	frameSkipAuto_ = graphicsSettings->Add(new CheckBox(&g_PConfig.bAutoFrameSkip, gr->T("Auto FrameSkip")));
	frameSkipAuto_->OnClick.Handle(this, &GameSettingsScreen::OnAutoFrameskip);

	PopupSliderChoice *altSpeed1 = graphicsSettings->Add(new PopupSliderChoice(&iAlternateSpeedPercent1_, 0, 1000, gr->T("Alternative Speed", "Alternative speed"), 5, screenManager(), gr->T("%, 0:unlimited")));
	altSpeed1->SetFormat("%i%%");
	altSpeed1->SetZeroLabel(gr->T("Unlimited"));
	altSpeed1->SetNegativeDisable(gr->T("Disabled"));

	PopupSliderChoice *altSpeed2 = graphicsSettings->Add(new PopupSliderChoice(&iAlternateSpeedPercent2_, 0, 1000, gr->T("Alternative Speed 2", "Alternative speed 2 (in %, 0 = unlimited)"), 5, screenManager(), gr->T("%, 0:unlimited")));
	altSpeed2->SetFormat("%i%%");
	altSpeed2->SetZeroLabel(gr->T("Unlimited"));
	altSpeed2->SetNegativeDisable(gr->T("Disabled"));

	graphicsSettings->Add(new ItemHeader(gr->T("Postprocessing effect")));

	std::vector<std::string> alreadyAddedShader;
	for (int i = 0; i < g_PConfig.vPostShaderNames.size() && i < ARRAY_SIZE(shaderNames_); ++i) {
		// Vector element pointer get invalidated on resize, cache name to have always a valid reference in the rendering thread
		shaderNames_[i] = g_PConfig.vPostShaderNames[i];
		postProcChoice_ = graphicsSettings->Add(new ChoiceWithValueDisplay(&shaderNames_[i], PStringFromFormat("%s #%d", gr->T("Postprocessing Shader"), i + 1), &PostShaderTranslateName));
		postProcChoice_->OnClick.Add([=](EventParams &e) {
			auto gr = GetI18NCategory("Graphics");
			auto procScreen = new PostProcScreen(gr->T("Postprocessing Shader"), i);
			procScreen->OnChoice.Handle(this, &GameSettingsScreen::OnPostProcShaderChange);
			if (e.v)
				procScreen->SetPopupOrigin(e.v);
			screenManager()->push(procScreen);
			return UI::EVENT_DONE;
		});
		postProcChoice_->SetEnabledFunc([] {
			return g_PConfig.iRenderingMode != FB_NON_BUFFERED_MODE;
		});

		auto shaderChain = GetPostShaderChain(g_PConfig.vPostShaderNames[i]);
		for (auto shaderInfo : shaderChain) {
			// Disable duplicated shader slider
			bool duplicated = std::find(alreadyAddedShader.begin(), alreadyAddedShader.end(), shaderInfo->section) != alreadyAddedShader.end();
			alreadyAddedShader.push_back(shaderInfo->section);
			for (size_t i = 0; i < ARRAY_SIZE(shaderInfo->settings); ++i) {
				auto &setting = shaderInfo->settings[i];
				if (!setting.name.empty()) {
					auto &value = g_PConfig.mPostShaderSetting[PStringFromFormat("%sSettingValue%d", shaderInfo->section.c_str(), i + 1)];
					if (duplicated) {
						auto sliderName = PStringFromFormat("%s %s", ps->T(setting.name), ps->T("(duplicated setting, previous slider will be used)"));
						PopupSliderChoiceFloat *settingValue = graphicsSettings->Add(new PopupSliderChoiceFloat(&value, setting.minValue, setting.maxValue, sliderName, setting.step, screenManager()));
						settingValue->SetEnabled(false);
					} else {
						PopupSliderChoiceFloat *settingValue = graphicsSettings->Add(new PopupSliderChoiceFloat(&value, setting.minValue, setting.maxValue, ps->T(setting.name), setting.step, screenManager()));
						settingValue->SetEnabledFunc([] {
							return g_PConfig.iRenderingMode != FB_NON_BUFFERED_MODE;
						});
					}
				}
			}
		}
	}

	graphicsSettings->Add(new ItemHeader(gr->T("Screen layout")));
#if !defined(MOBILE_DEVICE)
	graphicsSettings->Add(new CheckBox(&g_PConfig.bFullScreen, gr->T("FullScreen", "Full Screen")))->OnClick.Handle(this, &GameSettingsScreen::OnFullscreenChange);
	if (System_GetPropertyInt(SYSPROP_DISPLAY_COUNT) > 1) {
		CheckBox *fullscreenMulti = new CheckBox(&g_PConfig.bFullScreenMulti, gr->T("Use all displays"));
		fullscreenMulti->SetEnabledPtr(&g_PConfig.bFullScreen);
		graphicsSettings->Add(fullscreenMulti)->OnClick.Handle(this, &GameSettingsScreen::OnFullscreenChange);
	}
#endif
	// Display Layout Editor: To avoid overlapping touch controls on large tablets, meet geeky demands for integer zoom/unstretched image etc.
	displayEditor_ = graphicsSettings->Add(new Choice(gr->T("Display layout editor")));
	displayEditor_->OnClick.Handle(this, &GameSettingsScreen::OnDisplayLayoutEditor);

#if PPSSPP_PLATFORM(ANDROID)
	// Hide insets option if no insets, or OS too old.
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 29 &&
		(System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_LEFT) != 0.0f ||
		 System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_TOP) != 0.0f ||
		 System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_RIGHT) != 0.0f ||
		 System_GetPropertyFloat(SYSPROP_DISPLAY_SAFE_INSET_BOTTOM) != 0.0f)) {
		graphicsSettings->Add(new CheckBox(&g_PConfig.bIgnoreScreenInsets, gr->T("Ignore camera notch when centering")));
	}

	// Hide Immersive Mode on pre-kitkat Android
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 19) {
		// Let's reuse the Fullscreen translation string from desktop.
		graphicsSettings->Add(new CheckBox(&g_PConfig.bImmersiveMode, gr->T("FullScreen", "Full Screen")))->OnClick.Handle(this, &GameSettingsScreen::OnImmersiveModeChange);
	}
#endif

	graphicsSettings->Add(new ItemHeader(gr->T("Performance")));
	static const char *internalResolutions[] = { "Auto (1:1)", "1x PSP", "2x PSP", "3x PSP", "4x PSP", "5x PSP", "6x PSP", "7x PSP", "8x PSP", "9x PSP", "10x PSP" };
	resolutionChoice_ = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iInternalResolution, gr->T("Rendering Resolution"), internalResolutions, 0, ARRAY_SIZE(internalResolutions), gr->GetName(), screenManager()));
	resolutionChoice_->OnChoice.Handle(this, &GameSettingsScreen::OnResolutionChange);
	resolutionChoice_->SetEnabledFunc([] {
		return !g_PConfig.bSoftwareRendering && g_PConfig.iRenderingMode != FB_NON_BUFFERED_MODE;
	});

#if PPSSPP_PLATFORM(ANDROID)
	if (System_GetPropertyInt(SYSPROP_DEVICE_TYPE) != DEVICE_TYPE_TV) {
		static const char *deviceResolutions[] = { "Native device resolution", "Auto (same as Rendering)", "1x PSP", "2x PSP", "3x PSP", "4x PSP", "5x PSP" };
		int max_res_temp = std::max(System_GetPropertyInt(SYSPROP_DISPLAY_XRES), System_GetPropertyInt(SYSPROP_DISPLAY_YRES)) / 480 + 2;
		if (max_res_temp == 3)
			max_res_temp = 4;  // At least allow 2x
		int max_res = std::min(max_res_temp, (int)ARRAY_SIZE(deviceResolutions));
		UI::PopupMultiChoice *hwscale = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iAndroidHwScale, gr->T("Display Resolution (HW scaler)"), deviceResolutions, 0, max_res, gr->GetName(), screenManager()));
		hwscale->OnChoice.Handle(this, &GameSettingsScreen::OnHwScaleChange);  // To refresh the display mode
	}
#endif

#if !(PPSSPP_PLATFORM(ANDROID) || defined(USING_QT_UI) || PPSSPP_PLATFORM(UWP) || PPSSPP_PLATFORM(IOS))
	CheckBox *vSync = graphicsSettings->Add(new CheckBox(&g_PConfig.bVSync, gr->T("VSync")));
	vSync->OnClick.Add([=](EventParams &e) {
		NativeResized();
		return UI::EVENT_CONTINUE;
	});
#endif

	CheckBox *frameDuplication = graphicsSettings->Add(new CheckBox(&g_PConfig.bRenderDuplicateFrames, gr->T("Render duplicate frames to 60hz")));
	frameDuplication->OnClick.Add([=](EventParams &e) {
		settingInfo_->Show(gr->T("RenderDuplicateFrames Tip", "Can make framerate smoother in games that run at lower framerates"), e.v);
		return UI::EVENT_CONTINUE;
	});
	frameDuplication->SetEnabledFunc([] {
		return g_PConfig.iRenderingMode != FB_NON_BUFFERED_MODE && g_PConfig.iFrameSkip == 0;
	});

	if (GetGPUBackend() == GPUBackend::VULKAN || GetGPUBackend() == GPUBackend::OPENGL) {
		static const char *bufferOptions[] = { "No buffer", "Up to 1", "Up to 2" };
		PopupMultiChoice *inflightChoice = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iInflightFrames, gr->T("Buffer graphics commands (faster, input lag)"), bufferOptions, 0, ARRAY_SIZE(bufferOptions), gr->GetName(), screenManager()));
		inflightChoice->OnChoice.Handle(this, &GameSettingsScreen::OnInflightFramesChoice);
	}

	CheckBox *hwTransform = graphicsSettings->Add(new CheckBox(&g_PConfig.bHardwareTransform, gr->T("Hardware Transform")));
	hwTransform->OnClick.Handle(this, &GameSettingsScreen::OnHardwareTransform);
	hwTransform->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	CheckBox *swSkin = graphicsSettings->Add(new CheckBox(&g_PConfig.bSoftwareSkinning, gr->T("Software Skinning")));
	swSkin->OnClick.Add([=](EventParams &e) {
		settingInfo_->Show(gr->T("SoftwareSkinning Tip", "Combine skinned model draws on the CPU, faster in most games"), e.v);
		return UI::EVENT_CONTINUE;
	});
	swSkin->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	CheckBox *vtxCache = graphicsSettings->Add(new CheckBox(&g_PConfig.bVertexCache, gr->T("Vertex Cache")));
	vtxCache->OnClick.Add([=](EventParams &e) {
		settingInfo_->Show(gr->T("VertexCache Tip", "Faster, but may cause temporary flicker"), e.v);
		return UI::EVENT_CONTINUE;
	});
	vtxCache->SetEnabledFunc([] {
		return !g_PConfig.bSoftwareRendering && g_PConfig.bHardwareTransform;
	});

	CheckBox *clearHack = graphicsSettings->Add(new CheckBox(&g_PConfig.bClearFramebuffersOnFirstUseHack, gr->T("Clear Speedhack", "Clear framebuffers on first use (speedhack)")));
	clearHack->OnClick.Add([=](EventParams &e) {
		settingInfo_->Show(gr->T("ClearSpeedhack Tip", "Sometimes faster (mostly on mobile devices), may cause glitches"), e.v);
		return UI::EVENT_CONTINUE;
	});
	clearHack->SetEnabledFunc([] {
		return !g_PConfig.bSoftwareRendering;
	});

	CheckBox *texBackoff = graphicsSettings->Add(new CheckBox(&g_PConfig.bTextureBackoffCache, gr->T("Lazy texture caching", "Lazy texture caching (speedup)")));
	texBackoff->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	CheckBox *texSecondary_ = graphicsSettings->Add(new CheckBox(&g_PConfig.bTextureSecondaryCache, gr->T("Retain changed textures", "Retain changed textures (speedup, mem hog)")));
	texSecondary_->OnClick.Add([=](EventParams &e) {
		settingInfo_->Show(gr->T("RetainChangedTextures Tip", "Makes many games slower, but some games a lot faster"), e.v);
		return UI::EVENT_CONTINUE;
	});
	texSecondary_->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	CheckBox *framebufferSlowEffects = graphicsSettings->Add(new CheckBox(&g_PConfig.bDisableSlowFramebufEffects, gr->T("Disable slower effects (speedup)")));
	framebufferSlowEffects->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	// Seems solid, so we hide the setting.
	/*CheckBox *vtxJit = graphicsSettings->Add(new CheckBox(&g_PConfig.bVertexDecoderJit, gr->T("Vertex Decoder JIT")));

	if (PSP_IsInited()) {
		vtxJit->SetEnabled(false);
	}*/

	static const char *quality[] = { "Low", "Medium", "High" };
	PopupMultiChoice *beziersChoice = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iSplineBezierQuality, gr->T("LowCurves", "Spline/Bezier curves quality"), quality, 0, ARRAY_SIZE(quality), gr->GetName(), screenManager()));
	beziersChoice->OnChoice.Add([=](EventParams &e) {
		if (g_PConfig.iSplineBezierQuality != 0) {
			settingInfo_->Show(gr->T("LowCurves Tip", "Only used by some games, controls smoothness of curves"), e.v);
		}
		return UI::EVENT_CONTINUE;
	});

	CheckBox *tessellationHW = graphicsSettings->Add(new CheckBox(&g_PConfig.bHardwareTessellation, gr->T("Hardware Tessellation")));
	tessellationHW->OnClick.Add([=](EventParams &e) {
		settingInfo_->Show(gr->T("HardwareTessellation Tip", "Uses hardware to make curves"), e.v);
		return UI::EVENT_CONTINUE;
	});
	tessHWEnable_ = DoesBackendSupportHWTess() && !g_PConfig.bSoftwareRendering && g_PConfig.bHardwareTransform;
	tessellationHW->SetEnabledPtr(&tessHWEnable_);

	// In case we're going to add few other antialiasing option like MSAA in the future.
	// graphicsSettings->Add(new CheckBox(&g_PConfig.bFXAA, gr->T("FXAA")));
	graphicsSettings->Add(new ItemHeader(gr->T("Texture Scaling")));
#ifndef MOBILE_DEVICE
	static const char *texScaleLevels[] = {"Auto", "Off", "2x", "3x", "4x", "5x"};
#else
	static const char *texScaleLevels[] = {"Auto", "Off", "2x", "3x"};
#endif

	PopupMultiChoice *texScalingChoice = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iTexScalingLevel, gr->T("Upscale Level"), texScaleLevels, 0, ARRAY_SIZE(texScaleLevels), gr->GetName(), screenManager()));
	// TODO: Better check?  When it won't work, it scales down anyway.
	if (!gl_extensions.OES_texture_npot && GetGPUBackend() == GPUBackend::OPENGL) {
		texScalingChoice->HideChoice(3); // 3x
		texScalingChoice->HideChoice(5); // 5x
	}
	texScalingChoice->OnChoice.Add([=](EventParams &e) {
		if (g_PConfig.iTexScalingLevel != 1 && !UsingHardwareTextureScaling()) {
			settingInfo_->Show(gr->T("UpscaleLevel Tip", "CPU heavy - some scaling may be delayed to avoid stutter"), e.v);
		}
		return UI::EVENT_CONTINUE;
	});
	texScalingChoice->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	static const char *texScaleAlgos[] = { "xBRZ", "Hybrid", "Bicubic", "Hybrid + Bicubic", };
	PopupMultiChoice *texScalingType = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iTexScalingType, gr->T("Upscale Type"), texScaleAlgos, 0, ARRAY_SIZE(texScaleAlgos), gr->GetName(), screenManager()));
	texScalingType->SetEnabledFunc([]() {
		return !g_PConfig.bSoftwareRendering && !UsingHardwareTextureScaling();
	});

	CheckBox *deposterize = graphicsSettings->Add(new CheckBox(&g_PConfig.bTexDeposterize, gr->T("Deposterize")));
	deposterize->OnClick.Add([=](EventParams &e) {
		if (g_PConfig.bTexDeposterize == true) {
			settingInfo_->Show(gr->T("Deposterize Tip", "Fixes visual banding glitches in upscaled textures"), e.v);
		}
		return UI::EVENT_CONTINUE;
	});
	deposterize->SetEnabledFunc([]() {
		return !g_PConfig.bSoftwareRendering && !UsingHardwareTextureScaling();
	});

	ChoiceWithValueDisplay *textureShaderChoice = graphicsSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sTextureShaderName, gr->T("Texture Shader"), &TextureTranslateName));
	textureShaderChoice->OnClick.Handle(this, &GameSettingsScreen::OnTextureShader);
	textureShaderChoice->SetEnabledFunc([]() {
		return GetGPUBackend() == GPUBackend::VULKAN && !g_PConfig.bSoftwareRendering;
	});

	graphicsSettings->Add(new ItemHeader(gr->T("Texture Filtering")));
	static const char *anisoLevels[] = { "Off", "2x", "4x", "8x", "16x" };
	PopupMultiChoice *anisoFiltering = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iAnisotropyLevel, gr->T("Anisotropic Filtering"), anisoLevels, 0, ARRAY_SIZE(anisoLevels), gr->GetName(), screenManager()));
	anisoFiltering->SetDisabledPtr(&g_PConfig.bSoftwareRendering);

	static const char *texFilters[] = { "Auto", "Nearest", "Linear" };
	graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iTexFiltering, gr->T("Texture Filter"), texFilters, 1, ARRAY_SIZE(texFilters), gr->GetName(), screenManager()));

	static const char *bufFilters[] = { "Linear", "Nearest", };
	graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iBufFilter, gr->T("Screen Scaling Filter"), bufFilters, 1, ARRAY_SIZE(bufFilters), gr->GetName(), screenManager()));

#if PPSSPP_PLATFORM(ANDROID) || PPSSPP_PLATFORM(IOS)
	bool showCardboardSettings = true;
#else
	// If you enabled it through the ini, you can see this. Useful for testing.
	bool showCardboardSettings = g_PConfig.bEnableCardboardVR;
#endif
	if (showCardboardSettings) {
		graphicsSettings->Add(new ItemHeader(gr->T("Cardboard VR Settings", "Cardboard VR Settings")));
		graphicsSettings->Add(new CheckBox(&g_PConfig.bEnableCardboardVR, gr->T("Enable Cardboard VR", "Enable Cardboard VR")));
		PopupSliderChoice *cardboardScreenSize = graphicsSettings->Add(new PopupSliderChoice(&g_PConfig.iCardboardScreenSize, 30, 100, gr->T("Cardboard Screen Size", "Screen Size (in % of the viewport)"), 1, screenManager(), gr->T("% of viewport")));
		cardboardScreenSize->SetEnabledPtr(&g_PConfig.bEnableCardboardVR);
		PopupSliderChoice *cardboardXShift = graphicsSettings->Add(new PopupSliderChoice(&g_PConfig.iCardboardXShift, -100, 100, gr->T("Cardboard Screen X Shift", "X Shift (in % of the void)"), 1, screenManager(), gr->T("% of the void")));
		cardboardXShift->SetEnabledPtr(&g_PConfig.bEnableCardboardVR);
		PopupSliderChoice *cardboardYShift = graphicsSettings->Add(new PopupSliderChoice(&g_PConfig.iCardboardYShift, -100, 100, gr->T("Cardboard Screen Y Shift", "Y Shift (in % of the void)"), 1, screenManager(), gr->T("% of the void")));
		cardboardYShift->SetEnabledPtr(&g_PConfig.bEnableCardboardVR);
	}

	std::vector<std::string> cameraList = Camera::getDeviceList();
	if (cameraList.size() >= 1) {
		graphicsSettings->Add(new ItemHeader(gr->T("Camera")));
		PopupMultiChoiceDynamic *cameraChoice = graphicsSettings->Add(new PopupMultiChoiceDynamic(&g_PConfig.sCameraDevice, gr->T("Camera Device"), cameraList, nullptr, screenManager()));
		cameraChoice->OnChoice.Handle(this, &GameSettingsScreen::OnCameraDeviceChange);
	}

	graphicsSettings->Add(new ItemHeader(gr->T("Hack Settings", "Hack Settings (these WILL cause glitches)")));

	static const char *bloomHackOptions[] = { "Off", "Safe", "Balanced", "Aggressive" };
	PopupMultiChoice *bloomHack = graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iBloomHack, gr->T("Lower resolution for effects (reduces artifacts)"), bloomHackOptions, 0, ARRAY_SIZE(bloomHackOptions), gr->GetName(), screenManager()));
	bloomHack->SetEnabledFunc([] {
		return !g_PConfig.bSoftwareRendering && g_PConfig.iInternalResolution != 1;
	});

	graphicsSettings->Add(new ItemHeader(gr->T("Overlay Information")));
	static const char *fpsChoices[] = { "None", "Speed", "FPS", "Both" };
	graphicsSettings->Add(new PopupMultiChoice(&g_PConfig.iShowFPSCounter, gr->T("Show FPS Counter"), fpsChoices, 0, ARRAY_SIZE(fpsChoices), gr->GetName(), screenManager()));
	graphicsSettings->Add(new CheckBox(&g_PConfig.bShowDebugStats, gr->T("Show Debug Statistics")))->OnClick.Handle(this, &GameSettingsScreen::OnJitAffectingSetting);

	// Developer tools are not accessible ingame, so it goes here.
	graphicsSettings->Add(new ItemHeader(gr->T("Debugging")));
	Choice *dump = graphicsSettings->Add(new Choice(gr->T("Dump next frame to log")));
	dump->OnClick.Handle(this, &GameSettingsScreen::OnDumpNextFrameToLog);
	if (!PSP_IsInited())
		dump->SetEnabled(false);

	// Audio
	ViewGroup *audioSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	audioSettingsScroll->SetTag("GameSettingsAudio");
	LinearLayout *audioSettings = new LinearLayout(ORIENT_VERTICAL);
	audioSettings->SetSpacing(0);
	audioSettingsScroll->Add(audioSettings);
	tabHolder->AddTab(ms->T("Audio"), audioSettingsScroll);

	audioSettings->Add(new ItemHeader(ms->T("Audio")));

	audioSettings->Add(new CheckBox(&g_PConfig.bEnableSound, a->T("Enable Sound")));

	PopupSliderChoice *volume = audioSettings->Add(new PopupSliderChoice(&g_PConfig.iGlobalVolume, VOLUME_OFF, VOLUME_MAX, a->T("Global volume"), screenManager()));
	volume->SetEnabledPtr(&g_PConfig.bEnableSound);
	volume->SetZeroLabel(a->T("Mute"));

	PopupSliderChoice *altVolume = audioSettings->Add(new PopupSliderChoice(&g_PConfig.iAltSpeedVolume, VOLUME_OFF, VOLUME_MAX, a->T("Alternate speed volume"), screenManager()));
	altVolume->SetEnabledPtr(&g_PConfig.bEnableSound);
	altVolume->SetZeroLabel(a->T("Mute"));
	altVolume->SetNegativeDisable(a->T("Use global volume"));

#ifdef _WIN32
	if (IsVistaOrHigher()) {
		static const char *backend[] = { "Auto", "DSound (compatible)", "WASAPI (fast)" };
		PopupMultiChoice *audioBackend = audioSettings->Add(new PopupMultiChoice(&g_PConfig.iAudioBackend, a->T("Audio backend", "Audio backend (restart req.)"), backend, 0, ARRAY_SIZE(backend), a->GetName(), screenManager()));
		audioBackend->SetEnabledPtr(&g_PConfig.bEnableSound);
	}
#endif

	std::vector<std::string> micList = Microphone::getDeviceList();
	if (micList.size() >= 1) {
		audioSettings->Add(new ItemHeader(gr->T("Microphone")));
		PopupMultiChoiceDynamic *MicChoice = audioSettings->Add(new PopupMultiChoiceDynamic(&g_PConfig.sMicDevice, gr->T("Microphone Device"), micList, nullptr, screenManager()));
		MicChoice->OnChoice.Handle(this, &GameSettingsScreen::OnMicDeviceChange);
	}

#if defined(SDL)
	std::vector<std::string> audioDeviceList;
	PSplitString(System_GetProperty(SYSPROP_AUDIO_DEVICE_LIST), '\0', audioDeviceList);
	audioDeviceList.insert(audioDeviceList.begin(), a->T("Auto"));
	PopupMultiChoiceDynamic *audioDevice = audioSettings->Add(new PopupMultiChoiceDynamic(&g_PConfig.sAudioDevice, a->T("Device"), audioDeviceList, nullptr, screenManager()));
	audioDevice->OnChoice.Handle(this, &GameSettingsScreen::OnAudioDevice);

	audioSettings->Add(new CheckBox(&g_PConfig.bAutoAudioDevice, a->T("Switch on new audio device")));
#endif

#if defined(__ANDROID__)
	CheckBox *extraAudio = audioSettings->Add(new CheckBox(&g_PConfig.bExtraAudioBuffering, a->T("AudioBufferingForBluetooth", "Bluetooth-friendly buffer (slower)")));
	extraAudio->SetEnabledPtr(&g_PConfig.bEnableSound);
#endif

	// Control
	ViewGroup *controlsSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	controlsSettingsScroll->SetTag("GameSettingsControls");
	LinearLayout *controlsSettings = new LinearLayout(ORIENT_VERTICAL);
	controlsSettings->SetSpacing(0);
	controlsSettingsScroll->Add(controlsSettings);
	tabHolder->AddTab(ms->T("Controls"), controlsSettingsScroll);
	controlsSettings->Add(new ItemHeader(ms->T("Controls")));
	controlsSettings->Add(new Choice(co->T("Control Mapping")))->OnClick.Handle(this, &GameSettingsScreen::OnControlMapping);

#if defined(USING_WIN_UI)
	controlsSettings->Add(new CheckBox(&g_PConfig.bGamepadOnlyFocused, co->T("Ignore gamepads when not focused")));
#endif

#if defined(MOBILE_DEVICE)
	controlsSettings->Add(new CheckBox(&g_PConfig.bHapticFeedback, co->T("HapticFeedback", "Haptic Feedback (vibration)")));
	static const char *tiltTypes[] = { "None (Disabled)", "Analog Stick", "D-PAD", "PSP Action Buttons", "L/R Trigger Buttons" };
	controlsSettings->Add(new PopupMultiChoice(&g_PConfig.iTiltInputType, co->T("Tilt Input Type"), tiltTypes, 0, ARRAY_SIZE(tiltTypes), co->GetName(), screenManager()))->OnClick.Handle(this, &GameSettingsScreen::OnTiltTypeChange);

	Choice *customizeTilt = controlsSettings->Add(new Choice(co->T("Customize tilt")));
	customizeTilt->OnClick.Handle(this, &GameSettingsScreen::OnTiltCustomize);
	customizeTilt->SetEnabledFunc([] {
		return g_PConfig.iTiltInputType != 0;
	});
#endif

	// TVs don't have touch control, at least not yet.
	if (System_GetPropertyInt(SYSPROP_DEVICE_TYPE) != DEVICE_TYPE_TV) {
		controlsSettings->Add(new ItemHeader(co->T("OnScreen", "On-Screen Touch Controls")));
		controlsSettings->Add(new CheckBox(&g_PConfig.bShowTouchControls, co->T("OnScreen", "On-Screen Touch Controls")));
		layoutEditorChoice_ = controlsSettings->Add(new Choice(co->T("Custom layout...")));
		layoutEditorChoice_->OnClick.Handle(this, &GameSettingsScreen::OnTouchControlLayout);
		layoutEditorChoice_->SetEnabledPtr(&g_PConfig.bShowTouchControls);

		// Re-centers itself to the touch location on touch-down.
		CheckBox *floatingAnalog = controlsSettings->Add(new CheckBox(&g_PConfig.bAutoCenterTouchAnalog, co->T("Auto-centering analog stick")));
		floatingAnalog->SetEnabledPtr(&g_PConfig.bShowTouchControls);

		// Combo key setup
		Choice *comboKey = controlsSettings->Add(new Choice(co->T("Combo Key Setup")));
		comboKey->OnClick.Handle(this, &GameSettingsScreen::OnComboKey);
		comboKey->SetEnabledPtr(&g_PConfig.bShowTouchControls);

		// On non iOS systems, offer to let the user see this button.
		// Some Windows touch devices don't have a back button or other button to call up the menu.
		if (System_GetPropertyBool(SYSPROP_HAS_BACK_BUTTON)) {
			CheckBox *enablePauseBtn = controlsSettings->Add(new CheckBox(&g_PConfig.bShowTouchPause, co->T("Show Touch Pause Menu Button")));

			// Don't allow the user to disable it once in-game, so they can't lock themselves out of the menu.
			if (!PSP_IsInited()) {
				enablePauseBtn->SetEnabledPtr(&g_PConfig.bShowTouchControls);
			} else {
				enablePauseBtn->SetEnabled(false);
			}
		}

		CheckBox *disableDiags = controlsSettings->Add(new CheckBox(&g_PConfig.bDisableDpadDiagonals, co->T("Disable D-Pad diagonals (4-way touch)")));
		disableDiags->SetEnabledPtr(&g_PConfig.bShowTouchControls);
		PopupSliderChoice *opacity = controlsSettings->Add(new PopupSliderChoice(&g_PConfig.iTouchButtonOpacity, 0, 100, co->T("Button Opacity"), screenManager(), "%"));
		opacity->SetEnabledPtr(&g_PConfig.bShowTouchControls);
		opacity->SetFormat("%i%%");
		PopupSliderChoice *autoHide = controlsSettings->Add(new PopupSliderChoice(&g_PConfig.iTouchButtonHideSeconds, 0, 300, co->T("Auto-hide buttons after seconds"), screenManager(), co->T("seconds, 0 : off")));
		autoHide->SetEnabledPtr(&g_PConfig.bShowTouchControls);
		autoHide->SetFormat("%is");
		autoHide->SetZeroLabel(co->T("Off"));
		static const char *touchControlStyles[] = {"Classic", "Thin borders", "Glowing borders"};
		View *style = controlsSettings->Add(new PopupMultiChoice(&g_PConfig.iTouchButtonStyle, co->T("Button style"), touchControlStyles, 0, ARRAY_SIZE(touchControlStyles), co->GetName(), screenManager()));
		style->SetEnabledPtr(&g_PConfig.bShowTouchControls);
	}

#ifdef _WIN32
	static const char *inverseDeadzoneModes[] = { "Off", "X", "Y", "X + Y" };

	controlsSettings->Add(new ItemHeader(co->T("DInput Analog Settings", "DInput Analog Settings")));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fDInputAnalogDeadzone, 0.0f, 1.0f, co->T("Deadzone Radius"), 0.01f, screenManager(), "/ 1.0"));
	controlsSettings->Add(new PopupMultiChoice(&g_PConfig.iDInputAnalogInverseMode, co->T("Analog Mapper Mode"), inverseDeadzoneModes, 0, ARRAY_SIZE(inverseDeadzoneModes), co->GetName(), screenManager()));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fDInputAnalogInverseDeadzone, 0.0f, 1.0f, co->T("Analog Mapper Low End", "Analog Mapper Low End (Inverse Deadzone)"), 0.01f, screenManager(), "/ 1.0"));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fDInputAnalogSensitivity, 0.0f, 10.0f, co->T("Analog Mapper High End", "Analog Mapper High End (Axis Sensitivity)"), 0.01f, screenManager(), "x"));

	controlsSettings->Add(new ItemHeader(co->T("XInput Analog Settings", "XInput Analog Settings")));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fXInputAnalogDeadzone, 0.0f, 1.0f, co->T("Deadzone Radius"), 0.01f, screenManager(), "/ 1.0"));
	controlsSettings->Add(new PopupMultiChoice(&g_PConfig.iXInputAnalogInverseMode, co->T("Analog Mapper Mode"), inverseDeadzoneModes, 0, ARRAY_SIZE(inverseDeadzoneModes), co->GetName(), screenManager()));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fXInputAnalogInverseDeadzone, 0.0f, 1.0f, co->T("Analog Mapper Low End", "Analog Mapper Low End (Inverse Deadzone)"), 0.01f, screenManager(), "/ 1.0"));
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fXInputAnalogSensitivity, 0.0f, 10.0f, co->T("Analog Mapper High End", "Analog Mapper High End (Axis Sensitivity)"), 0.01f, screenManager(), "x"));
#else
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fXInputAnalogSensitivity, 0.0f, 10.0f, co->T("Analog Axis Sensitivity", "Analog Axis Sensitivity"), 0.01f, screenManager(), "x"));
#endif
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fAnalogAutoRotSpeed, 0.0f, 25.0f, co->T("Analog auto-rotation speed"), 1.0f, screenManager()));

	controlsSettings->Add(new ItemHeader(co->T("Keyboard", "Keyboard Control Settings")));
#if defined(USING_WIN_UI)
	controlsSettings->Add(new CheckBox(&g_PConfig.bIgnoreWindowsKey, co->T("Ignore Windows Key")));
#endif // #if defined(USING_WIN_UI)
	auto analogLimiter = new PopupSliderChoiceFloat(&g_PConfig.fAnalogLimiterDeadzone, 0.0f, 1.0f, co->T("Analog Limiter"), 0.10f, screenManager(), "/ 1.0");
	controlsSettings->Add(analogLimiter);
	analogLimiter->OnChange.Add([=](EventParams &e) {
		settingInfo_->Show(co->T("AnalogLimiter Tip", "When the analog limiter button is pressed"), e.v);
		return UI::EVENT_CONTINUE;
	});
#if defined(USING_WIN_UI) || defined(SDL)
	controlsSettings->Add(new ItemHeader(co->T("Mouse", "Mouse settings")));
	CheckBox *mouseControl = controlsSettings->Add(new CheckBox(&g_PConfig.bMouseControl, co->T("Use Mouse Control")));
	mouseControl->OnClick.Add([=](EventParams &e) {
		if(g_PConfig.bMouseControl)
			settingInfo_->Show(co->T("MouseControl Tip", "You can now map mouse in control mapping screen by pressing the 'M' icon."), e.v);
		return UI::EVENT_CONTINUE;
	});
	controlsSettings->Add(new CheckBox(&g_PConfig.bMouseConfine, co->T("Confine Mouse", "Trap mouse within window/display area")))->SetEnabledPtr(&g_PConfig.bMouseControl);
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fMouseSensitivity, 0.01f, 1.0f, co->T("Mouse sensitivity"), 0.01f, screenManager(), "x"))->SetEnabledPtr(&g_PConfig.bMouseControl);
	controlsSettings->Add(new PopupSliderChoiceFloat(&g_PConfig.fMouseSmoothing, 0.0f, 0.95f, co->T("Mouse smoothing"), 0.05f, screenManager(), "x"))->SetEnabledPtr(&g_PConfig.bMouseControl);
#endif

	ViewGroup *networkingSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	networkingSettingsScroll->SetTag("GameSettingsNetworking");
	LinearLayout *networkingSettings = new LinearLayout(ORIENT_VERTICAL);
	networkingSettings->SetSpacing(0);
	networkingSettingsScroll->Add(networkingSettings);
	tabHolder->AddTab(ms->T("Networking"), networkingSettingsScroll);

	networkingSettings->Add(new ItemHeader(n->T("Networking")));

	networkingSettings->Add(new Choice(n->T("Adhoc Multiplayer forum")))->OnClick.Handle(this, &GameSettingsScreen::OnAdhocGuides);

	networkingSettings->Add(new CheckBox(&g_PConfig.bEnableWlan, n->T("Enable networking", "Enable networking/wlan (beta)")));
	networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sMACAddress, n->T("Change Mac Address"), (const char*)nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeMacAddress);
	static const char* wlanChannels[] = { "Auto", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
	auto wlanChannelChoice = networkingSettings->Add(new PopupMultiChoice(&g_PConfig.iWlanAdhocChannel, n->T("WLAN Channel"), wlanChannels, 0, ARRAY_SIZE(wlanChannels), n->GetName(), screenManager()));
	for (int i = 0; i < 4; i++) {
		wlanChannelChoice->HideChoice(i + 2);
		wlanChannelChoice->HideChoice(i + 7);
	}
	networkingSettings->Add(new CheckBox(&g_PConfig.bDiscordPresence, n->T("Send Discord Presence information")));

	networkingSettings->Add(new ItemHeader(n->T("AdHoc Server")));
	networkingSettings->Add(new CheckBox(&g_PConfig.bEnableAdhocServer, n->T("Enable built-in PRO Adhoc Server", "Enable built-in PRO Adhoc Server")));
	networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.proAdhocServer, n->T("Change proAdhocServer Address (localhost = multiple instance)"), (const char*)nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeproAdhocServerAddress);

	networkingSettings->Add(new ItemHeader(n->T("UPnP (port-forwarding)")));
	networkingSettings->Add(new CheckBox(&g_PConfig.bEnableUPnP, n->T("Enable UPnP", "Enable UPnP (need a few seconds to detect)")));
	networkingSettings->Add(new CheckBox(&g_PConfig.bUPnPUseOriginalPort, n->T("UPnP use original port", "UPnP use original port (Enabled = PSP compatibility)")))->SetEnabledPtr(&g_PConfig.bEnableUPnP);

	networkingSettings->Add(new ItemHeader(n->T("Chat")));
	networkingSettings->Add(new CheckBox(&g_PConfig.bEnableNetworkChat, n->T("Enable network chat", "Enable network chat")));
	static const char *chatButtonPositions[] = { "Bottom Left", "Bottom Center", "Bottom Right", "Top Left", "Top Center", "Top Right", "Center Left", "Center Right" };
	networkingSettings->Add(new PopupMultiChoice(&g_PConfig.iChatButtonPosition, n->T("Chat Button Position"), chatButtonPositions, 0, ARRAY_SIZE(chatButtonPositions), n->GetName(), screenManager()))->SetEnabledPtr(&g_PConfig.bEnableNetworkChat);
	static const char *chatScreenPositions[] = { "Bottom Left", "Bottom Center", "Bottom Right", "Top Left", "Top Center", "Top Right" };
	networkingSettings->Add(new PopupMultiChoice(&g_PConfig.iChatScreenPosition, n->T("Chat Screen Position"), chatScreenPositions, 0, ARRAY_SIZE(chatScreenPositions), n->GetName(), screenManager()))->SetEnabledPtr(&g_PConfig.bEnableNetworkChat);
	networkingSettings->Add(new ItemHeader(n->T("QuickChat", "Quick Chat")));
	networkingSettings->Add(new CheckBox(&g_PConfig.bEnableQuickChat, n->T("EnableQuickChat", "Enable Quick Chat")));
#if !defined(MOBILE_DEVICE) && !defined(USING_QT_UI)  // TODO: Add all platforms where KEY_CHAR support is added
	PopupTextInputChoice *qc1 = networkingSettings->Add(new PopupTextInputChoice(&g_PConfig.sQuickChat0, n->T("Quick Chat 1"), "", 32, screenManager()));
	qc1->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	PopupTextInputChoice *qc2 = networkingSettings->Add(new PopupTextInputChoice(&g_PConfig.sQuickChat1, n->T("Quick Chat 2"), "", 32, screenManager()));
	qc2->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	PopupTextInputChoice *qc3 = networkingSettings->Add(new PopupTextInputChoice(&g_PConfig.sQuickChat2, n->T("Quick Chat 3"), "", 32, screenManager()));
	qc3->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	PopupTextInputChoice *qc4 = networkingSettings->Add(new PopupTextInputChoice(&g_PConfig.sQuickChat3, n->T("Quick Chat 4"), "", 32, screenManager()));
	qc4->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	PopupTextInputChoice *qc5 = networkingSettings->Add(new PopupTextInputChoice(&g_PConfig.sQuickChat4, n->T("Quick Chat 5"), "", 32, screenManager()));
	qc5->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
#elif defined(USING_QT_UI)
	Choice *qc1 = networkingSettings->Add(new Choice(n->T("Quick Chat 1")));
	qc1->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	qc1->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat0);
	Choice *qc2 = networkingSettings->Add(new Choice(n->T("Quick Chat 2")));
	qc2->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	qc2->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat1);
	Choice *qc3 = networkingSettings->Add(new Choice(n->T("Quick Chat 3")));
	qc3->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	qc3->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat2);
	Choice *qc4 = networkingSettings->Add(new Choice(n->T("Quick Chat 4")));
	qc4->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	qc4->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat3);
	Choice *qc5 = networkingSettings->Add(new Choice(n->T("Quick Chat 5")));
	qc5->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	qc5->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat4);
#elif defined(__ANDROID__)
	ChoiceWithValueDisplay *qc1 = networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sQuickChat0, n->T("Quick Chat 1"), (const char *)nullptr));
	qc1->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat0);
	qc1->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	ChoiceWithValueDisplay *qc2 = networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sQuickChat1, n->T("Quick Chat 2"), (const char *)nullptr));
	qc2->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat1);
	qc2->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	ChoiceWithValueDisplay *qc3 = networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sQuickChat2, n->T("Quick Chat 3"), (const char *)nullptr));
	qc3->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat2);
	qc3->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	ChoiceWithValueDisplay *qc4 = networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sQuickChat3, n->T("Quick Chat 4"), (const char *)nullptr));
	qc4->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat3);
	qc4->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
	ChoiceWithValueDisplay *qc5 = networkingSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sQuickChat4, n->T("Quick Chat 5"), (const char *)nullptr));
	qc5->OnClick.Handle(this, &GameSettingsScreen::OnChangeQuickChat4);
	qc5->SetEnabledPtr(&g_PConfig.bEnableQuickChat);
#endif

	networkingSettings->Add(new ItemHeader(n->T("Misc (default = compatiblity)")));
	networkingSettings->Add(new PopupSliderChoice(&g_PConfig.iPortOffset, 0, 60000, n->T("Port offset", "Port offset (0 = PSP compatibility)"), 100, screenManager()));
	networkingSettings->Add(new PopupSliderChoice(&g_PConfig.iMinTimeout, 1, 15000, n->T("Minimum Timeout", "Minimum Timeout (override low latency in ms)"), 100, screenManager()));
	networkingSettings->Add(new CheckBox(&g_PConfig.bTCPNoDelay, n->T("TCP No Delay", "TCP No Delay (faster TCP)")));
	networkingSettings->Add(new CheckBox(&g_PConfig.bForcedFirstConnect, n->T("Forced First Connect", "Forced First Connect (faster Connect)")));

	ViewGroup *toolsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	toolsScroll->SetTag("GameSettingsTools");
	LinearLayout *tools = new LinearLayout(ORIENT_VERTICAL);
	tools->SetSpacing(0);
	toolsScroll->Add(tools);
	tabHolder->AddTab(ms->T("Tools"), toolsScroll);

	tools->Add(new ItemHeader(ms->T("Tools")));
	// These were moved here so use the wrong translation objects, to avoid having to change all inis... This isn't a sustainable situation :P
	tools->Add(new Choice(sa->T("Savedata Manager")))->OnClick.Handle(this, &GameSettingsScreen::OnSavedataManager);
	tools->Add(new Choice(dev->T("System Information")))->OnClick.Handle(this, &GameSettingsScreen::OnSysInfo);
	tools->Add(new Choice(sy->T("Developer Tools")))->OnClick.Handle(this, &GameSettingsScreen::OnDeveloperTools);
	tools->Add(new Choice(ri->T("Remote disc streaming")))->OnClick.Handle(this, &GameSettingsScreen::OnRemoteISO);

	// System
	ViewGroup *systemSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	systemSettingsScroll->SetTag("GameSettingsSystem");
	LinearLayout *systemSettings = new LinearLayout(ORIENT_VERTICAL);
	systemSettings->SetSpacing(0);
	systemSettingsScroll->Add(systemSettings);
	tabHolder->AddTab(ms->T("System"), systemSettingsScroll);

	systemSettings->Add(new ItemHeader(sy->T("UI Language")));  // Should be renamed "UI"?
	systemSettings->Add(new Choice(dev->T("Language", "Language")))->OnClick.Handle(this, &GameSettingsScreen::OnLanguage);
	systemSettings->Add(new CheckBox(&g_PConfig.bUISound, sy->T("UI Sound")));

	systemSettings->Add(new ItemHeader(sy->T("Help the PPSSPP team")));
	enableReports_ = Reporting::IsEnabled();
	enableReportsCheckbox_ = new CheckBox(&enableReports_, sy->T("Enable Compatibility Server Reports"));
	enableReportsCheckbox_->SetEnabled(Reporting::IsSupported());
	systemSettings->Add(enableReportsCheckbox_);

	systemSettings->Add(new ItemHeader(sy->T("Emulation")));

	systemSettings->Add(new CheckBox(&g_PConfig.bFastMemory, sy->T("Fast Memory", "Fast Memory (Unstable)")))->OnClick.Handle(this, &GameSettingsScreen::OnJitAffectingSetting);
	systemSettings->Add(new CheckBox(&g_PConfig.bIgnoreBadMemAccess, sy->T("Ignore bad memory accesses")));

	systemSettings->Add(new CheckBox(&g_PConfig.bSeparateIOThread, sy->T("I/O on thread (experimental)")))->SetEnabled(!PSP_IsInited());
	static const char *ioTimingMethods[] = { "Fast (lag on slow storage)", "Host (bugs, less lag)", "Simulate UMD delays" };
	View *ioTimingMethod = systemSettings->Add(new PopupMultiChoice(&g_PConfig.iIOTimingMethod, sy->T("IO timing method"), ioTimingMethods, 0, ARRAY_SIZE(ioTimingMethods), sy->GetName(), screenManager()));
	ioTimingMethod->SetEnabledPtr(&g_PConfig.bSeparateIOThread);
	systemSettings->Add(new CheckBox(&g_PConfig.bForceLagSync, sy->T("Force real clock sync (slower, less lag)")));
	PopupSliderChoice *lockedMhz = systemSettings->Add(new PopupSliderChoice(&g_PConfig.iLockedCPUSpeed, 0, 1000, sy->T("Change CPU Clock", "Change CPU Clock (unstable)"), screenManager(), sy->T("MHz, 0:default")));
	lockedMhz->OnChange.Add([&](UI::EventParams &) {
		enableReportsCheckbox_->SetEnabled(Reporting::IsSupported());
		return UI::EVENT_CONTINUE;
	});
	lockedMhz->SetZeroLabel(sy->T("Auto"));
	PopupSliderChoice *rewindFreq = systemSettings->Add(new PopupSliderChoice(&g_PConfig.iRewindFlipFrequency, 0, 1800, sy->T("Rewind Snapshot Frequency", "Rewind Snapshot Frequency (mem hog)"), screenManager(), sy->T("frames, 0:off")));
	rewindFreq->SetZeroLabel(sy->T("Off"));

	systemSettings->Add(new CheckBox(&g_PConfig.bMemStickInserted, sy->T("Memory Stick inserted")));
	PopupSliderChoice *memStickSize = systemSettings->Add(new PopupSliderChoice(&g_PConfig.iMemStickSizeGB, 1, 32, sy->T("Change Memory Stick Size", "Change Memory Stick Size(GB)"), screenManager(), "GB"));

	systemSettings->Add(new ItemHeader(sy->T("General")));

#if PPSSPP_PLATFORM(ANDROID)
	if (System_GetPropertyInt(SYSPROP_DEVICE_TYPE) == DEVICE_TYPE_MOBILE) {
		static const char *screenRotation[] = {"Auto", "Landscape", "Portrait", "Landscape Reversed", "Portrait Reversed", "Landscape Auto"};
		PopupMultiChoice *rot = systemSettings->Add(new PopupMultiChoice(&g_PConfig.iScreenRotation, co->T("Screen Rotation"), screenRotation, 0, ARRAY_SIZE(screenRotation), co->GetName(), screenManager()));
		rot->OnChoice.Handle(this, &GameSettingsScreen::OnScreenRotation);

		if (System_GetPropertyBool(SYSPROP_SUPPORTS_SUSTAINED_PERF_MODE)) {
			systemSettings->Add(new CheckBox(&g_PConfig.bSustainedPerformanceMode, sy->T("Sustained performance mode")))->OnClick.Handle(this, &GameSettingsScreen::OnSustainedPerformanceModeChange);
		}
	}
#endif
	systemSettings->Add(new CheckBox(&g_PConfig.bCheckForNewVersion, sy->T("VersionCheck", "Check for new versions of PPSSPP")));
	const std::string bgPng = GetSysDirectory(DIRECTORY_SYSTEM) + "background.png";
	const std::string bgJpg = GetSysDirectory(DIRECTORY_SYSTEM) + "background.jpg";
	if (PFile::Exists(bgPng) || PFile::Exists(bgJpg)) {
		backgroundChoice_ = systemSettings->Add(new Choice(sy->T("Clear UI background")));
	} else if (System_GetPropertyBool(SYSPROP_HAS_IMAGE_BROWSER)) {
		backgroundChoice_ = systemSettings->Add(new Choice(sy->T("Set UI background...")));
	} else {
		backgroundChoice_ = nullptr;
	}
	if (backgroundChoice_ != nullptr) {
		backgroundChoice_->OnClick.Handle(this, &GameSettingsScreen::OnChangeBackground);
	}

	systemSettings->Add(new Choice(sy->T("Restore Default Settings")))->OnClick.Handle(this, &GameSettingsScreen::OnRestoreDefaultSettings);
	systemSettings->Add(new CheckBox(&g_PConfig.bEnableStateUndo, sy->T("Savestate slot backups")));
	static const char *autoLoadSaveStateChoices[] = { "Off", "Oldest Save", "Newest Save", "Slot 1", "Slot 2", "Slot 3", "Slot 4", "Slot 5" };
	systemSettings->Add(new PopupMultiChoice(&g_PConfig.iAutoLoadSaveState, sy->T("Auto Load Savestate"), autoLoadSaveStateChoices, 0, ARRAY_SIZE(autoLoadSaveStateChoices), sy->GetName(), screenManager()));
#if defined(USING_WIN_UI) || defined(USING_QT_UI) || PPSSPP_PLATFORM(ANDROID)
	systemSettings->Add(new CheckBox(&g_PConfig.bBypassOSKWithKeyboard, sy->T("Use system native keyboard")));
#endif
#if PPSSPP_PLATFORM(ANDROID)
	auto memstickPath = systemSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.memStickDirectory, sy->T("Change Memory Stick folder"), (const char *)nullptr));
	memstickPath->SetEnabled(!PSP_IsInited());
	memstickPath->OnClick.Handle(this, &GameSettingsScreen::OnChangeMemStickDir);
#elif defined(_WIN32) && !PPSSPP_PLATFORM(UWP)
	SavePathInMyDocumentChoice = systemSettings->Add(new CheckBox(&installed_, sy->T("Save path in My Documents", "Save path in My Documents")));
	SavePathInMyDocumentChoice->SetEnabled(!PSP_IsInited());
	SavePathInMyDocumentChoice->OnClick.Handle(this, &GameSettingsScreen::OnSavePathMydoc);
	SavePathInOtherChoice = systemSettings->Add(new CheckBox(&otherinstalled_, sy->T("Save path in installed.txt", "Save path in installed.txt")));
	SavePathInOtherChoice->SetEnabled(false);
	SavePathInOtherChoice->OnClick.Handle(this, &GameSettingsScreen::OnSavePathOther);
	const bool myDocsExists = W32Util::UserDocumentsPath().size() != 0;
	const std::string PPSSPPpath = PFile::GetExeDirectory();
	const std::string installedFile = PPSSPPpath + "installed.txt";
	installed_ = PFile::Exists(installedFile);
	otherinstalled_ = false;
	if (!installed_ && myDocsExists) {
		if (PFile::CreateEmptyFile(PPSSPPpath + "installedTEMP.txt")) {
			// Disable the setting whether cannot create & delete file
			if (!(PFile::Delete(PPSSPPpath + "installedTEMP.txt")))
				SavePathInMyDocumentChoice->SetEnabled(false);
			else
				SavePathInOtherChoice->SetEnabled(!PSP_IsInited());
		} else
			SavePathInMyDocumentChoice->SetEnabled(false);
	} else {
		if (installed_ && myDocsExists) {
#ifdef _MSC_VER
			std::ifstream inputFile(ConvertUTF8ToWString(installedFile));
#else
			std::ifstream inputFile(installedFile);
#endif
			if (!inputFile.fail() && inputFile.is_open()) {
				std::string tempString;
				std::getline(inputFile, tempString);

				// Skip UTF-8 encoding bytes if there are any. There are 3 of them.
				if (tempString.substr(0, 3) == "\xEF\xBB\xBF")
					tempString = tempString.substr(3);
				SavePathInOtherChoice->SetEnabled(!PSP_IsInited());
				if (!(tempString == "")) {
					installed_ = false;
					otherinstalled_ = true;
				}
			}
			inputFile.close();
		} else if (!myDocsExists) {
			SavePathInMyDocumentChoice->SetEnabled(false);
		}
	}
#endif

#if defined(_M_X64)
	systemSettings->Add(new CheckBox(&g_PConfig.bCacheFullIsoInRam, sy->T("Cache ISO in RAM", "Cache full ISO in RAM")))->SetEnabled(!PSP_IsInited());
#endif

	systemSettings->Add(new ItemHeader(sy->T("Cheats", "Cheats (experimental, see forums)")));
	CheckBox *enableCheats = systemSettings->Add(new CheckBox(&g_PConfig.bEnableCheats, sy->T("Enable Cheats")));
	enableCheats->OnClick.Add([&](UI::EventParams &) {
		enableReportsCheckbox_->SetEnabled(Reporting::IsSupported());
		return UI::EVENT_CONTINUE;
	});
	systemSettings->SetSpacing(0);

	systemSettings->Add(new ItemHeader(sy->T("PSP Settings")));
	static const char *models[] = {"PSP-1000", "PSP-2000/3000"};
	systemSettings->Add(new PopupMultiChoice(&g_PConfig.iPSPModel, sy->T("PSP Model"), models, 0, ARRAY_SIZE(models), sy->GetName(), screenManager()))->SetEnabled(!PSP_IsInited());
	// TODO: Come up with a way to display a keyboard for mobile users,
	// so until then, this is Windows/Desktop only.
#if !defined(MOBILE_DEVICE)  // TODO: Add all platforms where KEY_CHAR support is added
	systemSettings->Add(new PopupTextInputChoice(&g_PConfig.sNickName, sy->T("Change Nickname"), "", 32, screenManager()));
#elif defined(__ANDROID__)
	systemSettings->Add(new ChoiceWithValueDisplay(&g_PConfig.sNickName, sy->T("Change Nickname"), (const char *)nullptr))->OnClick.Handle(this, &GameSettingsScreen::OnChangeNickname);
#endif

	systemSettings->Add(new CheckBox(&g_PConfig.bScreenshotsAsPNG, sy->T("Screenshots as PNG")));

#if defined(_WIN32) || (defined(USING_QT_UI) && !defined(MOBILE_DEVICE))
	systemSettings->Add(new CheckBox(&g_PConfig.bDumpFrames, sy->T("Record Display")));
	systemSettings->Add(new CheckBox(&g_PConfig.bUseFFV1, sy->T("Use Lossless Video Codec (FFV1)")));
	systemSettings->Add(new CheckBox(&g_PConfig.bDumpVideoOutput, sy->T("Use output buffer (with overlay) for recording")));
	systemSettings->Add(new CheckBox(&g_PConfig.bDumpAudio, sy->T("Record Audio")));
	systemSettings->Add(new CheckBox(&g_PConfig.bSaveLoadResetsAVdumping, sy->T("Reset Recording on Save/Load State")));
#endif
	systemSettings->Add(new CheckBox(&g_PConfig.bDayLightSavings, sy->T("Day Light Saving")));
	static const char *dateFormat[] = { "YYYYMMDD", "MMDDYYYY", "DDMMYYYY"};
	systemSettings->Add(new PopupMultiChoice(&g_PConfig.iDateFormat, sy->T("Date Format"), dateFormat, 0, 3, sy->GetName(), screenManager()));
	static const char *timeFormat[] = { "24HR", "12HR" };
	systemSettings->Add(new PopupMultiChoice(&g_PConfig.iTimeFormat, sy->T("Time Format"), timeFormat, 0, 2, sy->GetName(), screenManager()));
	static const char *buttonPref[] = { "Use O to confirm", "Use X to confirm" };
	systemSettings->Add(new PopupMultiChoice(&g_PConfig.iButtonPreference, sy->T("Confirmation Button"), buttonPref, 0, 2, sy->GetName(), screenManager()));
}

UI::EventReturn GameSettingsScreen::OnAutoFrameskip(UI::EventParams &e) {
	if (g_PConfig.bAutoFrameSkip && g_PConfig.iFrameSkip == 0) {
		g_PConfig.iFrameSkip = 1;
	}
	if (g_PConfig.bAutoFrameSkip && g_PConfig.iRenderingMode == FB_NON_BUFFERED_MODE) {
		g_PConfig.iRenderingMode = FB_BUFFERED_MODE;
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSoftwareRendering(UI::EventParams &e) {
	tessHWEnable_ = DoesBackendSupportHWTess() && !g_PConfig.bSoftwareRendering && g_PConfig.bHardwareTransform;
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnHardwareTransform(UI::EventParams &e) {
	tessHWEnable_ = DoesBackendSupportHWTess() && !g_PConfig.bSoftwareRendering && g_PConfig.bHardwareTransform;
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnScreenRotation(UI::EventParams &e) {
	INFO_LOG(SYSTEM, "New display rotation: %d", g_PConfig.iScreenRotation);
	INFO_LOG(SYSTEM, "Sending rotate");
	System_SendMessage("rotate", "");
	INFO_LOG(SYSTEM, "Got back from rotate");
	return UI::EVENT_DONE;
}

void RecreateActivity() {
	const int SYSTEM_JELLYBEAN = 16;
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= SYSTEM_JELLYBEAN) {
		INFO_LOG(SYSTEM, "Sending recreate");
		System_SendMessage("recreate", "");
		INFO_LOG(SYSTEM, "Got back from recreate");
	} else {
		auto gr = GetI18NCategory("Graphics");
		System_SendMessage("toast", gr->T("Must Restart", "You must restart PPSSPP for this change to take effect"));
	}
}

UI::EventReturn GameSettingsScreen::OnAdhocGuides(UI::EventParams &e) {
	LaunchBrowser("https://forums.ppsspp.org/forumdisplay.php?fid=34");
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnImmersiveModeChange(UI::EventParams &e) {
	System_SendMessage("immersive", "");
	if (g_PConfig.iAndroidHwScale != 0) {
		RecreateActivity();
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSustainedPerformanceModeChange(UI::EventParams &e) {
	System_SendMessage("sustainedPerfMode", "");
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnRenderingMode(UI::EventParams &e) {
	// We do not want to report when rendering mode is Framebuffer to memory - so many issues
	// are caused by that (framebuffer copies overwriting display lists, etc).
	Reporting::UpdateConfig();
	enableReports_ = Reporting::IsEnabled();
	enableReportsCheckbox_->SetEnabled(Reporting::IsSupported());

	if (g_PConfig.iRenderingMode == FB_NON_BUFFERED_MODE) {
		g_PConfig.bAutoFrameSkip = false;
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnJitAffectingSetting(UI::EventParams &e) {
	NativeMessageReceived("clear jit", "");
	return UI::EVENT_DONE;
}

#if PPSSPP_PLATFORM(ANDROID)

UI::EventReturn GameSettingsScreen::OnChangeMemStickDir(UI::EventParams &e) {
	auto sy = GetI18NCategory("System");
	System_InputBoxGetString(sy->T("Memory Stick Folder"), g_PConfig.memStickDirectory, [&](bool result, const std::string &value) {
		auto sy = GetI18NCategory("System");
		auto di = GetI18NCategory("Dialog");

		if (result) {
			std::string newPath = value;
			size_t pos = newPath.find_last_not_of("/");
			// Gotta have at least something but a /, and also needs to start with a /.
			if (newPath.empty() || pos == newPath.npos || newPath[0] != '/') {
				settingInfo_->Show(sy->T("ChangingMemstickPathInvalid", "That path couldn't be used to save Memory Stick files."), nullptr);
				return;
			}
			if (pos != newPath.size() - 1) {
				newPath = newPath.substr(0, pos + 1);
			}

			pendingMemstickFolder_ = newPath;
			std::string promptMessage = sy->T("ChangingMemstickPath", "Save games, save states, and other data will not be copied to this folder.\n\nChange the Memory Stick folder?");
			if (!PFile::Exists(newPath)) {
				promptMessage = sy->T("ChangingMemstickPathNotExists", "That folder doesn't exist yet.\n\nSave games, save states, and other data will not be copied to this folder.\n\nCreate a new Memory Stick folder?");
			}
			// Add the path for clarity and proper confirmation.
			promptMessage += "\n\n" + newPath + "/";
			screenManager()->push(new PromptScreen(promptMessage, di->T("Yes"), di->T("No"), std::bind(&GameSettingsScreen::CallbackMemstickFolder, this, std::placeholders::_1)));
		}
	});
	return UI::EVENT_DONE;
}

#elif defined(_WIN32) && !PPSSPP_PLATFORM(UWP)

UI::EventReturn GameSettingsScreen::OnSavePathMydoc(UI::EventParams &e) {
	const std::string PPSSPPpath = PFile::GetExeDirectory();
	const std::string installedFile = PPSSPPpath + "installed.txt";
	installed_ = PFile::Exists(installedFile);
	if (otherinstalled_) {
		PFile::Delete(PPSSPPpath + "installed.txt");
		PFile::CreateEmptyFile(PPSSPPpath + "installed.txt");
		otherinstalled_ = false;
		const std::string myDocsPath = W32Util::UserDocumentsPath() + "/PPSSPP/";
		g_PConfig.memStickDirectory = myDocsPath;
	} else if (installed_) {
		PFile::Delete(PPSSPPpath + "installed.txt");
		installed_ = false;
		g_PConfig.memStickDirectory = PPSSPPpath + "memstick/";
	} else {
		std::ofstream myfile;
		myfile.open(PPSSPPpath + "installed.txt");
		if (myfile.is_open()){
			myfile.close();
		}

		const std::string myDocsPath = W32Util::UserDocumentsPath() + "/PPSSPP/";
		g_PConfig.memStickDirectory = myDocsPath;
		installed_ = true;
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSavePathOther(UI::EventParams &e) {
	const std::string PPSSPPpath = PFile::GetExeDirectory();
	if (otherinstalled_) {
		auto di = GetI18NCategory("Dialog");
		std::string folder = W32Util::BrowseForFolder(MainWindow::GetHWND(), di->T("Choose PPSSPP save folder"));
		if (folder.size()) {
			g_PConfig.memStickDirectory = folder;
			FILE *f = PFile::OpenCFile(PPSSPPpath + "installed.txt", "wb");
			if (f) {
				std::string utfstring("\xEF\xBB\xBF");
				utfstring.append(folder);
				fwrite(utfstring.c_str(), 1, utfstring.length(), f);
				fclose(f);
			}
			installed_ = false;
		}
		else
			otherinstalled_ = false;
	}
	else {
		PFile::Delete(PPSSPPpath + "installed.txt");
		SavePathInMyDocumentChoice->SetEnabled(true);
		otherinstalled_ = false;
		installed_ = false;
		g_PConfig.memStickDirectory = PPSSPPpath + "memstick/";
	}
	return UI::EVENT_DONE;
}

#endif

UI::EventReturn GameSettingsScreen::OnChangeBackground(UI::EventParams &e) {
	const std::string bgPng = GetSysDirectory(DIRECTORY_SYSTEM) + "background.png";
	const std::string bgJpg = GetSysDirectory(DIRECTORY_SYSTEM) + "background.jpg";
	if (PFile::Exists(bgPng) || PFile::Exists(bgJpg)) {
		if (PFile::Exists(bgPng)) {
			PFile::Delete(bgPng);
		}
		if (PFile::Exists(bgJpg)) {
			PFile::Delete(bgJpg);
		}

		NativeMessageReceived("bgImage_updated", "");
	} else {
		if (System_GetPropertyBool(SYSPROP_HAS_IMAGE_BROWSER)) {
			System_SendMessage("bgImage_browse", "");
		}
	}

	// Change to a browse or clear button.
	RecreateViews();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnFullscreenChange(UI::EventParams &e) {
	System_SendMessage("toggle_fullscreen", g_PConfig.bFullScreen ? "1" : "0");
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnDisplayLayoutEditor(UI::EventParams &e) {
	screenManager()->push(new DisplayLayoutScreen());
	return UI::EVENT_DONE;
};

UI::EventReturn GameSettingsScreen::OnResolutionChange(UI::EventParams &e) {
	if (g_PConfig.iAndroidHwScale == 1) {
		RecreateActivity();
	}
	Reporting::UpdateConfig();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnHwScaleChange(UI::EventParams &e) {
	RecreateActivity();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnDumpNextFrameToLog(UI::EventParams &e) {
	if (gpu) {
		gpu->DumpNextFrame();
	}
	return UI::EVENT_DONE;
}

void GameSettingsScreen::update() {
	UIScreen::update();

	g_PConfig.iFpsLimit1 = iAlternateSpeedPercent1_ < 0 ? -1 : (iAlternateSpeedPercent1_ * 60) / 100;
	g_PConfig.iFpsLimit2 = iAlternateSpeedPercent2_ < 0 ? -1 : (iAlternateSpeedPercent2_ * 60) / 100;

	bool vertical = UseVerticalLayout();
	if (vertical != lastVertical_) {
		RecreateViews();
		lastVertical_ = vertical;
	}
	if (g_ShaderNameListChanged) {
		g_ShaderNameListChanged = false;
		g_PConfig.bShaderChainRequires60FPS = PostShaderChainRequires60FPS(GetFullPostShadersChain(g_PConfig.vPostShaderNames));
		RecreateViews();
	}
}

void GameSettingsScreen::onFinish(DialogResult result) {
	if (g_PConfig.bEnableSound) {
		if (PSP_IsInited() && !IsAudioInitialised())
			Audio_Init();
	}

	Reporting::Enable(enableReports_, "report.ppsspp.org");
	Reporting::UpdateConfig();
	g_PConfig.Save("GameSettingsScreen::onFinish");
	if (editThenRestore_) {
		// In case we didn't have the title yet before, try again.
		std::shared_ptr<GameInfo> info = g_gameInfoCache->GetInfo(nullptr, gamePath_, 0);
		g_PConfig.changeGameSpecific(gameID_, info->GetTitle());
		g_PConfig.unloadGameConfig();
	}

	host->UpdateUI();

	KeyMap::UpdateNativeMenuKeys();

	// Wipe some caches after potentially changing settings.
	NativeMessageReceived("gpu_resized", "");
	NativeMessageReceived("gpu_clearCache", "");
}

#if PPSSPP_PLATFORM(ANDROID)
void GameSettingsScreen::CallbackMemstickFolder(bool yes) {
	auto sy = GetI18NCategory("System");

	if (yes) {
		std::string memstickDirFile = g_PConfig.internalDataDirectory + "/memstick_dir.txt";
		std::string testWriteFile = pendingMemstickFolder_ + "/.write_verify_file";

		// Already, create away.
		if (!PFile::Exists(pendingMemstickFolder_)) {
			PFile::CreateFullPath(pendingMemstickFolder_);
		}
		if (!writeDataToFile(true, "1", 1, testWriteFile.c_str())) {
			settingInfo_->Show(sy->T("ChangingMemstickPathInvalid", "That path couldn't be used to save Memory Stick files."), nullptr);
			return;
		}
		PFile::Delete(testWriteFile);

		writeDataToFile(true, pendingMemstickFolder_.c_str(), pendingMemstickFolder_.size(), memstickDirFile.c_str());
		// Save so the settings, at least, are transferred.
		g_PConfig.memStickDirectory = pendingMemstickFolder_ + "/";
		g_PConfig.Save("MemstickPathChanged");
		screenManager()->RecreateAllViews();
	}
}
#endif

void GameSettingsScreen::TriggerRestart(const char *why) {
	// Extra save here to make sure the choice really gets saved even if there are shutdown bugs in
	// the GPU backend code.
	g_PConfig.Save(why);
	std::string param = "--gamesettings";
	if (editThenRestore_) {
		// We won't pass the gameID, so don't resume back into settings.
		param = "";
	} else if (!gamePath_.empty()) {
		param += " \"" + ReplaceAll(ReplaceAll(gamePath_, "\\", "\\\\"), "\"", "\\\"") + "\"";
	}
	// Make sure the new instance is considered the first.
	ShutdownInstanceCounter();
	System_SendMessage("graphics_restart", param.c_str());
}

void GameSettingsScreen::CallbackRenderingBackend(bool yes) {
	// If the user ends up deciding not to restart, set the config back to the current backend
	// so it doesn't get switched by accident.
	if (yes) {
		TriggerRestart("GameSettingsScreen::RenderingBackendYes");
	} else {
		g_PConfig.iGPUBackend = (int)GetGPUBackend();
	}
}

void GameSettingsScreen::CallbackRenderingDevice(bool yes) {
	// If the user ends up deciding not to restart, set the config back to the current backend
	// so it doesn't get switched by accident.
	if (yes) {
		TriggerRestart("GameSettingsScreen::RenderingDeviceYes");
	} else {
		std::string *deviceNameSetting = GPUDeviceNameSetting();
		if (deviceNameSetting)
			*deviceNameSetting = GetGPUBackendDevice();
		// Needed to redraw the setting.
		RecreateViews();
	}
}

void GameSettingsScreen::CallbackInflightFrames(bool yes) {
	if (yes) {
		TriggerRestart("GameSettingsScreen::InflightFramesYes");
	} else {
		g_PConfig.iInflightFrames = prevInflightFrames_;
	}
}

UI::EventReturn GameSettingsScreen::OnRenderingBackend(UI::EventParams &e) {
	auto di = GetI18NCategory("Dialog");

	// It only makes sense to show the restart prompt if the backend was actually changed.
	if (g_PConfig.iGPUBackend != (int)GetGPUBackend()) {
		screenManager()->push(new PromptScreen(di->T("ChangingGPUBackends", "Changing GPU backends requires PPSSPP to restart. Restart now?"), di->T("Yes"), di->T("No"),
			std::bind(&GameSettingsScreen::CallbackRenderingBackend, this, std::placeholders::_1)));
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnRenderingDevice(UI::EventParams &e) {
	auto di = GetI18NCategory("Dialog");

	// It only makes sense to show the restart prompt if the device was actually changed.
	std::string *deviceNameSetting = GPUDeviceNameSetting();
	if (deviceNameSetting && *deviceNameSetting != GetGPUBackendDevice()) {
		screenManager()->push(new PromptScreen(di->T("ChangingGPUBackends", "Changing GPU backends requires PPSSPP to restart. Restart now?"), di->T("Yes"), di->T("No"),
			std::bind(&GameSettingsScreen::CallbackRenderingDevice, this, std::placeholders::_1)));
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnInflightFramesChoice(UI::EventParams &e) {
	auto di = GetI18NCategory("Dialog");
	if (g_PConfig.iInflightFrames != prevInflightFrames_) {
		screenManager()->push(new PromptScreen(di->T("ChangingInflightFrames", "Changing graphics command buffering requires PPSSPP to restart. Restart now?"), di->T("Yes"), di->T("No"),
			std::bind(&GameSettingsScreen::CallbackInflightFrames, this, std::placeholders::_1)));
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnCameraDeviceChange(UI::EventParams& e) {
	Camera::onCameraDeviceChange();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnMicDeviceChange(UI::EventParams& e) {
	Microphone::onMicDeviceChange();
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnAudioDevice(UI::EventParams &e) {
	auto a = GetI18NCategory("Audio");
	if (g_PConfig.sAudioDevice == a->T("Auto")) {
		g_PConfig.sAudioDevice.clear();
	}
	System_SendMessage("audio_resetDevice", "");
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeQuickChat0(UI::EventParams &e) {
#if PPSSPP_PLATFORM(WINDOWS) || defined(USING_QT_UI) || defined(__ANDROID__)
	auto n = GetI18NCategory("Networking");
	System_InputBoxGetString(n->T("Enter Quick Chat 1"), g_PConfig.sQuickChat0, [](bool result, const std::string &value) {
		if (result) {
			g_PConfig.sQuickChat0 = value;
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeQuickChat1(UI::EventParams &e) {
#if PPSSPP_PLATFORM(WINDOWS) || defined(USING_QT_UI) || defined(__ANDROID__)
	auto n = GetI18NCategory("Networking");
	System_InputBoxGetString(n->T("Enter Quick Chat 2"), g_PConfig.sQuickChat1, [](bool result, const std::string &value) {
		if (result) {
			g_PConfig.sQuickChat1 = value;
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeQuickChat2(UI::EventParams &e) {
#if PPSSPP_PLATFORM(WINDOWS) || defined(USING_QT_UI) || defined(__ANDROID__)
	auto n = GetI18NCategory("Networking");
	System_InputBoxGetString(n->T("Enter Quick Chat 3"), g_PConfig.sQuickChat2, [](bool result, const std::string &value) {
		if (result) {
			g_PConfig.sQuickChat2 = value;
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeQuickChat3(UI::EventParams &e) {
#if PPSSPP_PLATFORM(WINDOWS) || defined(USING_QT_UI) || defined(__ANDROID__)
	auto n = GetI18NCategory("Networking");
	System_InputBoxGetString(n->T("Enter Quick Chat 4"), g_PConfig.sQuickChat3, [](bool result, const std::string &value) {
		if (result) {
			g_PConfig.sQuickChat3 = value;
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeQuickChat4(UI::EventParams &e) {
#if PPSSPP_PLATFORM(WINDOWS) || defined(USING_QT_UI) || defined(__ANDROID__)
	auto n = GetI18NCategory("Networking");
	System_InputBoxGetString(n->T("Enter Quick Chat 5"), g_PConfig.sQuickChat4, [](bool result, const std::string &value) {
		if (result) {
			g_PConfig.sQuickChat4 = value;
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeNickname(UI::EventParams &e) {
#if PPSSPP_PLATFORM(WINDOWS) || defined(USING_QT_UI) || defined(__ANDROID__)
	auto n = GetI18NCategory("Networking");
	System_InputBoxGetString(n->T("Enter a new PSP nickname"), g_PConfig.sNickName, [](bool result, const std::string &value) {
		if (result) {
			g_PConfig.sNickName = StripSpaces(value);
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeproAdhocServerAddress(UI::EventParams &e) {
	auto n = GetI18NCategory("Networking");

	screenManager()->push(new HostnameSelectScreen(&g_PConfig.proAdhocServer, n->T("proAdhocServer Address:")));

	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnChangeMacAddress(UI::EventParams &e) {
	g_PConfig.sMACAddress = CreateRandMAC();

	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnComboKey(UI::EventParams &e) {
	screenManager()->push(new ComboKeyScreen(&g_PConfig.iComboMode));
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnLanguage(UI::EventParams &e) {
	auto dev = GetI18NCategory("Developer");
	auto langScreen = new NewLanguageScreen(dev->T("Language"));
	langScreen->OnChoice.Handle(this, &GameSettingsScreen::OnLanguageChange);
	if (e.v)
		langScreen->SetPopupOrigin(e.v);
	screenManager()->push(langScreen);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnLanguageChange(UI::EventParams &e) {
	screenManager()->RecreateAllViews();

	if (host) {
		host->UpdateUI();
	}
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnPostProcShaderChange(UI::EventParams &e) {
	g_PConfig.vPostShaderNames.erase(std::remove(g_PConfig.vPostShaderNames.begin(), g_PConfig.vPostShaderNames.end(), "Off"), g_PConfig.vPostShaderNames.end());
	g_PConfig.vPostShaderNames.push_back("Off");
	g_ShaderNameListChanged = true;
	NativeMessageReceived("gpu_resized", "");
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnTextureShader(UI::EventParams &e) {
	auto gr = GetI18NCategory("Graphics");
	auto shaderScreen = new TextureShaderScreen(gr->T("Texture Shader"));
	shaderScreen->OnChoice.Handle(this, &GameSettingsScreen::OnTextureShaderChange);
	if (e.v)
		shaderScreen->SetPopupOrigin(e.v);
	screenManager()->push(shaderScreen);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnTextureShaderChange(UI::EventParams &e) {
	NativeMessageReceived("gpu_resized", "");
	RecreateViews(); // Update setting name
	g_PConfig.bTexHardwareScaling = g_PConfig.sTextureShaderName != "Off";
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnDeveloperTools(UI::EventParams &e) {
	screenManager()->push(new DeveloperToolsScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnRemoteISO(UI::EventParams &e) {
	screenManager()->push(new RemoteISOScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnControlMapping(UI::EventParams &e) {
	screenManager()->push(new ControlMappingScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnTouchControlLayout(UI::EventParams &e) {
	screenManager()->push(new TouchControlLayoutScreen());
	return UI::EVENT_DONE;
}

//when the tilt event type is modified, we need to reset all tilt settings.
//refer to the ResetTiltEvents() function for a detailed explanation.
UI::EventReturn GameSettingsScreen::OnTiltTypeChange(UI::EventParams &e) {
	TiltEventProcessor::ResetTiltEvents();
	return UI::EVENT_DONE;
};

UI::EventReturn GameSettingsScreen::OnTiltCustomize(UI::EventParams &e) {
	screenManager()->push(new TiltAnalogSettingsScreen());
	return UI::EVENT_DONE;
};

UI::EventReturn GameSettingsScreen::OnSavedataManager(UI::EventParams &e) {
	auto saveData = new SavedataScreen("");
	screenManager()->push(saveData);
	return UI::EVENT_DONE;
}

UI::EventReturn GameSettingsScreen::OnSysInfo(UI::EventParams &e) {
	screenManager()->push(new SystemInfoScreen());
	return UI::EVENT_DONE;
}

void DeveloperToolsScreen::CreateViews() {
	using namespace UI;
	root_ = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
	ScrollView *settingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(1.0f));
	settingsScroll->SetTag("DevToolsSettings");
	root_->Add(settingsScroll);

	auto di = GetI18NCategory("Dialog");
	auto dev = GetI18NCategory("Developer");
	auto gr = GetI18NCategory("Graphics");
	auto a = GetI18NCategory("Audio");
	auto sy = GetI18NCategory("System");

	AddStandardBack(root_);

	LinearLayout *list = settingsScroll->Add(new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(1.0f)));
	list->SetSpacing(0);
	list->Add(new ItemHeader(sy->T("General")));

	bool canUseJit = true;
	// iOS can now use JIT on all modes, apparently.
	// The bool may come in handy for future non-jit platforms though (UWP XB1?)

	static const char *cpuCores[] = {"Interpreter", "Dynarec (JIT)", "IR Interpreter"};
	PopupMultiChoice *core = list->Add(new PopupMultiChoice(&g_PConfig.iCpuCore, gr->T("CPU Core"), cpuCores, 0, ARRAY_SIZE(cpuCores), sy->GetName(), screenManager()));
	core->OnChoice.Handle(this, &DeveloperToolsScreen::OnJitAffectingSetting);
	if (!canUseJit) {
		core->HideChoice(1);
	}

	list->Add(new Choice(dev->T("JIT debug tools")))->OnClick.Handle(this, &DeveloperToolsScreen::OnJitDebugTools);
	list->Add(new CheckBox(&g_PConfig.bShowDeveloperMenu, dev->T("Show Developer Menu")));
	list->Add(new CheckBox(&g_PConfig.bDumpDecryptedEboot, dev->T("Dump Decrypted Eboot", "Dump Decrypted EBOOT.BIN (If Encrypted) When Booting Game")));

/*#if !PPSSPP_PLATFORM(UWP)
	Choice *cpuTests = new Choice(dev->T("Run CPU Tests"));
	list->Add(cpuTests)->OnClick.Handle(this, &DeveloperToolsScreen::OnRunCPUTests);

	cpuTests->SetEnabled(TestsAvailable());
#endif*/
	// For now, we only implement GPU driver tests for Vulkan and OpenGL. This is simply
	// because the D3D drivers are generally solid enough to not need this type of investigation.
	if (g_PConfig.iGPUBackend == (int)GPUBackend::VULKAN || g_PConfig.iGPUBackend == (int)GPUBackend::OPENGL) {
		list->Add(new Choice(dev->T("GPU Driver Test")))->OnClick.Handle(this, &DeveloperToolsScreen::OnGPUDriverTest);
	}
	list->Add(new Choice(dev->T("Touchscreen Test")))->OnClick.Handle(this, &DeveloperToolsScreen::OnTouchscreenTest);

	allowDebugger_ = !WebServerStopped(WebServerFlags::DEBUGGER);
	canAllowDebugger_ = !WebServerStopping(WebServerFlags::DEBUGGER);
	CheckBox *allowDebugger = new CheckBox(&allowDebugger_, dev->T("Allow remote debugger"));
	list->Add(allowDebugger)->OnClick.Handle(this, &DeveloperToolsScreen::OnRemoteDebugger);
	allowDebugger->SetEnabledPtr(&canAllowDebugger_);

	list->Add(new CheckBox(&g_PConfig.bEnableLogging, dev->T("Enable Logging")))->OnClick.Handle(this, &DeveloperToolsScreen::OnLoggingChanged);
	list->Add(new CheckBox(&g_PConfig.bLogFrameDrops, dev->T("Log Dropped Frame Statistics")));
	list->Add(new Choice(dev->T("Logging Channels")))->OnClick.Handle(this, &DeveloperToolsScreen::OnLogConfig);
	list->Add(new ItemHeader(dev->T("Language")));
	list->Add(new Choice(dev->T("Load language ini")))->OnClick.Handle(this, &DeveloperToolsScreen::OnLoadLanguageIni);
	list->Add(new Choice(dev->T("Save language ini")))->OnClick.Handle(this, &DeveloperToolsScreen::OnSaveLanguageIni);
	list->Add(new ItemHeader(dev->T("Texture Replacement")));
	list->Add(new CheckBox(&g_PConfig.bSaveNewTextures, dev->T("Save new textures")));
	list->Add(new CheckBox(&g_PConfig.bReplaceTextures, dev->T("Replace textures")));

#if !defined(MOBILE_DEVICE)
	Choice *createTextureIni = list->Add(new Choice(dev->T("Create/Open textures.ini file for current game")));
	createTextureIni->OnClick.Handle(this, &DeveloperToolsScreen::OnOpenTexturesIniFile);
	if (!PSP_IsInited()) {
		createTextureIni->SetEnabled(false);
	}
#endif
}

void DeveloperToolsScreen::onFinish(DialogResult result) {
	g_PConfig.Save("DeveloperToolsScreen::onFinish");
}

void GameSettingsScreen::CallbackRestoreDefaults(bool yes) {
	if (yes)
		g_PConfig.RestoreDefaults();
	host->UpdateUI();
}

UI::EventReturn GameSettingsScreen::OnRestoreDefaultSettings(UI::EventParams &e) {
	auto dev = GetI18NCategory("Developer");
	auto di = GetI18NCategory("Dialog");
	if (g_PConfig.bGameSpecific)
	{
		screenManager()->push(
			new PromptScreen(dev->T("RestoreGameDefaultSettings", "Are you sure you want to restore the game-specific settings back to the ppsspp defaults?\n"), di->T("OK"), di->T("Cancel"),
			std::bind(&GameSettingsScreen::CallbackRestoreDefaults, this, std::placeholders::_1)));
	}
	else
	{
		screenManager()->push(
			new PromptScreen(dev->T("RestoreDefaultSettings", "Are you sure you want to restore all settings(except control mapping)\nback to their defaults?\nYou can't undo this.\nPlease restart PPSSPP after restoring settings."), di->T("OK"), di->T("Cancel"),
			std::bind(&GameSettingsScreen::CallbackRestoreDefaults, this, std::placeholders::_1)));
	}

	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnLoggingChanged(UI::EventParams &e) {
	host->ToggleDebugConsoleVisibility();
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnRunCPUTests(UI::EventParams &e) {
/*#if !PPSSPP_PLATFORM(UWP)
	RunTests();
#endif*/
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnSaveLanguageIni(UI::EventParams &e) {
	i18nrepo.SaveIni(g_PConfig.sLanguageIni);
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnLoadLanguageIni(UI::EventParams &e) {
	i18nrepo.LoadIni(g_PConfig.sLanguageIni);
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnOpenTexturesIniFile(UI::EventParams &e) {
	std::string gameID = g_paramSFO.GetDiscID();
	std::string generatedFilename;
	if (TextureReplacer::GenerateIni(gameID, &generatedFilename)) {
		PFile::openIniFile(generatedFilename);
	}
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnLogConfig(UI::EventParams &e) {
	screenManager()->push(new LogConfigScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnJitDebugTools(UI::EventParams &e) {
	screenManager()->push(new JitDebugScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnGPUDriverTest(UI::EventParams &e) {
	screenManager()->push(new GPUDriverTestScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnTouchscreenTest(UI::EventParams &e) {
	screenManager()->push(new TouchTestScreen());
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnJitAffectingSetting(UI::EventParams &e) {
	NativeMessageReceived("clear jit", "");
	return UI::EVENT_DONE;
}

UI::EventReturn DeveloperToolsScreen::OnRemoteDebugger(UI::EventParams &e) {
	if (allowDebugger_) {
		StartWebServer(WebServerFlags::DEBUGGER);
	} else {
		StopWebServer(WebServerFlags::DEBUGGER);
	}
	// Persist the setting.  Maybe should separate?
	g_PConfig.bRemoteDebuggerOnStartup = allowDebugger_;
	return UI::EVENT_CONTINUE;
}

void DeveloperToolsScreen::update() {
	UIDialogScreenWithBackground::update();
	allowDebugger_ = !WebServerStopped(WebServerFlags::DEBUGGER);
	canAllowDebugger_ = !WebServerStopping(WebServerFlags::DEBUGGER);
}

void HostnameSelectScreen::CreatePopupContents(UI::ViewGroup *parent) {
	using namespace UI;
	auto sy = GetI18NCategory("System");
	auto di = GetI18NCategory("Dialog");
	auto n = GetI18NCategory("Networking");

	LinearLayout *valueRow = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT, Margins(0, 0, 0, 10)));

	addrView_ = new TextEdit(*value_, "");
	addrView_->SetTextAlign(FLAG_DYNAMIC_ASCII);
	valueRow->Add(addrView_);
	parent->Add(valueRow);

	LinearLayout *buttonsRow1 = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	LinearLayout *buttonsRow2 = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	parent->Add(buttonsRow1);
	parent->Add(buttonsRow2);

	buttonsRow1->Add(new Spacer(new LinearLayoutParams(1.0, G_LEFT)));
	for (char c = '0'; c <= '9'; ++c) {
		char label[] = { c, '\0' };
		auto button = buttonsRow1->Add(new Button(label));
		button->OnClick.Handle(this, &HostnameSelectScreen::OnNumberClick);
		button->SetTag(label);
	}
	buttonsRow1->Add(new Button("."))->OnClick.Handle(this, &HostnameSelectScreen::OnPointClick);
	buttonsRow1->Add(new Spacer(new LinearLayoutParams(1.0, G_RIGHT)));

	buttonsRow2->Add(new Spacer(new LinearLayoutParams(1.0, G_LEFT)));
#if defined(__ANDROID__)
	buttonsRow2->Add(new Button(di->T("Edit")))->OnClick.Handle(this, &HostnameSelectScreen::OnEditClick); // Since we don't have OnClick Event triggered from TextEdit's Touch().. here we go!
#endif
	buttonsRow2->Add(new Button(di->T("Delete")))->OnClick.Handle(this, &HostnameSelectScreen::OnDeleteClick);
	buttonsRow2->Add(new Button(di->T("Delete all")))->OnClick.Handle(this, &HostnameSelectScreen::OnDeleteAllClick);
	buttonsRow2->Add(new Button(di->T("Toggle List")))->OnClick.Handle(this, &HostnameSelectScreen::OnShowIPListClick);
	buttonsRow2->Add(new Spacer(new LinearLayoutParams(1.0, G_RIGHT)));

	std::vector<std::string> listIP = {"myneighborsushicat.com", "socom.cc", "localhost"}; // TODO: Add some saved recent history too?
	net::GetIPList(listIP);
	ipRows_ = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(1.0));
	ScrollView* scrollView = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	LinearLayout* innerView = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	if (listIP.size() > 0) {
		for (const auto& label : listIP) {
			// Filter out IP prefixed with "127." and "169.254." also "0." since they can be rendundant or unusable
			if (label.find("127.") != 0 && label.find("169.254.") != 0 && label.find("0.") != 0) {
				auto button = innerView->Add(new Button(label, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT)));
				button->OnClick.Handle(this, &HostnameSelectScreen::OnIPClick);
				button->SetTag(label);
			}
		}
	}
	scrollView->Add(innerView);
	ipRows_->Add(scrollView);
	ipRows_->SetVisibility(V_GONE);
	parent->Add(ipRows_);
	listIP.clear(); listIP.shrink_to_fit();

	errorView_ = parent->Add(new TextView(n->T("Invalid IP or hostname"), ALIGN_HCENTER, false, new LinearLayoutParams(Margins(0, 10, 0, 0))));
	errorView_->SetTextColor(0xFF3030FF);
	errorView_->SetVisibility(V_GONE);

	progressView_ = parent->Add(new TextView(n->T("Validating address..."), ALIGN_HCENTER, false, new LinearLayoutParams(Margins(0, 10, 0, 0))));
	progressView_->SetVisibility(V_GONE);
}

void HostnameSelectScreen::SendEditKey(int keyCode, int flags) {
	auto oldView = UI::GetFocusedView();
	UI::SetFocusedView(addrView_);
	KeyInput fakeKey{ DEVICE_ID_KEYBOARD, keyCode, KEY_DOWN | flags };
	addrView_->Key(fakeKey);
	UI::SetFocusedView(oldView);
}

UI::EventReturn HostnameSelectScreen::OnNumberClick(UI::EventParams &e) {
	std::string text = e.v ? e.v->Tag() : "";
	if (text.length() == 1 && text[0] >= '0' && text[0] <= '9') {
		SendEditKey(text[0], KEY_CHAR);
	}
	return UI::EVENT_DONE;
}

UI::EventReturn HostnameSelectScreen::OnPointClick(UI::EventParams &e) {
	SendEditKey('.', KEY_CHAR);
	return UI::EVENT_DONE;
}

UI::EventReturn HostnameSelectScreen::OnDeleteClick(UI::EventParams &e) {
	SendEditKey(NKCODE_DEL);
	return UI::EVENT_DONE;
}

UI::EventReturn HostnameSelectScreen::OnDeleteAllClick(UI::EventParams &e) {
	addrView_->SetText("");
	return UI::EVENT_DONE;
}

UI::EventReturn HostnameSelectScreen::OnEditClick(UI::EventParams& e) {
	auto n = GetI18NCategory("Networking");
#if defined(__ANDROID__)
	System_InputBoxGetString(n->T("proAdhocServer Address:"), addrView_->GetText(), [this](bool result, const std::string& value) {
		if (result) {
		    addrView_->SetText(value);
		}
	});
#endif
	return UI::EVENT_DONE;
}

UI::EventReturn HostnameSelectScreen::OnShowIPListClick(UI::EventParams& e) {
	if (ipRows_->GetVisibility() == UI::V_GONE) {
		ipRows_->SetVisibility(UI::V_VISIBLE);
	}
	else {
		ipRows_->SetVisibility(UI::V_GONE);
	}
	return UI::EVENT_DONE;
}

UI::EventReturn HostnameSelectScreen::OnIPClick(UI::EventParams& e) {
	std::string text = e.v ? e.v->Tag() : "";
	if (text.length() > 0) {
		addrView_->SetText(text);
		// TODO: Copy the IP to clipboard for the host to easily share their IP through chatting apps.
		System_SendMessage("setclipboardtext", text.c_str()); // Doesn't seems to be working on windows (yet?)
	}
	return UI::EVENT_DONE;
}

void HostnameSelectScreen::ResolverThread() {
	std::unique_lock<std::mutex> guard(resolverLock_);

	while (resolverState_ != ResolverState::QUIT) {
		resolverCond_.wait(guard);

		if (resolverState_ == ResolverState::QUEUED) {
			resolverState_ = ResolverState::PROGRESS;

			addrinfo *resolved = nullptr;
			std::string err;
			toResolveResult_ = net::DNSResolve(toResolve_, "80", &resolved, err);
			if (resolved)
				net::DNSResolveFree(resolved);

			resolverState_ = ResolverState::READY;
		}
	}
}

bool HostnameSelectScreen::CanComplete(DialogResult result) {
	if (result != DR_OK)
		return true;

	std::string value = addrView_->GetText();
	if (lastResolved_ == value) {
		return true;
	}

	// Currently running.
	if (resolverState_ == ResolverState::PROGRESS)
		return false;

	std::lock_guard<std::mutex> guard(resolverLock_);
	switch (resolverState_) {
	case ResolverState::PROGRESS:
	case ResolverState::QUIT:
		return false;

	case ResolverState::QUEUED:
	case ResolverState::WAITING:
		break;

	case ResolverState::READY:
		if (toResolve_ == value) {
			// Reset the state, nothing there now.
			resolverState_ = ResolverState::WAITING;
			toResolve_.clear();
			lastResolved_ = value;
			lastResolvedResult_ = toResolveResult_;

			if (lastResolvedResult_) {
				errorView_->SetVisibility(UI::V_GONE);
			} else {
				errorView_->SetVisibility(UI::V_VISIBLE);
			}
			progressView_->SetVisibility(UI::V_GONE);

			return true;
		}

		// Throw away that last result, it was for a different value.
		break;
	}

	resolverState_ = ResolverState::QUEUED;
	toResolve_ = value;
	resolverCond_.notify_one();

	progressView_->SetVisibility(UI::V_VISIBLE);
	errorView_->SetVisibility(UI::V_GONE);

	return false;
}

void HostnameSelectScreen::OnCompleted(DialogResult result) {
	if (result == DR_OK)
		*value_ = StripSpaces(addrView_->GetText());
}

SettingInfoMessage::SettingInfoMessage(int align, UI::AnchorLayoutParams *lp)
	: UI::LinearLayout(UI::ORIENT_HORIZONTAL, lp) {
	using namespace UI;
	SetSpacing(0.0f);
	Add(new UI::Spacer(10.0f));
	text_ = Add(new UI::TextView("", align, false, new LinearLayoutParams(1.0, Margins(0, 10))));
	Add(new UI::Spacer(10.0f));
}

void SettingInfoMessage::Show(const std::string &text, UI::View *refView) {
	if (refView) {
		Bounds b = refView->GetBounds();
		const UI::AnchorLayoutParams *lp = GetLayoutParams()->As<UI::AnchorLayoutParams>();
		if (b.y >= cutOffY_) {
			ReplaceLayoutParams(new UI::AnchorLayoutParams(lp->width, lp->height, lp->left, 80.0f, lp->right, lp->bottom, lp->center));
		} else {
			ReplaceLayoutParams(new UI::AnchorLayoutParams(lp->width, lp->height, lp->left, dp_yres - 80.0f - 40.0f, lp->right, lp->bottom, lp->center));
		}
	}
	text_->SetText(text);
	timeShown_ = time_now_d();
}

void SettingInfoMessage::Draw(UIContext &dc) {
	static const double FADE_TIME = 1.0;
	static const float MAX_ALPHA = 0.9f;

	// Let's show longer messages for more time (guesstimate at reading speed.)
	// Note: this will give multibyte characters more time, but they often have shorter words anyway.
	double timeToShow = std::max(1.5, text_->GetText().size() * 0.05);

	double sinceShow = time_now_d() - timeShown_;
	float alpha = MAX_ALPHA;
	if (timeShown_ == 0.0 || sinceShow > timeToShow + FADE_TIME) {
		alpha = 0.0f;
	} else if (sinceShow > timeToShow) {
		alpha = MAX_ALPHA - MAX_ALPHA * (float)((sinceShow - timeToShow) / FADE_TIME);
	}

	if (alpha >= 0.1f) {
		UI::Style style = dc.theme->popupTitle;
		style.background.color = colorAlpha(style.background.color, alpha - 0.1f);
		dc.FillRect(style.background, bounds_);
	}

	text_->SetTextColor(whiteAlpha(alpha));
	ViewGroup::Draw(dc);
}
