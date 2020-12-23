// Copyright (c) 2013-2020 PPSSPP project
// Copyright (c) 2020 Marley project

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

#include "ppsspp_config.h"

#include <algorithm>
#include <list>

#include "Common/KeyMap.h"
#include "Common/GPU/OpenGL/GLFeatures.h"
#include "Common/Render/DrawBuffer.h"
#include "Common/UI/Root.h"
#include "Common/UI/View.h"
#include "Common/UI/ViewGroup.h"
#include "Common/UI/Context.h"
#include "Common/System/Display.h"
#include "Common/System/System.h"
#include "Common/System/NativeApp.h"
#include "Common/Data/Color/RGBAUtil.h"
#include "Common/Math/curves.h"
#include "Common/Data/Text/I18n.h"
#include "Common/Data/Encoding/Utf8.h"
#include "Common/File/FileUtil.h"
#include "Common/OSVersion.h"
#include "Common/TimeUtil.h"
#include "Common/StringUtils.h"
#include "UI/MainScreen.h"
#include "UI/MiscScreens.h"
#include <SDL.h>
#define FILE_BROWSER_WIDTH 1006.0f
void UISetBackground(SCREEN_UIContext &dc,std::string bgPng);
void DrawBackgroundSimple(SCREEN_UIContext &dc);
void findAllFiles(const char * directory, std::list<std::string> *tmpList, std::list<std::string> *toBeRemoved, bool recursiveSearch=true);
void stripList(std::list<std::string> *tmpList,std::list<std::string> *toBeRemoved);
void finalizeList(std::list<std::string> *tmpList);

extern bool launch_request_from_screen_manager;
extern std::string game_screen_manager;
extern std::string gBaseDir;
extern bool stopSearching;
extern int gTheme;

bool bGridViewMain1;
bool bGridViewMain2=false;
std::string lastGamePath;
SCREEN_UI::TextView* gamesPathView;
bool shutdown_now;
SCREEN_MainScreen::SCREEN_MainScreen() 
{
    printf("jc: SCREEN_MainScreen::SCREEN_MainScreen() \n");
    launch_request_from_screen_manager=false;
    
    std::string str, line;
    std::string marley_cfg = gBaseDir + "marley.cfg";
    std::ifstream marley_cfg_in_filehandle(marley_cfg);
    
    if (marley_cfg_in_filehandle.is_open())
    {
        while ( getline (marley_cfg_in_filehandle,line))
        {
            std::string subStr;
            if (line.find("last_game_path=") != std::string::npos)
            {
                lastGamePath = line.substr(line.find_last_of("=") + 1);
                if (!SCREEN_PFile::IsDirectory(lastGamePath))
                  lastGamePath = getenv("HOME");
            }
        }
        marley_cfg_in_filehandle.close();
    }
}
bool createDir(std::string name);
SCREEN_MainScreen::~SCREEN_MainScreen() 
{
    printf("jc: SCREEN_MainScreen::~SCREEN_MainScreen() \n");
    
    std::string str, line;
    std::vector<std::string> marley_cfg_entries;
    std::string marley_cfg = gBaseDir + "marley.cfg";
    std::ifstream marley_cfg_in_filehandle(marley_cfg);
    std::ofstream marley_cfg_out_filehandle;
    bool found_last_game_path = false;
    
    if (marley_cfg_in_filehandle.is_open())
    {
        while ( getline (marley_cfg_in_filehandle,line))
        {
            if (line.find("last_game_path=") != std::string::npos)
            {
                found_last_game_path = true;
                str = "last_game_path=" + lastGamePath;
                marley_cfg_entries.push_back(str);
            }else
            {
                marley_cfg_entries.push_back(line);
            }
        }
        if (!found_last_game_path)
        {
            str = "last_game_path=" + lastGamePath;
            marley_cfg_entries.push_back(str);
        }
        marley_cfg_in_filehandle.close();
    }
    
    // output marley.cfg
    marley_cfg_out_filehandle.open(marley_cfg.c_str(), std::ios_base::out); 
    if(marley_cfg_out_filehandle)
    {
        for(int i=0; i<marley_cfg_entries.size(); i++)
        {
            marley_cfg_out_filehandle << marley_cfg_entries[i] << "\n";
        }
        
        marley_cfg_out_filehandle.close();
    }
    
}

