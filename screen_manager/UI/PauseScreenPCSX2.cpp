// Copyright (c) 2020- Marley Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

#include "PauseScreenPCSX2.h"

void GamePauseScreenPCSX2::update() {
	//SCREEN_UpdateUIState(UISTATE_PAUSEMENU);
	SCREEN_UIScreen::update();

	if (finishNextFrame_) {
		TriggerFinish(DR_CANCEL);
		finishNextFrame_ = false;
	}
}

GamePauseScreenPCSX2::~GamePauseScreenPCSX2() {

}

void GamePauseScreenPCSX2::CreateViews() {
	using namespace SCREEN_UI;
	Margins scrollMargins(0, 20, 0, 0);
	Margins actionMenuMargins(0, 20, 15, 0);

	root_ = new LinearLayout(ORIENT_HORIZONTAL);

	ViewGroup *rightColumn = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(300, FILL_PARENT, actionMenuMargins));
	root_->Add(rightColumn);

	LinearLayout *rightColumnItems = new LinearLayout(ORIENT_VERTICAL);
	rightColumn->Add(rightColumnItems);

	rightColumnItems->SetSpacing(0.0f);
	
	Choice *continueChoice = rightColumnItems->Add(new Choice("Continue"));
	root_->SetDefaultFocusView(continueChoice);

	rightColumnItems->Add(new Choice("Settings"))->OnClick.Handle(this, &GamePauseScreenPCSX2::OnGameSettings);
	rightColumnItems->Add(new Spacer(25.0));
	rightColumnItems->Add(new Choice("Exit to Marley"))->OnClick.Handle(this, &GamePauseScreenPCSX2::OnExitToMarley);
}

void GamePauseScreenPCSX2::dialogFinished(const SCREEN_Screen *dialog, DialogResult dr) {

}


SCREEN_UI::EventReturn GamePauseScreenPCSX2::OnGameSettings(SCREEN_UI::EventParams &e) {
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn GamePauseScreenPCSX2::OnExitToMarley(SCREEN_UI::EventParams &e) {
	return SCREEN_UI::EVENT_DONE;
}



