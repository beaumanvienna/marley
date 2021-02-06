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

#include "../include/marley.h"
#include "../include/gui.h"
#include "../include/controller.h"
#include "../include/statemachine.h"
#include "../include/emu.h"
#include "../include/wii.h"
#include "../include/global.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <dirent.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "../resources/res.h"

#define PI 3.14159

double angle0L = 0;
double angle1L = 0;
double angle0R = 0;
double angle1R = 0;
double amplitude0L = 0;
double amplitude1L = 0;
double amplitude0R = 0;
double amplitude1R = 0;
double g_x0=0;
double g_y0=0;
double g_x1=0;
double g_y1=0;


void joyMotion(SDL_Event event, int designatedCtrl, double* x, double* y);
bool checkConf(void);
bool setBaseDir(void);
void initApp(void);
void render_splash(string onScreenDisplay);
void event_loop(void);
Uint32 splash_callbackfunc(Uint32 interval, void *param);
void checkFirmwareSEGA_SATURN(void);
void resetSearch(void);
TTF_Font* gFont = nullptr;
int gActiveController=-1;
bool ALT = false;
vector<string> gSearchDirectoriesGames;
vector<string> gSearchDirectoriesFirmware;
extern int findAllFiles_counter;
extern bool stopSearching;
extern bool stopSearchingDuringSplash;
extern SDL_TimerID splashTimer;
extern bool playSystemSounds;
extern int gTheme;
bool gStartUp=true;
bool gForceResourceUpdate = false;
string gPackageVersion;
bool pcsx2_window_tear_down = false;
//initializes SDL and creates main window
bool init(void)
{
    printf("jc: bool init(void)\n");
    //Initialization flag
    bool ok = true;
    int i,j;
    
    printf("This is marley version %s\n",PACKAGE_VERSION);

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        ok = false;
    }
    else
    {
        
        SDL_version compiled;
        SDL_version linked;

        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        
        printf("We compiled against SDL version %d.%d.%d.\n",
        compiled.major, compiled.minor, compiled.patch);
        printf("We are linking against SDL version %d.%d.%d.\n",
        linked.major, linked.minor, linked.patch);

        // load true type font
        TTF_Init();
        string font = "/fonts/dejavu-fonts-ttf/DejaVuSansMono-Bold.ttf";
        size_t file_size = 0;
        GBytes *mem_access = g_resource_lookup_data(res_get_resource(), font.c_str(), G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr);
        const void* dataPtr = g_bytes_get_data(mem_access, &file_size);

        if (dataPtr != nullptr && file_size) 
        {
            SDL_RWops* ttf_file = SDL_RWFromMem((void*) dataPtr,file_size);
            gFont = TTF_OpenFontRW(ttf_file, 1, 24);
            if (gFont == nullptr)
            {
                printf("Could not open %s\n",font.c_str());
                ok = false;
            }
        }
        else
        {        
            printf("Failed to retrieve resource data for %s\n", font.c_str());
            ok = false;
        }
        
        setBaseDir();
        initApp();
        if(!initJoy())
        {
            ok = false;
        } else
        {
            if (!initGUI())
            {
                ok = false;
            }
        }
    }
    
    checkConf();
#ifdef DOLPHIN
    initWii();
#endif
    initEMU();
    render_splash("");
    while (splashScreenRunning) 
    {
        //printf("jc: waiting for splash screen to finish  ");
        event_loop();
        render_splash("");
        SDL_Delay(100);
    }
    return ok;
}

