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

#include <cmath>
#include <algorithm>

#include "ppsspp_config.h"

#include "Common/System/Display.h"
#include "Common/System/System.h"
#include "Common/Render/TextureAtlas.h"
#include "Common/Render/DrawBuffer.h"
#include "Common/UI/Root.h"
#include "Common/UI/Context.h"
#include "Common/UI/View.h"
#include "Common/UI/ViewGroup.h"
#include "Common/KeyMap.h"
#include "Common/Data/Color/RGBAUtil.h"
#include "Common/Data/Encoding/Utf8.h"
#include "Common/File/PathBrowser.h"
#include "Common/Math/curves.h"
#include "Common/File/FileUtil.h"
#include "Common/TimeUtil.h"
#include "Common/StringUtils.h"

#include "UI/MainScreen.h"
#include "Common/Data/Text/I18n.h"


#include <sstream>
void SCREEN_UpdateUIState(GlobalUIState newState);
bool SCREEN_MainScreen::showHomebrewTab = false;

bool bGridView1;
bool bGridView2;
bool bGridView3;

/*int calcExtraThreadsPCSX2()
{
    int cnt = 2;
    if (cnt < 2) cnt = 0; // 1 is debugging that we do not want, negative values set to 0
    if (cnt > 7) cnt = 7; // limit to 1 main thread and 7 extra threads
    
    return cnt;
}*/

bool LaunchFile(SCREEN_ScreenManager *screenManager, std::string path) {
	// Depending on the file type, we don't want to launch EmuScreen at all.
	/*auto loader = ConstructFileLoader(path);
	if (!loader) {
		return false;
	}

	IdentifiedFileType type = Identify_File(loader);
	delete loader;

	switch (type) {
	case IdentifiedFileType::ARCHIVE_ZIP:
		//screenManager->push(new InstallZipScreen(path));
		break;
	default:
		// Let the EmuScreen take care of it.
		screenManager->switchScreen(new EmuScreen(path));
		break;
	}*/
	return true;
}

static bool IsTempPath(const std::string &str) {
	std::string item = str;

	const auto testPath = [&](std::string temp) {

		if (!temp.empty() && temp[temp.size() - 1] != '/')
			temp += "/";

		return startsWith(item, temp);
	};

	const auto testCPath = [&](const char *temp) {
		if (temp && temp[0])
			return testPath(temp);
		return false;
	};

	if (testCPath(getenv("TMPDIR")))
		return true;
	if (testCPath(getenv("TMP")))
		return true;
	if (testCPath(getenv("TEMP")))
		return true;

	return false;
}

class GameButton : public SCREEN_UI::Clickable {
public:
	GameButton(const std::string &gamePath, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams = 0)
		: SCREEN_UI::Clickable(layoutParams), gridStyle_(gridStyle), gamePath_(gamePath) {}

	void Draw(SCREEN_UIContext &dc) override;
	void GetContentDimensions(const SCREEN_UIContext &dc, float &w, float &h) const override {
		if (gridStyle_) {
			w = 144;
			h = 80;
		} else {
			w = 500;
			h = 50;
		}
	}

	const std::string &GamePath() const { return gamePath_; }

	void SetHoldEnabled(bool hold) {
		holdEnabled_ = hold;
	}
	void Touch(const TouchInput &input) override {
		SCREEN_UI::Clickable::Touch(input);
		hovering_ = bounds_.Contains(input.x, input.y);
		if (hovering_ && (input.flags & TOUCH_DOWN)) {
			holdStart_ = time_now_d();
		}
		if (input.flags & TOUCH_UP) {
			holdStart_ = 0;
		}
	}

	bool Key(const KeyInput &key) override {
        printf("jc: bool Key(const KeyInput &key) override\n");
		std::vector<int> pspKeys;
		bool showInfo = false;

		if (SCREEN_KeyMap::KeyToPspButton(key.deviceId, key.keyCode, &pspKeys)) {
			for (auto it = pspKeys.begin(), end = pspKeys.end(); it != end; ++it) {
				// If the button mapped to triangle, then show the info.
				if (HasFocus() && (key.flags & KEY_UP) && *it == CTRL_TRIANGLE) {
					showInfo = true;
				}
			}
		} else if (hovering_ && key.deviceId == DEVICE_ID_MOUSE && key.keyCode == NKCODE_EXT_MOUSEBUTTON_2) {
			// If it's the right mouse button, and it's not otherwise mapped, show the info also.
			if (key.flags & KEY_DOWN) {
				showInfoPressed_ = true;
			}
			if ((key.flags & KEY_UP) && showInfoPressed_) {
				showInfo = true;
				showInfoPressed_ = false;
			}
		}

		if (showInfo) {
			TriggerOnHoldClick();
			return true;
		}

		return Clickable::Key(key);
	}

