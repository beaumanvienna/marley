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
std::string currentDirectory;
bool bGridView1;
bool bGridView2;
bool bGridView3;

int calcExtraThreadsPCSX2()
{
    int cnt = 2;
    if (cnt < 2) cnt = 0; // 1 is debugging that we do not want, negative values set to 0
    if (cnt > 7) cnt = 7; // limit to 1 main thread and 7 extra threads
    
    return cnt;
}

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
	Refresh();
}

void SCREEN_GameBrowser::FocusGame(const std::string &gamePath) {
	focusGamePath_ = gamePath;
	Refresh();
	focusGamePath_.clear();
}

void SCREEN_GameBrowser::SetPath(const std::string &path) {
	path_.SetPath(path);
	currentDirectory = path_.GetPath();
	Refresh();
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e) {
	*gridStyle_ = e.a == 0 ? true : false;
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::LastClick(SCREEN_UI::EventParams &e) {
//	LaunchBrowser(lastLink_.c_str());
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::HomeClick(SCREEN_UI::EventParams &e) {

	SetPath(getenv("HOME"));

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::PinToggleClick(SCREEN_UI::EventParams &e) {
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
	return path_.GetPath() != "!RECENT";
}

bool SCREEN_GameBrowser::HasSpecialFiles(std::vector<std::string> &filenames) {
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
	
	return true;
}

void SCREEN_GameBrowser::Refresh() {
	using namespace SCREEN_UI;

	lastScale_ = 1.0f;
	lastLayoutWasGrid_ = *gridStyle_;

	// Kill all the contents
	Clear();

	Add(new Spacer(1.0f));
	auto mm = GetI18NCategory("MainMenu");

	// No topbar on recent screen
	if (DisplayTopBar()) {
		LinearLayout *topBar = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
		if (browseFlags_ & BrowseFlags::NAVIGATE) {
			topBar->Add(new Spacer(2.0f));
			topBar->Add(new TextView(path_.GetFriendlyPath().c_str(), ALIGN_VCENTER | FLAG_WRAP_TEXT, true, new LinearLayoutParams(FILL_PARENT, 64.0f, 1.0f)));
			if (SCREEN_System_GetPropertyBool(SYSPROP_HAS_FILE_BROWSER)) {
				topBar->Add(new Choice(mm->T("Browse", "Browse..."), new LayoutParams(WRAP_CONTENT, 64.0f)))->OnClick.Handle(this, &SCREEN_GameBrowser::HomeClick);
			} else {
				topBar->Add(new Choice(mm->T("Home"), new LayoutParams(WRAP_CONTENT, 64.0f)))->OnClick.Handle(this, &SCREEN_GameBrowser::HomeClick);
			}
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
		for (size_t i = 0; i < filenames.size(); i++) {
			gameButtons.push_back(new GameButton(filenames[i], *gridStyle_, new SCREEN_UI::LinearLayoutParams(*gridStyle_ == true ? SCREEN_UI::WRAP_CONTENT : SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT)));
		}
	} else if (!listingPending_) {
		std::vector<FileInfo> fileInfo;
		path_.GetListing(fileInfo, "iso:cso:pbp:elf:prx:ppdmp:");
		for (size_t i = 0; i < fileInfo.size(); i++) {
			bool isGame = !fileInfo[i].isDirectory;
			bool isSaveData = false;
			// Check if eboot directory
			if (!isGame && path_.GetPath().size() >= 4 && IsValidPBP(path_.GetPath() + fileInfo[i].name + "/EBOOT.PBP", true))
				isGame = true;
			else if (!isGame && SCREEN_PFile::Exists(path_.GetPath() + fileInfo[i].name + "/PSP_GAME/SYSDIR"))
				isGame = true;
			else if (!isGame && SCREEN_PFile::Exists(path_.GetPath() + fileInfo[i].name + "/PARAM.SFO"))
				isSaveData = true;

			if (!isGame && !isSaveData) {
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

		// Add any pinned paths before other directories.
		auto pinnedPaths = GetPinnedPaths();
		for (auto it = pinnedPaths.begin(), end = pinnedPaths.end(); it != end; ++it) {
			gameList_->Add(new DirButton(*it, GetBaseName(*it), *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)))->
				OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);
		}
	}

	if (listingPending_) {
		gameList_->Add(new SCREEN_UI::TextView(mm->T("Loading..."), ALIGN_CENTER, false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)));
	}

	for (size_t i = 0; i < dirButtons.size(); i++) {
		gameList_->Add(dirButtons[i])->OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);
	}

	for (size_t i = 0; i < gameButtons.size(); i++) {
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
	}
}

bool SCREEN_GameBrowser::IsCurrentPathPinned() {
    std::vector<std::string> vPinnedPaths = {"/home/yo"};
	const auto paths = vPinnedPaths;
	return std::find(paths.begin(), paths.end(), SCREEN_PFile::ResolvePath(path_.GetPath())) != paths.end();
}

const std::vector<std::string> SCREEN_GameBrowser::GetPinnedPaths() {
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

	static const std::string sepChars = "/";

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
	GameButton *button = static_cast<GameButton *>(e.v);
	SCREEN_UI::EventParams e2{};
	e2.s = button->GamePath();
	// Insta-update - here we know we are already on the right thread.
	OnChoice.Trigger(e2);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonHoldClick(SCREEN_UI::EventParams &e) {
	GameButton *button = static_cast<GameButton *>(e.v);
	SCREEN_UI::EventParams e2{};
	e2.s = button->GamePath();
	// Insta-update - here we know we are already on the right thread.
	OnHoldChoice.Trigger(e2);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonHighlight(SCREEN_UI::EventParams &e) {
	// Insta-update - here we know we are already on the right thread.
	OnHighlight.Trigger(e);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::NavigateClick(SCREEN_UI::EventParams &e) {
	DirButton *button = static_cast<DirButton *>(e.v);
	std::string text = button->GetPath();
	if (button->PathAbsolute()) {
		path_.SetPath(text);
	} else {
		path_.Navigate(text);
	}
	currentDirectory = path_.GetPath();
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GridSettingsClick(SCREEN_UI::EventParams &e) {
	auto sy = GetI18NCategory("System");
	auto gridSettings = new GridSettingsScreen(sy->T("Games list settings"));
	gridSettings->OnRecentChanged.Handle(this, &SCREEN_GameBrowser::OnRecentClear);
	if (e.v)
		gridSettings->SetPopupOrigin(e.v);

	screenManager_->push(gridSettings);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e) {
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
	// Information in the top left.
	// Back button to the bottom left.
	// Scrolling action menu to the right.
	using namespace SCREEN_UI;

	bool vertical = UseVerticalLayout();

	auto mm = GetI18NCategory("MainMenu");

	Margins actionMenuMargins(0, 10, 10, 0);

	tabHolder_ = new TabHolder(ORIENT_HORIZONTAL, 64, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 1.0f));
	ViewGroup *leftColumn = tabHolder_;
	tabHolder_->SetTag("MainScreenGames");
	gameBrowsers_.clear();

	tabHolder_->SetClip(true);

	bool showRecent = false;
	bool hasStorageAccess = SCREEN_System_GetPermissionStatus(SYSTEM_PERMISSION_STORAGE) == PERMISSION_STATUS_GRANTED;
	bool storageIsTemporary = false;
	if (showRecent && !hasStorageAccess) {
		showRecent = false;
	}

	

	Button *focusButton = nullptr;
	if (hasStorageAccess) {
		ScrollView *scrollAllGames = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
		scrollAllGames->SetTag("MainScreenAllGames");
		ScrollView *scrollHomebrew = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
		scrollHomebrew->SetTag("MainScreenHomebrew");

		SCREEN_GameBrowser *tabAllGames = new SCREEN_GameBrowser(currentDirectory, BrowseFlags::STANDARD, &bGridView2, screenManager(),
			mm->T("How to get games"), "https://www.ppsspp.org/getgames.html",
			new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
		SCREEN_GameBrowser *tabHomebrew = new SCREEN_GameBrowser("/home/yo/Gaming", BrowseFlags::HOMEBREW_STORE, &bGridView3, screenManager(),
			mm->T("How to get homebrew & demos", "How to get homebrew && demos"), "https://www.ppsspp.org/gethomebrew.html",
			new LinearLayoutParams(FILL_PARENT, FILL_PARENT));

		scrollAllGames->Add(tabAllGames);
		gameBrowsers_.push_back(tabAllGames);
		scrollHomebrew->Add(tabHomebrew);
		gameBrowsers_.push_back(tabHomebrew);

		tabHolder_->AddTab(mm->T("Games"), scrollAllGames);
		tabHolder_->AddTab(mm->T("Homebrew & Demos"), scrollHomebrew);

		tabAllGames->OnChoice.Handle(this, &SCREEN_MainScreen::OnGameSelectedInstant);
		tabHomebrew->OnChoice.Handle(this, &SCREEN_MainScreen::OnGameSelectedInstant);

		tabAllGames->OnHoldChoice.Handle(this, &SCREEN_MainScreen::OnGameSelected);
		tabHomebrew->OnHoldChoice.Handle(this, &SCREEN_MainScreen::OnGameSelected);

		tabAllGames->OnHighlight.Handle(this, &SCREEN_MainScreen::OnGameHighlight);
		tabHomebrew->OnHighlight.Handle(this, &SCREEN_MainScreen::OnGameHighlight);


		if (backFromStore_ || showHomebrewTab) {
			tabHolder_->SetCurrentTab(2, true);
			backFromStore_ = false;
			showHomebrewTab = false;
		}

		if (storageIsTemporary) {
			LinearLayout *buttonHolder = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));
			buttonHolder->Add(new Spacer(new LinearLayoutParams(1.0f)));
			focusButton = new Button(mm->T("SavesAreTemporaryIgnore", "Ignore warning"), new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));
			focusButton->SetPadding(32, 16);
			buttonHolder->Add(focusButton)->OnClick.Add([this](SCREEN_UI::EventParams &e) {
				confirmedTemporary_ = true;
				RecreateViews();
				return SCREEN_UI::EVENT_DONE;
			});
			buttonHolder->Add(new Spacer(new LinearLayoutParams(1.0f)));

			leftColumn->Add(new Spacer(new LinearLayoutParams(0.1f)));
			leftColumn->Add(new TextView(mm->T("SavesAreTemporary", "PPSSPP saving in temporary storage"), ALIGN_HCENTER, false));
			leftColumn->Add(new TextView(mm->T("SavesAreTemporaryGuidance", "Extract PPSSPP somewhere to save permanently"), ALIGN_HCENTER, false));
			leftColumn->Add(new Spacer(10.0f));
			leftColumn->Add(buttonHolder);
			leftColumn->Add(new Spacer(new LinearLayoutParams(0.1f)));
		}
	} else {
		if (!showRecent) {
			leftColumn = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 1.0f));
			// Just so it's destroyed on recreate.
			leftColumn->Add(tabHolder_);
			tabHolder_->SetVisibility(V_GONE);
		}

		LinearLayout *buttonHolder = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));
		buttonHolder->Add(new Spacer(new LinearLayoutParams(1.0f)));
		focusButton = new Button(mm->T("Give PPSSPP permission to access storage"), new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));
		focusButton->SetPadding(32, 16);
		buttonHolder->Add(focusButton)->OnClick.Handle(this, &SCREEN_MainScreen::OnAllowStorage);
		buttonHolder->Add(new Spacer(new LinearLayoutParams(1.0f)));

		leftColumn->Add(new Spacer(new LinearLayoutParams(0.1f)));
		leftColumn->Add(buttonHolder);
		leftColumn->Add(new Spacer(10.0f));
		leftColumn->Add(new TextView(mm->T("PPSSPP can't load games or save right now"), ALIGN_HCENTER, false));
		leftColumn->Add(new Spacer(new LinearLayoutParams(0.1f)));
	}

	ViewGroup *rightColumn = new ScrollView(ORIENT_VERTICAL);
	LinearLayout *rightColumnItems = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	rightColumnItems->SetSpacing(0.0f);
	rightColumn->Add(rightColumnItems);

	char versionString[256];
	sprintf(versionString, "%s", "1.0");
	rightColumnItems->SetSpacing(0.0f);
	LinearLayout *logos = new LinearLayout(ORIENT_HORIZONTAL);
	if (SCREEN_System_GetPropertyBool(SYSPROP_APP_GOLD)) {
		logos->Add(new ImageView(ImageID("I_ICONGOLD"), IS_DEFAULT, new AnchorLayoutParams(64, 64, 10, 10, NONE, NONE, false)));
	} else {
		logos->Add(new ImageView(ImageID("I_ICON"), IS_DEFAULT, new AnchorLayoutParams(64, 64, 10, 10, NONE, NONE, false)));
	}
	logos->Add(new ImageView(ImageID("I_LOGO"), IS_DEFAULT, new LinearLayoutParams(Margins(-12, 0, 0, 0))));
	rightColumnItems->Add(logos);
	TextView *ver = rightColumnItems->Add(new TextView(versionString, new LinearLayoutParams(Margins(70, -6, 0, 0))));
	ver->SetSmall(true);
	ver->SetClip(false);

	rightColumnItems->Add(new Choice(mm->T("Game Settings", "Settings")))->OnClick.Handle(this, &SCREEN_MainScreen::OnGameSettings);
	rightColumnItems->Add(new Choice(mm->T("Credits")))->OnClick.Handle(this, &SCREEN_MainScreen::OnCredits);
	rightColumnItems->Add(new Choice(mm->T("www.ppsspp.org")))->OnClick.Handle(this, &SCREEN_MainScreen::OnPPSSPPOrg);
	if (!SCREEN_System_GetPropertyBool(SYSPROP_APP_GOLD)) {
		Choice *gold = rightColumnItems->Add(new Choice(mm->T("Buy PPSSPP Gold")));
		gold->OnClick.Handle(this, &SCREEN_MainScreen::OnSupport);
		gold->SetIcon(ImageID("I_ICONGOLD"));
	}

	rightColumnItems->Add(new Spacer(25.0));
	rightColumnItems->Add(new Choice(mm->T("Exit")))->OnClick.Handle(this, &SCREEN_MainScreen::OnExit);

	if (vertical) {
		root_ = new LinearLayout(ORIENT_VERTICAL);
		rightColumn->ReplaceLayoutParams(new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 0.75));
		root_->Add(rightColumn);
		root_->Add(leftColumn);
	} else {
		root_ = new LinearLayout(ORIENT_HORIZONTAL);
		rightColumn->ReplaceLayoutParams(new LinearLayoutParams(300, FILL_PARENT, actionMenuMargins));
		root_->Add(leftColumn);
		root_->Add(rightColumn);
	}

	if (focusButton) {
		root_->SetDefaultFocusView(focusButton);
	} else if (tabHolder_->GetVisibility() != V_GONE) {
		root_->SetDefaultFocusView(tabHolder_);
	}

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
	return dp_yres > dp_xres * 1.1f;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnLoadFile(SCREEN_UI::EventParams &e) {
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

	std::string path = e.s;

	SCREEN_ScreenManager *screen = screenManager();
	LaunchFile(screen, path);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSettings(SCREEN_UI::EventParams &e) {
	//screenManager()->push(new GameSettingsScreen("", ""));
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnCredits(SCREEN_UI::EventParams &e) {
	//screenManager()->push(new CreditsScreen());
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnSupport(SCREEN_UI::EventParams &e) {

//	LaunchBrowser("https://central.ppsspp.org/buygold");

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnPPSSPPOrg(SCREEN_UI::EventParams &e) {
//	LaunchBrowser("https://www.ppsspp.org");
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnForums(SCREEN_UI::EventParams &e) {
//	LaunchBrowser("https://forums.ppsspp.org");
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnExit(SCREEN_UI::EventParams &e) {
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

void SCREEN_UmdReplaceScreen::CreateViews() {
	using namespace SCREEN_UI;
	Margins actionMenuMargins(0, 100, 15, 0);
	auto mm = GetI18NCategory("MainMenu");
	auto di = GetI18NCategory("Dialog");

	TabHolder *leftColumn = new TabHolder(ORIENT_HORIZONTAL, 64, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 1.0));
	leftColumn->SetTag("UmdReplace");
	leftColumn->SetClip(true);

	ViewGroup *rightColumn = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(270, FILL_PARENT, actionMenuMargins));
	LinearLayout *rightColumnItems = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	rightColumnItems->SetSpacing(0.0f);
	rightColumn->Add(rightColumnItems);

	ScrollView *scrollAllGames = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
	scrollAllGames->SetTag("UmdReplaceAllGames");

	SCREEN_GameBrowser *tabAllGames = new SCREEN_GameBrowser(currentDirectory, BrowseFlags::STANDARD, &bGridView2, screenManager(),
		mm->T("How to get games"), "https://www.ppsspp.org/getgames.html",
		new LinearLayoutParams(FILL_PARENT, FILL_PARENT));

	scrollAllGames->Add(tabAllGames);

	leftColumn->AddTab(mm->T("Games"), scrollAllGames);

	tabAllGames->OnChoice.Handle(this, &SCREEN_UmdReplaceScreen::OnGameSelectedInstant);

	tabAllGames->OnHoldChoice.Handle(this, &SCREEN_UmdReplaceScreen::OnGameSelected);

	rightColumnItems->Add(new Choice(di->T("Cancel")))->OnClick.Handle(this, &SCREEN_UmdReplaceScreen::OnCancel);
	rightColumnItems->Add(new Choice(mm->T("Game Settings")))->OnClick.Handle(this, &SCREEN_UmdReplaceScreen::OnGameSettings);

	root_ = new LinearLayout(ORIENT_HORIZONTAL);
	root_->Add(leftColumn);
	root_->Add(rightColumn);
}

void SCREEN_UmdReplaceScreen::update() {
	SCREEN_UpdateUIState(UISTATE_PAUSEMENU);
	SCREEN_UIScreen::update();
}

SCREEN_UI::EventReturn SCREEN_UmdReplaceScreen::OnGameSelected(SCREEN_UI::EventParams &e) {
//	__UmdReplace(e.s);
	TriggerFinish(DR_OK);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_UmdReplaceScreen::OnCancel(SCREEN_UI::EventParams &e) {
	TriggerFinish(DR_CANCEL);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_UmdReplaceScreen::OnGameSettings(SCREEN_UI::EventParams &e) {
	//screenManager()->push(new GameSettingsScreen(""));
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_UmdReplaceScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e) {
//	__UmdReplace(e.s);
	TriggerFinish(DR_OK);
	return SCREEN_UI::EVENT_DONE;
}

void GridSettingsScreen::CreatePopupContents(SCREEN_UI::ViewGroup *parent) {
	using namespace SCREEN_UI;

	auto di = GetI18NCategory("Dialog");
	auto sy = GetI18NCategory("System");

	ScrollView *scroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, 50, 1.0f));
	LinearLayout *items = new LinearLayout(ORIENT_VERTICAL);

	items->Add(new CheckBox(&bGridView1, sy->T("Display Recent on a grid")));
	items->Add(new CheckBox(&bGridView2, sy->T("Display Games on a grid")));
	items->Add(new CheckBox(&bGridView3, sy->T("Display Homebrew on a grid")));

	items->Add(new ItemHeader(sy->T("Grid icon size")));
	items->Add(new Choice(sy->T("Increase size")))->OnClick.Handle(this, &GridSettingsScreen::GridPlusClick);
	items->Add(new Choice(sy->T("Decrease size")))->OnClick.Handle(this, &GridSettingsScreen::GridMinusClick);

	scroll->Add(items);
	parent->Add(scroll);
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