void initApp(void)
{
    printf("jc: void initApp(void)\n");
    const char *homedir;
    string home_folder, slash, uri;
    string app_dir, app_starter;
    string icon_dir, app_icon;
    GError *error;
    GFile* out_file;
    GFile* src_file;
    DIR* dir;
    
    if ((homedir = getenv("HOME")) != nullptr) 
    {
        home_folder = homedir;
        
        // add slash to end if necessary
        slash = home_folder.substr(home_folder.length()-1,1);
        if (slash != "/")
        {
            home_folder += "/";
        }
    }
    else
    {
        home_folder += "~/";
    }
    
    app_dir = home_folder + ".local/share/applications/";
    
    dir = opendir(app_dir.c_str());

    if ((dir) && (isDirectory(app_dir.c_str()) ))
    {
        // Directory exists
        closedir(dir);
        
        app_starter = app_dir + "marley.desktop";
        if (( access( app_starter.c_str(), F_OK ) == -1 ) || gForceResourceUpdate)
        {
            //file does not exist or forced update
            uri = "resource:///app/marley.desktop";
            error = nullptr;
            out_file = g_file_new_for_path(app_starter.c_str());
            src_file = g_file_new_for_uri(uri.c_str());
            g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
        }
        
        icon_dir = home_folder + ".local/share/icons/";

        dir = opendir(icon_dir.c_str());
        if ((dir) && (isDirectory(icon_dir.c_str()) ))
        {
            // Directory exists
            closedir(dir);
        }
        else if (ENOENT == errno) 
        {
            // Directory does not exist
            printf("creating directory %s ",icon_dir.c_str());
            if (mkdir(icon_dir.c_str(), S_IRWXU ) == 0)
            {
                printf("(ok)\n");
            }
            else
            {
                printf("(failed)\n");
            }
        }

        app_icon = icon_dir + "marley.ico";
        if (( access( app_icon.c_str(), F_OK ) == -1 ) || gForceResourceUpdate)
        {
            //file does not exist or forced update
            uri = "resource:///app/marley.ico";
        
            error = nullptr;
            out_file = g_file_new_for_path(app_icon.c_str());
            src_file = g_file_new_for_uri(uri.c_str());
            g_file_copy (src_file, out_file, G_FILE_COPY_NONE, nullptr, nullptr, nullptr, &error);
        }
        
    } 
     
}

//Free media and shut down SDL
void closeAll(void)
{
    printf("jc: void closeAll(void)\n");
    //Free loaded textures
    freeTextures();
    
    //close gamepads
    closeAllJoy();
    
    //close GUI
    closeGUI();

    //Quit SDL and subsystems
    IMG_Quit();
    SDL_Quit();
}

int main( int argc, char* argv[] )
{
    printf("jc: int main( int argc, char* argv[] )\n");
    bool keepX11pointer=true;
    gFullscreen=false;
    gCurrentGame=0;
    
    for (int i = 1; i < gGame.size(); i++) 
    {
        gGame[i]="";
    }
    for (int i = 1; i < argc; i++) 
    {
        string str = argv[i];
        if (str.find("--version") == 0) 
        {
            // version from configure.ac
            printf("This is marley version %s\n",PACKAGE_VERSION);
            return 0;
        }
        if ((str.find("--help") == 0) || (str.find("-?") == 0))
        {
            printf("This is marley version %s\n\n",PACKAGE_VERSION);
            printSupportedEmus();
            printf("\nOptions:\n\n");
            printf("  --version             : print version\n");
            printf("  --fullscreen, -f      : start in fullscreen mode\n");
            printf("  --killX11pointer, -k  : switch off the mouse pointer for the entire desktop\n\n");
            printf("Use your controller or arrow keys/enter on your keyboard to navigate.\n\n");
            printf("Use \"l\" to print a list of detected controllers to the command line.\n\n");
            printf("Use \"f\" to toggle fullscreen.\n\n");
            printf("Use \"p\" to print the current gamepad mapping(s).\n\n");
            printf("Use \"F5\" to save and \"F7\" to load game states.\n\n");
            printf("Use the guide button to exit a game with no questions asked. The guide button is the big one in the middle.\n\n");
            printf("Use \"ESC\" to exit.\n\n");
            printf("Visit https://github.com/beaumanvienna/marley for more information.\n\n");
            return 0;
        }
        
        if ((str.find("--fullscreen") == 0) || (str.find("-f") == 0))
        {
            gFullscreen=true;
        } 
        
        if ((str.find("--killX11pointer") == 0) || (str.find("-k") == 0))
        {
            keepX11pointer=false;
        }
        
        if ((str.find("--update-resources") == 0) || (str.find("-u") == 0))
        {
            gForceResourceUpdate=true;
            printf("Updating resources\n");
        }
        
        if (str.find("-t") == 0)
        {
            pcsx2_window_tear_down=true;
        }
        
                    
        if (( access( str.c_str(), F_OK ) != -1 ))
        {
            //file exists
            gGame.push_back(str);
        }
        
    }
    
    if (!keepX11pointer) 
      hide_or_show_cursor_X11(CURSOR_HIDE);

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        gStartUp=false;
        gQuit=false;
        
        // launch into new interface
        statemachine(SDL_CONTROLLER_BUTTON_A);

        //main loop
        while( !gQuit )
        {
            SDL_Delay(33); // 30 fps
#ifdef DOLPHIN
            mainLoopWii();
#endif
            event_loop();
            renderScreen();
        }
        
    }

    //Free resources, shut down SDL
    closeAll();