	void Update() override {
		// Hold button for 1.5 seconds to launch the game options
		if (holdEnabled_ && holdStart_ != 0.0 && holdStart_ < time_now_d() - 1.5) {
			TriggerOnHoldClick();
		}
	}

	void FocusChanged(int focusFlags) override {
		SCREEN_UI::Clickable::FocusChanged(focusFlags);
		TriggerOnHighlight(focusFlags);
	}

	SCREEN_UI::Event OnHoldClick;
	SCREEN_UI::Event OnHighlight;

private:
	void TriggerOnHoldClick() {
		holdStart_ = 0.0;
		SCREEN_UI::EventParams e{};
		e.v = this;
		e.s = gamePath_;
		down_ = false;
		OnHoldClick.Trigger(e);
	}
	void TriggerOnHighlight(int focusFlags) {
		SCREEN_UI::EventParams e{};
		e.v = this;
		e.s = gamePath_;
		e.a = focusFlags;
		OnHighlight.Trigger(e);
	}

	bool gridStyle_;
	std::string gamePath_;
	std::string title_;

	double holdStart_ = 0.0;
	bool holdEnabled_ = true;
	bool showInfoPressed_ = false;
	bool hovering_ = false;
};

void GameButton::Draw(SCREEN_UIContext &dc) {
    
}

class DirButton : public SCREEN_UI::Button {
public:
	DirButton(const std::string &path, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams)
		: SCREEN_UI::Button(path, layoutParams), path_(path), gridStyle_(gridStyle), absolute_(false) {}
	DirButton(const std::string &path, const std::string &text, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams = 0)
		: SCREEN_UI::Button(text, layoutParams), path_(path), gridStyle_(gridStyle), absolute_(true) {}

	virtual void Draw(SCREEN_UIContext &dc);

	const std::string GetPath() const {
		return path_;
	}

	bool PathAbsolute() const {
		return absolute_;
	}

private:
	std::string path_;
	bool absolute_;
	bool gridStyle_;
};

void DirButton::Draw(SCREEN_UIContext &dc) {
	using namespace SCREEN_UI;
	Style style = dc.theme->buttonStyle;

	if (HasFocus()) style = dc.theme->buttonFocusedStyle;
	if (down_) style = dc.theme->buttonDownStyle;
	if (!IsEnabled()) style = dc.theme->buttonDisabledStyle;

	dc.FillRect(style.background, bounds_);

	const std::string text = GetText();

	ImageID image = ImageID("I_FOLDER");
	if (text == "..") {
		image = ImageID("I_UP_DIRECTORY");
	}

	float tw, th;
	dc.MeasureText(dc.GetFontStyle(), 1.0, 1.0, text.c_str(), &tw, &th, 0);

	bool compact = bounds_.w < 180;

	if (gridStyle_) {
		dc.SetFontScale(1.0f, 1.0f);
	}
	if (compact) {
		// No icon, except "up"
		dc.PushScissor(bounds_);
		if (image == ImageID("I_FOLDER")) {
			dc.DrawText(text.c_str(), bounds_.x + 5, bounds_.centerY(), style.fgColor, ALIGN_VCENTER);
		} else {
			dc.Draw()->DrawImage(image, bounds_.centerX(), bounds_.centerY(), gridStyle_ ? 1.0f : 1.0, 0xFFFFFFFF, ALIGN_CENTER);
		}
		dc.PopScissor();
	} else {
		bool scissor = false;
		if (tw + 150 > bounds_.w) {
			dc.PushScissor(bounds_);
			scissor = true;
		}
		dc.Draw()->DrawImage(image, bounds_.x + 72, bounds_.centerY(), 0.88f*(gridStyle_ ? 1.0f : 1.0), 0xFFFFFFFF, ALIGN_CENTER);
		dc.DrawText(text.c_str(), bounds_.x + 150, bounds_.centerY(), style.fgColor, ALIGN_VCENTER);

		if (scissor) {
			dc.PopScissor();
		}
	}
	if (gridStyle_) {
		dc.SetFontScale(1.0, 1.0);
	}
}

