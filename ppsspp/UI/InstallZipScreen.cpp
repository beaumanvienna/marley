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

#include "base/logging.h"
#include "i18n/i18n.h"
#include "ui/ui.h"
#include "ui/view.h"
#include "ui/viewgroup.h"
#include "UI/ui_atlas.h"
#include "file/file_util.h"

#include "Common/StringUtils.h"
#include "Core/Util/GameManager.h"
#include "UI/InstallZipScreen.h"
#include "UI/MainScreen.h"

void InstallZipScreen::CreateViews() {
	using namespace PUI;

	FileInfo fileInfo;
	bool success = getFileInfo(zipPath_.c_str(), &fileInfo);

	I18NCategory *di = GetI18NCategory("Dialog");
	I18NCategory *iz = GetI18NCategory("InstallZip");

	Margins actionMenuMargins(0, 100, 15, 0);

	root_ = new LinearLayout(ORIENT_HORIZONTAL);

	ViewGroup *leftColumn = new AnchorLayout(new LinearLayoutParams(1.0f));
	ViewGroup *rightColumnItems = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(300, FILL_PARENT, actionMenuMargins));
	root_->Add(leftColumn);
	root_->Add(rightColumnItems);

	std::string shortFilename = GetFilenameFromPath(zipPath_);
	
	// TODO: Do in the background?
	ZipFileInfo zipInfo;
	ZipFileContents contents = DetectZipFileContents(zipPath_, &zipInfo);

	if (contents == ZipFileContents::ISO_FILE || contents == ZipFileContents::PSP_GAME_DIR) {
		std::string question = iz->T("Install game from ZIP file?");
		leftColumn->Add(new TextView(question, ALIGN_LEFT, false, new AnchorLayoutParams(10, 10, NONE, NONE)));
		leftColumn->Add(new TextView(shortFilename, ALIGN_LEFT, false, new AnchorLayoutParams(10, 60, NONE, NONE)));

		doneView_ = leftColumn->Add(new TextView("", new AnchorLayoutParams(10, 120, NONE, NONE)));
		progressBar_ = leftColumn->Add(new ProgressBar(new AnchorLayoutParams(10, 200, 200, NONE)));

		installChoice_ = rightColumnItems->Add(new Choice(iz->T("Install")));
		installChoice_->OnClick.Handle(this, &InstallZipScreen::OnInstall);
		backChoice_ = rightColumnItems->Add(new Choice(di->T("Back")));
		rightColumnItems->Add(new CheckBox(&deleteZipFile_, iz->T("Delete ZIP file")));

		returnToHomebrew_ = true;
	} else if (contents == ZipFileContents::TEXTURE_PACK) {
		std::string question = iz->T("Install textures from ZIP file?");
		leftColumn->Add(new TextView(question, ALIGN_LEFT, false, new AnchorLayoutParams(10, 10, NONE, NONE)));
		leftColumn->Add(new TextView(shortFilename, ALIGN_LEFT, false, new AnchorLayoutParams(10, 60, NONE, NONE)));

		doneView_ = leftColumn->Add(new TextView("", new AnchorLayoutParams(10, 120, NONE, NONE)));
		progressBar_ = leftColumn->Add(new ProgressBar(new AnchorLayoutParams(10, 200, 200, NONE)));

		installChoice_ = rightColumnItems->Add(new Choice(iz->T("Install")));
		installChoice_->OnClick.Handle(this, &InstallZipScreen::OnInstall);
		backChoice_ = rightColumnItems->Add(new Choice(di->T("Back")));
		rightColumnItems->Add(new CheckBox(&deleteZipFile_, iz->T("Delete ZIP file")));

		returnToHomebrew_ = false;
	} else {
		leftColumn->Add(new TextView(iz->T("Zip file does not contain PSP software"), ALIGN_LEFT, false, new AnchorLayoutParams(10, 10, NONE, NONE)));
		backChoice_ = rightColumnItems->Add(new Choice(di->T("Back")));
	}

	// OK so that EmuScreen will handle it right.
	backChoice_->OnClick.Handle<UIScreen>(this, &UIScreen::OnOK);
}

bool InstallZipScreen::key(const KeyInput &key) {
	// Ignore all key presses during download and installation to avoid user escape
	if (g_GameManager.GetState() == GameManagerState::IDLE) {
		return UIScreen::key(key);
	}
	return false;
}

PUI::EventReturn InstallZipScreen::OnInstall(PUI::EventParams &params) {
	if (g_GameManager.InstallGameOnThread(zipPath_, zipPath_, deleteZipFile_)) {
		installStarted_ = true;
		installChoice_->SetEnabled(false);
	}
	return PUI::EVENT_DONE;
}

void InstallZipScreen::update() {
	I18NCategory *iz = GetI18NCategory("InstallZip");

	using namespace PUI;
	if (g_GameManager.GetState() != GameManagerState::IDLE) {
		if (progressBar_) {
			progressBar_->SetVisibility(V_VISIBLE);
			progressBar_->SetProgress(g_GameManager.GetCurrentInstallProgressPercentage());
		}
		backChoice_->SetEnabled(false);
	} else {
		if (progressBar_) {
			progressBar_->SetVisibility(V_GONE);
		}
		backChoice_->SetEnabled(true);
		std::string err = g_GameManager.GetInstallError();
		if (!err.empty()) {
			if (doneView_)
				doneView_->SetText(iz->T(err.c_str()));
		} else if (installStarted_) {
			if (doneView_)
				doneView_->SetText(iz->T("Installed!"));
			MainScreen::showHomebrewTab = returnToHomebrew_;
		}
	}
	UIScreen::update();
}