#ifdef DOLPHIN
    shutdownWii();
#endif
    return 0;
}

void event_loop(void)
{
    int k,l,m;
    //Event handler
    SDL_Event event;
    
    
    //Handle events on queue
    while( SDL_PollEvent( &event ) != 0 )
    {
        // main event loop
        switch (event.type)
        {
            case SDL_KEYUP: 
                ALT = false;
                break;
            case SDL_KEYDOWN: 
                if (!gTextInput)
                {
                    switch( event.key.keysym.sym )
                    {
                        case SDLK_l:
                            k = SDL_NumJoysticks();
                            if (k)
                            {
                                printf("++++++++++++ List all (number: %i) ++++++++++++ \n",k);
                                for (l=0; l<k; l++)
                                {
                                    printJoyInfo(l);
                                }
                                m=0; //count designated controllers
                                for (l=0; l < MAX_GAMEPADS;l++)
                                {
                                    if ( gDesignatedControllers[l].numberOfDevices != 0 )
                                    {
                                        for (int j=0; j<gDesignatedControllers[l].numberOfDevices;j++)
                                        {
                                            printf("found on designated controller %i with SDL instance %i %s\n",\
                                                l, gDesignatedControllers[l].instance[j],gDesignatedControllers[l].name[j].c_str());
                                        }
                                        m++;
                                    }
                                }
                                printf("%i designated controllers found\n",m);
                            } else
                            {
                                printf("++++++++++++ no controllers found ++++++++++++ \n");
                            }
                            break;
                        case SDLK_f:
                            gFullscreen= !gFullscreen;
                            if (gFullscreen)
                            {
                                setFullscreen();
                            }
                            else
                            {
                                setWindowed();
                            }
                            break;
                        case SDLK_p:
                            printJoyInfoAll();
                            break;
                        case SDLK_ESCAPE:
                            printf("jc: ++++++++++++++++++++++++++++++++++++++++++ case SDLK_ESCAPE: ++++++++++++++++++++++++++++++++++++++++++ \n");
                            stopSearching=true;
                            if (splashScreenRunning) 
                            {
                                if (!stopSearchingDuringSplash) // ESC hit once
                                {
                                    stopSearchingDuringSplash=true;
                                    printf("jc: ############ ESC hit once ########## stop search ########################################### s\n");
                                }
                                else // ESC hit twice
                                {
                                    printf("jc: ############ ESC hit twice ########## stop splash screen ########################################### s\n");
                                    SDL_RemoveTimer(splashTimer);
                                    splash_callbackfunc(0,nullptr);
                                }
                            } else
                            {
                                if (gState == STATE_OFF)
                                {
                                    gQuit=true;
                                }
                                else
                                {
                                    resetStatemachine();
                                }
                            }
                            break;
                        case SDLK_DOWN:
                            gActiveController=-1;
                            if (ALT)
                            {
                                statemachine(SDL_CONTROLLER_BUTTON_B);
                            }
                            else
                            {
                                statemachine(event.key.keysym.sym);
                            }
                            break;
                        case SDLK_UP:
                        case SDLK_LEFT:
                        case SDLK_RIGHT:
                            gActiveController=-1;
                            statemachine(event.key.keysym.sym);
                            break;
                        case SDLK_RETURN:
                            gActiveController=-1;
                            if (gControllerConf)
                            {
                                statemachineConf(STATE_CONF_SKIP_ITEM);
                            }
                            else
                            {
                                if (ALT)
                                {
                                    statemachine(SDL_CONTROLLER_BUTTON_B);
                                }
                                else
                                {
                                    statemachine(SDL_CONTROLLER_BUTTON_A);
                                }
                            }
                            break;
                        case SDLK_LALT:
                            ALT = true;
                            break;
                        default:
                            printf("key not recognized \n");
                            break;
                    }
                }
                else // text input mode
                {
                    string str;
                    switch( event.key.keysym.sym )
                    {
                        case SDLK_BACKSPACE:
                            if ( gText.length() > 0 )
                            {
                                gText.pop_back();
                            }
                            break;
                        case SDLK_ESCAPE:
                                gTextInput=false;
                                stopSearching=true;
                                statemachine(SDL_CONTROLLER_BUTTON_A);
                            break;
                        case SDLK_RETURN:
                                statemachine(SDL_CONTROLLER_BUTTON_A);
                            break;
                        default:
                            (void) 0;
                            break;
                    }
                }
                break;
            case SDL_TEXTINPUT: 
                if (gTextInput)
                {
                    gText += event.text.text;
                }
                break;
            case SDL_JOYDEVICEADDED: 
            case SDL_CONTROLLERDEVICEADDED:
                printf("\n+++ Found new controller ");
                openJoy(event.jdevice.which);
                break;
            case SDL_JOYDEVICEREMOVED: 
            case SDL_CONTROLLERDEVICEREMOVED:
                printf("+++ controller removed\n");
                closeJoy(event.jdevice.which);
                break;
            case SDL_JOYHATMOTION: 
            
                if (gControllerConf)
                {
                    if ( (event.jhat.value == SDL_HAT_UP) || (event.jhat.value == SDL_HAT_DOWN) || \
                            (event.jhat.value == SDL_HAT_LEFT) || (event.jhat.value == SDL_HAT_RIGHT) )
                    {
                        if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                        {
                            gActiveController=0;  
                            statemachineConfHat(event.jhat.hat,event.jhat.value);
                        }
                        else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                        {
                            gActiveController=1;  
                            statemachineConfHat(event.jhat.hat,event.jhat.value);
                        }
                    }
                }
            
                break;
            case SDL_JOYAXISMOTION: 
                if (abs(event.jaxis.value) > ANALOG_DEAD_ZONE)
                {
                    if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                    {
                        joyMotion(event,0,&g_x0,&g_y0);
                    }
                    else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                    {
                        joyMotion(event,1,&g_x1,&g_y1);
                    }
                    
                    if((event.jaxis.axis == 0) || (event.jaxis.axis == 1))
                    {
                        angle0L=atan2(g_y0,g_x0)*180.0 / PI;
                        angle1L=atan2(g_y1,g_x1)*180.0 / PI;
                        amplitude0L=sqrt(g_x0*g_x0+g_y0*g_y0);
                        amplitude1L=sqrt(g_x1*g_x1+g_y1*g_y1);
                    } 
                    else if((event.jaxis.axis == 3) || (event.jaxis.axis == 4))
                    {
                        angle0R=atan2(g_y0,g_x0)*180.0 / PI;
                        angle1R=atan2(g_y1,g_x1)*180.0 / PI;
                        amplitude0R=sqrt(g_x0*g_x0+g_y0*g_y0);
                        amplitude1R=sqrt(g_x1*g_x1+g_y1*g_y1);
                    }
                }
                if (gControllerConf)
                {
                    if (abs(event.jaxis.value) > 16384)
                    {
                        if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                        {
                            gActiveController=0; 
                            statemachineConfAxis(event.jaxis.axis,(event.jaxis.value < 0));
                        }
                        else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                        {
                            gActiveController=1;  
                            statemachineConfAxis(event.jaxis.axis,(event.jaxis.value < 0));
                        }
                    }
                }
                break;
            case SDL_QUIT: 
                gQuit = true;
                break;
            case SDL_JOYBUTTONDOWN: 
                if (gControllerConf)
                {
                    if (event.jdevice.which == gDesignatedControllers[0].instance[0])
                    {
                        gActiveController=0;  
                        statemachineConf(event.jbutton.button);
                    }
                    else if (event.jdevice.which == gDesignatedControllers[1].instance[0])
                    {
                        gActiveController=1;  
                        statemachineConf(event.jbutton.button);
                    }
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN: 
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_GUIDE) stopSearching=true;
                if (event.cdevice.which == gDesignatedControllers[0].instance[0])
                {
                    gActiveController=0;  
                    statemachine(event.cbutton.button);
                }
                else if (event.cdevice.which == gDesignatedControllers[1].instance[0])
                {
                    gActiveController=1;  
                    statemachine(event.cbutton.button);
                }
                break;
            default: 
                (void) 0;
                break;
        }
    }
}

