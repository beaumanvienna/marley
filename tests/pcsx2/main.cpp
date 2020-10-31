#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>
#include <SDL.h>
#include "../../include/controller.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <X11/Xlib.h>
#include <SDL_syswm.h>
using namespace std;

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 750;

int pcsx2_main(int argc, char* argv[]);


SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];
int devicesPerType[] = {CTRL_TYPE_STD_DEVICES,CTRL_TYPE_WIIMOTE_DEVICES};
T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];
int gNumDesignatedControllers;
string gBaseDir;
string gPathToFirmwarePS2;
SDL_Window* gWindow = nullptr;
Display* XDisplay;
Window Xwindow;

void initOpenGL(void)
{	
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
}

void create_new_window(void)
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    }
    // create new
    initOpenGL();

    string str = "marley pcsx2 unit test";
    gWindow = SDL_CreateWindow( str.c_str(), 
                                SDL_WINDOWPOS_CENTERED, 
                                SDL_WINDOWPOS_CENTERED, 
                                WINDOW_WIDTH, 
                                WINDOW_HEIGHT, 
                                (SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL) );
    
    SDL_SysWMinfo sdlWindowInfo;
    SDL_VERSION(&sdlWindowInfo.version);
    if(SDL_GetWindowWMInfo(gWindow, &sdlWindowInfo))
    {
        if(sdlWindowInfo.subsystem == SDL_SYSWM_X11) 
        {
            Xwindow      = sdlWindowInfo.info.x11.window;
            XDisplay     = sdlWindowInfo.info.x11.display;

        }
    } 
    else
    {
        printf("jc SDL_GetWindowWMInfo(gWindow, &sdlWindowInfo) failed\n");
    }
    SDL_ShowCursor(SDL_DISABLE);
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
                
        gPathToFirmwarePS2 = filename;
        gPathToFirmwarePS2 += ".marley/scph77001.bin";
        
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
    string gamecontrollerdb = gBaseDir;
    gamecontrollerdb += "gamecontrollerdb.txt";
    
    if ( SDL_GameControllerAddMappingsFromFile(internal.c_str()) == -1 )
    {
    }
    if( SDL_GameControllerAddMappingsFromFile(gamecontrollerdb.c_str()) == -1 )
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
    string line, append, filename, gamecontrollerdb;
    
    mappingOK[0] = false;
    
    //set up guidStr
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    gamecontrollerdb = gBaseDir + "gamecontrollerdb.txt";
    filename = gBaseDir + "internaldb.txt";
    if (findGuidInFile(filename.c_str(), guidStr,32,&line))
    {
        printf("GUID found in internal db\n");
        mappingOK[0] = true;
    }
    else
    {

        //check public db
        mappingOK[0] = findGuidInFile(gamecontrollerdb.c_str(), guidStr,32,&line);
        
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
                mappingOK[0] = findGuidInFile(gamecontrollerdb.c_str(),guidStr,i,&line);
                
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
    int pcsx2_argc;
    char *pcsx2_argv[10];    
    
    char arg1[1024];
    char arg2[1024];
    char arg3[1024];
    char arg4[1024];
    char arg5[1024];
    char arg6[1024];
    char arg7[1024];
    char arg8[1024];
    char arg9[1024];

    
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
    
    str = "pcsx2";
    n = str.length(); 
    strcpy(arg1, str.c_str()); 

	str = "--usb=";
	str += gBaseDir;
	str += "PCSX2/libUSBnull-0.7.0.so";  //ok
	n = str.length(); 
	strcpy(arg2, str.c_str()); 

	str = "--dev9=";
	str += gBaseDir;
	str += "PCSX2/libdev9null-0.5.0.so"; //ok
	n = str.length(); 
	strcpy(arg3, str.c_str()); 
    
    str = "--nogui";
    n = str.length(); 
    strcpy(arg4, str.c_str()); 

    str = "--fullboot";
    n = str.length(); 
    strcpy(arg5, str.c_str()); 

    pcsx2_argv[0] = arg1;
    pcsx2_argv[1] = arg2;
    pcsx2_argv[2] = arg3;
    pcsx2_argv[3] = arg4;
    pcsx2_argv[4] = arg5;

    if (argc > 1)
    {
        str = argv[1];
        n = str.length(); 
        strcpy(arg6, str.c_str());

        pcsx2_argv[5] = arg6;
        pcsx2_argc = 6;
    }
    else
    {
        pcsx2_argc = 3; // only plugins, no other options
    }

    SDL_SysWMinfo sdlWindowInfo;
    SDL_VERSION(&sdlWindowInfo.version);
    if(SDL_GetWindowWMInfo(gWindow, &sdlWindowInfo))
    {
        if(sdlWindowInfo.subsystem == SDL_SYSWM_X11) 
        {
            Xwindow      = sdlWindowInfo.info.x11.window;
            XDisplay     = sdlWindowInfo.info.x11.display;
            
            pcsx2_main(pcsx2_argc,pcsx2_argv);
            printf("jc *******************************\n");
            pcsx2_main(pcsx2_argc,pcsx2_argv);
            printf("jc *******************************\n");
            pcsx2_main(pcsx2_argc,pcsx2_argv);
            printf("jc *******************************\n");
        }
    }

    return 0;
}