SCREEN_GameBrowser::SCREEN_GameBrowser(std::string path, BrowseFlags browseFlags, bool *gridStyle, SCREEN_ScreenManager *screenManager, std::string lastText, std::string lastLink, SCREEN_UI::LayoutParams *layoutParams)
	: LinearLayout(SCREEN_UI::ORIENT_VERTICAL, layoutParams), path_(path), gridStyle_(gridStyle), screenManager_(screenManager), browseFlags_(browseFlags), lastText_(lastText), lastLink_(lastLink) {
	using namespace SCREEN_UI;
    printf("jc: SCREEN_GameBrowser::SCREEN_GameBrowser\n");
	Refresh();
}

void SCREEN_GameBrowser::FocusGame(const std::string &gamePath) {
    printf("jc: void SCREEN_GameBrowser::FocusGame(const std::string &gamePath)\n");
	focusGamePath_ = gamePath;
	Refresh();
	focusGamePath_.clear();
}

void SCREEN_GameBrowser::SetPath(const std::string &path) {
    printf("jc: void SCREEN_GameBrowser::SetPath(const std::string &path) %s\n",path.c_str());
	path_.SetPath(path);
	Refresh();
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e)\n");
	*gridStyle_ = e.a == 0 ? true : false;
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::LastClick(SCREEN_UI::EventParams &e) {
//	LaunchBrowser(lastLink_.c_str());
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::HomeClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::HomeClick(SCREEN_UI::EventParams &e)\n");
	SetPath(getenv("HOME"));

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::PinToggleClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::PinToggleClick(SCREEN_UI::EventParams &e)\n");
    std::vector<std::string> vPinnedPaths = {"/home/yo"};
	auto &pinnedPaths = vPinnedPaths;
	const std::string path = SCREEN_PFile::ResolvePath(path_.GetPath());
	if (IsCurrentPathPinned()) {
		pinnedPaths.erase(std::remove(pinnedPaths.begin(), pinnedPaths.end(), path), pinnedPaths.end());
	} else {
		pinnedPaths.push_back(path);
	}
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

bool SCREEN_GameBrowser::DisplayTopBar() {
    printf("jc: bool SCREEN_GameBrowser::DisplayTopBar()\n");
	return path_.GetPath() != "!RECENT";
}

bool SCREEN_GameBrowser::HasSpecialFiles(std::vector<std::string> &filenames) {
    printf("jc: bool SCREEN_GameBrowser::HasSpecialFiles(std::vector<std::string> &filenames) %ld\n",filenames.size());
    for (int i = 0; i<filenames.size();i++) 
    {
        std::string str = filenames[i];
        printf("jc: %s\n",str.c_str());
    }
	if (path_.GetPath() == "!RECENT") {
		//filenames = g_PConfig.recentIsos;
		return true;
	}
	return false;
}

void SCREEN_GameBrowser::Update() {
	LinearLayout::Update();
	if (listingPending_ && path_.IsListingReady()) {
		Refresh();
	}
}

void SCREEN_GameBrowser::Draw(SCREEN_UIContext &dc) {
	using namespace SCREEN_UI;

	if (lastScale_ != 1.0f || lastLayoutWasGrid_ != *gridStyle_) {
		Refresh();
	}

	if (hasDropShadow_) {
		// Darken things behind.
		dc.FillRect(SCREEN_UI::Drawable(0x60000000), dc.GetBounds().Expand(dropShadowExpand_));
		float dropsize = 30.0f;
		dc.Draw()->DrawImage4Grid(dc.theme->dropShadow4Grid,
			bounds_.x - dropsize, bounds_.y,
			bounds_.x2() + dropsize, bounds_.y2()+dropsize*1.5f, 0xDF000000, 3.0f);
	}

	if (clip_) {
		dc.PushScissor(bounds_);
	}

	dc.FillRect(bg_, bounds_);
	for (View *view : views_) {
		if (view->GetVisibility() == V_VISIBLE) {
			// Check if bounds are in current scissor rectangle.
			if (dc.GetScissorBounds().Intersects(dc.TransformBounds(view->GetBounds())))
				view->Draw(dc);
		}
	}
	if (clip_) {
		dc.PopScissor();
	}
}

static bool IsValidPBP(const std::string &path, bool allowHomebrew) {
	
	return false;
}

void SCREEN_GameBrowser::Refresh() {
	using namespace SCREEN_UI;
printf("jc: void SCREEN_GameBrowser::Refresh()\n");
	lastScale_ = 1.0f;
	lastLayoutWasGrid_ = *gridStyle_;

	// Reset content
	Clear();

	Add(new Spacer(1.0f));
	auto mm = GetI18NCategory("MainMenu");

	// No topbar on recent screen
	if (DisplayTopBar()) {
		LinearLayout *topBar = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
		if (browseFlags_ & BrowseFlags::NAVIGATE) {
			topBar->Add(new Spacer(2.0f));
			topBar->Add(new TextView(path_.GetFriendlyPath().c_str(), ALIGN_VCENTER | FLAG_WRAP_TEXT, true, new LinearLayoutParams(FILL_PARENT, 64.0f, 1.0f)));
			topBar->Add(new Choice(mm->T("Home"), new LayoutParams(WRAP_CONTENT, 64.0f)))->OnClick.Handle(this, &SCREEN_GameBrowser::HomeClick);
		} else {
			topBar->Add(new Spacer(new LinearLayoutParams(FILL_PARENT, 64.0f, 1.0f)));
		}
		ChoiceStrip *layoutChoice = topBar->Add(new ChoiceStrip(ORIENT_HORIZONTAL));
		layoutChoice->AddChoice(ImageID("I_GRID"));
		layoutChoice->AddChoice(ImageID("I_LINES"));
		layoutChoice->SetSelection(*gridStyle_ ? 0 : 1);
		layoutChoice->OnChoice.Handle(this, &SCREEN_GameBrowser::LayoutChange);
		topBar->Add(new Choice(ImageID("I_GEAR"), new LayoutParams(64.0f, 64.0f)))->OnClick.Handle(this, &SCREEN_GameBrowser::GridSettingsClick);
		Add(topBar);

		if (*gridStyle_) {
			gameList_ = new SCREEN_UI::GridLayout(SCREEN_UI::GridLayoutSettings(150*1.0f, 85*1.0f), new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
			Add(gameList_);
		} else {
			SCREEN_UI::LinearLayout *gl = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
			gl->SetSpacing(4.0f);
			gameList_ = gl;
			Add(gameList_);
		}
	} else {
		if (*gridStyle_) {
			gameList_ = new SCREEN_UI::GridLayout(SCREEN_UI::GridLayoutSettings(150*1.0f, 85*1.0f), new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
		} else {
			SCREEN_UI::LinearLayout *gl = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
			gl->SetSpacing(4.0f);
			gameList_ = gl;
		}
		
		Add(gameList_);
	}

	// Find games in the current directory and create new ones.
	std::vector<DirButton *> dirButtons;
	std::vector<GameButton *> gameButtons;

	listingPending_ = !path_.IsListingReady();

	std::vector<std::string> filenames;
	if (HasSpecialFiles(filenames)) {
        printf("jc: if (HasSpecialFiles(filenames))\n");
		for (size_t i = 0; i < filenames.size(); i++) {
			gameButtons.push_back(new GameButton(filenames[i], *gridStyle_, new SCREEN_UI::LinearLayoutParams(*gridStyle_ == true ? SCREEN_UI::WRAP_CONTENT : SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT)));
		}
	} else if (!listingPending_) {
        printf("jc: } else if (!listingPending_) {\n");
		std::vector<FileInfo> fileInfo;
		path_.GetListing(fileInfo, "iso:cso:pbp:elf:prx:ppdmp:");
        printf("jc: fileInfo.size()=%ld\n",fileInfo.size());
		for (size_t i = 0; i < fileInfo.size(); i++) {
            std::string str=fileInfo[i].name;
            printf("jc: fileInfo[i].name=%s\n",str.c_str());
			bool isGame = false;
			bool isSaveData = false;
			
			if (fileInfo[i].isDirectory) {
				if (browseFlags_ & BrowseFlags::NAVIGATE) {
					dirButtons.push_back(new DirButton(fileInfo[i].fullName, fileInfo[i].name, *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)));
				}
			} else {
				gameButtons.push_back(new GameButton(fileInfo[i].fullName, *gridStyle_, new SCREEN_UI::LinearLayoutParams(*gridStyle_ == true ? SCREEN_UI::WRAP_CONTENT : SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT)));
			}
		}
		
		if (browseFlags_ & BrowseFlags::ARCHIVES) {
			fileInfo.clear();
			path_.GetListing(fileInfo, "zip:rar:r01:7z:");
			if (!fileInfo.empty()) {
				SCREEN_UI::LinearLayout *zl = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
				zl->SetSpacing(4.0f);
				Add(zl);
				for (size_t i = 0; i < fileInfo.size(); i++) {
					if (!fileInfo[i].isDirectory) {
						GameButton *b = zl->Add(new GameButton(fileInfo[i].fullName, false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT)));
						b->OnClick.Handle(this, &SCREEN_GameBrowser::GameButtonClick);
						b->SetHoldEnabled(false);
					}
				}
			}
		}
	}

	if (browseFlags_ & BrowseFlags::NAVIGATE) {
		gameList_->Add(new DirButton("..", *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)))->
			OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);

		/*// Add any pinned paths before other directories.
		auto pinnedPaths = GetPinnedPaths();
		for (auto it = pinnedPaths.begin(), end = pinnedPaths.end(); it != end; ++it) {
			gameList_->Add(new DirButton(*it, GetBaseName(*it), *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)))->
				OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);
		}*/
	}

	if (listingPending_) {
		gameList_->Add(new SCREEN_UI::TextView(mm->T("Loading..."), ALIGN_CENTER, false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)));
	}

	for (size_t i = 0; i < dirButtons.size(); i++) {
        std::string str = dirButtons[i]->GetPath();
        printf("jc: for (size_t i = 0; i < dirButtons.size(); i++)  %s\n",str.c_str());
		gameList_->Add(dirButtons[i])->OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);
	}

	/*for (size_t i = 0; i < gameButtons.size(); i++) {
		GameButton *b = gameList_->Add(gameButtons[i]);
		b->OnClick.Handle(this, &SCREEN_GameBrowser::GameButtonClick);
		b->OnHoldClick.Handle(this, &SCREEN_GameBrowser::GameButtonHoldClick);
		b->OnHighlight.Handle(this, &SCREEN_GameBrowser::GameButtonHighlight);

		if (!focusGamePath_.empty() && b->GamePath() == focusGamePath_) {
			b->SetFocus();
		}
	}

	// Show a button to toggle pinning at the very end.
	if (browseFlags_ & BrowseFlags::PIN) {
		std::string caption = IsCurrentPathPinned() ? "-" : "+";
		if (!*gridStyle_) {
			caption = IsCurrentPathPinned() ? mm->T("UnpinPath", "Unpin") : mm->T("PinPath", "Pin");
		}
		gameList_->Add(new SCREEN_UI::Button(caption, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)))->
			OnClick.Handle(this, &SCREEN_GameBrowser::PinToggleClick);
	}

	if (browseFlags_ & BrowseFlags::HOMEBREW_STORE) {
		Add(new Spacer());
		Add(new Choice(mm->T("DownloadFromStore", "Download from the PPSSPP Homebrew Store"), new SCREEN_UI::LinearLayoutParams(SCREEN_UI::WRAP_CONTENT, SCREEN_UI::WRAP_CONTENT)))->OnClick.Handle(this, &SCREEN_GameBrowser::OnHomebrewStore);
	}

	if (!lastText_.empty() && gameButtons.empty()) {
		Add(new Spacer());
		Add(new Choice(lastText_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::WRAP_CONTENT, SCREEN_UI::WRAP_CONTENT)))->OnClick.Handle(this, &SCREEN_GameBrowser::LastClick);
	}*/
}

