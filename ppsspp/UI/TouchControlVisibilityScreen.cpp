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

#include "TouchControlVisibilityScreen.h"
#include "Core/Config.h"
#include "UI/ui_atlas.h"
#include "i18n/i18n.h"

static const int leftColumnWidth = 140;

class CheckBoxChoice : public PUI::Choice {
public:
	CheckBoxChoice(const std::string &text, PUI::CheckBox *checkbox, PUI::LayoutParams *lp)
		: Choice(text, lp), checkbox_(checkbox) {
		OnClick.Handle(this, &CheckBoxChoice::HandleClick);
	}
	CheckBoxChoice(ImageID imgID, PUI::CheckBox *checkbox, PUI::LayoutParams *lp)
		: Choice(imgID, lp), checkbox_(checkbox) {
		OnClick.Handle(this, &CheckBoxChoice::HandleClick);
	}

private:
	PUI::EventReturn HandleClick(PUI::EventParams &e);

	PUI::CheckBox *checkbox_;
};

void TouchControlVisibilityScreen::CreateViews() {
	using namespace PUI;

	I18NCategory *di = GetI18NCategory("Dialog");
	I18NCategory *co = GetI18NCategory("Controls");

	root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));

	Choice *back = new Choice(di->T("Back"), "", false, new AnchorLayoutParams(leftColumnWidth - 10, WRAP_CONTENT, 10, NONE, NONE, 10));
	root_->Add(back)->OnClick.Handle<UIScreen>(this, &UIScreen::OnBack);
	Choice *toggleAll = new Choice(di->T("Toggle All"), "", false, new AnchorLayoutParams(leftColumnWidth - 10, WRAP_CONTENT, 10, NONE, NONE, 84));
	root_->Add(toggleAll)->OnClick.Handle(this, &TouchControlVisibilityScreen::OnToggleAll);

	TabHolder *tabHolder = new TabHolder(ORIENT_VERTICAL, leftColumnWidth, new AnchorLayoutParams(10, 0, 10, 0, false));
	tabHolder->SetTag("TouchControlVisibility");
	root_->Add(tabHolder);
	ScrollView *rightPanel = new ScrollView(ORIENT_VERTICAL);
	tabHolder->AddTab(co->T("Visibility"), rightPanel);

	LinearLayout *vert = rightPanel->Add(new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT)));
	vert->SetSpacing(0);

	vert->Add(new ItemHeader(co->T("Touch Control Visibility")));

	const int cellSize = 380;

	PUI::GridLayoutSettings gridsettings(cellSize, 64, 5);
	gridsettings.fillCells = true;
	GridLayout *grid = vert->Add(new GridLayout(gridsettings, new LayoutParams(FILL_PARENT, WRAP_CONTENT)));

	toggles_.clear();
	toggles_.push_back({ "Circle", &g_PConfig.bShowTouchCircle, I_CIRCLE });
	toggles_.push_back({ "Cross", &g_PConfig.bShowTouchCross, I_CROSS });
	toggles_.push_back({ "Square", &g_PConfig.bShowTouchSquare, I_SQUARE });
	toggles_.push_back({ "Triangle", &g_PConfig.bShowTouchTriangle, I_TRIANGLE });
	toggles_.push_back({ "L", &g_PConfig.touchLKey.show, I_L });
	toggles_.push_back({ "R", &g_PConfig.touchRKey.show, I_R });
	toggles_.push_back({ "Start", &g_PConfig.touchStartKey.show, I_START });
	toggles_.push_back({ "Select", &g_PConfig.touchSelectKey.show, I_SELECT });
	toggles_.push_back({ "Dpad", &g_PConfig.touchDpad.show, -1 });
	toggles_.push_back({ "Analog Stick", &g_PConfig.touchAnalogStick.show, -1 });
	toggles_.push_back({ "Right Analog Stick\n(not used by most games)", &g_PConfig.touchRightAnalogStick.show, -1 });
	toggles_.push_back({ "Unthrottle", &g_PConfig.touchUnthrottleKey.show, -1 });
	toggles_.push_back({ "Combo0", &g_PConfig.touchCombo0.show, I_1 });
	toggles_.push_back({ "Combo1", &g_PConfig.touchCombo1.show, I_2 });
	toggles_.push_back({ "Combo2", &g_PConfig.touchCombo2.show, I_3 });
	toggles_.push_back({ "Combo3", &g_PConfig.touchCombo3.show, I_4 });
	toggles_.push_back({ "Combo4", &g_PConfig.touchCombo4.show, I_5 });
	toggles_.push_back({ "Alt speed 1", &g_PConfig.touchSpeed1Key.show, -1 });
	toggles_.push_back({ "Alt speed 2", &g_PConfig.touchSpeed2Key.show, -1 });

	I18NCategory *mc = GetI18NCategory("MappableControls");

	for (auto toggle : toggles_) {
		LinearLayout *row = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
		row->SetSpacing(0);

		CheckBox *checkbox = new CheckBox(toggle.show, "", "", new LinearLayoutParams(50, WRAP_CONTENT));
		row->Add(checkbox);

		Choice *choice;
		if (toggle.img != -1) {
			choice = new CheckBoxChoice(toggle.img, checkbox, new LinearLayoutParams(1.0f));
		} else {
			choice = new CheckBoxChoice(mc->T(toggle.key), checkbox, new LinearLayoutParams(1.0f));
		}

		choice->SetCentered(true);

		row->Add(choice);
		grid->Add(row);
	}
}

void TouchControlVisibilityScreen::onFinish(DialogResult result) {
	g_PConfig.Save("TouchControlVisibilityScreen::onFinish");
}

PUI::EventReturn TouchControlVisibilityScreen::OnToggleAll(PUI::EventParams &e) {
	for (auto toggle : toggles_) {
		*toggle.show = nextToggleAll_;
	}
	nextToggleAll_ = !nextToggleAll_;

	return PUI::EVENT_DONE;
}

PUI::EventReturn CheckBoxChoice::HandleClick(PUI::EventParams &e) {
	checkbox_->Toggle();

	return PUI::EVENT_DONE;
};
