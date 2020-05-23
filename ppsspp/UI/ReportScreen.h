// Copyright (c) 2014- PPSSPP Project.

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

#include "ui/ui_screen.h"
#include "ui/viewgroup.h"
#include "UI/MiscScreens.h"

enum class ReportingOverallScore : int {
	PERFECT = 0,
	PLAYABLE = 1,
	INGAME = 2,
	MENU = 3,
	NONE = 4,
	INVALID = -1,
};

class ReportScreen : public UIDialogScreenWithGameBackground {
public:
	ReportScreen(const std::string &gamePath);

protected:
	void update() override;
	void resized() override;
	void CreateViews() override;
	void UpdateSubmit();
	void UpdateOverallDescription();

	PUI::EventReturn HandleChoice(PUI::EventParams &e);
	PUI::EventReturn HandleSubmit(PUI::EventParams &e);
	PUI::EventReturn HandleBrowser(PUI::EventParams &e);
	PUI::EventReturn HandleReportingChange(PUI::EventParams &e);

	PUI::Choice *submit_ = nullptr;
	PUI::View *screenshot_ = nullptr;
	PUI::TextView *reportingNotice_ = nullptr;
	PUI::TextView *overallDescription_ = nullptr;
	std::string screenshotFilename_;

	ReportingOverallScore overall_ = ReportingOverallScore::INVALID;
	int graphics_ = -1;
	int speed_ = -1;
	int gameplay_ = -1;
	bool enableReporting_;
	bool ratingEnabled_;
	bool includeScreenshot_ = true;
};

class ReportFinishScreen : public UIDialogScreenWithGameBackground {
public:
	ReportFinishScreen(const std::string &gamePath, ReportingOverallScore score);

protected:
	void update() override;
	void CreateViews() override;
	void ShowSuggestions();

	PUI::EventReturn HandleViewFeedback(PUI::EventParams &e);

	PUI::TextView *resultNotice_ = nullptr;
	PUI::LinearLayout *resultItems_ = nullptr;
	ReportingOverallScore score_;
	bool setStatus_ = false;
};