void restoreSDL(void)
{
    printf("jc: void restoreSDL(void)\n");
    restoreController();
    restoreGUI();
}

//Motion on gamepad x
void joyMotion(SDL_Event event, int designatedCtrl, double* x, double* y)
{
    printf("jc: void joyMotion(SDL_Event event, int designatedCtrl, double* x, double* y)\n");
    if((event.jaxis.axis == 0) || (event.jaxis.axis == 3))
    {
        //X axis motion
        x[0]=event.jaxis.value;
    }
    else if((event.jaxis.axis == 1) || (event.jaxis.axis == 4))
    {
        //Y axis motion
        y[0]=event.jaxis.value;
    }
}


bool createTemplate(string name)
{
    printf("jc: bool createTemplate(string name=%s)\n",name.c_str());
    bool ok=false;
    std::ofstream outfile;

    outfile.open(name.c_str(), std::ios_base::app); 
    if(outfile) 
    {
        string l1 = "# marley ";
        l1 += PACKAGE_VERSION;
        l1 += "\n\n\n";
        outfile << l1;
        outfile << "#directories (marley will search here for the PSX firmware files)\n"; 
        outfile << "search_dir_firmware_PSX=/home/marley/gaming/firmware/\n\n"; 
        outfile << "#directories (marley will search here for games)\n"; 
        outfile << "search_dir_games=/home/marley/gaming/\n\n"; 
        outfile << "\n"; 
        outfile.close();
        ok=true;
    }
    else
    {
        printf("Could not create config file %s\n",name.c_str());
    }
    return ok;
}

