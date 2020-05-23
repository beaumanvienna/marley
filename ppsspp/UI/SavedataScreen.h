// Copyright (c) 2015- PPSSPP Project.

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
#include <string>

#include "ui/ui_screen.h"
#include "ui/view.h"
#include "ui/viewgroup.h"

#include "UI/MiscScreens.h"

enum class SavedataSortOption {
	FILENAME,
	SIZE,
	DATE,
};

class SavedataBrowser : public PUI::LinearLayout {
public:
	SavedataBrowser(std::string path, PUI::LayoutParams *layoutParams = 0);

	void SetSortOption(SavedataSortOption opt);

	PUI::Event OnChoice;

private:
	static bool ByFilename(const PUI::View *, const PUI::View *);
	static bool BySize(const PUI::View *, const PUI::View *);
	static bool ByDate(const PUI::View *, const PUI::View *);
	static bool SortDone();

	void Refresh();
	PUI::EventReturn SavedataButtonClick(PUI::EventParams &e);

	SavedataSortOption sortOption_ = SavedataSortOption::FILENAME;
	PUI::ViewGroup *gameList_ = nullptr;
	std::string path_;
};

class SavedataScreen : public UIDialogScreenWithGameBackground {
public:
	// gamePath can be empty, in that case this screen will show all savedata in the save directory.
	SavedataScreen(std::string gamePath);
	~SavedataScreen();

	void dialogFinished(const Screen *dialog, DialogResult result) override;

protected:
	PUI::EventReturn OnSavedataButtonClick(PUI::EventParams &e);
	PUI::EventReturn OnSortClick(PUI::EventParams &e);
	void CreateViews() override;
	bool gridStyle_;
	SavedataSortOption sortOption_ = SavedataSortOption::FILENAME;
	SavedataBrowser *dataBrowser_;
	SavedataBrowser *stateBrowser_;
};
