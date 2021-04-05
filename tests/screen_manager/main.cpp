#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <SDL.h>
#include "../../include/controller.h"
#include "../../include/global.h"
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <list>

using namespace std;
extern int gTheme;
int screen_manager_main(int argc, char* argv[]);
bool isDirectory(const char *filename);
vector<string> gFileTypes = {"smc","iso","smd","bin","cue","z64","v64","nes", "sfc", "gba", "gbc", "wbfs","mdf"};
bool launch_request_from_screen_manager;
bool restart_screen_manager;
string game_screen_manager;
bool stopSearching;
bool gSegaSaturn_firmware;
bool found_jp_ps1;
bool found_na_ps1;
bool found_eu_ps1;
bool found_jp_ps2;
bool found_na_ps2;
bool found_eu_ps2;
vector<string> gSearchDirectoriesGames;
string gPathToFirmwarePS2;
string gPathToGames;

int gControllerConfNum;
bool gControllerConf;
bool gFullscreen;
string gPackageVersion;
int window_width = 1280;
int window_height = 750;
int window_x=200; 
int window_y=200;
int gActiveController=-1;
string gConfText,gConfText2;
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 750;

SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];
int devicesPerType[] = {CTRL_TYPE_STD_DEVICES,CTRL_TYPE_WIIMOTE_DEVICES};
T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];
int gNumDesignatedControllers;
string gBaseDir;
SDL_Window* gWindow = nullptr;

void printJoyInfoAll(void)
{
    for (int l=0; l < MAX_GAMEPADS;l++)
    {
        if ( gDesignatedControllers[l].numberOfDevices != 0 )
        {
            char *mapping;
            for (int j=0;j<gDesignatedControllers[l].numberOfDevices;j++)
            {
                mapping = SDL_GameControllerMapping(gDesignatedControllers[l].gameCtrl[j]);
                printf("\n\n%s\n\n",mapping);
                SDL_free(mapping);
            }
        }
    }
}

void mainLoopWii(void) {}

bool closeJoy(int instance_id)
{
    printf("jc: bool closeJoy(int instance_id = %d)\n",instance_id);
    int designation, instance, n, num_controller, ctrlType, devPerType;
    
    //Close gamepad
    SDL_JoystickClose( gGamepad[instance_id] );
    
    // remove designated controller / from designated controller
    if (instance_id < MAX_GAMEPADS_PLUGGED)
    {
        gGamepad[instance_id] = NULL;
    
        for (designation=0;designation<MAX_GAMEPADS;designation++)
        {   
            ctrlType = gDesignatedControllers[designation].controllerType;
            devPerType = devicesPerType[ctrlType];
            for (int j=0; j < devPerType; j++)
            {
                instance= gDesignatedControllers[designation].instance[j];
                
                if ((instance == instance_id) && (instance != -1))
                {
                    printf("removing from designated controller %i (instance %i)",designation,gDesignatedControllers[designation].instance[j]);
                    gDesignatedControllers[designation].instance[j] = -1;
                    gDesignatedControllers[designation].index[j] = -1;
                    gDesignatedControllers[designation].name[j] = "";
                    gDesignatedControllers[designation].nameDB[j] = "";
                    gDesignatedControllers[designation].joy[j] = NULL;
                    gDesignatedControllers[designation].mappingOKDevice[j] = false;
                    gDesignatedControllers[designation].gameCtrl[j] = NULL;
                    gDesignatedControllers[designation].numberOfDevices--;
                    
                    printf(" number of devices on this controller remaining: %i\n",gDesignatedControllers[designation].numberOfDevices);
                    
                    if (gDesignatedControllers[designation].numberOfDevices == 0) 
                    {
                        gDesignatedControllers[designation].mappingOK = false;
                        gDesignatedControllers[designation].controllerType = -1;
                        gNumDesignatedControllers--;
                    }
                }
            }
        }
           
        printf("remaining devices: %i, remaining designated controllers: %i\n",SDL_NumJoysticks(),gNumDesignatedControllers);
        for (designation=0;designation<MAX_GAMEPADS;designation++)
        {
            for (int j=0;j<devPerType;j++)
            {
                instance= gDesignatedControllers[designation].instance[j];
                if (instance != -1)
                {
                   printf("remaining: instance %i on designated controller %i\n",gDesignatedControllers[designation].instance[j],designation);
                }
            }
        }
    }
    return true;
}

