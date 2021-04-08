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
#include "Common/Render/Sprite_Sheet.h"
#include "UI/MainScreen.h"
#include "UI/MiscScreens.h"
#include "UI/Scale.h"
#include <SDL.h>

#include "../include/controller.h"
#include "../include/log.h"

void UISetBackground(SCREEN_UIContext &dc,std::string bgPng);
void DrawBackgroundSimple(SCREEN_UIContext &dc, int page);
void findAllFiles(const char * directory, std::list<std::string> *tmpList, std::list<std::string> *toBeRemoved, bool recursiveSearch=true);
void stripList(std::list<std::string> *tmpList,std::list<std::string> *toBeRemoved);
void finalizeList(std::list<std::string> *tmpList);
extern bool launch_request_from_screen_manager;
extern bool restart_screen_manager;
extern std::string game_screen_manager;
extern std::string gBaseDir;
extern bool stopSearching;
extern int gTheme;
extern SCREEN_ScreenManager *SCREEN_screenManager;

extern std::string gPackageVersion;
SCREEN_UI::ScrollView *gameLauncherFrameScroll;

SCREEN_ImageID checkControllerType(std::string name, std::string nameDB, bool mappingOK);
float gFileBrowserWidth;

bool bGridViewMain1;
bool bGridViewMain2=false;
std::string lastGamePath;
SCREEN_UI::TextView* gamesPathView;
bool shutdown_now;
bool gUpdateCurrentScreen;
bool gUpdateMainScreen;
bool toolTipsShown[MAX_TOOLTIP_IDs] = {0,0,0,0,0,0};

