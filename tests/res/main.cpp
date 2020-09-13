
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <gtk/gtk.h>
#include "res.h"
#include <fstream>
#include <string>
#include <cstring>
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

#define NUM_TEXTURES        1
#define TEX_BACKGROUND   0

int mednafen_main(int argc, char* argv[]);
bool loadMedia(void);
bool setBaseDir(void);
bool openJoy(int i);
bool checkControllerIsSupported(int i);
bool checkMapping(SDL_JoystickGUID guid, bool* mappingOK, string name);
bool findGuidInFile(string filename, string text2match, int length, string* lineRet);
bool printJoyInfo(int i);
bool initJoy(void);

SDL_Joystick* gGamepad[MAX_GAMEPADS_PLUGGED];
int devicesPerType[] = {CTRL_TYPE_STD_DEVICES,CTRL_TYPE_WIIMOTE_DEVICES};
T_DesignatedControllers gDesignatedControllers[MAX_GAMEPADS];
int gNumDesignatedControllers;
string gBaseDir;

//rendering window 
SDL_Window* gWindow = nullptr;

//window renderer
SDL_Renderer* gRenderer = nullptr;

//textures
SDL_Texture* gTextures[NUM_TEXTURES];

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
    
    size_t file_size = 0;
    GBytes *mem_access = g_resource_lookup_data(res_get_resource(), "/database/gamecontrollerdb.txt", G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr);
    const void* dataPtr = g_bytes_get_data(mem_access, &file_size);
    
    if (dataPtr != nullptr && file_size) 
    {
    
           SDL_RWops* db_file = SDL_RWFromMem((void*) dataPtr,file_size);
           
           if (SDL_GameControllerAddMappingsFromRW(db_file, 1) == -1)
           {
               printf("gamecontrollerdb.txt could not be loaded\n");
           }
    }
    else
    {
           printf("Failed to retrieve resource data for gamecontrollerdb.txt\n");
    }
    
    g_bytes_unref(mem_access);
    
    return ok;
}


bool createRenderer(void)
{
    bool ok = true;
    SDL_GL_ResetAttributes();
    //Create renderer for main window
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( gRenderer == nullptr )
    {
        printf( "Error creating renderer. SDL error: %s\n", SDL_GetError() );
        ok = false;
    }
    else
    {
        SDL_RendererInfo rendererInfo;
        SDL_GetRendererInfo(gRenderer, &rendererInfo);
        std::cout << "Renderer: " << rendererInfo.name << std::endl;
        std::string str = rendererInfo.name;
        if (str.compare("opengl") != 0)
        {
            std::cout << "OpenGL not found. Please install an OpenGL driver for your graphics card. " << std::endl;
            exit(1);
        }
        
        SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
            ok = false;
        }
        SDL_RenderClear(gRenderer);
        
        //draw backround to main window
        SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],nullptr,nullptr);
        SDL_RenderPresent(gRenderer);
    }
    
    
    return ok;
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
    if( gWindow == nullptr )
    {
        printf( "Error creating main window. SDL Error: %s\n", SDL_GetError() );
        ok = false;
    }
    
    return ok;
}

SDL_Texture* loadTextureFromFile(string str)
{
    SDL_Surface* surf = nullptr;
    SDL_Texture* texture = nullptr;
    
    size_t file_size = 0;
    
    GBytes *mem_access = g_resource_lookup_data(res_get_resource(), "/pictures/beach.bmp", G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr);
    const void* dataPtr = g_bytes_get_data(mem_access, &file_size);

    if (dataPtr != nullptr && file_size) 
    {
    
           SDL_RWops* bmp_file = SDL_RWFromMem((void*) dataPtr,file_size);
           
           surf = SDL_LoadBMP_RW(bmp_file,0);
           if (!surf)
           {
               printf("File %s could not be loaded\n",str.c_str());
           }
           else
           {
               texture =SDL_CreateTextureFromSurface(gRenderer,surf);
               if (!texture)
               {
                      printf("texture for background could not be created.\n");
               }
               SDL_FreeSurface(surf);
           }
    }
    else
    {
           printf("Failed to retrieve resource data\n");
    }
    return texture;
}

bool loadMedia()
{
    bool ok = true;
    // background
    gTextures[TEX_BACKGROUND] = loadTextureFromFile("beach.bmp");
    if (!gTextures[TEX_BACKGROUND])
    {
        ok = false;
    }
    return ok;
}

void renderScreen(void)
{
    //render destination 
    SDL_Rect destination;
    
    //Clear screen
    SDL_RenderClear( gRenderer );
    //background
    SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],nullptr,nullptr);
    
    SDL_RenderPresent( gRenderer );

}


int main(int argc, char* argv[])
{
    
    int mednafen_argc;
    char *mednafen_argv[10];

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
    

    initGUI();
    SDL_ShowCursor(SDL_DISABLE);
    createRenderer();

    setBaseDir();
    initJoy();
    
    //main loop
    while( !gQuit )
    {
           SDL_Delay(33); // 30 fps
           
           //Handle events on queue
           while( SDL_PollEvent( &event ) != 0 )
           {
               // main event loop
               switch (event.type)
               {
                      case SDL_JOYDEVICEADDED: 
                    printf("\n*** Found new controller ");
                    openJoy(event.jdevice.which);
                    break;
                case SDL_KEYDOWN: 

                    switch( event.key.keysym.sym )
                    {                            
                        case SDLK_RETURN:
                        
                        
                            str = "mednafen";
                            n = str.length(); 
                            strcpy(arg1, str.c_str()); 

                            mednafen_argv[0] = arg1;

                            if (argc > 1)
                            {
                                str = argv[1];
                                n = str.length(); 
                                strcpy(arg2, str.c_str());

                                mednafen_argv[1] = arg2;
                                mednafen_argc = 2; 
                            }
                            else
                            {
                                mednafen_argc = 1; 
                            }

                            mednafen_main(mednafen_argc,mednafen_argv);
                        
                        
                            break;
                        case SDLK_ESCAPE:
                            gQuit=true;
                            break;
                        
                        default:
                            printf("key not recognized \n");
                            break;
                    }
                    
                    break;
                
                case SDL_QUIT: 
                    gQuit = true;
                    break;

                default: 
                    (void) 0;
                    break;
            }
        }
        renderScreen();
    }

    return 0;
}
bool setBaseDir(void)
{
    const char *homedir;
    string filename, slash;
    DIR* dir;
    bool ok = false;
    
    gBaseDir = "";
    
    if ((homedir = getenv("HOME")) != nullptr) 
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
    }    
    if (ok) gBaseDir=filename;
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