bool SCREEN_GameBrowser::IsCurrentPathPinned() {
    std::vector<std::string> vPinnedPaths = {"/home/yo"};
	const auto paths = vPinnedPaths;
    bool retVal = std::find(paths.begin(), paths.end(), SCREEN_PFile::ResolvePath(path_.GetPath())) != paths.end();
    printf("jc: bool SCREEN_GameBrowser::IsCurrentPathPinned() retVal=%d\n",retVal);
	return retVal;
}

const std::vector<std::string> SCREEN_GameBrowser::GetPinnedPaths() {
    printf("jc: const std::vector<std::string> SCREEN_GameBrowser::GetPinnedPaths()\n");
    std::vector<std::string> vPinnedPaths = {"/home/yo"};
	static const std::string sepChars = "/";

	const std::string currentPath = SCREEN_PFile::ResolvePath(path_.GetPath());
	const std::vector<std::string> paths = vPinnedPaths;
	std::vector<std::string> results;
	for (size_t i = 0; i < paths.size(); ++i) {
		// We want to exclude the current path, and its direct children.
		if (paths[i] == currentPath) {
			continue;
		}
		if (startsWith(paths[i], currentPath)) {
			std::string descendant = paths[i].substr(currentPath.size());
			// If there's only one separator (or none), its a direct child.
			if (descendant.find_last_of(sepChars) == descendant.find_first_of(sepChars)) {
				continue;
			}
		}

		results.push_back(paths[i]);
	}
	return results;
}

