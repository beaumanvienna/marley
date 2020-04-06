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

//initializes SDL and creates main window
bool init()
{
    //Initialization flag
    bool ok = true;
    int i,j;
    SDL_Joystick *joy;
    
    const char* c;

    printf( "init()\n");
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
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
        
        
        initJoy();
        ok = initGUI();
        
        printf( "init() end\n");
    }

    return ok;
}

//Frees media and shuts down SDL
void closeAll()
{
    //Free loaded textures
    gArrowTexture.free();

       
    closeAllJoy();
    closeGUI();

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

int main( int argc, char* args[] )
{
    int k,l,m,id;
    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {    
            //Main loop flag
            bool quit = false;
            
            bool controller0 = false;
            bool controller1 = false;

            //Event handler
            SDL_Event e;

            int xDir0 = 0;
            int yDir0 = 0;
            
            int xDir1 = 0;
            int yDir1 = 0;

            //main loop
            while( !quit )
            {
                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    // main event loop
                    switch (e.type)
                    {
                        case SDL_KEYDOWN: 
                            switch( e.key.keysym.sym )
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
                                default:
                                    printf("key not recognized \n");
                                    break;
                            }
                            break;
                        case SDL_JOYDEVICEADDED: 
                            printf("New device ");
                            openJoy(e.jdevice.which);
                            break;
                        case SDL_JOYDEVICEREMOVED: 
                            printf("xxxxxxxxxxxxxxx device removed xxxxxxxxxxxxxxx \n");
                            closeJoy(e.jdevice.which);
                            break;
                        case SDL_JOYAXISMOTION: 
                            //Motion on gamepad x
                            if (abs(e.jaxis.value) > ANALOG_DEAD_ZONE)
                            {
                                controller0 = (e.jaxis.which == gDesignatedControllers[0].instance);
                                controller1 = (e.jaxis.which == gDesignatedControllers[1].instance);
                            } else
                            {
                                controller0 = false;
                                controller1 = false;
                            }
                            
                            if (controller0)
                            {
                                //X axis motion
                                if( e.jaxis.axis == 0 )
                                {
                                    //Left of dead zone
                                    if( e.jaxis.value < -ANALOG_DEAD_ZONE )
                                    {
                                        xDir0 = -1;
                                    }
                                    //Right of dead zone
                                    else if( e.jaxis.value > ANALOG_DEAD_ZONE )
                                    {
                                        xDir0 =  1;
                                    }
                                    else
                                    {
                                        xDir0 = 0;
                                    }
                                }
                                //Y axis motion
                                else if( e.jaxis.axis == 1 )
                                {
                                    //printf( "Y axis motion\n" );
                                    //Below of dead zone
                                    if( e.jaxis.value < -ANALOG_DEAD_ZONE )
                                    {
                                        yDir0 = -1;
                                    }
                                    //Above of dead zone
                                    else if( e.jaxis.value > ANALOG_DEAD_ZONE )
                                    {
                                        yDir0 =  1;
                                    }
                                    else
                                    {
                                        yDir0 = 0;
                                    }
                                }
                            }
                            break;
                        case SDL_QUIT: 
                            quit = true;
                            break;
                        default: 
                            (void) 0;
                    }
                }

                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                SDL_RenderClear( gRenderer );

                //Angle first controller
                double joystickAngle0 = atan2( (double)yDir0, (double)xDir0 ) * ( 180.0 / M_PI );
                
                //Correct angle
                if( xDir0 == 0 && yDir0 == 0 )
                {
                    joystickAngle0 = 0;
                }

                //Render joystick 8 way angle
                gArrowTexture.render( ( WINDOW_WIDTH - gArrowTexture.getWidth() ) / 2, ( WINDOW_HEIGHT - gArrowTexture.getHeight() ) / 2, NULL, joystickAngle0 );

                //Update screen
                SDL_RenderPresent( gRenderer );
            }
        }
    }

    //Free resources, shut down SDL
    closeAll();

    return 0;
}
