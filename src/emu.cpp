/* Marley Copyright (c) 2020 Marley Development Team 
   https://github.com/beaumanvienna/marley

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>
#include <list>
#include <iterator>
#include <algorithm>

#include "../include/emu.h"
#include "../include/gui.h"
#include "../include/statemachine.h"
#include "../include/controller.h"

#include <gtk/gtk.h>
#include "../resources/pcsx2_res.h"

using namespace std;
typedef unsigned long long int checksum64;

#define PS1_BIOS_SIZE           524288
#define SEGA_SATURN_BIOS_SIZE   524288
#define PS2_BIOS_SIZE           4194304
#define SCPH5500_BIN            21715608
#define SCPH5501_BIN            22714036
#define SCPH5502_BIN            24215776
#define SCPH77000_BIN           56837872
#define SCPH77001_BIN           162002608
#define SCPH77002_BIN           169569744
#define SEGA_SATURN_BIOS_JP     19759492
#define SEGA_SATURN_BIOS_NA_EU  19688652


bool isDirectory(const char *filename);

bool gPS1_firmware;
bool gPS2_firmware;
bool gSegaSaturn_firmware;
string gPathToFirmwarePSX;
string gPathToFirmwarePS2;
string gPathToGames;
string gBaseDir;
vector<string> gSupportedEmulators = {"ps1","ps2","psp","md (sega genesis)","md (sega saturn)","snes","nes","gamecube","wii","n64", "gba", "gbc"};
vector<string> gFileTypes = {"smc","iso","smd","bin","cue","z64","v64","nes", "sfc", "gba", "gbc", "wbfs","mdf"};
bool gGamesFound;

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

checksum64 calcChecksum(const char * filename)
{
    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();
    checksum64 checksum = 0;

    int length = pos;

    char *pChars = new char[length];
    ifs.seekg(0, ios::beg);
    ifs.read(pChars, length);
    
    for(int i = 0; i < length; i++)
        checksum += pChars[i] + (pChars[i] ^ 0x55);

    delete pChars;

    return checksum;
}
bool IsBIOS_PCSX2(const char * filename);
void findAllBiosFiles(const char * directory, std::list<string> *tmpList_ps1, std::list<string> *tmpList_ps2 = nullptr)
{
    string str_with_path, str_without_path;
    string ext, str_with_path_lower_case;
    DIR *dir;
    
    struct dirent *ent;
    if ((dir = opendir (directory)) != NULL) 
    {
        // print all files and directories in directory
        while ((ent = readdir (dir)) != NULL) 
        {
            str_with_path = directory;
            str_with_path +=ent->d_name;
            
            if (isDirectory(str_with_path.c_str()))
            {
                str_without_path =ent->d_name;
                if ((str_without_path != ".") && (str_without_path != ".."))
                {
                    str_with_path +="/";
                    findAllBiosFiles(str_with_path.c_str(),tmpList_ps1,tmpList_ps2);
                }
            }
            else
            {
                ext = str_with_path.substr(str_with_path.find_last_of(".") + 1);
                
                std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c){ return std::tolower(c); });
                
                str_with_path_lower_case=str_with_path;
                std::transform(str_with_path_lower_case.begin(), str_with_path_lower_case.end(), str_with_path_lower_case.begin(),
                    [](unsigned char c){ return std::tolower(c); });
                
                if ((str_with_path_lower_case.find("battlenet") ==  string::npos) &&\
                    (str_with_path_lower_case.find("ps3") ==  string::npos) &&\
                    (str_with_path_lower_case.find("ps4") ==  string::npos) &&\
                    (str_with_path_lower_case.find("xbox") ==  string::npos))
                {
                    int file_size = filesize(str_with_path.c_str());
                    
                    if ((file_size == PS1_BIOS_SIZE)||(file_size == SEGA_SATURN_BIOS_SIZE))
                    {
                        printf("debug sega saturn bios: bios file name \"%s\", checksum: %i\n",str_with_path.c_str(),calcChecksum(str_with_path.c_str()));
                        tmpList_ps1[0].push_back(str_with_path);
                    }
                    else if (file_size == PS2_BIOS_SIZE)
                    {
                        if (IsBIOS_PCSX2(str_with_path.c_str()) && tmpList_ps2)
                        {
                            tmpList_ps2[0].push_back(str_with_path);
                        }
                    }
                }
            }
        }
        closedir (dir);
    } 
}

bool copyFile(const char *SRC, const char* DEST)
{
    std::ifstream src(SRC, std::ios::binary);
    std::ofstream dest(DEST, std::ios::binary);
    dest << src.rdbuf();
    return src && dest;
}

void checkFirmwarePSX(void)
{
    // ---------- PS1 ----------
    bool found_jp_ps1 = false;
    bool found_na_ps1 = false;
    bool found_eu_ps1 = false;
    
    string jp_ps1 = gBaseDir + "scph5500.bin";
    string na_ps1 = gBaseDir + "scph5501.bin";
    string eu_ps1 = gBaseDir + "scph5502.bin";

    ifstream jpF_ps1 (jp_ps1.c_str());
    ifstream naF_ps1 (na_ps1.c_str());
    ifstream euF_ps1 (eu_ps1.c_str());
    // check if ps1 files are already installed in base directory
    if (jpF_ps1.is_open())
    {
        jpF_ps1.close();
        if ( calcChecksum(jp_ps1.c_str()) == SCPH5500_BIN)
        {
            printf( "PS1 bios found with signature 'Japan SCPH-5500/v3.0J'        : %s\n", jp_ps1.c_str());
            found_jp_ps1 = true;
        }
    }
    
    if (naF_ps1.is_open())
    {
        naF_ps1.close();
        if ( calcChecksum(na_ps1.c_str()) == SCPH5501_BIN)
        {
            printf( "PS1 bios found with signature 'North America SCPH-5501/v3.0A': %s\n", na_ps1.c_str());
            found_na_ps1 = true;
        }
    }
    
    if (euF_ps1.is_open())
    {
        euF_ps1.close();
        if ( calcChecksum(eu_ps1.c_str()) == SCPH5502_BIN)
        {
            printf( "PS1 bios found with signature 'Europe SCPH-5502/v3.0E'       : %s\n", eu_ps1.c_str());
            found_eu_ps1 = true;
        }
    }
    // ---------- PS2 ----------
    bool found_jp_ps2 = false;
    bool found_na_ps2 = false;
    bool found_eu_ps2 = false;
    
    string jp_ps2 = gBaseDir + "scph77000.bin";
    string na_ps2 = gBaseDir + "scph77001.bin";
    string eu_ps2 = gBaseDir + "scph77002.bin";
    string tempBios_ps2_bios = gBaseDir + "tempBios.bin";

    ifstream jpF_ps2 (jp_ps2.c_str());
    ifstream naF_ps2 (na_ps2.c_str());
    ifstream euF_ps2 (eu_ps2.c_str());
    ifstream unF_ps2 (tempBios_ps2_bios.c_str());
    // check if ps2 files are already installed in base directory
    if (jpF_ps2.is_open())
    {
        jpF_ps2.close();
        if ( calcChecksum(jp_ps2.c_str()) == SCPH77000_BIN)
        {
            printf( "PS2 bios found with signature 'Japan SCPH-77000'        : %s\n", jp_ps2.c_str());
            found_jp_ps2 = true;
        }
    }
    
    if (naF_ps2.is_open())
    {
        naF_ps2.close();
        if ( calcChecksum(na_ps2.c_str()) == SCPH77001_BIN)
        {
            printf( "PS2 bios found with signature 'North America SCPH-77001': %s\n", na_ps2.c_str());
            found_na_ps2 = true;
        }
    }
    
    if (euF_ps2.is_open())
    {
        euF_ps2.close();
        if ( calcChecksum(eu_ps2.c_str()) == SCPH77002_BIN)
        {
            printf( "PS2 bios found with signature 'Europe SCPH-77002'       : %s\n", eu_ps2.c_str());
            found_eu_ps2 = true;
        }
    }
    
    if (unF_ps2.is_open())
    {
        unF_ps2.close();
    }
    else
    {
        tempBios_ps2_bios = "";
    }
    // if not all files are installed in base directory search firmware path
    if (!(found_jp_ps1 && found_na_ps1 && found_eu_ps1 && found_jp_ps2 && found_na_ps2 && found_eu_ps2))
    {
        if (gPathToFirmwarePSX!="")
        {
            std::list<string> tmpList_ps1;
            std::list<string> tmpList_ps2;
            
            findAllBiosFiles(gPathToFirmwarePSX.c_str(),&tmpList_ps1,&tmpList_ps2);
            
            for (string str : tmpList_ps1)
            {
                
                if (( calcChecksum(str.c_str()) == SCPH5500_BIN) && !found_jp_ps1)
                {
                    printf( "PS1 bios found with signature 'Japan SCPH-5500/v3.0J'        : %s\n", str.c_str());
                    found_jp_ps1 = copyFile(str.c_str(),jp_ps1.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH5501_BIN) && !found_na_ps1)
                {
                    printf( "PS1 bios found with signature 'North America SCPH-5501/v3.0A': %s\n", str.c_str());
                    found_na_ps1 = copyFile(str.c_str(),na_ps1.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH5502_BIN) && !found_eu_ps1)
                {
                    printf( "PS1 bios found with signature 'Europe SCPH-5502/v3.0E'       : %s\n", str.c_str());
                    found_eu_ps1 = copyFile(str.c_str(),eu_ps1.c_str());
                }
            }
            
            for (string str : tmpList_ps2)
            {
                tempBios_ps2_bios = str;
                if (( calcChecksum(str.c_str()) == SCPH77000_BIN) && !found_jp_ps2)
                {
                    printf( "PS2 bios found with signature 'Japan SCPH-77000'        : %s\n", str.c_str());
                    found_jp_ps2 = copyFile(str.c_str(),jp_ps2.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH77001_BIN) && !found_na_ps2)
                {
                    printf( "PS2 bios found with signature 'North America SCPH-77001': %s\n", str.c_str());
                    found_na_ps2 = copyFile(str.c_str(),na_ps2.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH77002_BIN) && !found_eu_ps2)
                {
                    printf( "PS2 bios found with signature 'Europe SCPH-77002'       : %s\n", str.c_str());
                    found_eu_ps2 = copyFile(str.c_str(),eu_ps2.c_str());
                }
            }
        }
    }
    // if still not all files are installed in base directory search game path
    if (!(found_jp_ps1 && found_na_ps1 && found_eu_ps1 && found_jp_ps2 && found_na_ps2 && found_eu_ps2))
    {
        if (gPathToGames!="")
        {
            std::list<string> tmpList_ps1;
            std::list<string> tmpList_ps2;
            findAllBiosFiles(gPathToGames.c_str(),&tmpList_ps1,&tmpList_ps2);
            
            for (string str : tmpList_ps1)
            {
                
                if (( calcChecksum(str.c_str()) == SCPH5500_BIN) && !found_jp_ps1)
                {
                    printf( "PS1 bios found with signature 'Japan SCPH-5500/v3.0J'        : %s\n", str.c_str());
                    found_jp_ps1 = copyFile(str.c_str(),jp_ps1.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH5501_BIN) && !found_na_ps1)
                {
                    printf( "PS1 bios found with signature 'North America SCPH-5501/v3.0A': %s\n", str.c_str());
                    found_na_ps1 = copyFile(str.c_str(),na_ps1.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH5502_BIN) && !found_eu_ps1)
                {
                    printf( "PS1 bios found with signature 'Europe SCPH-5502/v3.0E'       : %s\n", str.c_str());
                    found_eu_ps1 = copyFile(str.c_str(),eu_ps1.c_str());
                }
            }
            
            for (string str : tmpList_ps2)
            {
                tempBios_ps2_bios = str;
                if (( calcChecksum(str.c_str()) == SCPH77000_BIN) && !found_jp_ps2)
                {
                    printf( "PS2 bios found with signature 'Japan SCPH-77000'        : %s\n", str.c_str());
                    found_jp_ps2 = copyFile(str.c_str(),jp_ps2.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH77001_BIN) && !found_na_ps2)
                {
                    printf( "PS2 bios found with signature 'North America SCPH-77001': %s\n", str.c_str());
                    found_na_ps2 = copyFile(str.c_str(),na_ps2.c_str());
                }
                if (( calcChecksum(str.c_str()) == SCPH77002_BIN) && !found_eu_ps2)
                {
                    printf( "PS2 bios found with signature 'Europe SCPH-77002'       : %s\n", str.c_str());
                    found_eu_ps2 = copyFile(str.c_str(),eu_ps2.c_str());
                }
            }
        }
    }    
    gPS1_firmware = found_jp_ps1 || found_na_ps1 || found_eu_ps1;
    gPS2_firmware = found_jp_ps2 || found_na_ps2 || found_eu_ps2;
    
    if (gPS2_firmware)
    {
        if (found_jp_ps2)
            gPathToFirmwarePS2 = jp_ps2; // faster than NA and EU because it doesn't have a language settings dialog
        else if (found_na_ps2)
            gPathToFirmwarePS2 = na_ps2;
        else if (found_eu_ps2)
            gPathToFirmwarePS2 = eu_ps2;
    } 
    else
    {
        if (tempBios_ps2_bios!="")
        {
            gPathToFirmwarePS2 = gBaseDir + "tempBios.bin";
            if (tempBios_ps2_bios!=gPathToFirmwarePS2) 
                copyFile(tempBios_ps2_bios.c_str(),gPathToFirmwarePS2.c_str());
            printf( "PS2 bios found: %s\n", tempBios_ps2_bios.c_str());
            gPS2_firmware = true;
        }
    }    
}

void checkFirmwareSEGA_SATURN(void)
{
    bool found_jp_sega_saturn = false;
    bool found_na_eu_sega_saturn = false;
    
    string jp_sega_saturn = gBaseDir + "mednafen/firmware/sega_101.bin";
    string na_eu_sega_saturn = gBaseDir + "mednafen/firmware/mpr-17933.bin";

    ifstream jpF_sega_saturn (jp_sega_saturn.c_str());
    ifstream na_euF_sega_saturn (na_eu_sega_saturn.c_str());
    
    // check if sega saturn bios files are already installed in base directory
    if (jpF_sega_saturn.is_open())
    {
        jpF_sega_saturn.close();
        if ( calcChecksum(jp_sega_saturn.c_str()) == SEGA_SATURN_BIOS_JP)
        {
            printf( "Sega Saturn bios found with signature 'Japan'                 : %s\n", jp_sega_saturn.c_str());
            found_jp_sega_saturn = true;
        }
    }
    
    if (na_euF_sega_saturn.is_open())
    {
        na_euF_sega_saturn.close();
        if ( calcChecksum(na_eu_sega_saturn.c_str()) == SEGA_SATURN_BIOS_NA_EU)
        {
            printf( "Sega Saturn bios found with signature 'North America / Europa': %s\n", na_eu_sega_saturn.c_str());
            found_na_eu_sega_saturn = true;
        }
    }
    
    // if not all files are installed in base directory search firmware path
    if (!(found_jp_sega_saturn && found_na_eu_sega_saturn))
    {
        if (gPathToFirmwarePSX!="")
        {
            std::list<string> tmpList_sega_saturn;
            
            findAllBiosFiles(gPathToFirmwarePSX.c_str(),&tmpList_sega_saturn);
            
            for (string str : tmpList_sega_saturn)
            {
                if (( calcChecksum(str.c_str()) == SEGA_SATURN_BIOS_JP) && !found_jp_sega_saturn)
                {
                    printf( "Sega Saturn bios found with signature 'Japan'                 : %s\n", str.c_str());
                    found_jp_sega_saturn = copyFile(str.c_str(),jp_sega_saturn.c_str());
                }
                if (( calcChecksum(str.c_str()) == SEGA_SATURN_BIOS_NA_EU) && !found_na_eu_sega_saturn)
                {
                    printf( "Sega Saturn bios found with signature 'North America / Europa': %s\n", str.c_str());
                    found_na_eu_sega_saturn = copyFile(str.c_str(),na_eu_sega_saturn.c_str());
                }
            }
        }
    }
    // if still not all files are installed in base directory search game path
    if (!(found_jp_sega_saturn && found_na_eu_sega_saturn))
    {
        if (gPathToGames!="")
        {
            std::list<string> tmpList_sega_saturn;
            findAllBiosFiles(gPathToGames.c_str(),&tmpList_sega_saturn);
            
            for (string str : tmpList_sega_saturn)
            {
                
                if (( calcChecksum(str.c_str()) == SEGA_SATURN_BIOS_JP) && !found_jp_sega_saturn)
                {
                    printf( "Sega Saturn bios found with signature 'Japan'                 : %s\n", str.c_str());
                    found_jp_sega_saturn = copyFile(str.c_str(),jp_sega_saturn.c_str());
                }
                if (( calcChecksum(str.c_str()) == SEGA_SATURN_BIOS_NA_EU) && !found_na_eu_sega_saturn)
                {
                    printf( "Sega Saturn bios found with signature 'North America / Europa': %s\n", str.c_str());
                    found_na_eu_sega_saturn = copyFile(str.c_str(),na_eu_sega_saturn.c_str());
                }
            }
        }
    }    
    gSegaSaturn_firmware = found_jp_sega_saturn || found_na_eu_sega_saturn;
}


void printSupportedEmus(void)
{
    bool notEmpty;
    int i;
    notEmpty = (gSupportedEmulators.size() > 0);
    if (notEmpty)
    {
        printf("Supported emulators: ");
        for (i=0;i<gSupportedEmulators.size()-1;i++)
        {
            printf("%s, ",gSupportedEmulators[i].c_str());
        }
        printf("%s\n",gSupportedEmulators[i].c_str());    
    }
}
bool createDir(string name)
{	
    bool ok = true;
	DIR* dir;        
	dir = opendir(name.c_str());
	if ((dir) && (isDirectory(name.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		// Directory does not exist
		printf("creating directory %s ",name.c_str());
		if (mkdir(name.c_str(), S_IRWXU ) == 0)
		{
			printf("(ok)\n");
		}
		else
		{
			printf("(failed)\n");
            ok = false;
		}
	}
    return ok;
}
void initMEDNAFEN(void)
{
    createDir(gBaseDir+"mednafen");
    createDir(gBaseDir+"mednafen/firmware");
}
void initMUPEN64PLUS(void)
{
	string font_dir = gBaseDir;
	font_dir += "fonts";
	
	DIR* dir;        
	dir = opendir(font_dir.c_str());
	if ((dir) && (isDirectory(font_dir.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		// Directory does not exist
		printf("creating directory %s ",font_dir.c_str());
		if (mkdir(font_dir.c_str(), S_IRWXU ) == 0)
		{
			printf("(ok)\n");
		}
		else
		{
			printf("(failed)\n");
		}
	}
	
	vector<string> fonts = {"mupen64plus-core/data/font.ttf"};
	
    for (int i = 0; i < fonts.size(); i++)
    {	
		string font = gBaseDir;
		font += "fonts/font.ttf";
		
		if (( access( font.c_str(), F_OK ) == -1 ))
		{
			//file does not exist
			string uri = "resource:///fonts/";
			uri += fonts[i].c_str();
			GError *error;
			GFile* out_file = g_file_new_for_path(font.c_str());
			GFile* src_file = g_file_new_for_uri(uri.c_str());
			g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
		}
	}	
}

void initPCSX2(void)
{
	string plugin_dir = gBaseDir;
	plugin_dir += "PCSX2";
	
	DIR* dir;        
	dir = opendir(plugin_dir.c_str());
	if ((dir) && (isDirectory(plugin_dir.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		// Directory does not exist
		printf("creating directory %s ",plugin_dir.c_str());
		if (mkdir(plugin_dir.c_str(), S_IRWXU ) == 0)
		{
			printf("(ok)\n");
		}
		else
		{
			printf("(failed)\n");
		}
	}
	
	vector<string> plugins = {
		"libCDVDnull.so",
		"libcdvdGigaherz.so",
		"libUSBnull-0.7.0.so",
		"libspu2x-2.0.0.so",
		"libFWnull-0.7.0.so",
		"libdev9ghzdrk-0.4.so",
		"libdev9null-0.5.0.so"
	};
	
    for (int i = 0; i < plugins.size(); i++)
    {	
		string plugin = gBaseDir;
		plugin += "PCSX2/";
		plugin += plugins[i].c_str();
		if (( access( plugin.c_str(), F_OK ) == -1 ))
		{
			//file does not exist
			string uri = "resource:///plugins/pcsx2/";
			uri += plugins[i].c_str();
			GError *error;
			GFile* out_file = g_file_new_for_path(plugin.c_str());
			GFile* src_file = g_file_new_for_uri(uri.c_str());
			g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
		}
	}	
}

void initPPSSPP(void)
{
	string ppsspp_dir = gBaseDir + "ppsspp";
	string assets_dir = gBaseDir + "ppsspp/assets";
	string command;
	
	DIR* dir;        
	dir = opendir(ppsspp_dir.c_str());
	if ((dir) && (isDirectory(ppsspp_dir.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		// Directory does not exist
		printf("creating directory %s ",ppsspp_dir.c_str());
		if (mkdir(ppsspp_dir.c_str(), S_IRWXU ) == 0)
		{
			printf("(ok)\n");
		}
		else
		{
			printf("(failed)\n");
		}
	}
	
	dir = opendir(assets_dir.c_str());
	if ((dir) && (isDirectory(assets_dir.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		string zip_file = gBaseDir + "ppsspp/ppsspp_assets.zip";
		if (( access( zip_file.c_str(), F_OK ) == -1 ))
		{
			//file does not exist
			string uri = "resource:///assets/ppsspp/ppsspp_assets.zip";
			GError *error;
			GFile* out_file = g_file_new_for_path(zip_file.c_str());
			GFile* src_file = g_file_new_for_uri(uri.c_str());
			g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
		}
		
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		
		chdir(ppsspp_dir.c_str());
		
		command = "unzip ppsspp_assets.zip";
		system(command.c_str());
		
		chdir(cwd);
	}
}

void initDOLPHIN(void)
{
	string dolphin_dir = gBaseDir + "dolphin-emu";
	string assets_dir = gBaseDir + "dolphin-emu/Data";
	string command;
	
	DIR* dir;        
	dir = opendir(dolphin_dir.c_str());
	if ((dir) && (isDirectory(dolphin_dir.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		// Directory does not exist
		printf("creating directory %s ",dolphin_dir.c_str());
		if (mkdir(dolphin_dir.c_str(), S_IRWXU ) == 0)
		{
			printf("(ok)\n");
		}
		else
		{
			printf("(failed)\n");
		}
	}
	
	dir = opendir(assets_dir.c_str());
	if ((dir) && (isDirectory(assets_dir.c_str()) ))
	{
		// Directory exists
		closedir(dir);
	} 
	else if (ENOENT == errno) 
	{
		string zip_file = gBaseDir + "dolphin-emu/dolphin_data_sys.zip";
		if (( access( zip_file.c_str(), F_OK ) == -1 ))
		{
			//file does not exist
			string uri = "resource:///assets/dolphin/dolphin_data_sys.zip";
			GError *error;
			GFile* out_file = g_file_new_for_path(zip_file.c_str());
			GFile* src_file = g_file_new_for_uri(uri.c_str());
			g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
		}
		
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		
		chdir(dolphin_dir.c_str());
		
		command = "unzip dolphin_data_sys.zip";
		system(command.c_str());
		
		chdir(cwd);
	}
}

void initEMU(void)
{
    printSupportedEmus();
    initMEDNAFEN();
    initPCSX2();
    initPPSSPP();
    initDOLPHIN();
    initMUPEN64PLUS();
    
    checkFirmwarePSX();
    if (!gPS1_firmware)
        printf("No valid bios/firmware path for PS1 found --> Use the setup screen to enter a bios/firmware path. Marley will search it recursively.\n");
        
    if (!gPS2_firmware)
        printf("No valid bios/firmware path for PS2 found --> Use the setup screen to enter a bios/firmware path. Marley will search it recursively.\n");
    
    checkFirmwareSEGA_SATURN();
    if (!gSegaSaturn_firmware)
        printf("No valid bios/firmware path for Sega Saturn found --> Use the setup screen to enter a bios/firmware path. Marley will search it recursively.\n");
    
    if (gGame.size())
    {
            printf("Available games:\n");
    }
    
    for (int i=0;i<gGame.size();i++)
    {
        printf("%s\n",gGame[i].c_str());
    }
}

bool exists(const char *fileName)
{
    ifstream infile(fileName);
    return infile.good();
}

bool isDirectory(const char *filename)
{
    struct stat p_lstatbuf;
    struct stat p_statbuf;
    bool ok = false;

    if (lstat(filename, &p_lstatbuf) < 0) 
    {
        //printf("abort\n");
    }
    else
    {
        if (S_ISLNK(p_lstatbuf.st_mode) == 1) 
        {
            //printf("%s is a symbolic link\n", filename);
        } 
        else 
        {
            if (stat(filename, &p_statbuf) < 0) 
            {
                //printf("abort\n");
            }
            else
            {
                if (S_ISDIR(p_statbuf.st_mode) == 1) 
                {
                    //printf("%s is a directory\n", filename);
                    ok = true;
                } 
            }
        }
    }
    return ok;
}

void stripList(list<string> *tmpList,list<string> *toBeRemoved)
{
    list<string>::iterator iteratorTmpList;
    list<string>::iterator iteratorToBeRemoved;
    
    string strRemove, strRemove_no_path, strList, strList_no_path;
    int i,j;
    
    iteratorToBeRemoved = toBeRemoved[0].begin();
    
    for (i=0;i<toBeRemoved[0].size();i++)
    {
        strRemove = *iteratorToBeRemoved;
        iteratorToBeRemoved++;
        iteratorTmpList = tmpList[0].begin();
        
        strRemove_no_path = strRemove;
        if(strRemove_no_path.find("/") != string::npos)
        {
            strRemove_no_path = strRemove.substr(strRemove_no_path.find_last_of("/") + 1);
        }
        
        for (j=0;j<tmpList[0].size();j++)
        {
            strList = *iteratorTmpList;
            
            strList_no_path = strList;
            if(strList_no_path.find("/") != string::npos)
            {
                strList_no_path = strList_no_path.substr(strList_no_path.find_last_of("/") + 1);
            }

            if ( strRemove_no_path == strList_no_path )
            {
                tmpList[0].erase(iteratorTmpList++);
            }
            else
            {
                iteratorTmpList++;
            }
        }
    }
}

bool checkForCueFiles(string str_with_path,std::list<string> *toBeRemoved)
{
    string line, name;
    bool file_exists = false;

    ifstream cueFile (str_with_path.c_str());
    if (!cueFile.is_open())
    {
        printf("Could not open cue file: %s\n",str_with_path.c_str());
    }
    else 
    {
        while ( getline (cueFile,line))
        {
            if (line.find("FILE") != string::npos)
            {
                str_with_path.substr(str_with_path.find_last_of(".") + 1);
                
                int start  = line.find("\"")+1;
                int length = line.find_last_of("\"")-start;
                name = line.substr(start,length);
                
                string name_with_path = name;
                if (str_with_path.find("/") != string::npos)
                {
                    name_with_path = str_with_path.substr(0,str_with_path.find_last_of("/")+1) + name;
                }

                if (exists(name.c_str()) || (exists(name_with_path.c_str())))
                {
                    toBeRemoved[0].push_back(name);
                    file_exists = true;
                } else return false;
            }
        }
    }
    return file_exists;
}

void findAllFiles(const char * directory, std::list<string> *tmpList, std::list<string> *toBeRemoved)
{
    string str_with_path, str_without_path;
    string ext, str_with_path_lower_case;
    DIR *dir;
    
    struct dirent *ent;
    if ((dir = opendir (directory)) != NULL) 
    {
        // print all files and directories in directory
        while ((ent = readdir (dir)) != NULL) 
        {
            str_with_path = directory;
            str_with_path +=ent->d_name;
            
            if (isDirectory(str_with_path.c_str()))
            {
                str_without_path =ent->d_name;
                if ((str_without_path != ".") && (str_without_path != ".."))
                {
                    str_with_path +="/";
                    findAllFiles(str_with_path.c_str(),tmpList,toBeRemoved);
                }
            }
            else
            {
                ext = str_with_path.substr(str_with_path.find_last_of(".") + 1);
                
                std::transform(ext.begin(), ext.end(), ext.begin(),
                    [](unsigned char c){ return std::tolower(c); });
                
                str_with_path_lower_case=str_with_path;
                std::transform(str_with_path_lower_case.begin(), str_with_path_lower_case.end(), str_with_path_lower_case.begin(),
                    [](unsigned char c){ return std::tolower(c); });
                
                for (int i=0;i<gFileTypes.size();i++)
                {
                    if ((ext == gFileTypes[i])  && \
                        (str_with_path_lower_case.find("battlenet") ==  string::npos) &&\
                        (str_with_path_lower_case.find("ps3") ==  string::npos) &&\
                        (str_with_path_lower_case.find("ps4") ==  string::npos) &&\
                        (str_with_path_lower_case.find("xbox") ==  string::npos) &&\
                        (str_with_path_lower_case.find("bios") ==  string::npos) &&\
                        (str_with_path_lower_case.find("firmware") ==  string::npos))
                    {
                        if (ext == "mdf")
                        {
                            string bin_file;
                            bin_file = str_with_path.substr(0,str_with_path.find_last_of(".")) + ".bin";
                            if (!exists(bin_file.c_str())) tmpList[0].push_back(str_with_path);
                        }
                        else if (ext == "cue")
                        {
                            if(checkForCueFiles(str_with_path,toBeRemoved)) 
                              tmpList[0].push_back(str_with_path);
                        }
                        else
                        {
                            tmpList[0].push_back(str_with_path);
                        }
                    }
                }
            }
        }
        closedir (dir);
    } 
}

bool findInVector(vector<string>* vec, string str)
{
    bool ok = false;
    string element;
    
    vector<string>::iterator it;
    
    for(it=vec[0].begin();it<vec[0].end();it++)
    {
        element = *it;
        if (element == str)
        {
            ok = true;
            break;
        }
    }
    return ok;
}

void finalizeList(std::list<string> *tmpList)
{
    list<string>::iterator iteratorTmpList;
    string strList;
    
    iteratorTmpList = tmpList[0].begin();
    
    for (int i=0;i<tmpList[0].size();i++)
    {
        strList = *iteratorTmpList;
        if (!findInVector(&gGame,strList)) //avoid duplicates
        {
            gGame.push_back(strList);
        }
        iteratorTmpList++;
    }
    
    if (!gGame.size())
    {
        gGamesFound = false;
    }
    else
    {
        gGamesFound = true;
    }
}

void buildGameList(void)
{
    std::list<string> tmpList;
    std::list<string> toBeRemoved;
    
    findAllFiles(gPathToGames.c_str(),&tmpList,&toBeRemoved);
    stripList (&tmpList,&toBeRemoved); // strip cue file entries
    finalizeList(&tmpList);
}


