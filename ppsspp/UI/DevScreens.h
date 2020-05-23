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

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "file/file_util.h"
#include "i18n/i18n.h"
#include "ui/ui_screen.h"

#include "UI/MiscScreens.h"
#include "GPU/Common/ShaderCommon.h"

class DevMenu : public PopupScreen {
public:
	DevMenu(I18NCategory *i18n) : PopupScreen(i18n->T("Dev Tools")) {}

	void CreatePopupContents(PUI::ViewGroup *parent) override;
	void dialogFinished(const Screen *dialog, DialogResult result) override;

protected:
	PUI::EventReturn OnLogView(PUI::EventParams &e);
	PUI::EventReturn OnLogConfig(PUI::EventParams &e);
	PUI::EventReturn OnJitCompare(PUI::EventParams &e);
	PUI::EventReturn OnShaderView(PUI::EventParams &e);
	PUI::EventReturn OnFreezeFrame(PUI::EventParams &e);
	PUI::EventReturn OnDumpFrame(PUI::EventParams &e);
	PUI::EventReturn OnDeveloperTools(PUI::EventParams &e);
	PUI::EventReturn OnToggleAudioDebug(PUI::EventParams &e);
};

class JitDebugScreen : public UIDialogScreenWithBackground {
public:
	JitDebugScreen() {}
	virtual void CreateViews() override;

private:
	PUI::EventReturn OnEnableAll(PUI::EventParams &e);
	PUI::EventReturn OnDisableAll(PUI::EventParams &e);
};

class LogConfigScreen : public UIDialogScreenWithBackground {
public:
	LogConfigScreen() {}
	virtual void CreateViews() override;

private:
	PUI::EventReturn OnToggleAll(PUI::EventParams &e);
	PUI::EventReturn OnEnableAll(PUI::EventParams &e);
	PUI::EventReturn OnDisableAll(PUI::EventParams &e);
	PUI::EventReturn OnLogLevel(PUI::EventParams &e);
	PUI::EventReturn OnLogLevelChange(PUI::EventParams &e);
};

class LogScreen : public UIDialogScreenWithBackground {
public:
	LogScreen() : toBottom_(false) {}
	void CreateViews() override;
	void update() override;

private:
	void UpdateLog();
	PUI::EventReturn OnSubmit(PUI::EventParams &e);
	PUI::TextEdit *cmdLine_;
	PUI::LinearLayout *vert_;
	PUI::ScrollView *scroll_;
	bool toBottom_;
};

class LogLevelScreen : public ListPopupScreen {
public:
	LogLevelScreen(const std::string &title);

private:
	virtual void OnCompleted(DialogResult result);

};

class SystemInfoScreen : public UIDialogScreenWithBackground {
public:
	SystemInfoScreen() {}
	void CreateViews() override;
};

class AddressPromptScreen : public PopupScreen {
public:
	AddressPromptScreen(const std::string &title) : PopupScreen(title, "OK", "Cancel"), addrView_(NULL), addr_(0) {
		memset(buttons_, 0, sizeof(buttons_));
	}

	virtual bool key(const KeyInput &key) override;

	PUI::Event OnChoice;

protected:
	virtual void CreatePopupContents(PUI::ViewGroup *parent) override;
	virtual void OnCompleted(DialogResult result) override;
	PUI::EventReturn OnDigitButton(PUI::EventParams &e);
	PUI::EventReturn OnBackspace(PUI::EventParams &e);

private:
	void AddDigit(int n);
	void BackspaceDigit();
	void UpdatePreviewDigits();

	PUI::TextView *addrView_;
	PUI::Button *buttons_[16];
	unsigned int addr_;
};

class JitCompareScreen : public UIDialogScreenWithBackground {
public:
	JitCompareScreen() : currentBlock_(-1) {}
	virtual void CreateViews() override;

private:
	void UpdateDisasm();
	PUI::EventReturn OnRandomBlock(PUI::EventParams &e);
	PUI::EventReturn OnRandomFPUBlock(PUI::EventParams &e);
	PUI::EventReturn OnRandomVFPUBlock(PUI::EventParams &e);
	void OnRandomBlock(int flag);

	PUI::EventReturn OnCurrentBlock(PUI::EventParams &e);
	PUI::EventReturn OnSelectBlock(PUI::EventParams &e);
	PUI::EventReturn OnPrevBlock(PUI::EventParams &e);
	PUI::EventReturn OnNextBlock(PUI::EventParams &e);
	PUI::EventReturn OnBlockAddress(PUI::EventParams &e);
	PUI::EventReturn OnAddressChange(PUI::EventParams &e);
	PUI::EventReturn OnShowStats(PUI::EventParams &e);

	int currentBlock_;

	PUI::TextView *blockName_;
	PUI::TextEdit *blockAddr_;
	PUI::TextView *blockStats_;

	PUI::LinearLayout *leftDisasm_;
	PUI::LinearLayout *rightDisasm_;
};

class ShaderListScreen : public UIDialogScreenWithBackground {
public:
	ShaderListScreen() {}
	void CreateViews() override;

private:
	int ListShaders(DebugShaderType shaderType, PUI::LinearLayout *view);

	PUI::EventReturn OnShaderClick(PUI::EventParams &e);

	PUI::TabHolder *tabs_;
};

class ShaderViewScreen : public UIDialogScreenWithBackground {
public:
	ShaderViewScreen(std::string id, DebugShaderType type)
		: id_(id), type_(type) {}

	void CreateViews() override;
private:
	std::string id_;
	DebugShaderType type_;
};

void DrawProfile(UIContext &ui);
