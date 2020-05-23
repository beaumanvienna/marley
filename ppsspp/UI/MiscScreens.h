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
#include "ui/ui_screen.h"

struct ShaderInfo;

extern std::string boot_filename;
void UIBackgroundInit(UIContext &dc);
void UIBackgroundShutdown();

inline void NoOpVoidBool(bool) {}

class UIScreenWithBackground : public UIScreen {
public:
	UIScreenWithBackground() : UIScreen() {}
protected:
	void DrawBackground(UIContext &dc) override;
	void sendMessage(const char *message, const char *value) override;
};

class UIScreenWithGameBackground : public UIScreenWithBackground {
public:
	UIScreenWithGameBackground(const std::string &gamePath)
		: UIScreenWithBackground(), gamePath_(gamePath) {}
	void DrawBackground(UIContext &dc) override;
	void sendMessage(const char *message, const char *value) override;
protected:
	std::string gamePath_;
};

class UIDialogScreenWithBackground : public UIDialogScreen {
public:
	UIDialogScreenWithBackground() : UIDialogScreen() {}
protected:
	void DrawBackground(UIContext &dc) override;
	void sendMessage(const char *message, const char *value) override;

	void AddStandardBack(PUI::ViewGroup *parent);
};

class UIDialogScreenWithGameBackground : public UIDialogScreenWithBackground {
public:
	UIDialogScreenWithGameBackground(const std::string &gamePath)
		: UIDialogScreenWithBackground(), gamePath_(gamePath) {}
	void DrawBackground(UIContext &dc) override;
	void sendMessage(const char *message, const char *value) override;
protected:
	std::string gamePath_;
};

class PromptScreen : public UIDialogScreenWithBackground {
public:
	PromptScreen(std::string message, std::string yesButtonText, std::string noButtonText,
		std::function<void(bool)> callback = &NoOpVoidBool);

	void CreateViews() override;

	void TriggerFinish(DialogResult result) override;

private:
	PUI::EventReturn OnYes(PUI::EventParams &e);
	PUI::EventReturn OnNo(PUI::EventParams &e);

	std::string message_;
	std::string yesButtonText_;
	std::string noButtonText_;
	std::function<void(bool)> callback_;
};

class NewLanguageScreen : public ListPopupScreen {
public:
	NewLanguageScreen(const std::string &title);

private:
	void OnCompleted(DialogResult result) override;
	bool ShowButtons() const override { return true; }
	std::map<std::string, std::pair<std::string, int>> langValuesMapping;
	std::map<std::string, std::string> titleCodeMapping;
	std::vector<FileInfo> langs_;
};

class PostProcScreen : public ListPopupScreen {
public:
	PostProcScreen(const std::string &title);

private:
	void OnCompleted(DialogResult result) override;
	bool ShowButtons() const override { return true; }
	std::vector<ShaderInfo> shaders_;
};

class LogoScreen : public UIScreen {
public:
	LogoScreen()
		: frames_(0), switched_(false) {}
	bool key(const KeyInput &key) override;
	bool touch(const TouchInput &touch) override;
	void update() override;
	void render() override;
	void sendMessage(const char *message, const char *value) override;
	void CreateViews() override {}

private:
	void Next();
	int frames_;
	bool switched_;
};

class CreditsScreen : public UIDialogScreenWithBackground {
public:
	CreditsScreen() : frames_(0) {}
	void update() override;
	void render() override;

	void CreateViews() override;

private:
	PUI::EventReturn OnOK(PUI::EventParams &e);

	PUI::EventReturn OnSupport(PUI::EventParams &e);
	PUI::EventReturn OnPPSSPPOrg(PUI::EventParams &e);
	PUI::EventReturn OnPrivacy(PUI::EventParams &e);
	PUI::EventReturn OnForums(PUI::EventParams &e);
	PUI::EventReturn OnDiscord(PUI::EventParams &e);
	PUI::EventReturn OnShare(PUI::EventParams &e);
	PUI::EventReturn OnTwitter(PUI::EventParams &e);

	int frames_;
};
