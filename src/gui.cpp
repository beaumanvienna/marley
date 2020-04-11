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
#include "../include/statemachine.h"

//rendering window 
SDL_Window* gWindow = NULL;

//window renderer
SDL_Renderer* gRenderer = NULL;

//textures
SDL_Texture* gTextures[NUM_TEXTURES];

bool gFullscreen;

bool loadMedia()
{
    bool ok = true;
    
    // background
    gTextures[TEX_BACKGROUND] = loadTextureFromFile("pictures/beach.png");
    if (!gTextures[TEX_BACKGROUND])
    {
        ok = false;
    }
    //arrow
    gTextures[TEX_ARROW] = loadTextureFromFile("pictures/arrow.png");
    if (!gTextures[TEX_ARROW])
    {
        ok = false;
    }
    
    //PS3 dualshock
    gTextures[TEX_PS3] = loadTextureFromFile("pictures/PS3-DualShock.png");
    if (!gTextures[TEX_PS3])
    {
        ok = false;
    }

    //XBox 360 controller
    gTextures[TEX_XBOX360] = loadTextureFromFile("pictures/Xbox-360-S-Controller.png");
    if (!gTextures[TEX_XBOX360])
    {
        ok = false;
    }
    
    //Generic controller
    gTextures[TEX_GENERIC_CTRL] = loadTextureFromFile("pictures/generic-controller.png");
    if (!gTextures[TEX_GENERIC_CTRL])
    {
        ok = false;
    }
    
    //rudder to start configuration run
    gTextures[TEX_RUDDER] = loadTextureFromFile("pictures/rudder.png");
    if (!gTextures[TEX_RUDDER])
    {
        ok = false;
    }
    
    //rudder to start configuration run
    gTextures[TEX_RUDDER_GREY] = loadTextureFromFile("pictures/rudder_grey.png");
    if (!gTextures[TEX_RUDDER_GREY])
    {
        ok = false;
    }
    
    //play icon
    gTextures[TEX_ICON_PLAY] = loadTextureFromFile("pictures/Play.png");
    if (!gTextures[TEX_ICON_PLAY])
    {
        ok = false;
    }
    
    //play icon inactive
    gTextures[TEX_ICON_PLAY_IN] = loadTextureFromFile("pictures/Play_inactive.png");
    if (!gTextures[TEX_ICON_PLAY_IN])
    {
        ok = false;
    }
    
    //Setup icon
    gTextures[TEX_ICON_SETUP] = loadTextureFromFile("pictures/Setup.png");
    if (!gTextures[TEX_ICON_SETUP])
    {
        ok = false;
    }
    
    //Setup icon inactive
    gTextures[TEX_ICON_SETUP_IN] = loadTextureFromFile("pictures/Setup_inactive.png");
    if (!gTextures[TEX_ICON_SETUP_IN])
    {
        ok = false;
    }
    
    //Off icon
    gTextures[TEX_ICON_OFF] = loadTextureFromFile("pictures/Off.png");
    if (!gTextures[TEX_ICON_OFF])
    {
        ok = false;
    }
    
    //Off icon inactive
    gTextures[TEX_ICON_OFF_IN] = loadTextureFromFile("pictures/Off_inactive.png");
    if (!gTextures[TEX_ICON_OFF_IN])
    {
        ok = false;
    }
    
    return ok;
}

bool freeTextures(void)
{
    for (int i;i<NUM_TEXTURES;i++)
    {
        SDL_DestroyTexture(gTextures[i]);
    }
    return true;
}

SDL_Texture* loadTextureFromFile(string str)
{
    SDL_Surface* surf = NULL;
    SDL_Texture* texture = NULL;
    
    surf = IMG_Load(str.c_str());
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
    return texture;
}

bool initGUI(void)
{
    bool ok = true;
    Uint32 windowFlags;
    int imgFlags;
    
    SDL_ShowCursor(SDL_DISABLE);
    
    windowFlags = SDL_WINDOW_SHOWN;
    if (gFullscreen)
    {
        windowFlags = windowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    //Create main window
    string str = "marley ";
    str += PACKAGE_VERSION;
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
    else
    {
        //Create renderer for main window
        gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
        if( gRenderer == NULL )
        {
            printf( "Error creating renderer. SDL error: %s\n", SDL_GetError() );
            ok = false;
        }
        else
        {
            
            SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
            
            //Initialize libsdl-image for png files
            imgFlags = IMG_INIT_PNG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) )
            {
                printf( "Error initialzing image support. SDL_image error: %s\n", IMG_GetError() );
                ok = false;
            }
            
            //Load media
            if( !loadMedia() )
            {
                printf( "Failed to load media!\n" );
                ok = false;
            }
            
            SDL_RenderClear(gRenderer);
            
            //draw backround to main window
            SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],NULL,NULL);
            SDL_RenderPresent(gRenderer);
        }
    }
    return ok;
}


bool closeGUI(void)
{
    //Destroy main window    
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;
}
    
bool renderIcons(string name)
{
    //render destination 
    SDL_Rect destination;
    SDL_Surface* surfaceMessage = NULL; 
    SDL_Texture* message = NULL;     
    
    
    destination = { 50, 10, 132, 45 };
    if (gState == STATE_PLAY)
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_PLAY], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    } 
    else
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_PLAY_IN], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    }
    
    destination = { 200, 10, 132, 45 };
    if (gState == STATE_SETUP)
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_SETUP], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    } 
    else
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_SETUP_IN], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    }
    
    destination = { 1100, 10, 132, 45 };
    if (gState == STATE_OFF)
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_OFF], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    } 
    else
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_OFF_IN], NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    }
    
    SDL_Color inactive = {125, 46, 115};  
    SDL_Color active = {222, 81, 223};  
    
    destination = { 50, 660, 1000, 36 };
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
    SDL_RenderFillRect(gRenderer, &destination);
    
    if (gState == STATE_LAUNCH)
    {
        surfaceMessage = TTF_RenderText_Solid(gFont, name.c_str(), active); 
    } 
    else
    {
        surfaceMessage = TTF_RenderText_Solid(gFont, name.c_str(), inactive); 
    }
    message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); 
    SDL_RenderCopyEx( gRenderer, message, NULL, &destination, 0, NULL, SDL_FLIP_NONE );
    
}