bool setPathToFirmware(string str)
{
    printf("jc: bool setPathToFirmware(string str=%s)\n",str.c_str());
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
    for (int i=0; i<gSearchDirectoriesFirmware.size();i++)
    {
        if (gSearchDirectoriesFirmware[i] == filename)
        {
            printf("duplicate in ~/.marley/marley.cfg found for 'search_dir_firmware_PSX=' \n");
            return false;
        }
    }
    
    dir = opendir(filename.c_str());
    if ((dir) && (isDirectory(filename.c_str()) ))
    {
        // Directory exists
        closedir(dir);
        gSearchDirectoriesFirmware.push_back(filename);
        gPathToFirmwarePSX = filename;
        ok = true;
    }
    return ok;    
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
            gPathToGames = filename;
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

void loadConfig(ifstream* configFile)
{
    printf("jc: void loadConfig(ifstream* configFile)\n");
    string entry, line, slash;
    int pos;
    DIR* dir;
    
    gPathToFirmwarePSX="";
    gPathToGames="";
    stopSearching=false;
    stopSearchingDuringSplash=false;
    
    while (( getline (configFile[0],line)) && !stopSearching)
    {
        printf("jc: line = %s\n",line.c_str());
        if (line.find("search_dir_firmware_PSX=") == 0)
        {
            pos=23;
            entry = line.substr(pos+1,line.length()-pos);
            if ( setPathToFirmware(entry) )
            {
                checkFirmwarePSX();
            }
            else
            {
                printf("Could not find firmware path for PSX %s\n",entry.c_str());
            }
        } else
        if (line.find("search_dir_games=") == 0)
        {
            pos=16;
            entry = line.substr(pos+1,line.length()-pos);
            if ( setPathToGames(entry) )
            {
                checkFirmwarePSX();
                buildGameList();
            }
        } else
        if(line.find("ui_theme") != std::string::npos)
        {
            if (line.find("PC") != std::string::npos)
            {
                gTheme = THEME_PLAIN;
            } else
            {
                gTheme = THEME_RETRO;
            }
        } else
        if(line.find("# marley") != std::string::npos)
        {
            pos=8;
            entry = line.substr(pos+1,line.length()-pos);
            gPackageVersion = PACKAGE_VERSION;
            if (entry != gPackageVersion) 
            {
                printf("Updating resources\n");
                gForceResourceUpdate = true;
            }
        } else
        if(line.find("system_sounds") != std::string::npos)
        {
            playSystemSounds = (line.find("true") != std::string::npos);
        }
    }
    configFile[0].close();
}

void loadConfigEarly(void)
{
    printf("jc: void loadConfigEarly(ifstream* configFile)\n");
    bool found_system_sounds = false;
    string line;

    string marley_cfg = gBaseDir + "marley.cfg";

    //if marley.cfg exists get value from there
    ifstream marley_cfg_filehandle(marley_cfg);
    if (marley_cfg_filehandle.is_open())
    {
        while ( getline (marley_cfg_filehandle,line))
        {
            if(line.find("system_sounds") != std::string::npos)
            {
                found_system_sounds = true;
                playSystemSounds = (line.find("true") != std::string::npos);
            }
        }
        marley_cfg_filehandle.close();
    }
    if (!found_system_sounds) playSystemSounds = true;
}

bool setBaseDir(void)
{
    printf("jc: bool setBaseDir(void)\n");
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
    }
    else
    {
        filename += "~/";
    }
        
    filename = filename + ".marley/";
    
    dir = opendir(filename.c_str());
    if ((dir) && (isDirectory(filename.c_str()) ))
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

    if (ok) gBaseDir=filename;
    return ok;
}

