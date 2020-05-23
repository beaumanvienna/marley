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

#include <functional>

#include "ui/view.h"
#include "ui/ui_screen.h"
#include "ui/ui_context.h"
#include "UI/MiscScreens.h"

extern std::string activeCheatFile;
extern std::string gameTitle;

class CwCheatScreen : public UIDialogScreenWithBackground {
public:
	CwCheatScreen(std::string gamePath);
	CwCheatScreen() {}
	void CreateCodeList();
	void processFileOn(std::string activatedCheat);
	void processFileOff(std::string deactivatedCheat);
	const char * name;
	std::string activatedCheat, deactivatedCheat;
	PUI::EventReturn OnAddCheat(PUI::EventParams &params);
	PUI::EventReturn OnImportCheat(PUI::EventParams &params);
	PUI::EventReturn OnEditCheatFile(PUI::EventParams &params);
	PUI::EventReturn OnEnableAll(PUI::EventParams &params);

	void onFinish(DialogResult result) override;
protected:
	void CreateViews() override;

private:
	PUI::EventReturn OnCheckBox(PUI::EventParams &params);
	std::vector<std::string> formattedList_;
};

// TODO: Instead just hook the OnClick event on a regular checkbox.
class CheatCheckBox : public PUI::ClickableItem, public CwCheatScreen {
public:
	CheatCheckBox(bool *toggle, const std::string &text, const std::string &smallText = "", PUI::LayoutParams *layoutParams = 0)
		: PUI::ClickableItem(layoutParams), toggle_(toggle), text_(text) {
			OnClick.Handle(this, &CheatCheckBox::OnClicked);
	}

	virtual void Draw(UIContext &dc);

	PUI::EventReturn OnClicked(PUI::EventParams &e) {
		bool temp = false;
		if (toggle_) {
			*toggle_ = !(*toggle_);
			temp = *toggle_;
		}
		if (temp) {
			activatedCheat = text_;
			processFileOn(activatedCheat);
		} else {
			deactivatedCheat = text_;
			processFileOff(deactivatedCheat);
		}
		return PUI::EVENT_DONE;
	}

private:
	bool *toggle_;
	std::string text_;
	std::string smallText_;
};
