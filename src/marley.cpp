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
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <dirent.h>
#include <errno.h>

#define PI 3.14159

bool joyMotion(SDL_Event event, int designatedCtrl, double* x, double* y);
bool joyButton(SDL_Event event, int designatedCtrl, bool* ignoreESC);
bool checkConf(void);

TTF_Font* gFont = NULL;

//initializes SDL and creates main window
bool init()
{
    //Initialization flag
    bool ok = true;
    int i,j;
    
    printf("This is marley version %s\n",PACKAGE_VERSION);

    //printf( "init()\n");
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

        TTF_Init();
        string font = RESOURCES "font.ttf";
        gFont = TTF_OpenFont(font.c_str(), 24);
        if (gFont == NULL)
        {
            printf("%s not found\n",font.c_str());
            ok = false;
        }
        
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
        //printf( "init() end\n");
    }
    
    checkConf();

    return ok;
}

//Frees media and shuts down SDL
void closeAll()
{
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
    int k,l,m,id;
    string cmd;
    bool ignoreESC=false;
    double angle0L = 0;
    double angle1L = 0;
    double angle0R = 0;
    double angle1R = 0;
    double amplitude0L = 0;
    double amplitude1L = 0;
    double amplitude0R = 0;
    double amplitude1R = 0;
    
    //render destination 
    SDL_Rect destination;
    
    gFullscreen=false;
    gGame="";
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
            printf("This is marley version %s\n",PACKAGE_VERSION);
            printf("\nOptions:\n\n");
            printf("  --version             : print version\n");
            printf("  --fullscreen, -f      : start in fullscreen mode\n\n");
            printf("Use your controller or arrow keys/enter on your keyboard to navigate.\n\n");
            printf("Use \"l\" to print a list of detected controllers to the command line.\n\n");
            printf("Use \"f\" to toggle fullscreen.\n\n");
            printf("Use \"1\" to exit.\n\n");
            printf("Use \"p\" to print the current gamepad mapping(s).\n\n");
            printf("Visit https://github.com/beaumanvienna/marley for more information.\n\n");
            return 0;
        }
        
        if ((str.find("--fullscreen") == 0) || (str.find("-f") == 0))
        {
            gFullscreen=true;
        } 
        
        if ((str.find(".cue") > 0) && (str.find("PS1") > 0))
        {
            if (( access( str.c_str(), F_OK ) != -1 ))
            {
                //file exists
                printf("Found PS1 ROM %s\n", str.c_str());
                gGame=str;
            }
        }
    }

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Main loop flag
        bool emuReturn;
        double x0=0;
        double y0=0;
        double x1=0;
        double y1=0;

        //Event handler
        SDL_Event event;
        
        gQuit=false;

        //main loop
        while( !gQuit )
        {
            //Handle events on queue
            while( SDL_PollEvent( &event ) != 0 )
            {
                // main event loop
                switch (event.type)
                {
                    case SDL_KEYDOWN: 
                        switch( event.key.keysym.sym )
                        {
                            case SDLK_l:
                                k = SDL_NumJoysticks();
                                if (k)
                                {
                                    printf("************* List all (number: %i) ************* \n",k);
                                    for (l=0; l<k; l++)
                                    {
                                        printJoyInfo(l);
                                    }
                                    m=0; //count designated controllers
                                    for (l=0; l < MAX_GAMEPADS;l++)
                                    {
                                        if ( gDesignatedControllers[l].instance != -1 )
                                        {
                                            printf("found designated controller %i with SDL instance %i %s\n",l, gDesignatedControllers[l].instance,gDesignatedControllers[l].name.c_str());
                                            m++;
                                        }
                                    }
                                    printf("%i designated controllers found\n",m);
                                } else
                                {
                                    printf("************* no controllers found ************* \n");
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
                                
                                for (l=0; l < MAX_GAMEPADS;l++)
                                {
                                    if ( gDesignatedControllers[l].instance != -1 )
                                    {
                                        char *mapping;
                                        mapping = SDL_GameControllerMapping(gDesignatedControllers[l].gameCtrl);
                                        printf("\n\n%s\n\n",mapping);
                                        SDL_free(mapping);
                                    }
                                }
                                break;
                            case SDLK_1:
                                    gQuit=true;
                                break;
                            case SDLK_UP:
                            case SDLK_DOWN:
                            case SDLK_LEFT:
                            case SDLK_RIGHT:
                                    statemachine(event.key.keysym.sym);
                                break;
                            case SDLK_RETURN:
                                    statemachine(SDL_CONTROLLER_BUTTON_A);
                                break;
                            case SDLK_ESCAPE:
                                if (!gIgnore)
                                {
                                    //gQuit=true;
                                }
                                else
                                {
                                    gIgnore = false;
                                }
                                break;
                            default:
                                printf("key not recognized \n");
                                break;
                        }
                        break;
                    case SDL_JOYDEVICEADDED: 
                        printf("\n*** Found new controller ");
                        openJoy(event.jdevice.which);
                        break;
                    case SDL_JOYDEVICEREMOVED: 
                        printf("*** controller removed\n");
                        closeJoy(event.jdevice.which);
                        break;
                    case SDL_JOYAXISMOTION: 
                        if (abs(event.jaxis.value) > ANALOG_DEAD_ZONE)
                        {
                            if (event.jdevice.which == gDesignatedControllers[0].instance)
                            {
                                joyMotion(event,0,&x0,&y0);
                            }
                            else if (event.jdevice.which == gDesignatedControllers[1].instance)
                            {
                                joyMotion(event,1,&x1,&y1);
                            }
                            
                            if((event.jaxis.axis == 0) || (event.jaxis.axis == 1))
                            {
                                angle0L=atan2(y0,x0)*180.0 / PI;
                                angle1L=atan2(y1,x1)*180.0 / PI;
                                amplitude0L=sqrt(x0*x0+y0*y0);
                                amplitude1L=sqrt(x1*x1+y1*y1);
                            } 
                            else if((event.jaxis.axis == 3) || (event.jaxis.axis == 4))
                            {
                                angle0R=atan2(y0,x0)*180.0 / PI;
                                angle1R=atan2(y1,x1)*180.0 / PI;
                                amplitude0R=sqrt(x0*x0+y0*y0);
                                amplitude1R=sqrt(x1*x1+y1*y1);
                            }
                        }
                        break;
                    case SDL_QUIT: 
                        gQuit = true;
                        break;
                    case SDL_CONTROLLERBUTTONDOWN: 
                    
                        if (event.jdevice.which == gDesignatedControllers[0].instance)
                        {
                            joyButton(event,0,&ignoreESC);
                        }
                        else if (event.jdevice.which == gDesignatedControllers[1].instance)
                        {
                            joyButton(event,1,&ignoreESC);
                        }
                        break;
                    default: 
                        ignoreESC = false;
                        (void) 0;
                }
            }
            
            //Clear screen
            SDL_RenderClear( gRenderer );
            //background
            SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],NULL,NULL);
            renderIcons(gGame);
            
            int ctrlTex, height;
            //designated controller 0: Load image and render to screen
            if (gDesignatedControllers[0].instance != -1)
            {
                ctrlTex = TEX_GENERIC_CTRL;
                string name = gDesignatedControllers[0].name;
                string str;
                
                height=int(amplitude0L/200);
                if (height>250) height=250;
                destination = { 50, 100, 50, height };
                SDL_SetRenderDrawColor(gRenderer, 120, 162, 219, 128);
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 0 arrow: Set rendering space and render to screen
                destination = { 100, 100, 200, 200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ARROW], NULL, &destination, angle0L, NULL, SDL_FLIP_NONE );
                
                height=int(amplitude0R/200);
                if (height>250) height=250;
                destination = { 300, 100, 50, height };
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 0 arrow: Set rendering space and render to screen
                destination = { 350, 100, 200, 200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ARROW], NULL, &destination, angle0R, NULL, SDL_FLIP_NONE );
                
                //icon for configuration run
                destination = { 600, 160, 80, 80 };
                
                if (gState == STATE_CONF0)
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
                } 
                else
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER_GREY], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
                }

                //check if PS3
                str = "Sony PLAYSTATION(R)3";
                int str_pos = name.find(str);
                if (str_pos>=0)
                {
                    ctrlTex = TEX_PS3;
                } 
                
                //check if XBOX 360
                str = "360";
                str_pos = name.find(str);
                if (str_pos>=0)
                {
                    ctrlTex = TEX_XBOX360;
                }
                destination = { 700, 100, 250, 250 };
                SDL_RenderCopyEx( gRenderer, gTextures[ctrlTex], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
            }
            
            //designated controller 1: load image and render to screen
            if (gDesignatedControllers[1].instance != -1)
            {
                ctrlTex = TEX_GENERIC_CTRL;
                string name = gDesignatedControllers[1].name;
                string str;
                
                height=int(amplitude1L/200);
                if (height>250) height=250;
                destination = { 50, 400, 50, height };
                SDL_SetRenderDrawColor(gRenderer, 120, 162, 219, 128);
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 1 arrow: Set rendering space and render to screen
                destination = { 100, 400, 200, 200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ARROW], NULL, &destination, angle1L, NULL, SDL_FLIP_NONE );
                
                height=int(amplitude1R/200);
                if (height>250) height=250;
                destination = { 300, 400, 50, height };
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 1 arrow: Set rendering space and render to screen
                destination = { 350, 400, 200, 200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ARROW], NULL, &destination, angle1R, NULL, SDL_FLIP_NONE );
                
                //icon for configuration run
                destination = { 600, 460, 80, 80 };
                
                if (gState == STATE_CONF1)
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
                } 
                else
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER_GREY], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
                }

                //check if PS3
                str = "Sony PLAYSTATION(R)3";
                int str_pos = name.find(str);
                if (str_pos>=0)
                {
                    ctrlTex = TEX_PS3;
                } 
                
                //check if XBOX 360
                str = "360";
                str_pos = name.find(str);
                if (str_pos>=0)
                {
                    ctrlTex = TEX_XBOX360;
                }
                destination = { 700, 400, 250, 250 };
                SDL_RenderCopyEx( gRenderer, gTextures[ctrlTex], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
            }
            
            SDL_RenderPresent( gRenderer );
        }
        
    }

    //Free resources, shut down SDL
    closeAll();

    return 0;
}

