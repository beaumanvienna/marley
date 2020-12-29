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
#include "UI/SettingsScreen.h"
#include "UI/MiscScreens.h"
#include "UI/MainScreen.h"
#include <SDL.h>
#include "../include/controller.h"
#include "../include/statemachine.h"

#define BIOS_NA 10
#define BIOS_JP 11
#define BIOS_EU 12
#define EMPTY   0 

#define BACKEND_OPENGL_HARDWARE 0
#define BACKEND_OPENGL_HARDWARE_PLUS_SOFTWARE 1

void SCREEN_UIThemeInit();

bool addSearchPathToConfigFile(std::string searchPath);
bool searchAllFolders(void);
void UISetBackground(SCREEN_UIContext &dc,std::string bgPng);
void DrawBackground(SCREEN_UIContext &dc, float alpha);
void DrawBackgroundSimple(SCREEN_UIContext &dc, int page);
ImageID checkControllerType(std::string name, std::string nameDB);

extern std::string gBaseDir;
extern std::string gPathToFirmwarePS2;
extern bool found_jp_ps1;
extern bool found_na_ps1;
extern bool found_eu_ps1;
extern bool found_jp_ps2;
extern bool found_na_ps2;
extern bool found_eu_ps2;
extern bool gSegaSaturn_firmware;
extern bool stopSearching;
extern std::vector<std::string> gGame;
extern bool gGamesFound;
extern bool gPS1_firmware;
extern bool gPS2_firmware;
extern std::vector<std::string> gSearchDirectoriesGames;
extern double FILE_BROWSER_WIDTH;
extern bool gControllerConf;

bool bGridView1;
bool bGridView2=true;
bool searchDirAdded;
std::string currentSearchPath;
bool playSystemSounds;
int gTheme = THEME_RETRO;
SCREEN_SettingsInfoMessage *settingsInfo_;
std::string tooltipFileBrowser;
bool updateControllerText;

void setControllerConfText(std::string text)
{
    updateControllerText = true;
    gConfText = text;
}

int calcExtraThreadsPCSX2()
{
    int cnt = SDL_GetCPUCount() -2;
    if (cnt < 2) cnt = 0; // 1 is debugging that we do not want, negative values set to 0
    if (cnt > 7) cnt = 7; // limit to 1 main thread and 7 extra threads
    
    return cnt;
}

bool SCREEN_SettingsScreen::key(const KeyInput &key)
{
    if (gControllerConf)
    {
        // skip button
        if ((key.flags & KEY_DOWN) && (key.keyCode==NKCODE_ENTER))
        {
            statemachineConf(STATE_CONF_SKIP_ITEM);
        }
        
        return true;
    } 
    return SCREEN_UIDialogScreenWithBackground::key(key);
}

