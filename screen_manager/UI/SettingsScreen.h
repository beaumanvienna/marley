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

#include "ppsspp_config.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include "Common/UI/UIScreen.h"
#include "UI/MiscScreens.h"

class SCREEN_SettingInfoMessage;

// Per-game settings screen - enables you to configure graphic options, control options, etc
// per game.
class SCREEN_SettingsScreen : public SCREEN_UIDialogScreenWithBackground {
public:
    SCREEN_SettingsScreen();
    virtual ~SCREEN_SettingsScreen();
	void update() override;
	void onFinish(DialogResult result) override;
	std::string tag() const override { return "settings"; }

protected:
	void CreateViews() override;
	void CallbackRestoreDefaults(bool yes);
	void CallbackRenderingBackend(bool yes);
	void CallbackRenderingDevice(bool yes);
	void CallbackInflightFrames(bool yes);
	bool UseVerticalLayout() const;

private:

    int inputBackend;
    int inputBios;
    int inputRes;
    bool inputVSync;
    std::vector<std::string> GSdx_entries;
    std::vector<std::string> PCSX2_vm_entries;
    bool found_bios_ps2;
    int bios_selection[3];
    bool inputAdvancedSettings;
    int  inputInterlace;
    int  inputBiFilter;
    int  inputAnisotropy;
    int  inputDithering;
    int  inputHW_mipmapping;
    int  inputCRC_level;
    int  inputAcc_date_level;
    int  inputAcc_blend_level;
    bool inputUserHacks;
    bool inputUserHacks_AutoFlush;
    bool inputUserHacks_CPU_FB_Conversion;
    bool inputUserHacks_DisableDepthSupport;
    bool inputUserHacks_DisablePartialInvalidation;
    bool inputUserHacks_Disable_Safe_Features;
    bool inputUserHacks_HalfPixelOffset;
    int  inputUserHacks_Half_Bottom_Override;
    bool inputUserHacks_SkipDraw;
    bool inputUserHacks_SkipDraw_Offset;
    bool inputUserHacks_TCOffsetX;
    bool inputUserHacks_TCOffsetY;
    bool inputUserHacks_TextureInsideRt;
    int  inputUserHacks_TriFilter;
    bool inputUserHacks_WildHack;
    bool inputUserHacks_align_sprite_X;
    bool inputUserHacks_merge_pp_sprite;
    bool inputUserHacks_round_sprite_offset;
    bool inputAutoflush_sw;
    bool inputMipmapping_sw;
    int  inputExtrathreads_sw;
    bool inputAnti_aliasing_sw;
    bool inputInterpreter;


	void TriggerRestart(const char *why);

	std::string gameID_;
	bool lastVertical_;
	SCREEN_UI::CheckBox *enableReportsCheckbox_;
	SCREEN_UI::Choice *layoutEditorChoice_;
	SCREEN_UI::Choice *postProcChoice_;
	SCREEN_UI::Choice *displayEditor_;
	SCREEN_UI::Choice *backgroundChoice_ = nullptr;
	SCREEN_UI::SCREEN_PopupMultiChoice *resolutionChoice_;
	SCREEN_UI::CheckBox *frameSkipAuto_;
	SCREEN_SettingInfoMessage *settingInfo_;

	// Event handlers
	SCREEN_UI::EventReturn OnControlMapping(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnTouchControlLayout(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnDumpNextFrameToLog(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnTiltTypeChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnTiltCustomize(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnComboKey(SCREEN_UI::EventParams &e);

	// Global settings handlers
	SCREEN_UI::EventReturn OnLanguage(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnLanguageChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnAutoFrameskip(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnPostProcShaderChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnTextureShader(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnTextureShaderChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnDeveloperTools(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRemoteISO(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeQuickChat0(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeQuickChat1(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeQuickChat2(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeQuickChat3(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeQuickChat4(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeNickname(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeproAdhocServerAddress(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeMacAddress(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnChangeBackground(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnFullscreenChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnDisplayLayoutEditor(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnResolutionChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnHwScaleChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRestoreDefaultSettings(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRenderingMode(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRenderingBackend(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRenderingDevice(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnInflightFramesChoice(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnCameraDeviceChange(SCREEN_UI::EventParams& e);
	SCREEN_UI::EventReturn OnMicDeviceChange(SCREEN_UI::EventParams& e);
	SCREEN_UI::EventReturn OnAudioDevice(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnJitAffectingSetting(SCREEN_UI::EventParams &e);

	SCREEN_UI::EventReturn OnSoftwareRendering(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnHardwareTransform(SCREEN_UI::EventParams &e);

	SCREEN_UI::EventReturn OnScreenRotation(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnImmersiveModeChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnSustainedPerformanceModeChange(SCREEN_UI::EventParams &e);

	SCREEN_UI::EventReturn OnAdhocGuides(SCREEN_UI::EventParams &e);

	SCREEN_UI::EventReturn OnSavedataManager(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnSysInfo(SCREEN_UI::EventParams &e);

	// Temporaries to convert setting types, cache enabled, etc.
	int iAlternateSpeedPercent1_;
	int iAlternateSpeedPercent2_;
	int prevInflightFrames_;
	bool enableReports_;
	bool tessHWEnable_;
	std::string shaderNames_[256];

	//edit the game-specific settings and restore the global settings after exiting
	bool editThenRestore_;

};

class SCREEN_SettingInfoMessage : public SCREEN_UI::LinearLayout {
public:
	SCREEN_SettingInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp);

	void SetBottomCutoff(float y) {
		cutOffY_ = y;
	}
	void Show(const std::string &text, SCREEN_UI::View *refView = nullptr);

	void Draw(SCREEN_UIContext &dc);

private:
	SCREEN_UI::TextView *text_ = nullptr;
	double timeShown_ = 0.0;
	float cutOffY_;
};