const std::string SCREEN_GameBrowser::GetBaseName(const std::string &path) {
printf("jc: const std::string SCREEN_GameBrowser::GetBaseName(const std::string &path)\n");
	static const std::string sepChars = getenv("HOME");

	auto trailing = path.find_last_not_of(sepChars);
	if (trailing != path.npos) {
		size_t start = path.find_last_of(sepChars, trailing);
		if (start != path.npos) {
			return path.substr(start + 1, trailing - start);
		}
		return path.substr(0, trailing);
	}

	size_t start = path.find_last_of(sepChars);
	if (start != path.npos) {
		return path.substr(start + 1);
	}
	return path;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonClick(SCREEN_UI::EventParams &e)\n");
	GameButton *button = static_cast<GameButton *>(e.v);
	SCREEN_UI::EventParams e2{};
	e2.s = button->GamePath();
	// Insta-update - here we know we are already on the right thread.
	OnChoice.Trigger(e2);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonHoldClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonHoldClick(SCREEN_UI::EventParams &e)\n");
	GameButton *button = static_cast<GameButton *>(e.v);
	SCREEN_UI::EventParams e2{};
	e2.s = button->GamePath();
	// Insta-update - here we know we are already on the right thread.
	OnHoldChoice.Trigger(e2);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonHighlight(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonHighlight(SCREEN_UI::EventParams &e)\n");
	// Insta-update - here we know we are already on the right thread.
	OnHighlight.Trigger(e);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::NavigateClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::NavigateClick(SCREEN_UI::EventParams &e)\n");
	DirButton *button = static_cast<DirButton *>(e.v);
	std::string text = button->GetPath();
	if (button->PathAbsolute()) {
		path_.SetPath(text);
	} else {
		path_.Navigate(text);
	}
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GridSettingsClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::GridSettingsClick(SCREEN_UI::EventParams &e)\n");
	auto sy = GetI18NCategory("System");
	auto gridSettings = new GridSettingsScreen(sy->T("Games list settings"));
	gridSettings->OnRecentChanged.Handle(this, &SCREEN_GameBrowser::OnRecentClear);
	if (e.v)
		gridSettings->SetPopupOrigin(e.v);

	screenManager_->push(gridSettings);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e)\n");
	screenManager_->RecreateAllViews();
	/*if (host) {
		host->UpdateUI();
	}*/
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::OnHomebrewStore(SCREEN_UI::EventParams &e) {
	//screenManager_->push(new StoreScreen());
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_MainScreen::SCREEN_MainScreen() {
	SCREEN_System_SendMessage("event", "mainscreen");
	lastVertical_ = UseVerticalLayout();
}

SCREEN_MainScreen::~SCREEN_MainScreen() {
}

void SCREEN_MainScreen::CreateViews() {
	using namespace SCREEN_UI;
printf("jc: void SCREEN_MainScreen::CreateViews()\n");
	bool vertical = true;

	auto mm = GetI18NCategory("MainMenu");

	Margins actionMenuMargins(0, 10, 10, 0);

	tabHolder_ = new TabHolder(ORIENT_HORIZONTAL, 64, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 1.0f));
	ViewGroup *leftColumn = tabHolder_;
	tabHolder_->SetTag("MainScreenGames");
	gameBrowsers_.clear();

	tabHolder_->SetClip(true);
	
    ScrollView *scrollAllGames = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    scrollAllGames->SetTag("MainScreenAllGames");
    
    SCREEN_GameBrowser *tabAllGames = new SCREEN_GameBrowser(getenv("HOME"), BrowseFlags::STANDARD, &bGridView2, screenManager(),
        mm->T("Use the Y/❒/North button to confirm"), "https://github.com/beaumanvienna/marley",
        new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
    
    scrollAllGames->Add(tabAllGames);
    gameBrowsers_.push_back(tabAllGames);
    
    tabHolder_->AddTab(mm->T("Games"), scrollAllGames);
    tabAllGames->OnChoice.Handle(this, &SCREEN_MainScreen::OnGameSelectedInstant);
    tabAllGames->OnHoldChoice.Handle(this, &SCREEN_MainScreen::OnGameSelected);
    tabAllGames->OnHighlight.Handle(this, &SCREEN_MainScreen::OnGameHighlight);
		
    root_ = new LinearLayout(ORIENT_VERTICAL);
    root_->Add(leftColumn);

	root_->SetDefaultFocusView(tabHolder_);

	auto u = GetI18NCategory("Upgrade");

	upgradeBar_ = 0;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnAllowStorage(SCREEN_UI::EventParams &e) {
	SCREEN_System_AskForPermission(SYSTEM_PERMISSION_STORAGE);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnDownloadUpgrade(SCREEN_UI::EventParams &e) {

	// Go directly to ppsspp.org and let the user sort it out
//	LaunchBrowser("https://www.ppsspp.org/downloads.html");

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnDismissUpgrade(SCREEN_UI::EventParams &e) {

	upgradeBar_->SetVisibility(SCREEN_UI::V_GONE);
	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_MainScreen::sendMessage(const char *message, const char *value) {
    printf("jc: void SCREEN_MainScreen::sendMessage(const char *message, const char *value)\n");
	// Always call the base class method first to handle the most common messages.
	SCREEN_UIScreenWithBackground::sendMessage(message, value);

	if (screenManager()->topScreen() == this) {
		if (!strcmp(message, "boot")) {
			LaunchFile(screenManager(), std::string(value));
		}
		if (!strcmp(message, "browse_folderSelect")) {
			int tab = tabHolder_->GetCurrentTab();
			if (tab >= 0 && tab < (int)gameBrowsers_.size()) {
				gameBrowsers_[tab]->SetPath(value);
			}
		}
	}
	if (!strcmp(message, "permission_granted") && !strcmp(value, "storage")) {
		RecreateViews();
	}
}

void SCREEN_MainScreen::update() {
	SCREEN_UIScreen::update();
	SCREEN_UpdateUIState(UISTATE_MENU);
	bool vertical = UseVerticalLayout();
	if (vertical != lastVertical_) {
		RecreateViews();
		lastVertical_ = vertical;
	}
}

bool SCREEN_MainScreen::UseVerticalLayout() const {
    bool retVal = dp_yres > dp_xres * 1.1f;
	return retVal;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnLoadFile(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnLoadFile(SCREEN_UI::EventParams &e)\n");
	if (SCREEN_System_GetPropertyBool(SYSPROP_HAS_FILE_BROWSER)) {
		SCREEN_System_SendMessage("browse_file", "");
	}
	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_MainScreen::DrawBackground(SCREEN_UIContext &dc) {
	SCREEN_UIScreenWithBackground::DrawBackground(dc);
	if (highlightedGamePath_.empty() && prevHighlightedGamePath_.empty()) {
		return;
	}

	if (DrawBackgroundFor(dc, prevHighlightedGamePath_, 1.0f - prevHighlightProgress_)) {
		if (prevHighlightProgress_ < 1.0f) {
			prevHighlightProgress_ += 1.0f / 20.0f;
		}
	}
	if (!highlightedGamePath_.empty()) {
		if (DrawBackgroundFor(dc, highlightedGamePath_, highlightProgress_)) {
			if (highlightProgress_ < 1.0f) {
				highlightProgress_ += 1.0f / 20.0f;
			}
		}
	}
}

bool SCREEN_MainScreen::DrawBackgroundFor(SCREEN_UIContext &dc, const std::string &gamePath, float progress) {
/*	dc.Flush();

	std::shared_ptr<GameInfo> ginfo;
	if (!gamePath.empty()) {
		ginfo = g_gameInfoCache->GetInfo(dc.GetSCREEN_DrawContext(), gamePath, GAMEINFO_WANTBG);
		// Loading texture data may bind a texture.
		dc.RebindTexture();

		// Let's not bother if there's no picture.
		if (!ginfo || (!ginfo->pic1.texture && !ginfo->pic0.texture)) {
			return false;
		}
	} else {
		return false;
	}

	SCREEN_Draw::SCREEN_Texture *texture = nullptr;
	if (ginfo->pic1.texture) {
		texture = ginfo->pic1.texture->GetTexture();
	} else if (ginfo->pic0.texture) {
		texture = ginfo->pic0.texture->GetTexture();
	}

	uint32_t color = whiteAlpha(ease(progress)) & 0xFFc0c0c0;
	if (texture) {
		dc.GetSCREEN_DrawContext()->BindTexture(0, texture);
		dc.Draw()->DrawTexRect(dc.GetBounds(), 0, 0, 1, 1, color);
		dc.Flush();
		dc.RebindTexture();
	}*/
	return true;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelected(SCREEN_UI::EventParams &e) {
printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelected(SCREEN_UI::EventParams &e)\n");
	/*std::string path = e.s;

	std::shared_ptr<GameInfo> ginfo = g_gameInfoCache->GetInfo(nullptr, path, GAMEINFO_WANTBG);
	if (ginfo && ginfo->fileType == IdentifiedFileType::PSP_SAVEDATA_DIRECTORY) {
		return SCREEN_UI::EVENT_DONE;
	}

	if (g_GameManager.GetState() == GameManagerState::INSTALLING)
		return SCREEN_UI::EVENT_DONE;

	// Restore focus if it was highlighted (e.g. by gamepad.)
	restoreFocusGamePath_ = highlightedGamePath_;
	g_BackgroundAudio.SetGame(path);
	lockBackgroundAudio_ = true;
	screenManager()->push(new GameScreen(path));*/
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameHighlight(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameHighlight(SCREEN_UI::EventParams &e)\n");
	/*using namespace SCREEN_UI;

	std::string path = e.s;

	// Don't change when re-highlighting what's already highlighted.
	if (path != highlightedGamePath_ || e.a == FF_LOSTFOCUS) {
		if (!highlightedGamePath_.empty()) {
			if (prevHighlightedGamePath_.empty() || prevHighlightProgress_ >= 0.75f) {
				prevHighlightedGamePath_ = highlightedGamePath_;
				prevHighlightProgress_ = 1.0 - highlightProgress_;
			}
			highlightedGamePath_.clear();
		}
		if (e.a == FF_GOTFOCUS) {
			highlightedGamePath_ = path;
			highlightProgress_ = 0.0f;
		}
	}

	if ((!highlightedGamePath_.empty() || e.a == FF_LOSTFOCUS) && !lockBackgroundAudio_) {
		g_BackgroundAudio.SetGame(highlightedGamePath_);
	}

	lockBackgroundAudio_ = false;*/
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e) {
printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e)\n");
	std::string path = e.s;

	SCREEN_ScreenManager *screen = screenManager();
	LaunchFile(screen, path);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnExit(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnExit(SCREEN_UI::EventParams &e)\n");
	SCREEN_System_SendMessage("event", "exitprogram");

	// Request the framework to exit cleanly.
	SCREEN_System_SendMessage("finish", "");

	SCREEN_UpdateUIState(UISTATE_EXIT);
	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_MainScreen::dialogFinished(const SCREEN_Screen *dialog, DialogResult result) {
	if (dialog->tag() == "store") {
		backFromStore_ = true;
		RecreateViews();
	}
	if (dialog->tag() == "game") {
		if (!restoreFocusGamePath_.empty() && SCREEN_UI::IsFocusMovementEnabled()) {
			// Prevent the background from fading, since we just were displaying it.
			highlightedGamePath_ = restoreFocusGamePath_;
			highlightProgress_ = 1.0f;

			// Refocus the game button itself.
			int tab = tabHolder_->GetCurrentTab();
			if (tab >= 0 && tab < (int)gameBrowsers_.size()) {
				gameBrowsers_[tab]->FocusGame(restoreFocusGamePath_);
			}

			// Don't get confused next time.
			restoreFocusGamePath_.clear();
		} 
	}
}

SCREEN_UI::EventReturn GridSettingsScreen::GridPlusClick(SCREEN_UI::EventParams &e) {
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn GridSettingsScreen::GridMinusClick(SCREEN_UI::EventParams &e) {
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn GridSettingsScreen::OnRecentClearClick(SCREEN_UI::EventParams &e) {

	OnRecentChanged.Trigger(e);
	return SCREEN_UI::EVENT_DONE;
}
