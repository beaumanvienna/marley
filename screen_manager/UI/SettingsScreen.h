// Copyright (c) 2013-2020 PPSSPP project
// Copyright (c) 2020 Marley project

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
    
    // general
    int inputSearchDirectories;
    
    // dolphin
    int inputResDolphin;
    bool inputVSyncDolphin;
    std::vector<std::string> GFX_entries;
    
    int inputBackend;
    int inputBios;
    int inputResPCSX2;
    bool inputVSyncPCSX2;
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

	bool lastVertical_;
	SCREEN_SettingInfoMessage *settingInfo_;

	// Event handlers
    
    SCREEN_UI::EventReturn OnRenderingBackend(SCREEN_UI::EventParams &e);
    SCREEN_UI::EventReturn OnDeleteSearchDirectories(SCREEN_UI::EventParams &e);
    

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
