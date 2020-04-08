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
    
    windowFlags = SDL_WINDOW_SHOWN;
    if (gFullscreen)
    {
        windowFlags = windowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    //Create main window
    gWindow = SDL_CreateWindow( "marley", 
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