bool checkConf(void)
{
    printf("jc: bool checkConf(void)\n");
    string filename;
    int pos;
    DIR* dir;
    bool ok = false;

    if ( gBaseDir != "") 
    {
        DIR* dir = opendir(gBaseDir.c_str());
        if ((dir) && (isDirectory(gBaseDir.c_str()) ))
        {
            // Directory exists
            closedir(dir);
            filename = gBaseDir;
            filename += "marley.cfg";
            ifstream configFile (filename.c_str());
            if (!configFile.is_open())
            {
                printf("Could not open config file: %s, creating template\n",filename.c_str());
                ok = createTemplate(filename);
            }
            else 
            {
                loadConfig(&configFile);
                ok = true;
            }
        } 
    }    
    return ok;
}

bool addSettingToConfigFile(string setting)
{
    printf("jc: bool addSettingToConfigFile(string setting=%s)\n",setting.c_str());
    bool ok = false;
    std::string str, line;
    std::vector<std::string> marley_cfg_entries;
    std::string marley_cfg = gBaseDir + "marley.cfg";
    std::ifstream marley_cfg_in_filehandle(marley_cfg);
    std::ofstream marley_cfg_out_filehandle;
    bool duplicate;
    if (marley_cfg_in_filehandle.is_open())
    {
        while ( getline (marley_cfg_in_filehandle,line))
        {
            duplicate = (line == setting);
            if (duplicate)
            {
                printf("Not adding duplicate: %s\n",setting.c_str());
                break;
            }
        }
        marley_cfg_in_filehandle.close();
    }
    
    // output marley.cfg
    if (!duplicate)
    {
        marley_cfg_out_filehandle.open(marley_cfg.c_str(), std::ios_base::app); 
        if(marley_cfg_out_filehandle)
        {
            marley_cfg_out_filehandle << setting << "\n";
            marley_cfg_out_filehandle.close();
            printf("Added to configuration file: %s\n",setting.c_str());
            ok = true;
        } 
        else
        {
            printf("Could not open config file: %s, no setting added\n",marley_cfg.c_str());
        }
    }
    
    return ok;
}

