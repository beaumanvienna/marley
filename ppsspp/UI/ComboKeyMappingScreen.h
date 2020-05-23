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

#include "MiscScreens.h"

#include <map>

namespace PUI {
	class CheckBox;
}

class Combo_keyScreen : public UIDialogScreenWithBackground {
public:
	Combo_keyScreen(int *key): mode(key) {}

	void CreateViews() override;
	void onFinish(DialogResult result) override;
	PUI::EventReturn onCombo(PUI::EventParams &e);
private:
	std::map<std::string, bool*> keyToggles;
	bool array[16];
	int *mode;
	PUI::ChoiceStrip *comboselect;
	PUI::ScrollView *rightScroll_;
	class ChoiceEventHandler{
	public:
		ChoiceEventHandler(PUI::CheckBox *checkbox) : checkbox_(checkbox) {}
		PUI::EventReturn onChoiceClick(PUI::EventParams &e);
	private:
		PUI::CheckBox *checkbox_;
	};
};