void SCREEN_MainScreen::DrawBackground(SCREEN_UIContext &dc) {
    std::string bgPng;
    if (gTheme == THEME_RETRO)
      bgPng = gBaseDir + "screen_manager/beach.png";
    else
      bgPng = gBaseDir + "screen_manager/settings_general.png";
    
    UISetBackground(dc,bgPng);
    DrawBackgroundSimple(dc);
}

void SCREEN_MainScreen::CreateViews() {
	using namespace SCREEN_UI;
    
    printf("jc: void SCREEN_MainScreen::CreateViews() {\n");

	auto ma = GetI18NCategory("Main");

	root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
    root_->SetTag("root_");

    LinearLayout *verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
    verticalLayout->SetTag("verticalLayout");
    root_->Add(verticalLayout);
    
   	float leftSide = 40.0f;

	settingInfo_ = new SCREEN_MainInfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new AnchorLayoutParams(dp_xres - leftSide - 40.0f, WRAP_CONTENT, leftSide, dp_yres - 80.0f - 40.0f, NONE, NONE));
	settingInfo_->SetBottomCutoff(dp_yres - 200.0f);
	root_->Add(settingInfo_);

    verticalLayout->Add(new Spacer(10.0f));

    LinearLayout *topline = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    topline->SetTag("topLine");
    verticalLayout->Add(topline);
    
    float leftMargin = (dp_xres - FILE_BROWSER_WIDTH)/2-10;
    
    topline->Add(new Spacer(leftMargin+FILE_BROWSER_WIDTH-138.0f,0.0f));
    ImageID icon;
    if (gTheme == THEME_RETRO) icon = ImageID("I_GEAR_R"); else icon = ImageID("I_GEAR");
    topline->Add(new Choice(icon, new LayoutParams(64.0f, 64.0f)))->OnClick.Handle(this, &SCREEN_MainScreen::settingsClick);
    if (gTheme == THEME_RETRO) icon = ImageID("I_OFF_R"); else icon = ImageID("I_OFF");
    Choice* offButton = new Choice(icon, new LayoutParams(64.0f, 64.0f),true);
    offButton->OnClick.Handle(this, &SCREEN_MainScreen::offClick);
    offButton->OnHold.Handle(this, &SCREEN_MainScreen::offHold);
    topline->Add(offButton);
    
    verticalLayout->Add(new Spacer(233.0f));
  
    // -------- horizontal main launcher frame --------
    LinearLayout *gameLauncherMainFrame = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(1.0f));
    verticalLayout->Add(gameLauncherMainFrame);
    gameLauncherMainFrame->SetTag("gameLauncherMainFrame");
    gameLauncherMainFrame->Add(new Spacer(leftMargin));
    
    // vetical layout for the game browser's top bar and the scroll view
    Margins mgn(0,0,0,94);
    LinearLayout *gameLauncherColumn = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, 300, 0.0f,G_TOPLEFT, mgn));
    gameLauncherMainFrame->Add(gameLauncherColumn);
    gameLauncherColumn->SetTag("gameLauncherColumn");
    
    // game browser's top bar
    LinearLayout *topBar = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    gameLauncherColumn->Add(topBar);
    topBar->SetTag("topBar");
    if (gTheme == THEME_RETRO) icon = ImageID("I_HOME_R"); else icon = ImageID("I_HOME");
    topBar->Add(new Choice(icon, new LayoutParams(64.0f, 64.0f)))->OnClick.Handle(this, &SCREEN_MainScreen::HomeClick);
    gamesPathView = new TextView(lastGamePath, ALIGN_VCENTER | FLAG_WRAP_TEXT, true, new LinearLayoutParams(WRAP_CONTENT, 64.0f));
    
    if (gTheme == THEME_RETRO) 
    {
        gamesPathView->SetTextColor(0xFFde51e0);
        gamesPathView->SetShadow(true);
    }
    topBar->Add(gamesPathView);

    // frame for scolling
    ViewGroup *gameLauncherFrameScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	gameLauncherFrameScroll->SetTag("gameLauncherFrameScroll");
	LinearLayout *gameLauncherFrame = new LinearLayout(ORIENT_VERTICAL);
    gameLauncherFrame->SetTag("gameLauncherFrame");
	gameLauncherFrame->SetSpacing(0);
	gameLauncherFrameScroll->Add(gameLauncherFrame);
	gameLauncherColumn->Add(gameLauncherFrameScroll);

    // game browser
    
    ROM_browser = new SCREEN_GameBrowser(lastGamePath, SCREEN_BrowseFlags::STANDARD, &bGridViewMain2, screenManager(),
        ma->T("Use the Start button to confirm"), "https://github.com/beaumanvienna/marley",
        new LinearLayoutParams(FILE_BROWSER_WIDTH, FILL_PARENT));
    ROM_browser->SetTag("ROM_browser");
    gameLauncherFrame->Add(ROM_browser);
    
    ROM_browser->OnChoice.Handle(this, &SCREEN_MainScreen::OnGameSelectedInstant);
    ROM_browser->OnHoldChoice.Handle(this, &SCREEN_MainScreen::OnGameSelected);
    ROM_browser->OnHighlight.Handle(this, &SCREEN_MainScreen::OnGameHighlight);
    
    root_->SetDefaultFocusView(ROM_browser);
    
	SCREEN_Draw::SCREEN_DrawContext *draw = screenManager()->getSCREEN_DrawContext();

}

