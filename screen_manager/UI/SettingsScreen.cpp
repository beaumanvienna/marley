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

#include "ppsspp_config.h"

#include <algorithm>

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
#include "UI/SettingsScreen.h"
#include "UI/MiscScreens.h"

#include "Common/File/FileUtil.h"
#include "Common/OSVersion.h"
#include "Common/TimeUtil.h"
#include "Common/StringUtils.h"

extern std::string gBaseDir;
extern std::string gPathToFirmwarePS2;
extern bool found_jp_ps2;
extern bool found_na_ps2;
extern bool found_eu_ps2;

#define BIOS_NA 10
#define BIOS_JP 11
#define BIOS_EU 12
#define EMPTY   0 

SCREEN_SettingsScreen::SCREEN_SettingsScreen() {
    found_bios_ps2 = found_jp_ps2 || found_na_ps2 || found_eu_ps2;
    
    if (found_bios_ps2)
    {
        bios_selection[0] = EMPTY;
        bios_selection[1] = EMPTY;
        bios_selection[2] = EMPTY;
        inputBios = 0;

        inputVSync = true;
        inputRes = 1; // UI starts with 0 = native, 1 = 2x native PCSX2
        inputBackend = 1; // OpenGL Hardware + Software
        inputUserHacks = false;

        std::string GSdx_ini = gBaseDir + "PCSX2/inis/GSdx.ini";
        std::string line,str_dec;
        std::string::size_type sz;   // alias of size_t

        //if PCSX2 config file exists get value from there
        std::ifstream GSdx_ini_filehandle(GSdx_ini);
        if (GSdx_ini_filehandle.is_open())
        {
            while ( getline (GSdx_ini_filehandle,line))
            {
                if(line.find("upscale_multiplier") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputRes = std::stoi(str_dec,&sz) - 1;
                } else 
                if(line.find("vsync") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputVSync = true;
                    } else
                    {
                        inputVSync = false;
                    }
                } else 
                if(line.find("UserHacks =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks = true;
                    } else
                    {
                        inputUserHacks = false;
                    }
                } else 
                if(line.find("bios_region") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    int bios_ini_val = std::stoi(str_dec,&sz);
                    if ((bios_ini_val < BIOS_NA) || (bios_ini_val > BIOS_EU))
                    {
                        bios_ini_val = BIOS_NA;
                    }
                    
                    if (found_na_ps2 && found_jp_ps2 && !found_eu_ps2)
                    {

                        bios_selection[0] = BIOS_NA;
                        bios_selection[1] = BIOS_JP;
                        bios_selection[2] = EMPTY;
                        if (bios_ini_val == BIOS_JP) inputBios = 1;

                    } else
                    if (found_na_ps2 && !found_jp_ps2 && found_eu_ps2)
                    {

                        bios_selection[0] = BIOS_NA;
                        bios_selection[1] = BIOS_EU;
                        bios_selection[2] = EMPTY;
                        if (bios_ini_val == BIOS_EU) inputBios = 1;
                        
                    } else
                    if (!found_na_ps2 && found_jp_ps2 && found_eu_ps2)
                    {

                        bios_selection[0] = BIOS_JP;
                        bios_selection[1] = BIOS_EU;
                        bios_selection[2] = EMPTY;
                        if (bios_ini_val == BIOS_EU) inputBios = 1;
                        
                    } else
                    if (found_na_ps2 && found_jp_ps2 && found_eu_ps2)
                    {

                        bios_selection[0] = BIOS_NA;
                        bios_selection[1] = BIOS_JP;
                        bios_selection[2] = BIOS_EU;
                        inputBios = bios_ini_val - BIOS_NA;
                    }
                } else 
                if(line.find("Renderer") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz) == 12) 
                    {
                        inputBackend = 0;
                    } else
                    if(std::stoi(str_dec,&sz) == 13) 
                    {
                        inputBackend = 1;
                    }
                } else
                {
                    GSdx_entries.push_back(line);
                }
            }
            GSdx_ini_filehandle.close();
        }
    }
}
bool createDir(std::string name);
SCREEN_SettingsScreen::~SCREEN_SettingsScreen() {
    if (found_bios_ps2)
    {
        std::string line, str;
        std::string GSdx_ini = gBaseDir + "PCSX2/inis/GSdx.ini";
        std::ofstream GSdx_ini_filehandle;
        
        createDir(gBaseDir + "PCSX2");
        createDir(gBaseDir + "PCSX2/inis");

        GSdx_ini_filehandle.open(GSdx_ini.c_str(), std::ios_base::out); 
        if(GSdx_ini_filehandle)
        {
            for(int i=0; i<GSdx_entries.size(); i++)
            {
                line = GSdx_entries[i];
                GSdx_ini_filehandle << line << "\n";
            }
            GSdx_ini_filehandle << "upscale_multiplier = " <<  inputRes+1 << "\n";
            GSdx_ini_filehandle << "vsync = " << inputVSync << "\n";
            GSdx_ini_filehandle << "Renderer = " <<  inputBackend+12 << "\n";
            if (bios_selection[inputBios] != EMPTY)
            {
                GSdx_ini_filehandle << "bios_region = " <<  bios_selection[inputBios] << "\n";
                if (bios_selection[inputBios] == BIOS_NA)
                {
                    gPathToFirmwarePS2 = gBaseDir + "scph77001.bin";
                } else
                if (bios_selection[inputBios] == BIOS_JP)
                {
                    gPathToFirmwarePS2 = gBaseDir + "scph77000.bin";
                } else
                if (bios_selection[inputBios] == BIOS_EU)
                {
                    gPathToFirmwarePS2 = gBaseDir + "scph77002.bin";
                }
            }
            GSdx_ini_filehandle << "UserHacks = " << inputUserHacks << "\n";
            GSdx_ini_filehandle.close();
        }
    }
}