void finalizeList(std::list<string> *tmpList)
{
//    printf("jc: void finalizeList(std::list<string> *tmpList)\n");
    list<string>::iterator iteratorTmpList;
    string strList;
    
    iteratorTmpList = tmpList[0].begin();
    
    for (int i=0;i<tmpList[0].size();i++)
    {
        strList = *iteratorTmpList;
        iteratorTmpList++;
    }
}


void stripList(list<string> *tmpList,list<string> *toBeRemoved)
{
//    printf("jc: void stripList(list<string> *tmpList,list<string> *toBeRemoved)\n");
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

bool exists(const char *filename)
{
    printf("jc: bool exists(const char *fileName=%s)\n",filename);
    ifstream infile(filename);
    return infile.good();
}

bool checkForCueFiles(string str_with_path,std::list<string> *toBeRemoved)
{
    printf("jc: bool checkForCueFiles(string str_with_path=%s,std::list<string> *toBeRemoved)\n",str_with_path.c_str());
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

void findAllFiles(const char * directory, std::list<string> *tmpList, std::list<string> *toBeRemoved, bool recursiveSearch=true)
{
//    printf("jc: void findAllFiles(const char * directory=%s, std::list<string> *tmpList, std::list<string> *toBeRemoved)\n",directory);
 
    string str_with_path, str_without_path;
    string ext, str_with_path_lower_case;
    DIR *dir;
    
    struct dirent *ent;
    if ((dir = opendir (directory)) != NULL) 
    {
        // search all files and directories in directory
        while (((ent = readdir (dir)) != NULL))
        {
            str_with_path = directory;
            str_with_path +=ent->d_name;
            
            if (isDirectory(str_with_path.c_str()) && (recursiveSearch))
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

bool setPathToGames(string str)
{
    printf("jc: bool setPathToGames(string str=%s)\n",str.c_str());
    DIR* dir;
    string filename=str;
    string slash;
    bool ok = false;
    
    slash = filename.substr(filename.length()-1,1);
    if (slash != "/")
    {
        filename += "/";
    }
    
    //check if already in list
    for (int i=0; i<gSearchDirectoriesGames.size();i++)
    {
        if (gSearchDirectoriesGames[i] == filename)
        {
            printf("duplicate in ~/.marley/marley.cfg found for 'search_dir_games=' \n");
            return false;
        }
    }
    
    dir = opendir(filename.c_str());
    if ((dir) && (isDirectory(filename.c_str()) ))
    {
        // Directory exists
        closedir(dir);
        gSearchDirectoriesGames.push_back(filename);
        gPathToGames = filename;
        ok = true;
    }
    return ok;    
}

bool isDirectory(const char *filename)
{
    //printf("jc: bool isDirectory(const char *filename=%s)\n",filename);
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


bool createDir(string name)
{	
    printf("jc: bool createDir(string name=%s)\n",name.c_str());
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

bool searchAllFolders(void)
{
    bool canceled=false;
        
    return canceled;
}
bool addSearchPathToConfigFile(string searchPath)
{
    bool searchDirAdded=true;
    
    return searchDirAdded;
}

void setTheme(void)
{
    std::string marley_cfg = gBaseDir + "marley.cfg";
    std::string line;

    //if marley.cfg exists get value from there
    std::ifstream marley_cfg_filehandle(marley_cfg);
    if (marley_cfg_filehandle.is_open())
    {
        while ( getline (marley_cfg_filehandle,line))
        {
            if(line.find("ui_theme") != std::string::npos)
            {
                if (line.find("PC") != std::string::npos)
                {
                    gTheme = THEME_PLAIN;
                } else
                {
                    gTheme = THEME_RETRO;
                }
            }
        }
        marley_cfg_filehandle.close();
    }
}
bool setBaseDir(void)
{
    const char *homedir;
    string filename, slash;
    DIR* dir;
    bool ok = false;
    
    gBaseDir = "";
    
    if ((homedir = getenv("HOME")) != NULL) 
    {
        filename = homedir;
        
        // add slash to end if necessary
        slash = filename.substr(filename.length()-1,1);
        if (slash != "/")
        {
            filename += "/";
        }
        
        filename = filename + ".marley/";
        
        dir = opendir(filename.c_str());
        if (dir)
        {
            // Directory exists
            closedir(dir);
            ok = true;
        }
        else if (ENOENT == errno) 
        {
            // Directory does not exist
            printf("creating directory %s ",filename.c_str());
            if (mkdir(filename.c_str(), S_IRWXU ) == 0)
            {
                printf("(ok)\n");
                ok = true;
            }
            else
            {
                printf("(failed)\n");
            }
        }
    }    
    if (ok) gBaseDir=filename;
    return ok;
}

bool initJoy(void)
{
    bool ok = true;
    int i;
    
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    string internal = gBaseDir;
    internal += "internaldb.txt";
    
    if ( SDL_GameControllerAddMappingsFromFile(internal.c_str()) == -1 )
    {
    }
    if( SDL_GameControllerAddMappingsFromFile("../../resources/gamecontrollerdb.txt") == -1 )
    {
        printf( "Warning: Unable to open gamecontrollerdb.txt\n");
    }
    
    for (i=0; i< MAX_GAMEPADS_PLUGGED; i++)
    {
        gGamepad[i] = NULL;
    }
    
    for (i=0; i< MAX_GAMEPADS; i++)
    {
        for (int j=0; j<MAX_DEVICES_PER_CONTROLLER;j++)
        {
            gDesignatedControllers[i].instance[j] = -1;
            gDesignatedControllers[i].index[j] = -1;
            gDesignatedControllers[i].name[j] = "";
            gDesignatedControllers[i].nameDB[j] = "";
            gDesignatedControllers[i].joy[j] = NULL;
            gDesignatedControllers[i].mappingOKDevice[j] = false;
            gDesignatedControllers[i].gameCtrl[j] = NULL;
        }
        gDesignatedControllers[i].mappingOK = false;
        gDesignatedControllers[i].controllerType = -1;
        gDesignatedControllers[i].numberOfDevices = 0;
    }
    gNumDesignatedControllers = 0;
    return ok;
}

bool printJoyInfo(int i)
{
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    char guidStr[1024];
    const char* name = SDL_JoystickName(joy);
    int num_axes = SDL_JoystickNumAxes(joy);
    int num_buttons = SDL_JoystickNumButtons(joy);
    int num_hats = SDL_JoystickNumHats(joy);
    int num_balls = SDL_JoystickNumBalls(joy);
    int instance = SDL_JoystickInstanceID(joy);
    char *mapping;
    SDL_GameController *gameCtrl;
    
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
    
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    printf("Index: %i  ", i);
    printf("Instance: %i  ", instance);
    printf("Name: %s\n", SDL_JoystickNameForIndex(i));
    printf("Number of Axes: %d  ", SDL_JoystickNumAxes(joy));
    printf("Number of Buttons: %d  ", SDL_JoystickNumButtons(joy));
    printf("Number of Balls: %d,  ", SDL_JoystickNumBalls(joy));
    //printf("GUID: %s", guidStr); //printed later
    
    if (SDL_IsGameController(i)) 
    {
        gameCtrl = SDL_GameControllerOpen(i);
        mapping = SDL_GameControllerMapping(gameCtrl);
        if (mapping) 
        {
            printf(" compatible and mapped as\n\n%s\n\n", mapping);
            SDL_free(mapping);
        }
    }
    else 
    {
        printf("\nIndex \'%i\' is not a compatible controller.", i);
    }
    
    return true;
}

bool findGuidInFile(string filename, string text2match, int length, string* lineRet)
{
    const char* file = filename.c_str();
    bool ok = false;
    string line;
    string text = text2match.substr(0,length);
    
    lineRet[0] = "";
    
    ifstream fileHandle (file);
    if (!fileHandle.is_open())
    {
        printf("Could not open file: findGuidInFile(%s,%s,%i)\n",filename.c_str(),text2match.c_str(),length);
    }
    else 
    {
        while ( getline (fileHandle,line) && !ok)
        {
            if (line.find(text.c_str()) == 0)
            {
                //printf("found!\n");
                ok = true;
                lineRet[0]=line;
            }
        }
        fileHandle.close();
    }
   
    return ok;
}


bool checkMapping(SDL_JoystickGUID guid, bool* mappingOK, string name)
{
    char guidStr[1024];
    string line, append, filename;
    
    mappingOK[0] = false;
    
    //set up guidStr
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    
    filename = gBaseDir + "internaldb.txt";
    if (findGuidInFile(filename.c_str(), guidStr,32,&line))
    {
        printf("GUID found in internal db\n");
        mappingOK[0] = true;
    }
    else
    {

        //check public db
        mappingOK[0] = findGuidInFile("../../resources/gamecontrollerdb.txt", guidStr,32,&line);
        
        if (mappingOK[0])
        {
            printf("GUID found in public db\n");
        }
        else
        {
            string lineOriginal;
            printf("GUID %s not found in public db", guidStr);
            for (int i=27;i>18;i--)
            {
                
                //check in public db
                mappingOK[0] = findGuidInFile("../../resources/gamecontrollerdb.txt",guidStr,i,&line);
                
                if (mappingOK[0])
                {
                    // initialize controller with this line
                    lineOriginal = line;
                    int pos = line.find(",");
                    append = line.substr(pos+1,line.length()-pos-1);
                    
                    pos = append.find(",");
                    append = append.substr(pos+1,append.length()-pos-1);
                    
                    line = guidStr;
                    append=line+","+name+","+append;
                    
                    
                    std::ofstream outfile;
                    filename = gBaseDir + "tmpdb.txt";

                    outfile.open(filename.c_str(), std::ios_base::app); 
                    if(outfile) 
                    {
                        outfile << append + "\n"; 
                        outfile.close();
                        SDL_GameControllerAddMappingsFromFile(filename.c_str());
                        mappingOK[0]=true;
                    }
                    
                    break;
                }
            }
            if (mappingOK[0]) printf("\n%s: trying to load mapping from closest match\n%s\n",guidStr, lineOriginal.c_str());
            printf("\n");
        }    
    }
    return mappingOK[0];
}

bool checkControllerIsSupported(int i)
{
    // This function is rough draft.
    // There might be controllers that SDL allows but Marley not
    
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    
    bool ok= false;
    string unsupported = "Nintendo Wii";
    string name = SDL_JoystickName(joy);
    int str_pos = name.find(unsupported);
    
    // check for unsupported
    if (str_pos>=0)
    {
        printf("not supported, ignoring controller: %s\n",name.c_str());
        ok=false;
    } 
    else
    {
        ok=true;
    }
    return ok;
}


bool openJoy(int i)
{
    int designation, instance, devPerType;
    int device, numberOfDevices, ctrlType;
    bool mappingOK;
    char *mapping;
    
    if (i<MAX_GAMEPADS_PLUGGED)
    {
        SDL_Joystick *joy;
        joy = SDL_JoystickOpen(i);
        if (joy) {
            
            //Load gamepad
            gGamepad[i] = joy;
            if( gGamepad[i] == NULL )
            {
                printf( "Warning: Unable to open game gamepad! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                printJoyInfo(i);
                printf("opened %i  ",i);
                printf("active controllers: %i\n",SDL_NumJoysticks());
                if ( checkControllerIsSupported(i))
                {
                    //search for 1st empty slot
                    for (designation=0;designation< MAX_GAMEPADS; designation++)
                    {
                        numberOfDevices = gDesignatedControllers[designation].numberOfDevices;
                        ctrlType = gDesignatedControllers[designation].controllerType;
                        if (numberOfDevices != 0)
                        {
                            devPerType = devicesPerType[ctrlType];
                        }
                        else
                        {
                            devPerType = 0;
                        }
                        
                        if ( (numberOfDevices == 0) || ( (devPerType - numberOfDevices) > 0) )
                        {
                            if (numberOfDevices < MAX_DEVICES_PER_CONTROLLER)
                            {
                                device = gDesignatedControllers[designation].numberOfDevices;
                                gDesignatedControllers[designation].numberOfDevices++;
                                
                                instance = SDL_JoystickInstanceID(gGamepad[i]);
                                gDesignatedControllers[designation].instance[device] = instance;
                                gDesignatedControllers[designation].index[device] = i;
                                
                                string jName = SDL_JoystickNameForIndex(i);
                                transform(jName.begin(), jName.end(), jName.begin(),
                                    [](unsigned char c){ return tolower(c); });
                                gDesignatedControllers[designation].name[device] = jName;
                                
                                if (jName.find("nintendo wii remote") == 0)
                                {
                                    gDesignatedControllers[designation].controllerType = CTRL_TYPE_WIIMOTE;
                                }
                                else
                                {
                                    gDesignatedControllers[designation].controllerType = CTRL_TYPE_STD;
                                }
                                gDesignatedControllers[designation].joy[device] = joy;
                                gDesignatedControllers[designation].gameCtrl[device] = SDL_GameControllerFromInstanceID(instance);
                                SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
                                checkMapping(guid, &mappingOK,gDesignatedControllers[designation].name[device]);
                                gDesignatedControllers[designation].mappingOK = mappingOK;
                                
                                ctrlType = gDesignatedControllers[designation].controllerType;
                                if (gDesignatedControllers[designation].numberOfDevices == devicesPerType[ctrlType])
                                {
                                    gNumDesignatedControllers++;
                                }
                                
                                mapping = SDL_GameControllerMapping(gDesignatedControllers[designation].gameCtrl[device]);
                                if (mapping) 
                                {
                                    string str = mapping;
                                    SDL_free(mapping);
                                    //remove guid
                                    str = str.substr(str.find(",")+1,str.length()-(str.find(",")+1));
                                    // extract name from db
                                    str = str.substr(0,str.find(","));
                                    
                                    transform(str.begin(), str.end(), str.begin(),
                                        [](unsigned char c){ return tolower(c); });
                                    
                                    gDesignatedControllers[designation].nameDB[device] = str;
                                }
                                
                                printf("adding to designated controller %i (instance %i)\n",designation,gDesignatedControllers[designation].instance[device]);
                                //cancel loop
                                designation = MAX_GAMEPADS;
                            }
                        }
                    }
                    //print all designated controllers to terminal
                    for (designation=0;designation<MAX_GAMEPADS;designation++)
                    {        
                        for (int j=0;j < gDesignatedControllers[designation].numberOfDevices;j++)
                        {
                            instance= gDesignatedControllers[designation].instance[j];
                            if (instance != -1)
                            {
                               printf("active: instance %i on designated controller %i\n",gDesignatedControllers[designation].instance[j],designation);
                            }
                        }
                    }
                }
            }
            
        } else {
            printf("Couldn't open Joystick %i\n",i);
        }

        // Close if opened
        if (SDL_JoystickGetAttached(joy)) {
            SDL_JoystickClose(joy);
        }            
        return true;
    } else
    {   
        return false;
    }
}

bool initGUI(void)
{
    bool ok = true;
    Uint32 windowFlags;
    int imgFlags;
    
    windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    

    //Create main window
    string str = "marley ";
    gWindow = SDL_CreateWindow( str.c_str(), 
                                SDL_WINDOWPOS_CENTERED, 
                                SDL_WINDOWPOS_CENTERED, 
                                WINDOW_WIDTH, 
                                WINDOW_HEIGHT, 
                                windowFlags );
    if( gWindow == NULL )
    {
        printf( "Error creating main window. SDL Error: %s\n", SDL_GetError() );
        ok = false;
    }
    
    return ok;
}

int main(int argc, char* argv[])
{

    int ppsspp_argc;
    char *ppsspp_argv[10];

    char arg1[1024];
    char arg2[1024];
    
    int n;
    string str;
    SDL_Event event;
    bool gQuit=false;

    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        return -1;
    }
    
    SDL_GL_ResetAttributes();
  
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
    
    setBaseDir();
    setTheme();
    initJoy();
    initGUI();
    SDL_ShowCursor(SDL_DISABLE);
    
    while( !gQuit )
    {
        while( SDL_PollEvent( &event ) != 0 )
        {
            // main event loop
            switch (event.type)
            {                    
                case SDL_JOYDEVICEADDED: 
                    printf("\n*** Found new controller ");
                    openJoy(event.jdevice.which);
                    gQuit=true;
                default:
                    break;
            }
        }
    } 
    
    str = "ppsspp";
    n = str.length(); 
    strcpy(arg1, str.c_str()); 

    ppsspp_argv[0] = arg1;

    if (argc > 1)
    {
        str = argv[1];
        n = str.length(); 
        strcpy(arg2, str.c_str());

        ppsspp_argv[1] = arg2;
        ppsspp_argc = 2; 
    }
    else
    {
        ppsspp_argc = 1; 
    }

    screen_manager_main(ppsspp_argc,ppsspp_argv);

    printf("jc exit test\n");

    return 0;
}
void statemachineConf(int cmd)
{
 
}

void statemachineConfAxis(int cmd, bool negative)
{
 
}

bool checkAxis(int cmd)
{
    
    bool ok = false;

    return ok;
}

bool checkTrigger(int cmd)
{
    bool ok = false;
    
    return ok;
}


void statemachineConfHat(int hat, int value)
{    
    
}

void resetStatemachine(void)
{
    
}

void startControllerConf(int controllerNum) {}