SCREEN_MainScreen::SCREEN_MainScreen() 
{
    DEBUG_PRINTF("   SCREEN_MainScreen::SCREEN_MainScreen() \n");
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
            }
        }
        marley_cfg_in_filehandle.close();
    }
    if (!SCREEN_PFile::IsDirectory(lastGamePath)) lastGamePath = getenv("HOME");
}
bool createDir(std::string name);
SCREEN_MainScreen::~SCREEN_MainScreen() 
{
    DEBUG_PRINTF("   SCREEN_MainScreen::~SCREEN_MainScreen() \n");
    
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
            if (line.find("# marley") != std::string::npos)
            {
                str = "# marley " + gPackageVersion;
                marley_cfg_entries.push_back(str);
            } else
            if (line.find("last_game_path=") != std::string::npos)
            {
                found_last_game_path = true;
                str = "last_game_path=" + lastGamePath;
                marley_cfg_entries.push_back(str);
            } else
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

bool SCREEN_MainScreen::key(const SCREEN_KeyInput &key)
{
    if ( !(offButton->HasFocus()) && (key.flags & KEY_DOWN) && ((key.keyCode==NKCODE_BACK) || (key.keyCode==NKCODE_ESCAPE))) {
        SCREEN_UI::SetFocusedView(offButton);
        return true;       
    }
    if ( (offButton->HasFocus()) && (key.flags & KEY_DOWN) && ((key.keyCode==NKCODE_BACK) || (key.keyCode==NKCODE_ESCAPE))) {
        SCREEN_UI::EventParams e{};
        e.v = offButton;
        SCREEN_UIScreen::OnBack(e);
        return true;       
    } 
    return SCREEN_UIDialogScreenWithBackground::key(key);
}

void SCREEN_MainScreen::DrawBackground(SCREEN_UIContext &dc) {
    std::string bgPng;
    if (gTheme != THEME_RETRO)
    {
      bgPng = gBaseDir + "screen_manager/settings_general.png";
      UISetBackground(dc,bgPng);
    }
    
    DrawBackgroundSimple(dc,SCREEN_MAIN);
}

void SCREEN_MainScreen::CreateViews() {
    using namespace SCREEN_UI;
    setGlobalScaling();
    
    DEBUG_PRINTF("   void SCREEN_MainScreen::CreateViews() {\n");

    auto ma = GetI18NCategory("Main");

    root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
    root_->SetTag("root_");

    LinearLayout *verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
    verticalLayout->SetTag("verticalLayout");
    verticalLayout->SetSpacing(0.0f);
    root_->Add(verticalLayout);
    
       float leftSide = f40;

    mainInfo_ = new SCREEN_MainInfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new AnchorLayoutParams(dp_xres - leftSide - f500, WRAP_CONTENT, leftSide, f20, NONE, NONE));
    root_->Add(mainInfo_);

    verticalLayout->Add(new Spacer(f10));
    
    // top line
    LinearLayout *topline = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    topline->SetTag("topLine");
    verticalLayout->Add(topline);
    
    gFileBrowserWidth = dp_xres*0.8;   
    float leftMargin = (dp_xres - gFileBrowserWidth)/2-10;
    
    bool controllerPlugged = (gDesignatedControllers[0].numberOfDevices != 0) || (gDesignatedControllers[1].numberOfDevices != 0);
    
    topline->Add(new Spacer(leftMargin+gFileBrowserWidth-f266,0.0f));
    
    SCREEN_ImageID icon, icon_active, icon_depressed;
    // settings button
    Choice* settingsButton;
    if (gTheme == THEME_RETRO) 
    {
        icon = SCREEN_ImageID("I_GEAR_R", BUTTON_STATE_NOT_FOCUSED); 
        icon_active = SCREEN_ImageID("I_GEAR_R", BUTTON_STATE_FOCUSED); 
        icon_depressed = SCREEN_ImageID("I_GEAR_R",BUTTON_STATE_FOCUSED_DEPRESSED);
        settingsButton = new Choice(icon, icon_active, icon_depressed, new LayoutParams(f128, f128));
    } else 
    {
        icon = SCREEN_ImageID("I_GEAR");
        settingsButton = new Choice(icon, new LayoutParams(f128, f128));
    }
    settingsButton->OnClick.Handle(this, &SCREEN_MainScreen::settingsClick);
    settingsButton->OnHighlight.Add([=](EventParams &e) {
        if (!toolTipsShown[MAIN_SETTINGS])
        {
            toolTipsShown[MAIN_SETTINGS] = true;
            mainInfo_->Show(ma->T("Settings", "Settings"), e.v);
        }
        return SCREEN_UI::EVENT_CONTINUE;
    });
    topline->Add(settingsButton);
    
    // off button
    if (gTheme == THEME_RETRO) 
    {
        icon = SCREEN_ImageID("I_OFF_R", BUTTON_STATE_NOT_FOCUSED); 
        icon_active = SCREEN_ImageID("I_OFF_R", BUTTON_STATE_FOCUSED); 
        icon_depressed = SCREEN_ImageID("I_OFF_R",BUTTON_STATE_FOCUSED_DEPRESSED); 
        offButton = new Choice(icon, icon_active, icon_depressed, new LayoutParams(f128, f128),true);
    } else 
    {
        icon = SCREEN_ImageID("I_OFF");
        offButton = new Choice(icon, new LayoutParams(f128, f128),true);
    }
    offButton->OnClick.Handle(this, &SCREEN_MainScreen::offClick);
    offButton->OnHold.Handle(this, &SCREEN_MainScreen::offHold);
    offButton->OnHighlight.Add([=](EventParams &e) {
        if (!toolTipsShown[MAIN_OFF])
        {
            toolTipsShown[MAIN_OFF] = true;
            mainInfo_->Show(ma->T("Off", "Off: exit Marley; keep this button pressed to switch the computer off"), e.v);
        }
        return SCREEN_UI::EVENT_CONTINUE;
    });
    topline->Add(offButton);
    
    double verticalSpace = (dp_yres-f476)/2;
    
    if (gDesignatedControllers[0].numberOfDevices != 0)
    {
        
        LinearLayout *controller_horizontal = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(1.0f));
        verticalLayout->Add(controller_horizontal);
        std::string name = gDesignatedControllers[0].name[0];
        std::string nameDB = gDesignatedControllers[0].nameDB[0];
        SCREEN_ImageID controllerSCREEN_ImageID = checkControllerType(name,nameDB,gDesignatedControllers[0].mappingOK);
        
        ImageView* controllerImage = new ImageView(controllerSCREEN_ImageID, IS_DEFAULT, new AnchorLayoutParams(verticalSpace, verticalSpace, 1.0f, 1.0f, NONE, NONE, false));
        controller_horizontal->Add(new Spacer(dp_xres-leftMargin-verticalSpace,1));
        controller_horizontal->Add(controllerImage);
        
    } else
    {
        verticalLayout->Add(new Spacer(verticalSpace));
    }
    if (gDesignatedControllers[1].numberOfDevices != 0)
    {
        LinearLayout *controller_horizontal = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(1.0f));
        verticalLayout->Add(controller_horizontal);
        std::string name = gDesignatedControllers[1].name[0];
        std::string nameDB = gDesignatedControllers[1].nameDB[0];
        SCREEN_ImageID controllerSCREEN_ImageID = checkControllerType(name,nameDB,gDesignatedControllers[1].mappingOK);
        
        ImageView* controllerImage = new ImageView(controllerSCREEN_ImageID, IS_DEFAULT, new AnchorLayoutParams(verticalSpace, verticalSpace, 1.0f, 1.0f, NONE, NONE, false));
        controller_horizontal->Add(new Spacer(dp_xres-leftMargin-verticalSpace,1));
        controller_horizontal->Add(controllerImage);
    }
    else
    {
        verticalLayout->Add(new Spacer(verticalSpace));
    }
    if (!controllerPlugged) verticalLayout->Add(new Spacer(f40));
    // -------- horizontal main launcher frame --------
    LinearLayout *gameLauncherMainFrame = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, f273,1.0f));
    verticalLayout->Add(gameLauncherMainFrame);
    gameLauncherMainFrame->SetTag("gameLauncherMainFrame");
    gameLauncherMainFrame->Add(new Spacer(leftMargin));
    
    if (controllerPlugged)
    {
        
        // vertical layout for the game browser's top bar and the scroll view
        Margins mgn(0,0,0,0);
        LinearLayout *gameLauncherColumn = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(gFileBrowserWidth, f243, 0.0f,G_TOPLEFT, mgn));
        gameLauncherMainFrame->Add(gameLauncherColumn);
        gameLauncherColumn->SetTag("gameLauncherColumn");
        gameLauncherColumn->SetSpacing(0.0f);
        
        // game browser's top bar
        LinearLayout *topBar = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
        gameLauncherColumn->Add(topBar);
        topBar->SetTag("topBar");
        
        // home button
        Choice* homeButton;
        if (gTheme == THEME_RETRO) 
        {
            icon = SCREEN_ImageID("I_HOME_R", BUTTON_STATE_NOT_FOCUSED); 
            icon_active = SCREEN_ImageID("I_HOME_R", BUTTON_STATE_FOCUSED); 
            icon_depressed = SCREEN_ImageID("I_HOME_R",BUTTON_STATE_FOCUSED_DEPRESSED);
            homeButton = new Choice(icon, icon_active, icon_depressed, new LayoutParams(f128, f128));
        }
        else
        {
            icon = SCREEN_ImageID("I_HOME");
            homeButton = new Choice(icon, new LayoutParams(f128, f128));    
        }        
        homeButton->OnClick.Handle(this, &SCREEN_MainScreen::HomeClick);
        homeButton->OnHighlight.Add([=](EventParams &e) {
            if (!toolTipsShown[MAIN_HOME])
            {
                toolTipsShown[MAIN_HOME] = true;
                mainInfo_->Show(ma->T("Home", "Jump in file browser to home directory"), e.v);
            }
            return SCREEN_UI::EVENT_CONTINUE;
        });
        topBar->Add(homeButton);
        
        LinearLayout *gamesPathViewFrame = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, f128));
        gamesPathViewFrame->Add(new Spacer(f54));
        
        gamesPathView = new TextView(lastGamePath, ALIGN_BOTTOMLEFT | FLAG_WRAP_TEXT, true, new LinearLayoutParams(WRAP_CONTENT, f64));
        gamesPathViewFrame->Add(gamesPathView);
        
        if (gTheme == THEME_RETRO) 
        {
            gamesPathView->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
            gamesPathView->SetShadow(true);
        }
        topBar->Add(gamesPathViewFrame);
        
        gameLauncherColumn->Add(new Spacer(f10));

        // frame for scolling 
        gameLauncherFrameScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, 169),true);
        gameLauncherFrameScroll->SetTag("gameLauncherFrameScroll");
        LinearLayout *gameLauncherFrame = new LinearLayout(ORIENT_VERTICAL);
        gameLauncherFrame->SetTag("gameLauncherFrame");
        gameLauncherFrame->SetSpacing(0);
        gameLauncherFrameScroll->Add(gameLauncherFrame);
        gameLauncherColumn->Add(gameLauncherFrameScroll);

        // game browser
        
        ROM_browser = new SCREEN_GameBrowser(lastGamePath, SCREEN_BrowseFlags::STANDARD, &bGridViewMain2, screenManager(),
            ma->T("Use the Start button to confirm"), "https://github.com/beaumanvienna/marley",
            new LinearLayoutParams(gFileBrowserWidth, WRAP_CONTENT));
        ROM_browser->SetTag("ROM_browser");
        gameLauncherFrame->Add(ROM_browser);
        
        ROM_browser->OnChoice.Handle(this, &SCREEN_MainScreen::OnGameSelectedInstant);
        ROM_browser->OnHoldChoice.Handle(this, &SCREEN_MainScreen::OnGameSelected);
        ROM_browser->OnHighlight.Handle(this, &SCREEN_MainScreen::OnGameHighlight);
        
        root_->SetDefaultFocusView(ROM_browser);
    } else    
    {
        TextView* noController = new TextView(" Please connect a controller", ALIGN_VCENTER | FLAG_WRAP_TEXT, 
                                    true, new LinearLayoutParams(gFileBrowserWidth, f64));
        if (gTheme == THEME_RETRO) 
        {
            noController->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
            noController->SetShadow(true);
        }
        gameLauncherMainFrame->Add(noController);
    }
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
    if ((gUpdateCurrentScreen) || (gUpdateMainScreen)) {
        DEBUG_PRINTF("   void SCREEN_MainScreen::update() if ((gUpdateCurrentScreen) || (gUpdateMainScreen))\n");
        RecreateViews();
        gUpdateCurrentScreen = false;
        gUpdateMainScreen = false;
    }
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelected(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelected(SCREEN_UI::EventParams &e)\n");
    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameHighlight(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameHighlight(SCREEN_UI::EventParams &e)\n");
    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::OnGameSelectedInstant(SCREEN_UI::EventParams &e)\n");
    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::HomeClick(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::HomeClick(SCREEN_UI::EventParams &e)\n");
    ROM_browser->SetPath(getenv("HOME"));

    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::settingsClick(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::settingsClick(SCREEN_UI::EventParams &e)\n");
    screenManager()->push(new SCREEN_SettingsScreen());
    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::offClick(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::offClick(SCREEN_UI::EventParams &e)\n");
    
    auto ma = GetI18NCategory("System");
    auto offClick = new SCREEN_OffDiagScreen(ma->T("Exit Marley?"),OFFDIAG_QUIT);
    if (e.v)
        offClick->SetPopupOrigin(e.v);

    screenManager()->push(offClick);
    
    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_MainScreen::offHold(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_MainScreen::offHold(SCREEN_UI::EventParams &e)\n");
    
    auto ma = GetI18NCategory("System");
    auto offDiag = new SCREEN_OffDiagScreen(ma->T("Switch off computer?"),OFFDIAG_SHUTDOWN);
    if (e.v)
        offDiag->SetPopupOrigin(e.v);

    screenManager()->push(offDiag);
    
    return SCREEN_UI::EVENT_DONE;
}

SCREEN_MainInfoMessage::SCREEN_MainInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp)
    : SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, lp) {
    using namespace SCREEN_UI;
    SetSpacing(0.0f);
    Add(new SCREEN_UI::Spacer(f10));
    text_ = Add(new SCREEN_UI::TextView("", align, false, new LinearLayoutParams(1.0, Margins(0, 10))));
    Add(new SCREEN_UI::Spacer(f10));
}

void SCREEN_MainInfoMessage::Show(const std::string &text, SCREEN_UI::View *refView) {
    if (refView) {
        Bounds b = refView->GetBounds();
        const SCREEN_UI::AnchorLayoutParams *lp = GetLayoutParams()->As<SCREEN_UI::AnchorLayoutParams>();
        if (b.y >= cutOffY_) {
            ReplaceLayoutParams(new SCREEN_UI::AnchorLayoutParams(lp->width, lp->height, lp->left, f80, lp->right, lp->bottom, lp->center));
        } else {
            ReplaceLayoutParams(new SCREEN_UI::AnchorLayoutParams(lp->width, lp->height, lp->left, dp_yres - f80 - f40, lp->right, lp->bottom, lp->center));
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
    text_->SetShadow(false);
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
    void Touch(const SCREEN_TouchInput &input) override {
        SCREEN_UI::Clickable::Touch(input);
        hovering_ = bounds_.Contains(input.x, input.y);
        if (hovering_ && (input.flags & TOUCH_DOWN)) {
            holdStart_ = time_now_d();
        }
        if (input.flags & TOUCH_UP) {
            holdStart_ = 0;
        }
    }

    bool Key(const SCREEN_KeyInput &key) override {
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
    
    SCREEN_ImageID image = SCREEN_ImageID("I_BARREL");

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
            dc.DrawText(text.c_str(), bounds_.x + 152, bounds_.centerY()+2, RETRO_COLOR_FONT_BACKGROUND, ALIGN_VCENTER);
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
    
    bool Key(const SCREEN_KeyInput &key) override {
        std::string searchPath;
        if (key.flags & KEY_DOWN) {
            if (HasFocus() && ((key.keyCode==NKCODE_BUTTON_STRT) || (key.keyCode==NKCODE_SPACE))) {
                DEBUG_PRINTF("   if (HasFocus() && ((key.keyCode==NKCODE_BUTTON_STRT) || (key.keyCode==NKCODE_SPACE)))\n");
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
    SCREEN_ImageID image;
    if (gTheme == THEME_RETRO) image = SCREEN_ImageID("I_FOLDER_R"); else image = SCREEN_ImageID("I_FOLDER");
    if (text == "..") {
        isRegularFolder = false;
        if (gTheme == THEME_RETRO) image = SCREEN_ImageID("I_UP_DIRECTORY_R"); else image = SCREEN_ImageID("I_UP_DIRECTORY");
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
            dc.DrawText(text.c_str(), bounds_.x + 7, bounds_.centerY()+2, RETRO_COLOR_FONT_BACKGROUND, ALIGN_VCENTER);
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
          dc.DrawText(text.c_str(), bounds_.x + 152, bounds_.centerY()+2, RETRO_COLOR_FONT_BACKGROUND, ALIGN_VCENTER);
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
    DEBUG_PRINTF("   SCREEN_GameBrowser::SCREEN_GameBrowser\n");
    Refresh();
}

SCREEN_GameBrowser::~SCREEN_GameBrowser() {
    DEBUG_PRINTF("   SCREEN_GameBrowser::~SCREEN_GameBrowser()\n");
}

void SCREEN_GameBrowser::SetPath(const std::string &path) {
    DEBUG_PRINTF("   void SCREEN_GameBrowser::SetPath(const std::string &path) %s\n",path.c_str());
    path_.SetPath(path);
    Refresh();
}

std::string SCREEN_GameBrowser::GetPath() {
    DEBUG_PRINTF("   std::string SCREEN_GameBrowser::GetPath() \n");
    std::string str = path_.GetPath();
    return str;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_GameBrowser::LayoutChange(SCREEN_UI::EventParams &e)\n");
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
        float dropsize = f30;
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
    DEBUG_PRINTF("   void SCREEN_GameBrowser::Refresh()\n");
    
    lastScale_ = 1.0f;
    lastLayoutWasGrid_ = *gridStyle_;

    // Reset content
    Clear();

    Add(new Spacer(1.0f));
    auto mm = GetI18NCategory("MainMenu");
    
    if (*gridStyle_) {
        gameList_ = new SCREEN_UI::GridLayout(SCREEN_UI::GridLayoutSettings(f150, f85), new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    } else {
        SCREEN_UI::LinearLayout *gl = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
        gl->SetSpacing(f4);
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
        gamesPathView->SetText(path_.GetFriendlyPath().c_str());
        
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
            gameButtons.push_back(new SCREEN_GameButton(strList, *gridStyle_, new SCREEN_UI::LinearLayoutParams(*gridStyle_ == true ? SCREEN_UI::WRAP_CONTENT : SCREEN_UI::FILL_PARENT, f50)));
        }
        
        std::vector<FileInfo> fileInfo;
        path_.GetListing(fileInfo, "");
        for (size_t i = 0; i < fileInfo.size(); i++) {
            if (fileInfo[i].isDirectory) {
                if (browseFlags_ & SCREEN_BrowseFlags::NAVIGATE) {
                    dirButtons.push_back(new SCREEN_DirButtonMain(fileInfo[i].fullName, fileInfo[i].name, *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, f50)));
                }
            }
        }
    }

    if (browseFlags_ & SCREEN_BrowseFlags::NAVIGATE) {
        if (lastGamePath != "/")
        {
            SCREEN_DirButtonMain* UP_button = new SCREEN_DirButtonMain("..", *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, f50));
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
DEBUG_PRINTF("   const std::string SCREEN_GameBrowser::GetBaseName(const std::string &path)\n");
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
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_GameBrowser::NavigateClick(SCREEN_UI::EventParams &e)\n");
    
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
    
    DEBUG_PRINTF("   ###############  launching %s ############### \n",text.c_str());
    launch_request_from_screen_manager = true;
    game_screen_manager = text;

    SCREEN_System_SendMessage("finish", "");

    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_GameBrowser::OnRecentClear(SCREEN_UI::EventParams &e)\n");
    screenManager_->RecreateAllViews();
    return SCREEN_UI::EVENT_DONE;
}

#define TRANSPARENT_BACKGROUND true
void SCREEN_OffDiagScreen::CreatePopupContents(SCREEN_UI::ViewGroup *parent) {
    using namespace SCREEN_UI;

    auto ma = GetI18NCategory("Main");

    LinearLayout *items = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));

    if (gTheme == THEME_RETRO)
    {
        if (m_offDiagEvent == OFFDIAG_QUIT)
        {
            items->Add(new Choice(ma->T("YES"), TRANSPARENT_BACKGROUND, new LayoutParams(f200, f64)))->OnClick.Handle(this, &SCREEN_OffDiagScreen::QuitMarley);
            items->Add(new Choice(ma->T("CANCEL"), TRANSPARENT_BACKGROUND, new LayoutParams(f200, f64)))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
        }
        else
        {
            items->Add(new Choice(ma->T("YES"), TRANSPARENT_BACKGROUND, new LayoutParams(f200, f64)))->OnClick.Handle(this, &SCREEN_OffDiagScreen::SwitchOff);
            items->Add(new Choice(ma->T("CANCEL"), TRANSPARENT_BACKGROUND, new LayoutParams(f200, f64)))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
        }
    } else
    {
        if (m_offDiagEvent == OFFDIAG_QUIT)
        {
            items->Add(new Choice(ma->T("YES"), new LayoutParams(f200, f64)))->OnClick.Handle(this, &SCREEN_OffDiagScreen::QuitMarley);
            items->Add(new Choice(ma->T("CANCEL"), new LayoutParams(f200, f64)))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
        }
        else
        {    
            items->Add(new Choice(ma->T("YES"), new LayoutParams(f200, f64)))->OnClick.Handle(this, &SCREEN_OffDiagScreen::SwitchOff);
            items->Add(new Choice(ma->T("CANCEL"), new LayoutParams(f200, f64)))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
        }
    }

    parent->Add(items);
}

SCREEN_UI::EventReturn SCREEN_OffDiagScreen::SwitchOff(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_OffDiagScreen::SwitchOff(SCREEN_UI::EventParams &e) \n");
    shutdown_now = true;
    SCREEN_System_SendMessage("finish", "");

    return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_OffDiagScreen::QuitMarley(SCREEN_UI::EventParams &e) {
    DEBUG_PRINTF("   SCREEN_UI::EventReturn SCREEN_OffDiagScreen::QuitMarley(SCREEN_UI::EventParams &e) \n");
    shutdown_now = false;
    SCREEN_System_SendMessage("finish", "");

    return SCREEN_UI::EVENT_DONE;
}

SCREEN_ImageID checkControllerType(std::string name, std::string nameDB, bool mappingOK)
{
    if (!mappingOK)
    {
        return SCREEN_ImageID("I_CTRL_NOT_FOUND");
    }
    size_t str_pos1;
    size_t str_pos2;
    size_t str_pos3;
    size_t str_pos4;
    
    //check if SNES
    str_pos1  =   name.find("snes");
    str_pos2 = nameDB.find("snes");
    if ( (str_pos1 != std::string::npos) || ((str_pos2 != std::string::npos)) )
    {
        return SCREEN_ImageID("I_CTRL_SNES");
    } 

    //check if PS2 or PS3
    str_pos1  =   name.find("sony playstation(r)2");
    str_pos2 = nameDB.find("ps2");
    str_pos3 =   name.find("sony playstation(r)3");
    str_pos4 = nameDB.find("ps3");
    if ( (str_pos1 != std::string::npos) || ((str_pos2 != std::string::npos)) || ((str_pos3 != std::string::npos)) || ((str_pos4 != std::string::npos)))
    {
        return SCREEN_ImageID("I_CTRL_PS3");
    } 
    
    //check if XBOX
    str_pos1  =   name.find("box");
    str_pos2 = nameDB.find("box");
    if ( (str_pos1 != std::string::npos) || ((str_pos2 != std::string::npos)) )
    {
        return SCREEN_ImageID("I_CTRL_XB");
    }
    
    //check if PS4
    str_pos1  =   name.find("ps4");
    str_pos2 = nameDB.find("ps4");
    if ( (str_pos1 != std::string::npos) || ((str_pos2 != std::string::npos)) )
    {
        return SCREEN_ImageID("I_CTRL_PS4");
    }
    
    //check if Wiimote
    str_pos1  =   name.find("wiimote");
    str_pos2 = nameDB.find("wiimote");
    if ( (str_pos1 != std::string::npos) || ((str_pos2 != std::string::npos)) )
    {
        return SCREEN_ImageID("I_CTRL_WII");
    }
    
    return SCREEN_ImageID("I_CTRL_GENERIC");
}