void SCREEN_MainScreen::onFinish(DialogResult result) {
    SCREEN_System_SendMessage("finish", "");
}

void SCREEN_MainScreen::update() {
	SCREEN_UIScreen::update();

	bool theme = (gTheme == THEME_RETRO);
	if (theme != lasttheme_) {
		RecreateViews();
		lasttheme_ = theme;
	}
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelected(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelected(SCREEN_UI::EventParams &e)\n");
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameHighlight(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameHighlight(SCREEN_UI::EventParams &e)\n");
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e)\n");
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::HomeClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::HomeClick(SCREEN_UI::EventParams &e)\n");
	ROM_browser->SetPath(getenv("HOME"));

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::settingsClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::settingsClick(SCREEN_UI::EventParams &e)\n");
	screenManager()->push(new SCREEN_SettingsScreen());
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::offClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::offClick(SCREEN_UI::EventParams &e)\n");
    shutdown_now = false;
	OnBack(e);
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::offHold(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_MainScreen::offHold(SCREEN_UI::EventParams &e)\n");
    
    auto ma = GetI18NCategory("System");
	auto offDiag = new SCREEN_OffDiagScreen(ma->T("switch off computer"));
	if (e.v)
		offDiag->SetPopupOrigin(e.v);

	screenManager()->push(offDiag);
    
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_MainInfoMessage::SCREEN_MainInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp)
	: SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, lp) {
	using namespace SCREEN_UI;
	SetSpacing(0.0f);
	Add(new SCREEN_UI::Spacer(10.0f));
	text_ = Add(new SCREEN_UI::TextView("", align, false, new LinearLayoutParams(1.0, Margins(0, 10))));
	Add(new SCREEN_UI::Spacer(10.0f));
}

void SCREEN_MainInfoMessage::Show(const std::string &text, SCREEN_UI::View *refView) {
	if (refView) {
		Bounds b = refView->GetBounds();
		const SCREEN_UI::AnchorLayoutParams *lp = GetLayoutParams()->As<SCREEN_UI::AnchorLayoutParams>();
		if (b.y >= cutOffY_) {
			ReplaceLayoutParams(new SCREEN_UI::AnchorLayoutParams(lp->width, lp->height, lp->left, 80.0f, lp->right, lp->bottom, lp->center));
		} else {
			ReplaceLayoutParams(new SCREEN_UI::AnchorLayoutParams(lp->width, lp->height, lp->left, dp_yres - 80.0f - 40.0f, lp->right, lp->bottom, lp->center));
		}
	}
	text_->SetText(text);
	timeShown_ = time_now_d();
}

void SCREEN_MainInfoMessage::Draw(SCREEN_UIContext &dc) {
	static const double FADE_TIME = 1.0;
	static const float MAX_ALPHA = 0.9f;

	// Let's show longer messages for more time (guesstimate at reading speed.)
	// Note: this will give multibyte characters more time, but they often have shorter words anyway.
	double timeToShow = std::max(1.5, text_->GetText().size() * 0.05);

	double sinceShow = time_now_d() - timeShown_;
	float alpha = MAX_ALPHA;
	if (timeShown_ == 0.0 || sinceShow > timeToShow + FADE_TIME) {
		alpha = 0.0f;
	} else if (sinceShow > timeToShow) {
		alpha = MAX_ALPHA - MAX_ALPHA * (float)((sinceShow - timeToShow) / FADE_TIME);
	}

	if (alpha >= 0.1f) {
		SCREEN_UI::Style style = dc.theme->popupTitle;
		style.background.color = colorAlpha(style.background.color, alpha - 0.1f);
		dc.FillRect(style.background, bounds_);
	}

	text_->SetTextColor(whiteAlpha(alpha));
	ViewGroup::Draw(dc);
}

class SCREEN_GameButton : public SCREEN_UI::Clickable {
public:
	SCREEN_GameButton(const std::string &gamePath, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams = 0)
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

	const std::string &GetPath() const { return gamePath_; }

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

void SCREEN_GameButton::Draw(SCREEN_UIContext &dc) {
    	using namespace SCREEN_UI;
	Style style = dc.theme->buttonStyle;

	if (HasFocus()) style = dc.theme->buttonFocusedStyle;
	if (down_) style = dc.theme->buttonDownStyle;
	if (!IsEnabled()) style = dc.theme->buttonDisabledStyle;

	dc.FillRect(style.background, bounds_);
    
    int startChar = gamePath_.find_last_of("/") + 1;  //show only file name
    int endChar = gamePath_.find_last_of("."); // remove extension
	const std::string text = gamePath_.substr(startChar,endChar-startChar);
    
	ImageID image = ImageID("I_BARREL");

	float tw, th;
	dc.MeasureText(dc.GetFontStyle(), 1.0, 1.0, text.c_str(), &tw, &th, 0);

	bool compact = bounds_.w < 180;

	if (gridStyle_) {
		dc.SetFontScale(1.0f, 1.0f);
	}
	if (compact) {
		dc.PushScissor(bounds_);
		dc.DrawText(text.c_str(), bounds_.x + 5, bounds_.centerY(), style.fgColor, ALIGN_VCENTER); 
		dc.PopScissor();
	} else {
		bool scissor = false;
		if (tw + 150 > bounds_.w) {
			dc.PushScissor(bounds_);
			scissor = true;
		}
		dc.Draw()->DrawImage(image, bounds_.x + 72, bounds_.centerY(), 0.88f, 0xFFFFFFFF, ALIGN_CENTER);
        if (gTheme == THEME_RETRO)
            dc.DrawText(text.c_str(), bounds_.x + 152, bounds_.centerY()+2, 0xFF000000, ALIGN_VCENTER);
		dc.DrawText(text.c_str(), bounds_.x + 150, bounds_.centerY(), style.fgColor, ALIGN_VCENTER);

		if (scissor) {
			dc.PopScissor();
		}
	}
	if (gridStyle_) {
		dc.SetFontScale(1.0, 1.0);
	}
}


class SCREEN_DirButtonMain : public SCREEN_UI::Button {
public:
	SCREEN_DirButtonMain(const std::string &path, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams)
		: SCREEN_UI::Button(path, layoutParams), path_(path), gridStyle_(gridStyle), absolute_(false) {}
	SCREEN_DirButtonMain(const std::string &path, const std::string &text, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams = 0)
		: SCREEN_UI::Button(text, layoutParams), path_(path), gridStyle_(gridStyle), absolute_(true) {}

	virtual void Draw(SCREEN_UIContext &dc);

	const std::string GetPath() const {
		return path_;
	}

	bool PathAbsolute() const {
		return absolute_;
	}
    
    bool Key(const KeyInput &key) override {
        std::string searchPath;
        if (key.flags & KEY_DOWN) {
            if (HasFocus() && ((key.keyCode==NKCODE_BUTTON_STRT) || (key.keyCode==NKCODE_SPACE))) {
                printf("jc: if (HasFocus() && ((key.keyCode==NKCODE_BUTTON_STRT) || (key.keyCode==NKCODE_SPACE)))\n");
            }
        } 

		return Clickable::Key(key);
	}

private:
	std::string path_;
	bool absolute_;
	bool gridStyle_;
};

void SCREEN_DirButtonMain::Draw(SCREEN_UIContext &dc) {
	using namespace SCREEN_UI;
	Style style = dc.theme->buttonStyle;

	if (HasFocus()) style = dc.theme->buttonFocusedStyle;
	if (down_) style = dc.theme->buttonDownStyle;
	if (!IsEnabled()) style = dc.theme->buttonDisabledStyle;

	dc.FillRect(style.background, bounds_);

	const std::string text = GetText();
    
    bool isRegularFolder = true;
	ImageID image;
    if (gTheme == THEME_RETRO) image = ImageID("I_FOLDER_R"); else image = ImageID("I_FOLDER");
	if (text == "..") {
        isRegularFolder = false;
        if (gTheme == THEME_RETRO) image = ImageID("I_UP_DIRECTORY_R"); else image = ImageID("I_UP_DIRECTORY");
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
		if (isRegularFolder) {
			dc.DrawText(text.c_str(), bounds_.x + 7, bounds_.centerY()+2, 0xFF000000, ALIGN_VCENTER);
            dc.DrawText(text.c_str(), bounds_.x + 5, bounds_.centerY(), style.fgColor, ALIGN_VCENTER);
		} else {
			dc.Draw()->DrawImage(image, bounds_.centerX(), bounds_.centerY(), 1.0, 0xFFFFFFFF, ALIGN_CENTER);
		}
		dc.PopScissor();
	} else {
		bool scissor = false;
		if (tw + 150 > bounds_.w) {
			dc.PushScissor(bounds_);
			scissor = true;
		}
		dc.Draw()->DrawImage(image, bounds_.x + 72, bounds_.centerY(), 0.88f, 0xFFFFFFFF, ALIGN_CENTER);
        if (gTheme == THEME_RETRO)
          dc.DrawText(text.c_str(), bounds_.x + 152, bounds_.centerY()+2, 0xFF000000, ALIGN_VCENTER);
		dc.DrawText(text.c_str(), bounds_.x + 150, bounds_.centerY(), style.fgColor, ALIGN_VCENTER);

		if (scissor) {
			dc.PopScissor();
		}
	}
	if (gridStyle_) {
		dc.SetFontScale(1.0, 1.0);
	}
}

SCREEN_GameBrowser::SCREEN_GameBrowser(std::string path, SCREEN_BrowseFlags browseFlags, bool *gridStyle, SCREEN_ScreenManager *screenManager, std::string lastText, std::string lastLink, SCREEN_UI::LayoutParams *layoutParams)
	: LinearLayout(SCREEN_UI::ORIENT_VERTICAL, layoutParams), path_(path), gridStyle_(gridStyle), screenManager_(screenManager), browseFlags_(browseFlags), lastText_(lastText), lastLink_(lastLink) {
	using namespace SCREEN_UI;
    printf("jc: SCREEN_GameBrowser::SCREEN_GameBrowser\n");
	Refresh();
}

SCREEN_GameBrowser::~SCREEN_GameBrowser() {
    printf("jc: SCREEN_GameBrowser::~SCREEN_GameBrowser()\n");
}

void SCREEN_GameBrowser::SetPath(const std::string &path) {
    printf("jc: void SCREEN_GameBrowser::SetPath(const std::string &path) %s\n",path.c_str());
	path_.SetPath(path);
	Refresh();
}

std::string SCREEN_GameBrowser::GetPath() {
    printf("jc: std::string SCREEN_GameBrowser::GetPath() \n");
    std::string str = path_.GetPath();
	return str;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e)\n");
	*gridStyle_ = e.a == 0 ? true : false;
	Refresh();
	return SCREEN_UI::EVENT_DONE;
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

void SCREEN_GameBrowser::Refresh() {
	using namespace SCREEN_UI;
    printf("jc: void SCREEN_GameBrowser::Refresh()\n");
    
	lastScale_ = 1.0f;
	lastLayoutWasGrid_ = *gridStyle_;

	// Reset content
	Clear();

	Add(new Spacer(1.0f));
	auto mm = GetI18NCategory("MainMenu");
	
    if (*gridStyle_) {
        gameList_ = new SCREEN_UI::GridLayout(SCREEN_UI::GridLayoutSettings(150*1.0f, 85*1.0f), new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    } else {
        SCREEN_UI::LinearLayout *gl = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
        gl->SetSpacing(4.0f);
        gameList_ = gl;
    }
    Add(gameList_);
    
    // Show games in the current directory
	std::vector<SCREEN_GameButton *> gameButtons;

	// Show folders in the current directory
	std::vector<SCREEN_DirButtonMain *> dirButtons;

	listingPending_ = !path_.IsListingReady();

	std::vector<std::string> filenames;
	if (!listingPending_) {
        lastGamePath = path_.GetPath();
        gamesPathView->SetText(lastGamePath);
        
        std::list<std::string> tmpList;
        std::list<std::string> toBeRemoved;
        std::string pathToBeSearched;
        std::string strList;
        std::list<std::string>::iterator iteratorTmpList;
        
        pathToBeSearched = path_.GetPath();
        stopSearching=false;
        findAllFiles(pathToBeSearched.c_str(),&tmpList,&toBeRemoved,false);
        stripList(&tmpList,&toBeRemoved); // strip cue file entries
        finalizeList(&tmpList);
        
        iteratorTmpList = tmpList.begin();
        for (int i=0;i<tmpList.size();i++)
        {
            strList = *iteratorTmpList;
            iteratorTmpList++;
            gameButtons.push_back(new SCREEN_GameButton(strList, *gridStyle_, new SCREEN_UI::LinearLayoutParams(*gridStyle_ == true ? SCREEN_UI::WRAP_CONTENT : SCREEN_UI::FILL_PARENT, 50.0f)));
        }
        
		std::vector<FileInfo> fileInfo;
		path_.GetListing(fileInfo, "");
		for (size_t i = 0; i < fileInfo.size(); i++) {
			if (fileInfo[i].isDirectory) {
				if (browseFlags_ & SCREEN_BrowseFlags::NAVIGATE) {
					dirButtons.push_back(new SCREEN_DirButtonMain(fileInfo[i].fullName, fileInfo[i].name, *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, 50.0f)));
				}
			}
		}
	}

	if (browseFlags_ & SCREEN_BrowseFlags::NAVIGATE) {
        if (lastGamePath != "/")
        {
            SCREEN_DirButtonMain* UP_button = new SCREEN_DirButtonMain("..", *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, 50.0f));
            UP_button->OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);
            gameList_->Add(UP_button);
        }
	}

	if (listingPending_) {
		gameList_->Add(new SCREEN_UI::TextView(mm->T("Loading..."), ALIGN_CENTER, false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)));
	}
    
   	for (size_t i = 0; i < gameButtons.size(); i++) {
		gameList_->Add(gameButtons[i])->OnClick.Handle(this, &SCREEN_GameBrowser::GameButtonClick);
	}
    
	for (size_t i = 0; i < dirButtons.size(); i++) {
        std::string str = dirButtons[i]->GetPath();
		gameList_->Add(dirButtons[i])->OnClick.Handle(this, &SCREEN_GameBrowser::NavigateClick);
	}
}