bool updateSearchPath(string searchPath)
{
    setPathToGames(searchPath);
        
    //update games list
    stopSearching=false;
    buildGameList();
    checkFirmwarePSX();
    checkFirmwareSEGA_SATURN();
    if (stopSearching) resetSearch();
    return stopSearching;
}

bool addSearchPathToConfigFile(string searchPath)
{
    bool searchDirAdded=false;
    string setting = "search_dir_games=" + searchPath;
    if (addSettingToConfigFile(setting))
    {
        updateSearchPath(searchPath);
        searchDirAdded=true;
    }
    return searchDirAdded;
}

bool searchAllFolders(void)
{
    bool canceled = false;
    
    //reset all
    resetSearch();
    
    //rebuild for every search directory
    for (int i = 0; i < gSearchDirectoriesGames.size();i++)
    {
        canceled = updateSearchPath(gSearchDirectoriesGames[i]);
        if (canceled) break;
    }
    
    return canceled;
}

void removeDuplicatesInDB(void)
{
    printf("jc: void removeDuplicatesInDB(void)\n");
    // If duplicate GUIDs are found,
    // this function keeps only the 1st encounter.
    // This is why addControllerToInternalDB()
    // is inserting new entries at the beginning.
    string line, guidStr;
    long guid;
    vector<string> entryVec;
    vector<string> guidVec;
    string filename;
    bool found;
    
    filename = gBaseDir + "internaldb.txt";
    
    ifstream internalDB(filename);
    if (!internalDB.is_open())
    {
        printf("Could not open file: removeDuplicate(), file %s \n",filename.c_str());
    }
    else 
    {
        while ( getline (internalDB,line))
        {
            found = false;
            if (line.find(",") != std::string::npos)
            {
                guidStr = line.substr(0,line.find(","));
                for (int i = 0;i < guidVec.size();i++)
                {
                    if (guidVec[i]==guidStr)
                    {
                        found=true;
                        break;
                    }
                }
            }
            if (!found)
            {
                guidVec.push_back(guidStr);
                entryVec.push_back(line);
            }
        }
        
        internalDB.close();

        ofstream internal_db_output_filehandle;
        internal_db_output_filehandle.open(filename.c_str(), ios_base::out);
        if (internal_db_output_filehandle.fail())
        {
            printf("Could not write internal game controller database: %s, no entry added\n",filename.c_str());
        }
        else 
        {
            for (int i=0;i < entryVec.size();i++)
            {
                internal_db_output_filehandle << entryVec[i] << + "\n";
            }
            internal_db_output_filehandle.close();
        }
    }
}

bool addControllerToInternalDB(string entry)
{
    printf("jc: bool addControllerToInternalDB(string entry=%s)\n",entry.c_str());
    bool ok = false;
    string line;
    string filename = gBaseDir + "internaldb.txt";
    vector<string> internal_db_entries;
    
    ifstream internal_db_input_filehandle(filename);
    
    if (internal_db_input_filehandle.is_open())
    {
        while ( getline (internal_db_input_filehandle,line))
        {
            internal_db_entries.push_back(line);
        }
        internal_db_input_filehandle.close();
    } else
    {
        printf("Creating internal game controller database %s\n",filename.c_str());
    }
    
    ofstream internal_db_output_filehandle;
    internal_db_output_filehandle.open(filename.c_str(), ios_base::out);
    if (internal_db_output_filehandle.fail())
    {
        printf("Could not write internal game controller database: %s, no entry added\n",filename.c_str());
    }
    else 
    {
        internal_db_output_filehandle << entry << + "\n"; 
        for (int i = 0; i < internal_db_entries.size(); i++)
        {
            internal_db_output_filehandle << internal_db_entries[i] << + "\n"; 
        }
        internal_db_output_filehandle.close();
        ok = true;
    }
    return ok;
}