SCREEN_SettingsScreen::SCREEN_SettingsScreen() 
{
    resetStatemachine();
    searchDirAdded=false;
    printf("jc: SCREEN_SettingsScreen::SCREEN_SettingsScreen() \n");
    
    // General
    std::string marley_cfg = gBaseDir + "marley.cfg";
    std::string line,str_dec;
    std::string::size_type sz;   // alias of size_t

    //if marley.cfg exists get value from there
    std::ifstream marley_cfg_filehandle(marley_cfg);
    if (marley_cfg_filehandle.is_open())
    {
        while ( getline (marley_cfg_filehandle,line))
        {
            if(line.find("system_sounds") != std::string::npos)
            {
                playSystemSounds = (line.find("true") != std::string::npos);
            } 
            else if(line.find("ui_theme") != std::string::npos)
            {
                if (line.find("PC") != std::string::npos)
                {
                    gTheme = THEME_PLAIN;
                } else
                {
                    gTheme = THEME_RETRO;
                }
            }
            marley_cfg_entries.push_back(line);
        }
        marley_cfg_filehandle.close();
    }
    
    // Dolphin
    inputVSyncDolphin = true;
    inputResDolphin = 1; // UI starts with 0, dolphin has 1 = native, 2 = 2x native
    
    std::string GFX_ini = gBaseDir + "dolphin-emu/Config/GFX.ini";

    //if GFX.ini exists get value from there
    std::ifstream GFX_ini_filehandle(GFX_ini);
    if (GFX_ini_filehandle.is_open())
    {
        while ( getline (GFX_ini_filehandle,line))
        {
            if(line.find("InternalResolution =") != std::string::npos)
            {
                str_dec = line.substr(line.find_last_of("=") + 1);
                inputResDolphin = std::stoi(str_dec,&sz) - 1;
            } else 
            if(line.find("VSync =") != std::string::npos)
            {
                inputVSyncDolphin = (line.find("True") != std::string::npos);
            }
            GFX_entries.push_back(line);
        }
        GFX_ini_filehandle.close();
    }
    
    // PCSX2
    found_bios_ps2 = found_jp_ps2 || found_na_ps2 || found_eu_ps2;
    
    if (found_bios_ps2)
    {
        bios_selection[0] = EMPTY;
        bios_selection[1] = EMPTY;
        bios_selection[2] = EMPTY;
        inputBios = 0;

        inputVSyncPCSX2 = true;
        inputResPCSX2 = 1; // UI starts with 0 = native, 1 = 2x native PCSX2
        inputBackend = BACKEND_OPENGL_HARDWARE_PLUS_SOFTWARE; // OpenGL Hardware + Software
        
        inputExtrathreads_sw = calcExtraThreadsPCSX2();
        if (inputExtrathreads_sw > 1) inputExtrathreads_sw = inputExtrathreads_sw -1;
        
        inputAdvancedSettings = false;
        
        inputInterlace = 7;
        inputBiFilter = 2;
        inputAnisotropy = 0;
        inputDithering = 2;
        inputHW_mipmapping = 0;
        inputCRC_level = 0;
        inputAcc_date_level = 1;
        inputAcc_blend_level = 1;
        
        inputUserHacks = false;
        inputUserHacks_AutoFlush = false;
        inputUserHacks_CPU_FB_Conversion = false;
        inputUserHacks_DisableDepthSupport = false;
        inputUserHacks_DisablePartialInvalidation = false;
        inputUserHacks_Disable_Safe_Features = false;
        inputUserHacks_HalfPixelOffset = false;
        inputUserHacks_Half_Bottom_Override = -1;
        inputUserHacks_SkipDraw = false;
        inputUserHacks_SkipDraw_Offset = false;
        inputUserHacks_TCOffsetX = false;
        inputUserHacks_TCOffsetY = false;
        inputUserHacks_TextureInsideRt = false;
        inputUserHacks_TriFilter = 0;
        inputUserHacks_WildHack = false;
        inputUserHacks_align_sprite_X = false;
        inputUserHacks_merge_pp_sprite = false;
        inputUserHacks_round_sprite_offset = false;
        inputAutoflush_sw = true;
        inputMipmapping_sw = true;
        inputInterpreter = false;

        std::string GSdx_ini = gBaseDir + "PCSX2/inis/GSdx.ini";
        
        //if GSdx.ini exists get value from there
        std::ifstream GSdx_ini_filehandle(GSdx_ini);
        if (GSdx_ini_filehandle.is_open())
        {
            while ( getline (GSdx_ini_filehandle,line))
            {
                if(line.find("upscale_multiplier =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputResPCSX2 = std::stoi(str_dec,&sz) - 1;
                } else 
                if(line.find("extrathreads =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputExtrathreads_sw = std::stoi(str_dec,&sz) ;
                    if (inputExtrathreads_sw > 1) inputExtrathreads_sw = inputExtrathreads_sw -1;
                } else 
                if(line.find("mipmap =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputMipmapping_sw = true;
                    } else
                    {
                        inputMipmapping_sw = false;
                    }
                } else 
                if(line.find("advanced_settings =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputAdvancedSettings = true;
                    } else
                    {
                        inputAdvancedSettings = false;
                    }
                } else 
                if(line.find("aa1 ") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputAnti_aliasing_sw = true;
                    } else
                    {
                        inputAnti_aliasing_sw = false;
                    }
                } else 
                if(line.find("autoflush_sw =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputAutoflush_sw = true;
                    } else
                    {
                        inputAutoflush_sw = false;
                    }
                } else 
                if(line.find("vsync =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputVSyncPCSX2 = true;
                    } else
                    {
                        inputVSyncPCSX2 = false;
                    }
                } else 
                if(line.find("interlace =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputInterlace = std::stoi(str_dec,&sz) ;
                } else 
                if(line.find("filter =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputBiFilter = std::stoi(str_dec,&sz) ;
                } else 
                if(line.find("MaxAnisotropy =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputAnisotropy = std::stoi(str_dec,&sz) ;
                    switch(inputAnisotropy)
                    {
                        case 2:
                            inputAnisotropy = 1;
                            break;
                        case 4:
                            inputAnisotropy = 2;
                            break;
                        case 8:
                            inputAnisotropy = 3;
                            break;
                        case 16:
                            inputAnisotropy = 4;
                            break;
                        default:
                            inputAnisotropy = 0;
                            break;
                    }
                } else 
                if(line.find("dithering_ps2 =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputDithering = std::stoi(str_dec,&sz) ;
                } else 
                if(line.find("mipmap_hw =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputHW_mipmapping = std::stoi(str_dec,&sz) +1;
                } else 
                if(line.find("crc_hack_level =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputCRC_level = std::stoi(str_dec,&sz) +1;
                } else 
                if(line.find("accurate_date =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputAcc_date_level = std::stoi(str_dec,&sz);
                } else 
                if(line.find("accurate_blending_unit =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputAcc_blend_level = std::stoi(str_dec,&sz);
                } else 
                if(line.find("UserHacks =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks = true;
                    } else
                    {
                        inputUserHacks = false;
                    }
                } else 
                if(line.find("UserHacks_AutoFlush =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_AutoFlush = true;
                    } else
                    {
                        inputUserHacks_AutoFlush = false;
                    }
                } else 
                if(line.find("UserHacks_CPU_FB_Conversion =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_CPU_FB_Conversion = true;
                    } else
                    {
                        inputUserHacks_CPU_FB_Conversion = false;
                    }
                } else 
                if(line.find("Hacks_DisableDepthSupport =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_DisableDepthSupport = true;
                    } else
                    {
                        inputUserHacks_DisableDepthSupport = false;
                    }
                } else 
                if(line.find("UserHacks_DisablePartialInvalidation =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_DisablePartialInvalidation = true;
                    } else
                    {
                        inputUserHacks_DisablePartialInvalidation = false;
                    }
                } else 
                if(line.find("UserHacks_Disable_Safe_Features =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_Disable_Safe_Features = true;
                    } else
                    {
                        inputUserHacks_Disable_Safe_Features = false;
                    }
                } else 
                if(line.find("UserHacks_HalfPixelOffset =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_HalfPixelOffset = true;
                    } else
                    {
                        inputUserHacks_HalfPixelOffset = false;
                    }
                } else 
                if(line.find("UserHacks_Half_Bottom_Override =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputUserHacks_Half_Bottom_Override = std::stoi(str_dec,&sz) + 1;
                } else 
                if(line.find("UserHacks_SkipDraw =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_SkipDraw = true;
                    } else
                    {
                        inputUserHacks_SkipDraw = false;
                    }
                } else 
                if(line.find("UserHacks_SkipDraw_Offset =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_SkipDraw_Offset = true;
                    } else
                    {
                        inputUserHacks_SkipDraw_Offset = false;
                    }
                } else 
                if(line.find("UserHacks_TCOffsetX =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_TCOffsetX = true;
                    } else
                    {
                        inputUserHacks_TCOffsetX = false;
                    }
                } else 
                if(line.find("UserHacks_TCOffsetY =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_TCOffsetY = true;
                    } else
                    {
                        inputUserHacks_TCOffsetY = false;
                    }
                } else 
                if(line.find("UserHacks_TextureInsideRt =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_TextureInsideRt = true;
                    } else
                    {
                        inputUserHacks_TextureInsideRt = false;
                    }
                } else 
                if(line.find("UserHacks_TriFilter =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    inputUserHacks_TriFilter = std::stoi(str_dec,&sz);
                } else 
                if(line.find("UserHacks_WildHack =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_WildHack = true;
                    } else
                    {
                        inputUserHacks_WildHack = false;
                    }
                } else 
                if(line.find("UserHacks_align_sprite_X =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_align_sprite_X = true;
                    } else
                    {
                        inputUserHacks_align_sprite_X = false;
                    }
                } else 
                if(line.find("UserHacks_merge_pp_sprite =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_merge_pp_sprite = true;
                    } else
                    {
                        inputUserHacks_merge_pp_sprite = false;
                    }
                } else 
                if(line.find("UserHacks_round_sprite_offset =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz)) 
                    {
                        inputUserHacks_round_sprite_offset = true;
                    } else
                    {
                        inputUserHacks_round_sprite_offset = false;
                    }
                } else 
                if(line.find("bios_region =") != std::string::npos)
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
                if(line.find("Renderer =") != std::string::npos)
                {
                    str_dec = line.substr(line.find_last_of("=") + 1);
                    if(std::stoi(str_dec,&sz) == 12) 
                    {
                        inputBackend = BACKEND_OPENGL_HARDWARE;
                    } else
                    if(std::stoi(str_dec,&sz) == 13) 
                    {
                        inputBackend = BACKEND_OPENGL_HARDWARE_PLUS_SOFTWARE;
                    }
                } else
                {
                    GSdx_entries.push_back(line);
                }
            }
            GSdx_ini_filehandle.close();
        }
        
        //read PCSX2_vm into buffer
        std::string PCSX2_vm_ini = gBaseDir + "PCSX2/inis/PCSX2_vm.ini";
        std::ifstream PCSX2_vm_ini_filehandle(PCSX2_vm_ini);
        if (PCSX2_vm_ini_filehandle.is_open())
        {
            while ( getline (PCSX2_vm_ini_filehandle,line))
            {
                PCSX2_vm_entries.push_back(line);
                
                if(line.find("EnableEE=") != std::string::npos)
                {
                    if(line.find("disabled") != std::string::npos)
                    {
                        inputInterpreter = true;
                    } 
                    else
                    {
                        inputInterpreter = false;
                    }
                }
            }
            PCSX2_vm_ini_filehandle.close();
        }
    }
}
bool createDir(std::string name);
SCREEN_SettingsScreen::~SCREEN_SettingsScreen() 
{
    printf("jc: SCREEN_SettingsScreen::~SCREEN_SettingsScreen() \n");
    std::string str, line;
    
    std::string marley_cfg = gBaseDir + "marley.cfg";
    std::ofstream marley_cfg_filehandle;
    bool found_system_sounds = false;
    bool found_ui_theme = false;
    // output marley.cfg
    marley_cfg_filehandle.open(marley_cfg.c_str(), std::ios_base::out); 
    if(marley_cfg_filehandle)
    {
        if (marley_cfg_entries.size())
        {
            for(int i=0; i<marley_cfg_entries.size(); i++)
            {
                line = marley_cfg_entries[i];
                
                if(line.find("system_sounds") != std::string::npos)
                {
                    found_system_sounds = true;
                    if (playSystemSounds)
                    {
                        marley_cfg_filehandle << "system_sounds=true\n";
                    }
                    else
                    {
                        marley_cfg_filehandle << "system_sounds=false\n";
                    }
                } 
                else if(line.find("ui_theme") != std::string::npos)
                {
                    found_ui_theme = true;
                    if (gTheme == THEME_PLAIN)
                    {
                        marley_cfg_filehandle << "ui_theme=PC\n";
                    }
                    else
                    {
                        marley_cfg_filehandle << "ui_theme=Retro\n";
                    }
                } else
                {
                    if (line.find("search_dir_games=") == std::string::npos)
                    {
                        marley_cfg_filehandle << marley_cfg_entries[i] << "\n";
                    }
                }
            }
        }
        if (!found_system_sounds)
        {
            if (playSystemSounds)
            {
                marley_cfg_filehandle << "system_sounds=true\n";
            }
            else
            {
                marley_cfg_filehandle << "system_sounds=false\n";
            }
        }
        if (!found_ui_theme)
        {
            if (gTheme == THEME_PLAIN)
            {
                marley_cfg_filehandle << "ui_theme=PC\n";
            }
            else
            {
                marley_cfg_filehandle << "ui_theme=Retro\n";
            }
        }
        for (int i=0;i<gSearchDirectoriesGames.size();i++)
        {
            marley_cfg_filehandle << "search_dir_games=" << gSearchDirectoriesGames[i] << "\n";
        }
        
        marley_cfg_filehandle.close();
    }
    
    std::string GFX_ini = gBaseDir + "dolphin-emu/Config/GFX.ini";
    std::ofstream GFX_ini_filehandle;
    
    createDir(gBaseDir + "dolphin-emu");
    createDir(gBaseDir + "dolphin-emu/Config");
    
    // output GFX.ini
    GFX_ini_filehandle.open(GFX_ini.c_str(), std::ios_base::out); 
    if(GFX_ini_filehandle)
    {
        if (GFX_entries.size())
        {
            for(int i=0; i<GFX_entries.size(); i++)
            {
                line = GFX_entries[i];
                if(line.find("InternalResolution =") != std::string::npos)
                {
                    GFX_ini_filehandle << "InternalResolution = ";
                    GFX_ini_filehandle <<  inputResDolphin+1;
                    GFX_ini_filehandle << "\n";
                } else 
                if(line.find("VSync =") != std::string::npos)
                {
                    GFX_ini_filehandle << "VSync = ";
                    if (inputVSyncDolphin)
                    {
                        GFX_ini_filehandle << "True\n";
                    }
                    else
                    {
                        GFX_ini_filehandle << "False\n";
                    }
                } else
                {
                    GFX_ini_filehandle << GFX_entries[i] << "\n";
                }
            }
        }
        else
        {
            GFX_ini_filehandle << "[Enhancements]\n";
            GFX_ini_filehandle << "ArbitraryMipmapDetection = True\n";
            GFX_ini_filehandle << "DisableCopyFilter = True\n";
            GFX_ini_filehandle << "ForceTrueColor = True\n";
            GFX_ini_filehandle << "[Hacks]\n";
            GFX_ini_filehandle << "BBoxEnable = False\n";
            GFX_ini_filehandle << "DeferEFBCopies = True\n";
            GFX_ini_filehandle << "EFBEmulateFormatChanges = False\n";
            GFX_ini_filehandle << "EFBScaledCopy = True\n";
            GFX_ini_filehandle << "EFBToTextureEnable = True\n";
            GFX_ini_filehandle << "SkipDuplicateXFBs = True\n";
            GFX_ini_filehandle << "XFBToTextureEnable = True\n";
            GFX_ini_filehandle << "[Hardware]\n";
            if (inputVSyncDolphin)
            {
                GFX_ini_filehandle << "VSync = True\n";
            }
            else
            {
                GFX_ini_filehandle << "VSync = False\n";
            }
            GFX_ini_filehandle << "[Settings]\n";
            GFX_ini_filehandle << "BackendMultithreading = True\n";
            GFX_ini_filehandle << "DumpBaseTextures = True\n";
            GFX_ini_filehandle << "DumpMipTextures = True\n";
            GFX_ini_filehandle << "FastDepthCalc = True\n";
            GFX_ini_filehandle << "InternalResolution = ";
            GFX_ini_filehandle <<  inputResDolphin+1;
            GFX_ini_filehandle << "\n";
            GFX_ini_filehandle << "SaveTextureCacheToState = True\n";
        }
        GFX_ini_filehandle.close();
    }


    if (found_bios_ps2)
    {
        std::string GSdx_ini = gBaseDir + "PCSX2/inis/GSdx.ini";
        std::ofstream GSdx_ini_filehandle;
        std::string PCSX2_vm_ini = gBaseDir + "PCSX2/inis/PCSX2_vm.ini";
        std::ofstream PCSX2_vm_ini_filehandle;
        
        createDir(gBaseDir + "PCSX2");
        createDir(gBaseDir + "PCSX2/inis");
        
        // output GSdx.ini
        GSdx_ini_filehandle.open(GSdx_ini.c_str(), std::ios_base::out); 
        if(GSdx_ini_filehandle)
        {
            for(int i=0; i<GSdx_entries.size(); i++)
            {
                GSdx_ini_filehandle << GSdx_entries[i] << "\n";
            }
            GSdx_ini_filehandle << "upscale_multiplier = " <<  inputResPCSX2+1 << "\n";
            GSdx_ini_filehandle << "vsync = " << inputVSyncPCSX2 << "\n";
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
            // advanced settings
            GSdx_ini_filehandle << "advanced_settings = " << inputAdvancedSettings << "\n";
            GSdx_ini_filehandle << "interlace = " << inputInterlace << "\n";
            GSdx_ini_filehandle << "filter = " << inputBiFilter << "\n";

            switch(inputAnisotropy)
            {
                case 1:
                    GSdx_ini_filehandle << "MaxAnisotropy = 2\n";
                    break;
                case 2:
                    GSdx_ini_filehandle << "MaxAnisotropy = 4\n";
                    break;
                case 3:
                    GSdx_ini_filehandle << "MaxAnisotropy = 8\n";
                    break;
                case 4:
                    GSdx_ini_filehandle << "MaxAnisotropy = 16\n";
                    break;
                default:
                    GSdx_ini_filehandle << "MaxAnisotropy = 0\n";
                    break;
            }            
            
            GSdx_ini_filehandle << "dithering_ps2 = " << inputDithering << "\n";
            GSdx_ini_filehandle << "mipmap_hw = " << inputHW_mipmapping - 1 << "\n";
            GSdx_ini_filehandle << "crc_hack_level = " << inputCRC_level - 1 << "\n";
            GSdx_ini_filehandle << "accurate_date = " << inputAcc_date_level << "\n";
            GSdx_ini_filehandle << "accurate_blending_unit = " << inputAcc_blend_level << "\n";

            // user hacks
            GSdx_ini_filehandle << "UserHacks = " << inputUserHacks << "\n";
            GSdx_ini_filehandle << "UserHacks_AutoFlush = " << inputUserHacks_AutoFlush << "\n";
            GSdx_ini_filehandle << "UserHacks_CPU_FB_Conversion = " << inputUserHacks_CPU_FB_Conversion << "\n";
            GSdx_ini_filehandle << "UserHacks_DisableDepthSupport = " << inputUserHacks_DisableDepthSupport << "\n";
            GSdx_ini_filehandle << "UserHacks_DisablePartialInvalidation = " << inputUserHacks_DisablePartialInvalidation << "\n";
            GSdx_ini_filehandle << "UserHacks_Disable_Safe_Features = " << inputUserHacks_Disable_Safe_Features << "\n";
            GSdx_ini_filehandle << "UserHacks_HalfPixelOffset = " << inputUserHacks_HalfPixelOffset << "\n";
            GSdx_ini_filehandle << "UserHacks_Half_Bottom_Override = " << inputUserHacks_Half_Bottom_Override -1 << "\n";
            GSdx_ini_filehandle << "UserHacks_SkipDraw = " << inputUserHacks_SkipDraw << "\n";
            GSdx_ini_filehandle << "UserHacks_SkipDraw_Offset = " << inputUserHacks_SkipDraw_Offset << "\n";
            GSdx_ini_filehandle << "UserHacks_TCOffsetX = " << inputUserHacks_TCOffsetX << "\n";
            GSdx_ini_filehandle << "UserHacks_TCOffsetY = " << inputUserHacks_TCOffsetY << "\n";
            GSdx_ini_filehandle << "UserHacks_TCOffsetY = " << inputUserHacks_TCOffsetY << "\n";
            GSdx_ini_filehandle << "UserHacks_TriFilter = " << inputUserHacks_TriFilter << "\n";
            GSdx_ini_filehandle << "UserHacks_WildHack = " << inputUserHacks_WildHack << "\n";
            GSdx_ini_filehandle << "UserHacks_align_sprite_X = " << inputUserHacks_align_sprite_X << "\n";
            GSdx_ini_filehandle << "UserHacks_merge_pp_sprite = " << inputUserHacks_merge_pp_sprite << "\n";
            GSdx_ini_filehandle << "UserHacks_round_sprite_offset = " << inputUserHacks_round_sprite_offset << "\n";
            GSdx_ini_filehandle << "UserHacks_TextureInsideRt = " << inputUserHacks_TextureInsideRt << "\n";

            // settings for software rendering
            GSdx_ini_filehandle << "autoflush_sw = " << inputAutoflush_sw << "\n";
            GSdx_ini_filehandle << "mipmap = " << inputMipmapping_sw << "\n";
            if (inputExtrathreads_sw == 0)
            {
                GSdx_ini_filehandle << "extrathreads = 0" << "\n";
            } else
            {
                GSdx_ini_filehandle << "extrathreads = " << inputExtrathreads_sw + 1<< "\n";
            }
            GSdx_ini_filehandle << "aa1 = " << inputAnti_aliasing_sw << "\n";

            GSdx_ini_filehandle.close();
        }

        // output PCSX2_vm.ini
        PCSX2_vm_ini_filehandle.open(PCSX2_vm_ini.c_str(), std::ios_base::out); 
        if(PCSX2_vm_ini_filehandle)
        {
            for(int i=0; i<PCSX2_vm_entries.size(); i++)
            {
                line = PCSX2_vm_entries[i];
                if(line.find("VsyncEnable") != std::string::npos)
                {
                    PCSX2_vm_ini_filehandle << "VsyncEnable=" << inputVSyncPCSX2 << "\n";
                } else if(line.find("EnableEE=") != std::string::npos)
                {
                    if(inputInterpreter)
                    {
                        PCSX2_vm_ini_filehandle << "EnableEE=disabled\n";
                    } 
                    else
                    {
                        PCSX2_vm_ini_filehandle << "EnableEE=enabled\n";
                    }
                } else if(line.find("EnableIOP=") != std::string::npos)
                {
                    if(inputInterpreter)
                    {
                        PCSX2_vm_ini_filehandle << "EnableIOP=disabled\n";
                    } 
                    else
                    {
                        PCSX2_vm_ini_filehandle << "EnableIOP=enabled\n";
                    }
                } else if(line.find("EnableVU0=") != std::string::npos)
                {
                    if(inputInterpreter)
                    {
                        PCSX2_vm_ini_filehandle << "EnableVU0=disabled\n";
                    } 
                    else
                    {
                        PCSX2_vm_ini_filehandle << "EnableVU0=enabled\n";
                    }
                } else if(line.find("EnableVU1=") != std::string::npos)
                {
                    if(inputInterpreter)
                    {
                        PCSX2_vm_ini_filehandle << "EnableVU1=disabled\n";
                    } 
                    else
                    {
                        PCSX2_vm_ini_filehandle << "EnableVU1=enabled\n";
                    }
                } else
                {
                    PCSX2_vm_ini_filehandle << line << "\n";
                }
            }
            PCSX2_vm_ini_filehandle.close();
        }
    }
}

enum settings_tabs { 
    SETTINGS_SEARCH=0,
    SETTINGS_CONTROLLER,
    SETTINGS_DOLPHIN,
    SETTINGS_PCSX2,
    SETTINGS_GENERAL,
    SETTINGS_INFO,
};

void SCREEN_SettingsScreen::DrawBackground(SCREEN_UIContext &dc) {
    std::string bgPng; 
    int tab = tabHolder->GetCurrentTab();
    switch(tab)
    {
        case SETTINGS_PCSX2:
            bgPng = gBaseDir + "screen_manager/settings_pcsx2.png";
            UISetBackground(dc,bgPng);
            ::DrawBackground(dc,1.0f);
            break;
        case SETTINGS_DOLPHIN:
            bgPng = gBaseDir + "screen_manager/settings_dolphin.png";
            UISetBackground(dc,bgPng);
            ::DrawBackgroundSimple(dc, SCREEN_DOLPHIN);
            break;
        case SETTINGS_GENERAL:
            bgPng = gBaseDir + "screen_manager/settings_general.png";
            UISetBackground(dc,bgPng);
            ::DrawBackgroundSimple(dc, SCREEN_GENERAL);
            break;
        case SETTINGS_SEARCH:
        case SETTINGS_INFO:
        default:
            bgPng = gBaseDir + "screen_manager/settings_general.png";
            UISetBackground(dc,bgPng);
            ::DrawBackgroundSimple(dc, SCREEN_GENERIC);
            break;
    }
}

SCREEN_UI::TextView* biosInfo(std::string infoText, bool biosFound)
{
    uint32_t warningColor = 0xFF000000;
    uint32_t okColor = 0xFF006400;
    SCREEN_UI::TextView* bios_found_info;
    if (biosFound) 
    {
        bios_found_info = new SCREEN_UI::TextView("            " + infoText + ": found", ALIGN_VCENTER | FLAG_WRAP_TEXT, 
                    false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, 32.0f, 1.0f));
        bios_found_info->SetTextColor(okColor);
    }
    else
    {
        bios_found_info = new SCREEN_UI::TextView("            " + infoText + ": not found", ALIGN_VCENTER | FLAG_WRAP_TEXT, 
                    false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, 32.0f, 1.0f));
        bios_found_info->SetTextColor(warningColor);
    }
    bios_found_info->SetShadow(false);
    return bios_found_info;
}

void SCREEN_SettingsScreen::CreateViews() {
	using namespace SCREEN_UI;
    
    printf("jc: void SCREEN_SettingsScreen::CreateViews() {\n");

	auto ge = GetI18NCategory("Search");
	auto ps2 = GetI18NCategory("PCSX2");
    auto dol = GetI18NCategory("Dolphin");

	root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));

    LinearLayout *verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
    tabHolder = new TabHolder(ORIENT_HORIZONTAL, 200, new LinearLayoutParams(1.0f));
    verticalLayout->Add(tabHolder);
    
    ImageID icon;
    if (gTheme == THEME_RETRO) icon = ImageID("I_BACK_R"); else icon = ImageID("I_BACK");
    verticalLayout->Add(new Choice(icon, new LayoutParams(64.0f, 64.0f)))->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);

    root_->Add(verticalLayout);

	tabHolder->SetTag("Settings");
	root_->SetDefaultFocusView(tabHolder);

    // info message
	float leftSide = 40.0f;

	settingsInfo_ = new SCREEN_SettingsInfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new AnchorLayoutParams(dp_xres - leftSide - 40.0f, WRAP_CONTENT, leftSide, dp_yres - 80.0f - 40.0f, NONE, NONE));
	settingsInfo_->SetBottomCutoff(dp_yres - 200.0f);
	root_->Add(settingsInfo_);

    // horizontal layout for margins
    LinearLayout *horizontalLayoutSearch = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(dp_xres - 40.f, FILL_PARENT));
    tabHolder->AddTab(ge->T("Search"), horizontalLayoutSearch);
    horizontalLayoutSearch->Add(new Spacer(10.0f));
    
	// -------- search --------
    ViewGroup *searchSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(dp_xres - 40.f, FILL_PARENT));
    horizontalLayoutSearch->Add(searchSettingsScroll);
	searchSettingsScroll->SetTag("SearchSettings");
	LinearLayout *searchSettings = new LinearLayout(ORIENT_VERTICAL);
	searchSettings->SetSpacing(0);
	searchSettingsScroll->Add(searchSettings);
	

    // bios file browser
    
    searchDirBrowser = new SCREEN_DirBrowser(getenv("HOME"), SCREEN_BrowseFlags::STANDARD, &bGridView2, screenManager(),
        ge->T("Use the Start button to confirm"), "https://github.com/beaumanvienna/marley",
        new LinearLayoutParams(FILL_PARENT, FILL_PARENT));
    searchSettings->Add(searchDirBrowser);
    
    // bios info
    searchSettings->Add(new Spacer(32.0f));
    
    
    searchSettings->Add(biosInfo("PS1 bios file for North America",found_na_ps1));
    searchSettings->Add(new Spacer(32.0f));
    
    searchSettings->Add(biosInfo("PS1 bios file for Japan",found_jp_ps1));
    searchSettings->Add(new Spacer(32.0f));
    
    searchSettings->Add(biosInfo("PS1 bios file for Europe",found_eu_ps1));
    searchSettings->Add(new Spacer(32.0f));
    
    searchSettings->Add(biosInfo("PS2 bios file for North America",found_na_ps2));
    searchSettings->Add(new Spacer(32.0f));
    
    searchSettings->Add(biosInfo("PS2 bios file for Japan",found_jp_ps2));
    searchSettings->Add(new Spacer(32.0f));
    
    searchSettings->Add(biosInfo("PS2 bios file for Europe",found_eu_ps2));
    searchSettings->Add(new Spacer(32.0f));
    
    searchSettings->Add(biosInfo("Sega Saturn bios file",gSegaSaturn_firmware));
    searchSettings->Add(new Spacer(32.0f));
    

    // -------- delete search path entry --------
    searchSettings->Add(new ItemHeader(ge->T("")));
    static const char *selectSearchDirectories[128];
    int numChoices=gSearchDirectoriesGames.size();
    static const char *emptyStr="";
    
    if (numChoices)
    {
        for (int i=0;i<numChoices;i++)
        {
            selectSearchDirectories[i]=gSearchDirectoriesGames[i].c_str();
        }
        selectSearchDirectories[numChoices]=emptyStr;
    }
    else
    {
        selectSearchDirectories[0]=emptyStr;
    }

    SCREEN_PopupMultiChoice *selectSearchDirectoriesChoice = searchSettings->Add(new SCREEN_PopupMultiChoice(&inputSearchDirectories, ge->T("Remove search path entry from settings"), selectSearchDirectories, 0, numChoices, ge->GetName(), screenManager()));
    selectSearchDirectoriesChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnDeleteSearchDirectories);
    
    //controller setup
    
    // horizontal layout for margins
    LinearLayout *horizontalLayoutController = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(dp_xres - 40.f, FILL_PARENT));
    tabHolder->AddTab(ge->T("Controller"), horizontalLayoutController);
    float leftMargin = (dp_xres - FILE_BROWSER_WIDTH)/2-10;
    horizontalLayoutController->Add(new Spacer(leftMargin));
    
	// -------- controller setup --------
    ViewGroup *controllerSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(dp_xres - 40.f, FILL_PARENT));
    horizontalLayoutController->Add(controllerSettingsScroll);
	controllerSettingsScroll->SetTag("ControllerSettings");
	LinearLayout *controllerSettings = new LinearLayout(ORIENT_VERTICAL);
	controllerSettings->SetSpacing(0);
	controllerSettingsScroll->Add(controllerSettings);
    controllerSettings->Add(new Spacer(64.0f));
    
    bool controllerPlugged = (gDesignatedControllers[0].numberOfDevices != 0) || (gDesignatedControllers[1].numberOfDevices != 0);
    double verticalSpace = (dp_yres-256.0f)/2;
    updateControllerText = false;
    std::string text_setup_1 = "Start controller setup (1)";
    std::string text_setup_2 = "Start controller setup (2)";
    
    if (gDesignatedControllers[0].numberOfDevices != 0)
    {
        
        LinearLayout *controller_horizontal = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT,verticalSpace));
        controllerSettings->Add(controller_horizontal);
        
        // setup button
        LinearLayout *v = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(64.0f,verticalSpace));
        controller_horizontal->Add(v);
        ImageID icon;
        if (gTheme == THEME_RETRO) icon = ImageID("I_GEAR_R"); else icon = ImageID("I_GEAR");
        Choice* setupButton = new Choice(icon, new LayoutParams(64.0f, 64.0f));
        setupButton->OnClick.Handle(this, &SCREEN_SettingsScreen::OnStartSetup1);
        v->Add(new Spacer(64.0f,(verticalSpace-64.0f)/2));
        v->Add(setupButton);
        controller_horizontal->Add(new Spacer(44.0f));
        
        // text view
        text_setup1 = new TextView(text_setup_1, ALIGN_VCENTER | ALIGN_HCENTER | FLAG_WRAP_TEXT, 
                                    true, new LinearLayoutParams(dp_xres-leftMargin-leftMargin-verticalSpace-128.0f-20, verticalSpace));
        if (gTheme == THEME_RETRO) 
        {
            text_setup1->SetTextColor(0xFFde51e0);
            text_setup1->SetShadow(true);
        }
        controller_horizontal->Add(text_setup1);
        controller_horizontal->Add(new Spacer(44.0f));
        
        // controller pic
        std::string name = gDesignatedControllers[0].name[0];
        std::string nameDB = gDesignatedControllers[0].nameDB[0];
        ImageID controllerImageID = checkControllerType(name,nameDB);
        ImageView* controllerImage = new ImageView(controllerImageID, IS_DEFAULT, new AnchorLayoutParams(verticalSpace, verticalSpace, 1.0f, 1.0f, NONE, NONE, false));
        controller_horizontal->Add(controllerImage);
        
    } else
    {
        controllerSettings->Add(new Spacer(verticalSpace));
    }
    
    controllerSettings->Add(new Spacer(20.0f));
    
    if (gDesignatedControllers[1].numberOfDevices != 0)
    {
        
        LinearLayout *controller_horizontal = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT,verticalSpace));
        controllerSettings->Add(controller_horizontal);
        
        // setup button
        LinearLayout *v = new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(64.0f,verticalSpace));
        controller_horizontal->Add(v);
        ImageID icon;
        if (gTheme == THEME_RETRO) icon = ImageID("I_GEAR_R"); else icon = ImageID("I_GEAR");
        Choice* setupButton = new Choice(icon, new LayoutParams(64.0f, 64.0f));
        setupButton->OnClick.Handle(this, &SCREEN_SettingsScreen::OnStartSetup2);
        v->Add(new Spacer(64.0f,(verticalSpace-64.0f)/2));
        v->Add(setupButton);
        controller_horizontal->Add(new Spacer(44.0f));
        
        // text view
        text_setup2 = new TextView(text_setup_2, ALIGN_VCENTER | ALIGN_HCENTER | FLAG_WRAP_TEXT, 
                                    true, new LinearLayoutParams(dp_xres-leftMargin-leftMargin-verticalSpace-128.0f-20, verticalSpace));
        if (gTheme == THEME_RETRO) 
        {
            text_setup2->SetTextColor(0xFFde51e0);
            text_setup2->SetShadow(true);
        }
        controller_horizontal->Add(text_setup2);
        controller_horizontal->Add(new Spacer(44.0f));
        
        // controller pic
        std::string name = gDesignatedControllers[1].name[0];
        std::string nameDB = gDesignatedControllers[1].nameDB[0];
        ImageID controllerImageID = checkControllerType(name,nameDB);
        ImageView* controllerImage = new ImageView(controllerImageID, IS_DEFAULT, new AnchorLayoutParams(verticalSpace, verticalSpace, 1.0f, 1.0f, NONE, NONE, false));
        controller_horizontal->Add(controllerImage);
    }
    else
    {
        controllerSettings->Add(new Spacer(verticalSpace));
    }

    // -------- Dolphin --------
    
    // horizontal layout for margins
    LinearLayout *horizontalLayoutDolphin = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(dp_xres - 40.f, FILL_PARENT));
    tabHolder->AddTab(ge->T("Dolphin"), horizontalLayoutDolphin);
    horizontalLayoutDolphin->Add(new Spacer(10.0f));
    
	ViewGroup *dolphinSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(dp_xres - 40.f, FILL_PARENT));
    horizontalLayoutDolphin->Add(dolphinSettingsScroll);
	dolphinSettingsScroll->SetTag("DolphinSettings");
	LinearLayout *dolphinSettings = new LinearLayout(ORIENT_VERTICAL);
	dolphinSettings->SetSpacing(0);
    dolphinSettings->Add(new Spacer(10.0f));
	dolphinSettingsScroll->Add(dolphinSettings);
    
    // -------- resolution --------
    static const char *selectResolutionDolphin[] = { "Native Wii", "2x Native (720p)", "3x Native (1080p)", "4x Native (1440p)", "5x Native ", "6x Native (4K)", "7x Native ", "8x Native (5K)" };
    
    SCREEN_PopupMultiChoice *selectResolutionDolphinChoice = dolphinSettings->Add(new SCREEN_PopupMultiChoice(&inputResDolphin, dol->T("Resolution"), selectResolutionDolphin, 0, ARRAY_SIZE(selectResolutionDolphin), dol->GetName(), screenManager()));
    selectResolutionDolphinChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);        
            
    // -------- vsync --------
    CheckBox *vSyncDolphin = dolphinSettings->Add(new CheckBox(&inputVSyncDolphin, dol->T("Supress screen tearing", "Supress screen tearing (VSync)")));
    vSyncDolphin->OnClick.Add([=](EventParams &e) {
        return SCREEN_UI::EVENT_CONTINUE;
    });

    // -------- PCSX2 --------
    
    // horizontal layout for margins
    LinearLayout *horizontalLayoutPCSX2 = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(dp_xres - 40.f, FILL_PARENT));
    tabHolder->AddTab(ge->T("PCSX2"), horizontalLayoutPCSX2);
    horizontalLayoutPCSX2->Add(new Spacer(10.0f));
    
    ViewGroup *PCSX2SettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(dp_xres - 40.f, FILL_PARENT));
    horizontalLayoutPCSX2->Add(PCSX2SettingsScroll);
	PCSX2SettingsScroll->SetTag("PCSX2Settings");
	LinearLayout *PCSX2Settings = new LinearLayout(ORIENT_VERTICAL);
	PCSX2Settings->SetSpacing(0);
	PCSX2SettingsScroll->Add(PCSX2Settings);
    std::string header = "";

    // -------- PCSX2 --------
    if (found_bios_ps2)
    {
        int cnt = 0;
        std::string biosRegions;
        if (found_na_ps2) 
        {
            biosRegions += "North America";
            cnt++;
        }
        if (found_jp_ps2) 
        {
            if (cnt) biosRegions += ", ";
            biosRegions += "Japan";
            cnt++;
        }
        if (found_eu_ps2) 
        {
            if (cnt) biosRegions += ", ";
            biosRegions += "Europe ";
            cnt++;
        }
        if (cnt > 1) 
        {
            header = "PS2 Bios files found for: " + biosRegions;
        } else
        {
            header = "PS2 Bios file found for: " + biosRegions;
        }
        
        PCSX2Settings->Add(new ItemHeader(ps2->T(header)));
        
        // -------- bios --------
        if (found_na_ps2 && found_jp_ps2 && !found_eu_ps2)
        {
            bios_selection[0] = BIOS_NA;
            bios_selection[1] = BIOS_JP;
            bios_selection[2] = EMPTY;
            
            static const char *selectBIOS[] = { "North America", "Japan" };
            
            SCREEN_PopupMultiChoice *selectBIOSChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputBios, ps2->T("Bios selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), ps2->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        } else
        if (found_na_ps2 && !found_jp_ps2 && found_eu_ps2)
        {
            bios_selection[0] = BIOS_NA;
            bios_selection[1] = BIOS_EU;
            bios_selection[2] = EMPTY;
            
            static const char *selectBIOS[] = { "North America", "Europe" };

            SCREEN_PopupMultiChoice *selectBIOSChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputBios, ps2->T("Bios Selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), ps2->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        } else
        if (!found_na_ps2 && found_jp_ps2 && found_eu_ps2)
        {
            bios_selection[0] = BIOS_JP;
            bios_selection[1] = BIOS_EU;
            bios_selection[2] = EMPTY;
            
            static const char *selectBIOS[] = { "Japan", "Europe" };
            
            SCREEN_PopupMultiChoice *selectBIOSChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputBios, ps2->T("Bios Selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), ps2->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        } else
        if (found_na_ps2 && found_jp_ps2 && found_eu_ps2)
        {
            bios_selection[0] = BIOS_NA;
            bios_selection[1] = BIOS_JP;
            bios_selection[2] = BIOS_EU;
            
            static const char *selectBIOS[] = { "North America", "Japan", "Europe" };

            SCREEN_PopupMultiChoice *selectBIOSChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputBios, ps2->T("Bios Selection"), selectBIOS, 0, ARRAY_SIZE(selectBIOS), ps2->GetName(), screenManager()));
            selectBIOSChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
        }

        // -------- rendering mode --------
        static const char *renderingBackend[] = {
            "OpenGL (Hardware)",
            "OpenGL (Software)"};

        SCREEN_PopupMultiChoice *renderingBackendChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputBackend, ps2->T("Backend"), renderingBackend, 0, ARRAY_SIZE(renderingBackend), ps2->GetName(), screenManager()));
        renderingBackendChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);

        if (inputBackend == BACKEND_OPENGL_HARDWARE_PLUS_SOFTWARE)
        {
            // -------- advanced settings --------
            CheckBox *vAdvancedSettings = PCSX2Settings->Add(new CheckBox(&inputAdvancedSettings, ps2->T("Advanced Settings", "Advanced Settings")));
            vAdvancedSettings->OnClick.Add([=](EventParams &e) {
                RecreateViews();
                return SCREEN_UI::EVENT_CONTINUE;
            });
        }

        if ((inputBackend == BACKEND_OPENGL_HARDWARE_PLUS_SOFTWARE) && (inputAdvancedSettings) )
        {          
            // -------- auto flush --------
            CheckBox *vAutoflush_sw = PCSX2Settings->Add(new CheckBox(&inputAutoflush_sw, ps2->T("Enable 'Auto flush framebuffer'", "Enable 'Auto flush framebuffer'")));
            vAutoflush_sw->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
                   
            // -------- mipmapping --------
            CheckBox *vMipmapping_sw = PCSX2Settings->Add(new CheckBox(&inputMipmapping_sw, ps2->T("Enable 'Mipmapping'", "Enable 'Mipmapping'")));
            vMipmapping_sw->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            // -------- anti aliasing --------
            CheckBox *vAnti_aliasing_sw = PCSX2Settings->Add(new CheckBox(&inputAnti_aliasing_sw, ps2->T("Enable 'Edge anti-aliasing'", "Enable 'Edge anti-aliasing'")));
            vAnti_aliasing_sw->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });

            // -------- extra threads --------
            static const char *Extrathreads_sw[] = { "0","2", "3","4","5","6","7"};

            SCREEN_PopupMultiChoice *Extrathreads_swChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputExtrathreads_sw, ps2->T("Extra threads"), Extrathreads_sw, 0, ARRAY_SIZE(Extrathreads_sw), ps2->GetName(), screenManager()));
            Extrathreads_swChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- interpreter --------
            CheckBox *vInterpreter = PCSX2Settings->Add(new CheckBox(&inputInterpreter, ps2->T("Enable 'Interpreter'", "Enable 'Interpreter'")));
            vInterpreter->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
                
        }
        
        if (inputBackend == BACKEND_OPENGL_HARDWARE)
        {
            // -------- resolution --------
            static const char *selectResolution[] = { "Native PS2", "720p", "1080p", "1440p 2K", "1620p 3K", "2160p 4K" };
            
            SCREEN_PopupMultiChoice *selectResolutionChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputResPCSX2, ps2->T("Resolution"), selectResolution, 0, ARRAY_SIZE(selectResolution), ps2->GetName(), screenManager()));
            selectResolutionChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);        
            
            // -------- advanced settings --------
            CheckBox *vAdvancedSettings = PCSX2Settings->Add(new CheckBox(&inputAdvancedSettings, ps2->T("Advanced Settings", "Advanced Settings")));
            vAdvancedSettings->OnClick.Add([=](EventParams &e) {
                RecreateViews();
                return SCREEN_UI::EVENT_CONTINUE;
            });
        }
        
        if ((inputBackend == BACKEND_OPENGL_HARDWARE) && (inputAdvancedSettings))
        {
            
            // -------- interlace --------
            static const char *interlace[] = {
                "None",
                "Weave tff",
                "Weave bff",
                "Bob tff",
                "Bob bff",
                "Blend tff",
                "Blend bff",
                "Automatic"};

            SCREEN_PopupMultiChoice *interlaceChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputInterlace, ps2->T("Interlace"), interlace, 0, ARRAY_SIZE(interlace), ps2->GetName(), screenManager()));
            interlaceChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- bi filter --------
            static const char *biFilter[] = {
                "Nearest",
                "Bilinear Forced excluding sprite",
                "Bilinear Forced",
                "Bilinear PS2"};

            SCREEN_PopupMultiChoice *biFilterChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputBiFilter, ps2->T("Bi Filter"), biFilter, 0, ARRAY_SIZE(biFilter), ps2->GetName(), screenManager()));
            biFilterChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- max anisotropy --------
            static const char *anisotropy[] = {
                "Off",
                "2x",
                "4x",
                "8x",
                "16x"};

            SCREEN_PopupMultiChoice *anisotropyChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputAnisotropy, ps2->T("Max anisotropy"), anisotropy, 0, ARRAY_SIZE(anisotropy), ps2->GetName(), screenManager()));
            anisotropyChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- dithering --------
            static const char *dithering[] = {
                "Off",
                "Scaled",
                "Unscaled"};

            SCREEN_PopupMultiChoice *ditheringChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputDithering, ps2->T("Dithering"), dithering, 0, ARRAY_SIZE(dithering), ps2->GetName(), screenManager()));
            ditheringChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- HW mipmapping --------
            static const char *hw_mipmapping[] = {
                "Automatic",
                "Off",
                "Basic",
                "Full"};

            SCREEN_PopupMultiChoice *hw_mipmappingChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputHW_mipmapping, ps2->T("HW mipmapping"), hw_mipmapping, 0, ARRAY_SIZE(hw_mipmapping), ps2->GetName(), screenManager()));
            hw_mipmappingChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- CRC level --------
            static const char *crc_level[] = {
                "Automatic",
                "None",
                "Minimum",
                "Partial",
                "Full",
                "Aggressive"};
                
            SCREEN_PopupMultiChoice *crc_levelChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputCRC_level, ps2->T("CRC level"), crc_level, 0, ARRAY_SIZE(crc_level), ps2->GetName(), screenManager()));
            crc_levelChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- accurate date --------
            static const char *acc_date_level[] = {
                "Off",
                "Fast",
                "Full"};
                
            SCREEN_PopupMultiChoice *acc_date_levelChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputAcc_date_level, ps2->T("DATE accuracy"), acc_date_level, 0, ARRAY_SIZE(acc_date_level), ps2->GetName(), screenManager()));
            acc_date_levelChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- accurate blending unit --------
            static const char *acc_blend_level[] = {
                "None",
                "Basic",
                "Medium",
                "High",
                "Full",
                "Ultra"};
                                
            SCREEN_PopupMultiChoice *acc_blend_levelChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputAcc_blend_level, ps2->T("Blending accuracy"), acc_blend_level, 0, ARRAY_SIZE(acc_blend_level), ps2->GetName(), screenManager()));
            acc_blend_levelChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);
            
            // -------- interpreter --------
            CheckBox *vInterpreter = PCSX2Settings->Add(new CheckBox(&inputInterpreter, ps2->T("Enable 'Interpreter'", "Enable 'Interpreter'")));
            vInterpreter->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            if (inputUserHacks)
            {
                // -------- vsync --------
                CheckBox *vSync = PCSX2Settings->Add(new CheckBox(&inputVSyncPCSX2, ps2->T("Supress screen tearing", "Supress screen tearing (VSync)")));
                vSync->OnClick.Add([=](EventParams &e) {
                    return SCREEN_UI::EVENT_CONTINUE;
                });
            }
            
            // -------- user hacks --------
            CheckBox *vUserHacks = PCSX2Settings->Add(new CheckBox(&inputUserHacks, ps2->T("Enable user hacks", "Enable user hacks")));
            vUserHacks->OnClick.Add([=](EventParams &e) {
                RecreateViews();
                return SCREEN_UI::EVENT_CONTINUE;
            });
        }
        
        if ((inputUserHacks) && (inputBackend == BACKEND_OPENGL_HARDWARE) && (inputAdvancedSettings))
        {
            PCSX2Settings->Add(new ItemHeader(ps2->T("User hacks")));
            
            CheckBox *vUserHacks_AutoFlush = PCSX2Settings->Add(new CheckBox(&inputUserHacks_AutoFlush, ps2->T("Enable 'auto flush'", "Enable 'auto flush'")));
            vUserHacks_AutoFlush->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_CPU_FB_Conversion = PCSX2Settings->Add(new CheckBox(&inputUserHacks_CPU_FB_Conversion, ps2->T("Enable 'CPU framebuffer conversion'", "Enable 'CPU framebuffer conversion'")));
            vUserHacks_CPU_FB_Conversion->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_DisableDepthSupport = PCSX2Settings->Add(new CheckBox(&inputUserHacks_DisableDepthSupport, ps2->T("Enable 'no depth support'", "Enable 'no depth support'")));
            vUserHacks_DisableDepthSupport->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_DisablePartialInvalidation = PCSX2Settings->Add(new CheckBox(&inputUserHacks_DisablePartialInvalidation, ps2->T("Enable 'no partial invalidation'", "Enable 'no partial invalidation'")));
            vUserHacks_DisablePartialInvalidation->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_Disable_Safe_Features = PCSX2Settings->Add(new CheckBox(&inputUserHacks_Disable_Safe_Features, ps2->T("Enable 'no safe features'", "Enable 'no safe features'")));
            vUserHacks_Disable_Safe_Features->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_HalfPixelOffset = PCSX2Settings->Add(new CheckBox(&inputUserHacks_HalfPixelOffset, ps2->T("Enable 'half pixel offset'", "Enable 'half pixel offset'")));
            vUserHacks_HalfPixelOffset->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            
            static const char *half_Bottom[] = { "Auto", "Force-disable", "Force-enable"};
            
            SCREEN_PopupMultiChoice *half_BottomChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputUserHacks_Half_Bottom_Override, ps2->T("Half bottom override"), half_Bottom, 0, ARRAY_SIZE(half_Bottom), ps2->GetName(), screenManager()));
            half_BottomChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);        
            
            CheckBox *vUserHacks_SkipDraw = PCSX2Settings->Add(new CheckBox(&inputUserHacks_SkipDraw, ps2->T("Enable 'skip draw'", "Enable 'skip draw'")));
            vUserHacks_SkipDraw->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_SkipDraw_Offset = PCSX2Settings->Add(new CheckBox(&inputUserHacks_SkipDraw_Offset, ps2->T("Enable 'skip draw offset'", "Enable 'skip draw offset'")));
            vUserHacks_SkipDraw_Offset->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_TCOffsetX = PCSX2Settings->Add(new CheckBox(&inputUserHacks_TCOffsetX, ps2->T("Enable 'TC offset X'", "Enable 'TC offset X'")));
            vUserHacks_TCOffsetX->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });

            CheckBox *vUserHacks_TCOffsetY = PCSX2Settings->Add(new CheckBox(&inputUserHacks_TCOffsetY, ps2->T("Enable 'TC offset Y'", "Enable 'TC offset Y'")));
            vUserHacks_TCOffsetY->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });

            static const char *UserHacks_TriFilter[] = { "None","PS2","Forced"};
            SCREEN_PopupMultiChoice *UserHacks_TriFilterChoice = PCSX2Settings->Add(new SCREEN_PopupMultiChoice(&inputUserHacks_TriFilter, ps2->T("Enable 'tri filter'"), UserHacks_TriFilter, 0, ARRAY_SIZE(UserHacks_TriFilter), ps2->GetName(), screenManager()));
            UserHacks_TriFilterChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnRenderingBackend);

            CheckBox *vUserHacks_WildHack = PCSX2Settings->Add(new CheckBox(&inputUserHacks_WildHack, ps2->T("Enable 'wild hack'", "Enable 'wild hack'")));
            vUserHacks_WildHack->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });

            CheckBox *vUserHacks_align_sprite_X = PCSX2Settings->Add(new CheckBox(&inputUserHacks_align_sprite_X, ps2->T("Enable 'align sprite X'", "Enable 'align sprite X'")));
            vUserHacks_align_sprite_X->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });

            CheckBox *vUserHacks_merge_pp_sprite = PCSX2Settings->Add(new CheckBox(&inputUserHacks_merge_pp_sprite, ps2->T("Enable 'merge pp sprite'", "Enable 'merge pp sprite'")));
            vUserHacks_merge_pp_sprite->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });

            CheckBox *vUserHacks_round_sprite_offset = PCSX2Settings->Add(new CheckBox(&inputUserHacks_round_sprite_offset, ps2->T("Enable 'round sprite offset'", "Enable 'round sprite offset'")));
            vUserHacks_round_sprite_offset->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
            
            CheckBox *vUserHacks_TextureInsideRt = PCSX2Settings->Add(new CheckBox(&inputUserHacks_TextureInsideRt, ps2->T("Enable 'texture inside Rt'", "Enable 'texture inside Rt'")));
            vUserHacks_TextureInsideRt->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
        }

        if (!((inputUserHacks) && (inputBackend == BACKEND_OPENGL_HARDWARE) && (inputAdvancedSettings)))
        {
            // -------- vsync --------
            CheckBox *vSync = PCSX2Settings->Add(new CheckBox(&inputVSyncPCSX2, ps2->T("Supress screen tearing", "Supress screen tearing (VSync)")));
            vSync->OnClick.Add([=](EventParams &e) {
                return SCREEN_UI::EVENT_CONTINUE;
            });
        }
        
    } else
    {
        PCSX2Settings->Add(new ItemHeader(ps2->T("PCSX2: No bios files found. Set up a path to a PS2 bios in tab 'Search Path'.")));
    }
    
    
    // -------- general --------
    
    // horizontal layout for margins
    LinearLayout *horizontalLayoutGeneral = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(dp_xres - 40.f, FILL_PARENT));
    tabHolder->AddTab(ge->T("General"), horizontalLayoutGeneral);
    horizontalLayoutGeneral->Add(new Spacer(10.0f));
    
    ViewGroup *generalSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(dp_xres - 40.f, FILL_PARENT));
    horizontalLayoutGeneral->Add(generalSettingsScroll);
	generalSettingsScroll->SetTag("GeneralSettings");
	LinearLayout *generalSettings = new LinearLayout(ORIENT_VERTICAL);
	generalSettings->SetSpacing(0);
	generalSettingsScroll->Add(generalSettings);

    generalSettings->Add(new ItemHeader(ge->T("General settings for Marley")));
    
    // -------- system sounds --------
    CheckBox *vSystemSounds = generalSettings->Add(new CheckBox(&playSystemSounds, ge->T("Enable system sounds", "Enable system sounds")));
    vSystemSounds->OnClick.Add([=](EventParams &e) {
        return SCREEN_UI::EVENT_CONTINUE;
    });
    
    // -------- theme --------
    static const char *ui_theme[] = {
        "Retro",
        "Plain"};
                        
    SCREEN_PopupMultiChoice *ui_themeChoice = generalSettings->Add(new SCREEN_PopupMultiChoice(&gTheme, ge->T("Theme"), ui_theme, 0, ARRAY_SIZE(ui_theme), ge->GetName(), screenManager()));
    ui_themeChoice->OnChoice.Handle(this, &SCREEN_SettingsScreen::OnThemeChanged);


    // -------- credits --------
    
        // horizontal layout for margins
    LinearLayout *horizontalLayoutCredits = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(dp_xres - 40.f, FILL_PARENT));
    tabHolder->AddTab(ge->T("Credits"), horizontalLayoutCredits);
    horizontalLayoutCredits->Add(new Spacer(10.0f));
    
    ViewGroup *creditsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(dp_xres - 40.f, FILL_PARENT));
    horizontalLayoutCredits->Add(creditsScroll);
	creditsScroll->SetTag("Credits");
	LinearLayout *credits = new LinearLayout(ORIENT_VERTICAL);
	credits->SetSpacing(0);
	creditsScroll->Add(credits);

    credits->Add(new ItemHeader(ge->T("Mednafen: https://mednafen.github.io, License: GNU GPLv2")));
    credits->Add(new ItemHeader(ge->T("Mupen64Plus: https://mupen64plus.org, License: GNU GPL")));
    credits->Add(new ItemHeader(ge->T("PPSSPP: https://www.ppsspp.org, License: GNU GPLv2")));
    credits->Add(new ItemHeader(ge->T("Dolphin: https://dolphin-emu.org/, License: GNU GPLv2")));
    credits->Add(new ItemHeader(ge->T("PCSX2: https://pcsx2.net, License: GNU GPLv2")));
    
	SCREEN_Draw::SCREEN_DrawContext *draw = screenManager()->getSCREEN_DrawContext();

}

SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnRenderingBackend(SCREEN_UI::EventParams &e) {
    RecreateViews();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnThemeChanged(SCREEN_UI::EventParams &e) {
    SCREEN_UIThemeInit();
    RecreateViews();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnDeleteSearchDirectories(SCREEN_UI::EventParams &e) {
    
    std::string str, line;

    for (int i=0;i<marley_cfg_entries.size();i++)
    {
        line = marley_cfg_entries[i];
        std::string subStr, slash;
        if (line.find("search_dir_games=") != std::string::npos)
        {
            subStr = line.substr(line.find("=")+1,line.length());
            
            // add slash to end if necessary
            slash = subStr.substr(subStr.length()-1,1);
            if (slash != "/")
            {
                subStr += "/";
            }
            
            if(subStr == gSearchDirectoriesGames[inputSearchDirectories])
            {
                marley_cfg_entries.erase(marley_cfg_entries.begin()+i);
                break;
            }
        }
    }
    
    gSearchDirectoriesGames.erase(gSearchDirectoriesGames.begin()+inputSearchDirectories);
    searchAllFolders();
    RecreateViews();
	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_SettingsScreen::onFinish(DialogResult result) {
    printf("jc: void SCREEN_SettingsScreen::onFinish(DialogResult result)\n");
    //SCREEN_System_SendMessage("finish", "");
}

void SCREEN_SettingsScreen::update() {
	SCREEN_UIScreen::update();

    bool vertical = true;
	if (vertical != lastVertical_) {
		RecreateViews();
		lastVertical_ = vertical;
	}
    
    if (searchDirAdded)
    {
        searchDirAdded=false;
        RecreateViews();
    }
    
    if (updateControllerText)
    {
        if (gControllerConfNum == CONTROLLER_1)
        {
            text_setup1->SetText(gConfText);
        } else 
        if (gControllerConfNum == CONTROLLER_2)
        {
            text_setup2->SetText(gConfText);
        }
        updateControllerText = false;
    }
}

SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnStartSetup1(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnStartSetup1(SCREEN_UI::EventParams &e)\n");
    
    startControllerConf(CONTROLLER_1);
    setControllerConfText("press dpad up");

	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnStartSetup2(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_SettingsScreen::OnStartSetup2(SCREEN_UI::EventParams &e)\n");
    
    startControllerConf(CONTROLLER_2);
    setControllerConfText("press dpad up");
    
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_SettingsInfoMessage::SCREEN_SettingsInfoMessage(int align, SCREEN_UI::AnchorLayoutParams *lp)
	: SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, lp) {
	using namespace SCREEN_UI;
	SetSpacing(0.0f);
	Add(new Spacer(10.0f));
	text_ = Add(new SCREEN_UI::TextView("", align, false, new LinearLayoutParams(1.0, Margins(0, 10))));
	Add(new Spacer(10.0f));
}

void SCREEN_SettingsInfoMessage::Show(const std::string &text, SCREEN_UI::View *refView) {
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

void SCREEN_SettingsInfoMessage::Draw(SCREEN_UIContext &dc) {
	static const double FADE_TIME = 1.0;
	static const float MAX_ALPHA = 0.9f;

	// Show for a variable time based on length and estimated reading speed
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

class SCREEN_DirButton : public SCREEN_UI::Button {
public:
	SCREEN_DirButton(const std::string &path, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams)
		: SCREEN_UI::Button(path, layoutParams), path_(path), gridStyle_(gridStyle), absolute_(false) {}
	SCREEN_DirButton(const std::string &path, const std::string &text, bool gridStyle, SCREEN_UI::LayoutParams *layoutParams = 0)
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
                if (path_=="..")
                {
                    searchPath = currentSearchPath;
                }
                else
                {
                    searchPath = path_;
                }
                tooltipFileBrowser = "Search path for bios files added: " + searchPath;
                searchDirAdded = addSearchPathToConfigFile(searchPath);
            }
        } 

		return Clickable::Key(key);
	}

private:
	std::string path_;
	bool absolute_;
	bool gridStyle_;
};

void SCREEN_DirButton::Draw(SCREEN_UIContext &dc) {
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
    
    dc.SetFontStyle(dc.theme->uiFontSmall);
    
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
            if (gTheme == THEME_RETRO)
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

SCREEN_DirBrowser::SCREEN_DirBrowser(std::string path, SCREEN_BrowseFlags browseFlags, bool *gridStyle, SCREEN_ScreenManager *screenManager, std::string lastText, std::string lastLink, SCREEN_UI::LayoutParams *layoutParams)
	: LinearLayout(SCREEN_UI::ORIENT_VERTICAL, layoutParams), path_(path), gridStyle_(gridStyle), screenManager_(screenManager), browseFlags_(browseFlags), lastText_(lastText), lastLink_(lastLink) {
	using namespace SCREEN_UI;
    printf("jc: SCREEN_DirBrowser::SCREEN_DirBrowser\n");
    if (tooltipFileBrowser != "")
    {
        SCREEN_UI::EventParams e{};
		e.v = this;
        settingsInfo_->Show(tooltipFileBrowser, e.v);
        tooltipFileBrowser = "";
    }
	Refresh();
}

SCREEN_DirBrowser::~SCREEN_DirBrowser() {
    printf("jc: SCREEN_DirBrowser::~SCREEN_DirBrowser()\n");
}

void SCREEN_DirBrowser::FocusGame(const std::string &gamePath) {
    printf("jc: void SCREEN_DirBrowser::FocusGame(const std::string &gamePath)\n");
	focusGamePath_ = gamePath;
	Refresh();
	focusGamePath_.clear();
}

void SCREEN_DirBrowser::SetPath(const std::string &path) {
    printf("jc: void SCREEN_DirBrowser::SetPath(const std::string &path) %s\n",path.c_str());
	path_.SetPath(path);
	Refresh();
}

std::string SCREEN_DirBrowser::GetPath() {
    printf("jc: std::string SCREEN_DirBrowser::GetPath() \n");
    std::string str = path_.GetPath();
	return str;
}

SCREEN_UI::EventReturn SCREEN_DirBrowser::LayoutChange(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_DirBrowser::LayoutChange(SCREEN_UI::EventParams &e)\n");
	*gridStyle_ = e.a == 0 ? true : false;
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_DirBrowser::HomeClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_DirBrowser::HomeClick(SCREEN_UI::EventParams &e)\n");
	SetPath(getenv("HOME"));

	return SCREEN_UI::EVENT_DONE;
}

void SCREEN_DirBrowser::Update() {
	LinearLayout::Update();
	if (listingPending_ && path_.IsListingReady()) {
		Refresh();
	}
}

void SCREEN_DirBrowser::Draw(SCREEN_UIContext &dc) {
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

void SCREEN_DirBrowser::Refresh() {
	using namespace SCREEN_UI;
    printf("jc: void SCREEN_DirBrowser::Refresh()\n");
    
	lastScale_ = 1.0f;
	lastLayoutWasGrid_ = *gridStyle_;

	// Reset content
	Clear();

	Add(new Spacer(10.0f));
	auto mm = GetI18NCategory("MainMenu");
	
    LinearLayout *topBar = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
    // display working directory
    TextView* workingDirectory;
    workingDirectory = new TextView(path_.GetFriendlyPath().c_str(), ALIGN_VCENTER | FLAG_WRAP_TEXT, false, new LinearLayoutParams(FILL_PARENT, 64.0f, 1.0f));
    topBar->Add(workingDirectory);
    
    currentSearchPath=path_.GetPath();
    ImageID icon;
    
        // home button
    if (gTheme == THEME_RETRO) icon = ImageID("I_HOME_R"); else icon = ImageID("I_HOME");
    Choice* homeButton = new Choice(icon, new LayoutParams(64.0f, 64.0f));
    homeButton->OnClick.Handle(this, &SCREEN_DirBrowser::HomeClick);
    homeButton->OnHighlight.Add([=](EventParams &e) {
        if (!toolTipsShown[SETTINGS_HOME])
        {
            toolTipsShown[SETTINGS_HOME] = true;
            settingsInfo_->Show(mm->T("Home", "Home: jump in file browser to home directory"), e.v);
        }
		return SCREEN_UI::EVENT_CONTINUE;
	});
    topBar->Add(homeButton);

    ChoiceStrip *layoutChoice = topBar->Add(new ChoiceStrip(ORIENT_HORIZONTAL));
    
    if (gTheme == THEME_RETRO) icon = ImageID("I_GRID_R"); else icon = ImageID("I_GRID");
    layoutChoice->AddChoice(icon,"show file browser in a grid",&toolTipsShown[SETTINGS_GRID]);
    if (gTheme == THEME_RETRO) icon = ImageID("I_LINES_R"); else icon = ImageID("I_LINES");
    layoutChoice->AddChoice(icon,"show file browser with lines",&toolTipsShown[SETTINGS_LINES]);
    layoutChoice->SetSelection(*gridStyle_ ? 0 : 1);
    layoutChoice->OnChoice.Handle(this, &SCREEN_DirBrowser::LayoutChange);
    
    Add(topBar);
    Add(new Spacer(5.0f));
    SetSpacing(0.0f);
    // info text
    TextView* infoText1;
    TextView* infoText2;
    infoText1 = new TextView("To add a search path, highlight a folder and use the start button or space. ", 
                 ALIGN_VCENTER | FLAG_WRAP_TEXT, false, new LinearLayoutParams(FILL_PARENT, 32.0f));
    infoText2 = new TextView("To remove a search path, scroll all the way down.", 
                 ALIGN_VCENTER | FLAG_WRAP_TEXT, false, new LinearLayoutParams(FILL_PARENT, 32.0f));
    
    Add(infoText1);
    Add(infoText2);
    Add(new Spacer(5.0f));
    if (*gridStyle_) {
        gameList_ = new SCREEN_UI::GridLayout(SCREEN_UI::GridLayoutSettings(150*1.0f, 85*1.0f), new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
        Add(gameList_);
    } else {
        SCREEN_UI::LinearLayout *gl = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, WRAP_CONTENT));
        gl->SetSpacing(4.0f);
        gameList_ = gl;
        Add(gameList_);
    }

	// Show folders in the current directory
	std::vector<SCREEN_DirButton *> dirButtons;

	listingPending_ = !path_.IsListingReady();

	std::vector<std::string> filenames;
	if (!listingPending_) {
        printf("jc: if (!listingPending_)\n");
		std::vector<FileInfo> fileInfo;
		path_.GetListing(fileInfo, "iso:cso:pbp:elf:prx:ppdmp:");
        printf("jc: fileInfo.size()=%ld\n",fileInfo.size());
		for (size_t i = 0; i < fileInfo.size(); i++) {
            std::string str=fileInfo[i].name;
            printf("jc: fileInfo[i].name=%s\n",str.c_str());
			
			if (fileInfo[i].isDirectory) {
				if (browseFlags_ & SCREEN_BrowseFlags::NAVIGATE) {
					dirButtons.push_back(new SCREEN_DirButton(fileInfo[i].fullName, fileInfo[i].name, *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)));
				}
			} 
		}
	}

	if (browseFlags_ & SCREEN_BrowseFlags::NAVIGATE) {
		gameList_->Add(new SCREEN_DirButton("..", *gridStyle_, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)))->
			OnClick.Handle(this, &SCREEN_DirBrowser::NavigateClick);

	}

	if (listingPending_) {
		gameList_->Add(new SCREEN_UI::TextView(mm->T("Loading..."), ALIGN_CENTER, false, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT)));
	}

	for (size_t i = 0; i < dirButtons.size(); i++) {
        std::string str = dirButtons[i]->GetPath();
        printf("jc: for (size_t i = 0; i < dirButtons.size(); i++)  %s\n",str.c_str());
		gameList_->Add(dirButtons[i])->OnClick.Handle(this, &SCREEN_DirBrowser::NavigateClick);
	}
}

const std::string SCREEN_DirBrowser::GetBaseName(const std::string &path) {
printf("jc: const std::string SCREEN_DirBrowser::GetBaseName(const std::string &path)\n");
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

SCREEN_UI::EventReturn SCREEN_DirBrowser::NavigateClick(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_DirBrowser::NavigateClick(SCREEN_UI::EventParams &e)\n");
	SCREEN_DirButton *button = static_cast<SCREEN_DirButton *>(e.v);
	std::string text = button->GetPath();
	if (button->PathAbsolute()) {
		path_.SetPath(text);
	} else {
		path_.Navigate(text);
	}
	Refresh();
	return SCREEN_UI::EVENT_DONE;
}

SCREEN_UI::EventReturn SCREEN_DirBrowser::OnRecentClear(SCREEN_UI::EventParams &e) {
    printf("jc: SCREEN_UI::EventReturn SCREEN_DirBrowser::OnRecentClear(SCREEN_UI::EventParams &e)\n");
	screenManager_->RecreateAllViews();
	return SCREEN_UI::EVENT_DONE;
}
