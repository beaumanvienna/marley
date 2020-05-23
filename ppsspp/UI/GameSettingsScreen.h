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
#include "ui/ui_screen.h"
#include "UI/MiscScreens.h"

class SettingInfoMessage;

// Per-game settings screen - enables you to configure graphic options, control options, etc
// per game.
class GameSettingsScreen : public UIDialogScreenWithGameBackground {
public:
	GameSettingsScreen(std::string gamePath, std::string gameID = "", bool editThenRestore = false);

	void update() override;
	void onFinish(DialogResult result) override;
	void sendMessage(const char *message, const char *value) override;
	std::string tag() const override { return "settings"; }

	PUI::Event OnRecentChanged;

protected:
	void CreateViews() override;
	void CallbackRestoreDefaults(bool yes);
	void CallbackRenderingBackend(bool yes);
	void CallbackRenderingDevice(bool yes);
#if PPSSPP_PLATFORM(ANDROID)
	void CallbackMemstickFolder(bool yes);
#endif
	bool UseVerticalLayout() const;

private:
	std::string gameID_;
	bool lastVertical_;
	PUI::CheckBox *enableReportsCheckbox_;
	PUI::Choice *layoutEditorChoice_;
	PUI::Choice *postProcChoice_;
	PUI::Choice *displayEditor_;
	PUI::Choice *backgroundChoice_ = nullptr;
	PUI::PopupMultiChoice *resolutionChoice_;
	PUI::CheckBox *frameSkipAuto_;
	SettingInfoMessage *settingInfo_;
#ifdef _WIN32
	PUI::CheckBox *SavePathInMyDocumentChoice;
	PUI::CheckBox *SavePathInOtherChoice;
	// Used to enable/disable the above two options.
	bool installed_;
	bool otherinstalled_;
#endif

	// Event handlers
	PUI::EventReturn OnControlMapping(PUI::EventParams &e);
	PUI::EventReturn OnTouchControlLayout(PUI::EventParams &e);
	PUI::EventReturn OnDumpNextFrameToLog(PUI::EventParams &e);
	PUI::EventReturn OnTiltTypeChange(PUI::EventParams &e);
	PUI::EventReturn OnTiltCustomize(PUI::EventParams &e);
	PUI::EventReturn OnComboKey(PUI::EventParams &e);

	// Global settings handlers
	PUI::EventReturn OnLanguage(PUI::EventParams &e);
	PUI::EventReturn OnLanguageChange(PUI::EventParams &e);
	PUI::EventReturn OnAutoFrameskip(PUI::EventParams &e);
	PUI::EventReturn OnPostProcShader(PUI::EventParams &e);
	PUI::EventReturn OnPostProcShaderChange(PUI::EventParams &e);
	PUI::EventReturn OnDeveloperTools(PUI::EventParams &e);
	PUI::EventReturn OnRemoteISO(PUI::EventParams &e);
	PUI::EventReturn OnChangeNickname(PUI::EventParams &e);
	PUI::EventReturn OnChangeproAdhocServerAddress(PUI::EventParams &e);
	PUI::EventReturn OnChangeMacAddress(PUI::EventParams &e);
	PUI::EventReturn OnClearRecents(PUI::EventParams &e);
	PUI::EventReturn OnChangeBackground(PUI::EventParams &e);
	PUI::EventReturn OnFullscreenChange(PUI::EventParams &e);
	PUI::EventReturn OnDisplayLayoutEditor(PUI::EventParams &e);
	PUI::EventReturn OnResolutionChange(PUI::EventParams &e);
	PUI::EventReturn OnHwScaleChange(PUI::EventParams &e);
	PUI::EventReturn OnRestoreDefaultSettings(PUI::EventParams &e);
	PUI::EventReturn OnRenderingMode(PUI::EventParams &e);
	PUI::EventReturn OnRenderingBackend(PUI::EventParams &e);
	PUI::EventReturn OnRenderingDevice(PUI::EventParams &e);
	PUI::EventReturn OnJitAffectingSetting(PUI::EventParams &e);
#if PPSSPP_PLATFORM(ANDROID)
	PUI::EventReturn OnChangeMemStickDir(PUI::EventParams &e);
#elif defined(_WIN32) && !PPSSPP_PLATFORM(UWP)
	PUI::EventReturn OnSavePathMydoc(PUI::EventParams &e);
	PUI::EventReturn OnSavePathOther(PUI::EventParams &e);
#endif
	PUI::EventReturn OnSoftwareRendering(PUI::EventParams &e);
	PUI::EventReturn OnHardwareTransform(PUI::EventParams &e);

