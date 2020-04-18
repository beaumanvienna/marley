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

using namespace std;

bool gPSX_firmware;
string gPathToFirnwarePSX;
string gPathToGames;
string gBaseDir;
vector<string> gSupportedEmulators = {"psx","md (sega)","snes","nes"}; 
vector<string> gFileTypes = {"smc","iso","smd","bin","cue","z64","v64","nes"};
bool gGamesFound;

bool checkFirmwarePSX(void)
{
    if (gPathToFirnwarePSX!="")
    {
        int count = 0;
        string jp = gPathToFirnwarePSX + "scph5500.bin";
        string na = gPathToFirnwarePSX + "scph5501.bin";
        string eu = gPathToFirnwarePSX + "scph5502.bin";
    
        ifstream jpF (jp.c_str());
        ifstream naF (na.c_str());
        ifstream euF (eu.c_str());
        
        gPSX_firmware = false;
        
        if (jpF.is_open())
        {
            gPSX_firmware = true;
            count++;
        }
        else
        {
            printf("%s not found\n",jp.c_str());
        }
        
        if (naF.is_open())
        {
            gPSX_firmware = true;
            count++;
        }
        else
        {
            printf("%s not found\n",na.c_str());
        }
        
        if (euF.is_open())
        {
            gPSX_firmware = true;
            count++;
        }
        else
        {
            printf("%s not found\n",eu.c_str());
        }
        
        if (gPSX_firmware)
        {
            if (count<3)
            {
                printf("Not all PSX firmware files found. You might not be able to play games from all regions.\n");
            }
        }
    }
}

bool printSupportedEmus(void)
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

bool initEMU(void)
{
    printSupportedEmus();
    
    //check for PSX firmware
    if (gPathToFirnwarePSX == "")
    {
        printf("No valid firmware path for PSX found in marley.cfg\n");
    }
    else
    {
        checkFirmwarePSX();
    }
    
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

bool stripList(list<string> *tmpList,list<string> *toBeRemoved)
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
                break;
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

    ifstream cueFile (str_with_path.c_str());
    if (!cueFile.is_open())
    {
        printf("Could not open cue file: %s\n",str_with_path.c_str());
    }
    else 
    {
        while ( getline (cueFile,line))
        {
            if (line.find("FILE ") == 0)
            {
                str_with_path.substr(str_with_path.find_last_of(".") + 1);
                
                int start  = line.find("\"")+1;
                int length = line.find_last_of("\"")-start;
                name = line.substr(start,length);
                
                toBeRemoved[0].push_back(name);
            }
        }
    }
}

void findAllFiles(const char * directory, std::list<string> *tmpList, std::list<string> *toBeRemoved)
{
    string str_with_path, str_without_path;
    string ext, str_with_path_lower_case;
    DIR *dir;
    
    struct dirent *ent;
    if ((dir = opendir (directory)) != NULL) 
    {
        // print all the files and directories within directory
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
                        tmpList[0].push_back(str_with_path);
                        
                        //check if cue file
                        if (ext == "cue")
                        {
                            checkForCueFiles(str_with_path,toBeRemoved);
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

bool finalizeList(std::list<string> *tmpList)
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

bool buildGameList(void)
{
    std::list<string> tmpList;
    std::list<string> toBeRemoved;
    
    findAllFiles(gPathToGames.c_str(),&tmpList,&toBeRemoved);
    stripList (&tmpList,&toBeRemoved); // strip cue file entries
    finalizeList(&tmpList);
}


