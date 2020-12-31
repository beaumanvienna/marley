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
#include "Common/File/PathBrowser.h"
#include "UI/MiscScreens.h"

class SCREEN_SettingsInfoMessage;
class SCREEN_DirBrowser;

// Per-game settings screen - enables you to configure graphic options, control options, etc
// per game.
class SCREEN_SettingsScreen : public SCREEN_UIDialogScreenWithBackground {
public:
    SCREEN_SettingsScreen();
    virtual ~SCREEN_SettingsScreen();
	void update() override;
	void onFinish(DialogResult result) override;
    bool key(const KeyInput &key) override;
	std::string tag() const override { return "settings"; }
    void DrawBackground(SCREEN_UIContext &dc) override;
    void showToolTip(std::string text);

protected:
	void CreateViews() override;
	void CallbackRestoreDefaults(bool yes);
	void CallbackRenderingBackend(bool yes);
	void CallbackRenderingDevice(bool yes);
	void CallbackInflightFrames(bool yes);
	bool UseVerticalLayout() const;
    
    // game browser
    SCREEN_DirBrowser *searchDirBrowser;
   	SCREEN_UI::EventReturn OnStartSetup1(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnStartSetup2(SCREEN_UI::EventParams &e);


private:
    SCREEN_UI::TabHolder *tabHolder = nullptr;
    // search
    int inputSearchDirectories;
    
    // controller
    SCREEN_UI::TextView* text_setup1;
    SCREEN_UI::TextView* text_setup1b;
    SCREEN_UI::TextView* text_setup2;
    SCREEN_UI::TextView* text_setup2b;
    
    // Dolphin
    int inputResDolphin;
    bool inputVSyncDolphin;
    std::vector<std::string> GFX_entries;
    std::vector<std::string> WiimoteNew_entries;
    bool inputEnable2ndWiimote;
    
    // PCSX2
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
    
    // General
    std::vector<std::string> marley_cfg_entries;

	bool lastVertical_;

	// Event handlers
    
    SCREEN_UI::EventReturn OnRenderingBackend(SCREEN_UI::EventParams &e);
    SCREEN_UI::EventReturn OnThemeChanged(SCREEN_UI::EventParams &e);
    SCREEN_UI::EventReturn OnDeleteSearchDirectories(SCREEN_UI::EventParams &e);
    

};

class SCREEN_SettingsInfoMessage : public SCREEN_UI::LinearLayout {
public:
	SCREEN_SettingsInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp);

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


enum class SCREEN_BrowseFlags {
	NONE = 0,
	NAVIGATE = 1,
	ARCHIVES = 2,
	PIN = 4,
	HOMEBREW_STORE = 8,
	STANDARD = 1 | 2 | 4,
};
ENUM_CLASS_BITOPS(SCREEN_BrowseFlags);

class SCREEN_DirBrowser : public SCREEN_UI::LinearLayout {
public:
	SCREEN_DirBrowser(std::string path, SCREEN_BrowseFlags browseFlags, bool *gridStyle, SCREEN_ScreenManager *screenManager, std::string lastText, std::string lastLink, SCREEN_UI::LayoutParams *layoutParams = nullptr);
    ~SCREEN_DirBrowser();
	SCREEN_UI::Event OnChoice;
	SCREEN_UI::Event OnHoldChoice;
	SCREEN_UI::Event OnHighlight;

	void FocusGame(const std::string &gamePath);
	void SetPath(const std::string &path);
    std::string GetPath();
	void Draw(SCREEN_UIContext &dc) override;
	void Update() override;

protected:

	void Refresh();

private:
	bool IsCurrentPathPinned();
	const std::vector<std::string> GetPinnedPaths();
	const std::string GetBaseName(const std::string &path);

	SCREEN_UI::EventReturn NavigateClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn LayoutChange(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn HomeClick(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnRecentClear(SCREEN_UI::EventParams &e);
	

	SCREEN_UI::ViewGroup *gameList_ = nullptr;
	SCREEN_PathBrowser path_;
	bool *gridStyle_ = nullptr;
	SCREEN_BrowseFlags browseFlags_;
	std::string lastText_;
	std::string lastLink_;
	std::string focusGamePath_;
	bool listingPending_ = false;
	float lastScale_ = 1.0f;
	bool lastLayoutWasGrid_ = true;
	SCREEN_ScreenManager *screenManager_;
};
