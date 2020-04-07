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

#include "../include/gui.h"
#include "../include/controller.h"
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

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
        
        printf("We compiled against SDL version %d.%d.%d ...\n",
        compiled.major, compiled.minor, compiled.patch);
        printf("But we are linking against SDL version %d.%d.%d.\n",
        linked.major, linked.minor, linked.patch);
        
        
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
    string game, cmd;
    bool ignore=false;
    double angle = 0;
    
    gFullscreen=false;
    game="";
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
            printf("Use your controller or arrow keys/enter/ESC on your keyboard to navigate.\n\n");
            printf("Use \"l\" to print a list of detected controllers to the command line.\n\n");
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
                game=str;
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
        bool quit = false;
        bool emuReturn;

        //Event handler
        SDL_Event event;
        int xDir0 = 0;
        int yDir0 = 0;

        //main loop
        while( !quit )
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
                            case SDLK_ESCAPE:
                                if(!ignore)
                                {
                                    quit = true;
                                }
                                break;
                            default:
                                printf("key not recognized \n");
                                break;
                        }
                        break;
                    case SDL_JOYDEVICEADDED: 
                        printf("New device ");
                        openJoy(event.jdevice.which);
                        break;
                    case SDL_JOYDEVICEREMOVED: 
                        printf("xxxxxxxxxxxxxxx device removed xxxxxxxxxxxxxxx \n");
                        closeJoy(event.jdevice.which);
                        break;
                    case SDL_JOYAXISMOTION: 
                        //Motion on gamepad x
                        if (abs(event.jaxis.value) > ANALOG_DEAD_ZONE)
                        {
                            //X axis motion
                            if( event.jaxis.axis == 0 )
                            {
                                //Left of dead zone
                                if( event.jaxis.value < -ANALOG_DEAD_ZONE )
                                {
                                    xDir0 = -1;
                                }
                                //Right of dead zone
                                else if( event.jaxis.value > ANALOG_DEAD_ZONE )
                                {
                                    xDir0 =  1;
                                }
                                else
                                {
                                    xDir0 = 0;
                                }
                            }
                            //Y axis motion
                            else if( event.jaxis.axis == 1 )
                            {
                                //printf( "Y axis motion\n" );
                                //Below of dead zone
                                if( event.jaxis.value < -ANALOG_DEAD_ZONE )
                                {
                                    yDir0 = -1;
                                }
                                //Above of dead zone
                                else if( event.jaxis.value > ANALOG_DEAD_ZONE )
                                {
                                    yDir0 =  1;
                                }
                                else
                                {
                                    yDir0 = 0;
                                }
                            }
                        }
                        if ((xDir0 == -1) && (yDir0 == 1))
                        {
                            angle = 180;
                        }
                        if ((xDir0 == 1) && (yDir0 == 1))
                        {
                            angle = 0;
                        }
                        if ((xDir0 == -1) && (yDir0 == -1))
                        {
                            angle = 90;
                        }
                        if ((xDir0 == 1) && (yDir0 == -1))
                        {
                            angle = 270;
                        }
                        break;
                    case SDL_QUIT: 
                        quit = true;
                        break;
                    case SDL_JOYBUTTONDOWN: 
                        if (game != "")
                        {
                            cmd = "mednafen \""+game+"\"";
                            printf("launching %s\n",cmd.c_str());
                            emuReturn = system(cmd.c_str());
                            ignore=true;
                        } else
                        {
                            printf("no valid ROM found\n");
                        }
                        break;
                    default: 
                        ignore = false;
                        (void) 0;
                }
            }
            
            
            
            //Clear screen
            SDL_RenderClear( gRenderer );
            //Update screen
            SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],NULL,NULL);
            
            
            //Set rendering space and render to screen
            gDest = { 300, 100, 500, 500 };

            //Render to screen
            SDL_RenderCopyEx( gRenderer, gTextures[TEX_ARROW], NULL, &gDest, angle, NULL, SDL_FLIP_NONE );
            
            
            //SDL_RenderCopy(gRenderer, gTextures[TEX_ARROW], NULL, NULL);
            SDL_RenderPresent( gRenderer );
        }
        
    }

    //Free resources, shut down SDL
    closeAll();

    return 0;
}