void SCREEN_SettingsScreen::CreateViews() {
    
	using namespace SCREEN_UI;

	auto di = GetI18NCategory("General");
	auto gr = GetI18NCategory("PCSX2");
    auto ms = GetI18NCategory("Main Settings");

	root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));

	TabHolder *tabHolder;

    LinearLayout *verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
    tabHolder = new TabHolder(ORIENT_HORIZONTAL, 200, new LinearLayoutParams(1.0f));
    verticalLayout->Add(tabHolder);
    verticalLayout->Add(new Choice(di->T("Back"), "", false, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT, 0.0f, Margins(0))))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
    root_->Add(verticalLayout);

	tabHolder->SetTag("GameSettings");
	root_->SetDefaultFocusView(tabHolder);

	float leftSide = 40.0f;

	settingInfo_ = new SCREEN_SettingInfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new AnchorLayoutParams(dp_xres - leftSide - 40.0f, WRAP_CONTENT, leftSide, dp_yres - 80.0f - 40.0f, NONE, NONE));
	settingInfo_->SetBottomCutoff(dp_yres - 200.0f);
	root_->Add(settingInfo_);

	// Graphics
	ViewGroup *graphicsSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
	graphicsSettingsScroll->SetTag("GameSettingsGraphics");
	LinearLayout *graphicsSettings = new LinearLayout(ORIENT_VERTICAL);
	graphicsSettings->SetSpacing(0);
	graphicsSettingsScroll->Add(graphicsSettings);
	tabHolder->AddTab(ms->T("PCSX2"), graphicsSettingsScroll);

    // -------- PCSX2 --------
    if (found_bios_ps2)
    {
        int cnt = 0;
        std::string biosRegions, header;
        if (found_na_ps2) 
        {
            biosRegions += "North America ";
            cnt++;
        }
        if (found_jp_ps2) 
        {
            biosRegions += "Japan ";
            cnt++;
        }
        if (found_eu_ps2) 
        {
            biosRegions += "Europe ";
            cnt++;
        }
        if (cnt > 1) 
        {
            header = "Bios files found for: " + biosRegions;
        } else
        {
            header = "Bios file found for: " + biosRegions;
        }
        
        graphicsSettings->Add(new ItemHeader(gr->T(header)));
        
        // -------- rendering mode --------
        static const char *renderingBackend[] = { "Fastest rendering, lower graphics accuracy", "Fast rendering, high graphics accuracy" };
        
        SCREEN_PopupMultiChoice *renderingBackendChoice = graphicsSettings->Add(new SCREEN_PopupMultiChoice(&inputBackend, gr->T("Backend"), renderingBackend, 0, ARRAY_SIZE(renderingBackend), gr->GetName(), screenManager()));
        renderingBackendChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        
        // -------- bios --------
        if (found_na_ps2 && found_jp_ps2 && !found_eu_ps2)
        {
            bios_selection[0] = BIOS_NA;
            bios_selection[1] = BIOS_JP;
            bios_selection[2] = EMPTY;
            
            static const char *selectBIOS[] = { "North America", "Japan" };
            
            SCREEN_PopupMultiChoice *selectBIOSChoice = graphicsSettings->Add(new SCREEN_PopupMultiChoice(&inputBios, gr->T("Bios selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), gr->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        } else
        if (found_na_ps2 && !found_jp_ps2 && found_eu_ps2)
        {
            bios_selection[0] = BIOS_NA;
            bios_selection[1] = BIOS_EU;
            bios_selection[2] = EMPTY;
            
            static const char *selectBIOS[] = { "North America", "Europe" };

            SCREEN_PopupMultiChoice *selectBIOSChoice = graphicsSettings->Add(new SCREEN_PopupMultiChoice(&inputBios, gr->T("Bios Selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), gr->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        } else
        if (!found_na_ps2 && found_jp_ps2 && found_eu_ps2)
        {
            bios_selection[0] = BIOS_JP;
            bios_selection[1] = BIOS_EU;
            bios_selection[2] = EMPTY;
            
            static const char *selectBIOS[] = { "Japan", "Europe" };
            
            SCREEN_PopupMultiChoice *selectBIOSChoice = graphicsSettings->Add(new SCREEN_PopupMultiChoice(&inputBios, gr->T("Bios Selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), gr->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        } else
        if (found_na_ps2 && found_jp_ps2 && found_eu_ps2)
        {
            bios_selection[0] = BIOS_NA;
            bios_selection[1] = BIOS_JP;
            bios_selection[2] = BIOS_EU;
            
            static const char *selectBIOS[] = { "North America", "Japan", "Europe" };

            SCREEN_PopupMultiChoice *selectBIOSChoice = graphicsSettings->Add(new SCREEN_PopupMultiChoice(&inputBios, gr->T("Bios Selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), gr->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        }

        // -------- resolution --------
        static const char *selectResolution[] = { "Native PS2", "720p", "1080p", "1440p 2K", "1620p 3K", "2160p 4K" };
        
        SCREEN_PopupMultiChoice *selectResolutionChoice = graphicsSettings->Add(new SCREEN_PopupMultiChoice(&inputRes, gr->T("Resolution"), selectResolution, 0, ARRAY_SIZE(selectResolution), gr->GetName(), screenManager()));
        selectResolutionChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        
        // -------- vsync --------
        CheckBox *vSync = graphicsSettings->Add(new CheckBox(&inputVSync, gr->T("Disable screen tearing", "Disable screen tearing (VSync)")));
        vSync->OnClick.Add([=](EventParams &e) {
            return SCREEN_UI::EVENT_CONTINUE;
        });
        
        // -------- user hacks --------
        CheckBox *vUserHacks = graphicsSettings->Add(new CheckBox(&inputUserHacks, gr->T("Enable user hacks", "Enable user hacks")));
        vUserHacks->OnClick.Add([=](EventParams &e) {
            return SCREEN_UI::EVENT_CONTINUE;
        });
        
        if (inputUserHacks)
        {
            graphicsSettings->Add(new ItemHeader(gr->T("User hacks")));
            
            CheckBox *vUserHacks_AutoFlush = graphicsSettings->Add(new CheckBox(&inputUserHacks_AutoFlush, gr->T("Enable 'auto flush'", "Enable 'auto flush'")));
            vUserHacks_AutoFlush->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_CPU_FB_Conversion = graphicsSettings->Add(new CheckBox(&inputUserHacks_CPU_FB_Conversion, gr->T("Enable 'CPU framebuffer conversion'", "Enable 'CPU framebuffer conversion'")));
            vUserHacks_CPU_FB_Conversion->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_DisableDepthSupport = graphicsSettings->Add(new CheckBox(&inputUserHacks_DisableDepthSupport, gr->T("Enable 'no depth support'", "Enable 'no depth support'")));
            vUserHacks_DisableDepthSupport->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_DisablePartialInvalidation = graphicsSettings->Add(new CheckBox(&inputUserHacks_DisablePartialInvalidation, gr->T("Enable 'no partial invalidation'", "Enable 'no partial invalidation'")));
            vUserHacks_DisablePartialInvalidation->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_Disable_Safe_Features = graphicsSettings->Add(new CheckBox(&inputUserHacks_Disable_Safe_Features, gr->T("Enable 'no safe features'", "Enable 'no safe features'")));
            vUserHacks_Disable_Safe_Features->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_HalfPixelOffset = graphicsSettings->Add(new CheckBox(&inputUserHacks_HalfPixelOffset, gr->T("Enable 'half pixel offset'", "Enable 'half pixel offset'")));
            vUserHacks_HalfPixelOffset->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
        }
    } else
    {
        graphicsSettings->Add(new ItemHeader(gr->T("PCSX2: No bios files found. Set up a path to a PS2 bios under 'Main screen/Setup'.")));
    }
	SCREEN_Draw::SCREEN_DrawContext *draw = screenManager()->getSCREEN_DrawContext();

}

SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnRenderingBackend(SCREEN_UI::EventParams &e) {

	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_SettingsScreen::onFinish(DialogResult result) {
    SCREEN_System_SendMessage("finish", "");
}

void SCREEN_SettingsScreen::update() {
	SCREEN_UIScreen::update();

	bool vertical = true;
	if (vertical != lastVertical_) {
		RecreateViews();
		lastVertical_ = vertical;
	}
}


SCREEN_SettingInfoMessage::SCREEN_SettingInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp)
	: SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, lp) {
	using namespace SCREEN_UI;
	SetSpacing(0.0f);
	Add(new SCREEN_UI::Spacer(10.0f));
	text_ = Add(new SCREEN_UI::TextView("", align, false, new LinearLayoutParams(1.0, Margins(0, 10))));
	Add(new SCREEN_UI::Spacer(10.0f));
}

void SCREEN_SettingInfoMessage::Show(const std::string &text, SCREEN_UI::View *refView) {
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

void SCREEN_SettingInfoMessage::Draw(SCREEN_UIContext &dc) {
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