const std::string SCREEN_GameBrowser::GetBaseName(const std::string &path) {
printf("jc: const std::string SCREEN_GameBrowser::GetBaseName(const std::string &path)\n");
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

SCREEN_UI::EventReturn SCREEN_GameBrowser::NavigateClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::NavigateClick(SCREEN_UI::EventParams &e)\n");
    
	SCREEN_DirButtonMain *button = static_cast<SCREEN_DirButtonMain *>(e.v);
	std::string text = button->GetPath();
	if (button->PathAbsolute()) {
		path_.SetPath(text);
	} else { // cd ..
		path_.Navigate(text);
	}
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::GameButtonClick(SCREEN_UI::EventParams &e) {
	SCREEN_GameButton *button = static_cast<SCREEN_GameButton *>(e.v);
	std::string text = button->GetPath();
    
    printf("jc: ###############  launching %s ############### \n",text.c_str());
    launch_request_from_screen_manager = true;
    game_screen_manager = text;

    SCREEN_System_SendMessage("finish", "");

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e)\n");
	screenManager_->RecreateAllViews();
	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_OffDiagScreen::CreatePopupContents(SCREEN_UI::ViewGroup *parent) {
	using namespace SCREEN_UI;

	auto ma = GetI18NCategory("Main");

	LinearLayout *items = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));

	items->Add(new Choice(ma->T("YES"), new LayoutParams(200.0f, 64.0f)))->OnClick.Handle(this, &SCREEN_OffDiagScreen::SwitchOff);
	items->Add(new Choice(ma->T("CANCEL"), new LayoutParams(200.0f, 64.0f)))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);

	parent->Add(items);
}

SCREEN_UI::EventReturn SCREEN_OffDiagScreen::SwitchOff(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_OffDiagScreen::SwitchOff(SCREEN_UI::EventParams &e) \n");
    shutdown_now = true;
	SCREEN_System_SendMessage("finish", "");

	return SCREEN_UI::EVENT_DONE;
}