	PUI::EventReturn OnScreenRotation(PUI::EventParams &e);
	PUI::EventReturn OnImmersiveModeChange(PUI::EventParams &e);
	PUI::EventReturn OnSustainedPerformanceModeChange(PUI::EventParams &e);

	PUI::EventReturn OnAdhocGuides(PUI::EventParams &e);

	PUI::EventReturn OnSavedataManager(PUI::EventParams &e);
	PUI::EventReturn OnSysInfo(PUI::EventParams &e);

	// Temporaries to convert setting types.
	int iAlternateSpeedPercent1_;
	int iAlternateSpeedPercent2_;
	bool enableReports_;

	//edit the game-specific settings and restore the global settings after exiting
	bool editThenRestore_;

	// Cached booleans
	bool vtxCacheEnable_;
	bool postProcEnable_;
	bool resolutionEnable_;
	bool bloomHackEnable_;
	bool tessHWEnable_;

#if PPSSPP_PLATFORM(ANDROID)
	std::string pendingMemstickFolder_;
#endif
};

class SettingInfoMessage : public PUI::LinearLayout {
public:
	SettingInfoMessage(int align, PUI::AnchorLayoutParams *lp);

	void SetBottomCutoff(float y) {
		cutOffY_ = y;
	}
	void Show(const std::string &text, PUI::View *refView = nullptr);

	void Draw(UIContext &dc);

private:
	PUI::TextView *text_ = nullptr;
	double timeShown_ = 0.0;
	float cutOffY_;
};

class DeveloperToolsScreen : public UIDialogScreenWithBackground {
public:
	DeveloperToolsScreen() {}
	void update() override;
	void onFinish(DialogResult result) override;

protected:
	void CreateViews() override;

private:
	PUI::EventReturn OnRunCPUTests(PUI::EventParams &e);
	PUI::EventReturn OnLoggingChanged(PUI::EventParams &e);
	PUI::EventReturn OnLoadLanguageIni(PUI::EventParams &e);
	PUI::EventReturn OnSaveLanguageIni(PUI::EventParams &e);
	PUI::EventReturn OnOpenTexturesIniFile(PUI::EventParams &e);
	PUI::EventReturn OnLogConfig(PUI::EventParams &e);
	PUI::EventReturn OnJitAffectingSetting(PUI::EventParams &e);
	PUI::EventReturn OnJitDebugTools(PUI::EventParams &e);
	PUI::EventReturn OnRemoteDebugger(PUI::EventParams &e);
	PUI::EventReturn OnGPUDriverTest(PUI::EventParams &e);
	PUI::EventReturn OnTouchscreenTest(PUI::EventParams &e);

	bool allowDebugger_ = false;
	bool canAllowDebugger_ = true;
};

class HostnameSelectScreen : public PopupScreen {
public:
	HostnameSelectScreen(std::string *value, const std::string &title)
		: PopupScreen(title, "OK", "Cancel"), value_(value) {
		resolver_ = std::thread([](HostnameSelectScreen *thiz) {
			thiz->ResolverThread();
		}, this);
	}
	~HostnameSelectScreen() {
		resolverState_ = ResolverState::QUIT;
		resolverCond_.notify_one();
		resolver_.join();
	}

	void CreatePopupContents(PUI::ViewGroup *parent) override;

protected:
	void OnCompleted(DialogResult result) override;
	bool CanComplete(DialogResult result) override;

private:
	void ResolverThread();
	void SendEditKey(int keyCode, int flags = 0);
	PUI::EventReturn OnNumberClick(PUI::EventParams &e);
	PUI::EventReturn OnPointClick(PUI::EventParams &e);
	PUI::EventReturn OnDeleteClick(PUI::EventParams &e);
	PUI::EventReturn OnDeleteAllClick(PUI::EventParams &e);

	enum class ResolverState {
		WAITING,
		QUEUED,
		PROGRESS,
		READY,
		QUIT,
	};

	std::string *value_;
	PUI::TextEdit *addrView_ = nullptr;
	PUI::TextView *errorView_ = nullptr;
	PUI::TextView *progressView_ = nullptr;

	std::thread resolver_;
	ResolverState resolverState_ = ResolverState::WAITING;
	std::mutex resolverLock_;
	std::condition_variable resolverCond_;
	std::string toResolve_ = "";
	bool toResolveResult_ = false;
	std::string lastResolved_ = "";
	bool lastResolvedResult_ = false;
};