//Motion on gamepad x
bool joyMotion(SDL_Event event, int designatedCtrl, double* x, double* y)
{
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


bool joyButton(SDL_Event event, int designatedCtrl, bool* ignoreESC)
{
    bool emuReturn;
    string cmd;
    
    switch( event.cbutton.button )
    {
        case SDL_CONTROLLER_BUTTON_A:
            //printf("a\n");
            break;
        case SDL_CONTROLLER_BUTTON_B:
            //printf("b\n");
            break;
        case SDL_CONTROLLER_BUTTON_X:
            //printf("x\n");
            break;
        case SDL_CONTROLLER_BUTTON_Y:
            //printf("y\n");
            break;
        case SDL_CONTROLLER_BUTTON_START:
            //printf("start\n");
            break;
        case SDL_CONTROLLER_BUTTON_BACK:
            //printf("back\n");
            break;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            //printf("guide\n");
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            //printf("left stick\n");
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            //printf("right stick\n");
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            //printf("left shoulder\n");
            break;
         case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            //printf("right shoulder\n");
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            //printf("dpad up\n");
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            //printf("dpad down\n");
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            //printf("left up\n");
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            //printf("dpad right\n");
            break;
        default:
            //printf("other\n");
            (void) 0;
            break;
    }     
    statemachine(event.cbutton.button);
}



bool createTemplate(string name)
{
    std::ofstream outfile;

    outfile.open(name.c_str(), std::ios_base::app); 
    if(outfile) 
    {
        //printf("writing to file\n");
        outfile << "#directories (marley will search here for games and BIOS files)\n"; 
        outfile << "search_dir=\n"; 
        outfile << "search_dir=\n"; 
        outfile << "\n"; 
        outfile.close();
    }
    else
    {
        printf("Could not create config file %s\n",name.c_str());
    }
}

bool loadConfig(ifstream* configFile)
{
    string entry, line, slash;
    int pos;
    DIR* dir;
    
    while ( getline (configFile[0],line))
    {
        if (line.find("search_dir=") == 0)
        {
            pos=10;
            entry = line.substr(pos+1,line.length()-pos);
            dir = opendir(entry.c_str());
            if (dir) 
            {
                // Directory exists
                closedir(dir);
                slash = entry.substr(entry.length()-1,1);
                if (slash != "/")
                {
                    entry += "/";
                }
                printf("found search directory: %s\n",entry.c_str());
            }
        }
    }
    configFile[0].close();
}

bool checkConf(void)
{
    const char *homedir;
    string filename, entry, line, slash;
    int pos;
    DIR* dir;
    
    if ((homedir = getenv("HOME")) != NULL) 
    {
        filename = homedir;
        
        // add slash to end if necessary
        slash = filename.substr(filename.length()-1,1);
        if (slash != "/")
        {
            filename += "/";
        }
        
        filename = filename + ".marley";
        
        DIR* dir = opendir(filename.c_str());
        if (dir) 
        {
            // Directory exists
            closedir(dir);
            filename = filename + "/" + "marley.conf";
            ifstream configFile (filename.c_str());
            if (!configFile.is_open())
            {
                printf("Could not open config file: %s, creating template\n",filename.c_str());
                createTemplate(filename);
            }
            else 
            {
                loadConfig(&configFile);
            }
        } 
        else if (ENOENT == errno) 
        {
            // Directory does not exist
            printf("creating directory %s ",filename.c_str());
            if (mkdir(filename.c_str(), S_IRWXU ) == 0)
            {
                printf("(ok)\n");
                filename = filename + "/" + "marley.conf";
                createTemplate(filename);
            }
            else
            {
                printf("(failed)\n");
            }
        }
    }    
}


bool restoreSDL(void)
{
    restoreController();
    restoreGUI();
}
