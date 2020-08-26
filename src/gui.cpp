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
#include "../include/statemachine.h"
#include "../include/controller.h"
#include "../include/emu.h"
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include "../resources/res.h"

//rendering window 
SDL_Window* gWindow = nullptr;

//window renderer
SDL_Renderer* gRenderer = nullptr;

//textures
SDL_Texture* gTextures[NUM_TEXTURES];

bool gFullscreen;
int xOffset,yOffset;

Display* XDisplay;
Window Xwindow;

int WINDOW_WIDTH;
int WINDOW_HEIGHT;

int x_offset_1150;
int x_offset_1068;
int x_offset_900;
int x_offset_720;
int x_offset_630;
int x_offset_600;
int x_offset_560;
int x_offset_530;
int x_offset_350;
int x_offset_345;
int x_offset_300;
int x_offset_250;
int x_offset_200;
int x_offset_150;
int x_offset_132;
int x_offset_100;
int x_offset_80;
int x_offset_50;
int x_offset_20;
int y_offset_675;
int y_offset_620;
int y_offset_450;
int y_offset_390;
int y_offset_370;
int y_offset_250;
int y_offset_215;
int y_offset_200;
int y_offset_140;
int y_offset_130;
int y_offset_80;
int y_offset_65;
int y_offset_45;
int y_offset_36;
int y_offset_10;

bool loadMedia()
{
    bool ok = true;
    
    // background
    gTextures[TEX_BACKGROUND] = loadTextureFromFile("/pictures/../pictures/beach.bmp");
    if (!gTextures[TEX_BACKGROUND])
    {
        ok = false;
    }
    //barrel
    gTextures[TEX_BARREL] = loadTextureFromFile("/pictures/../pictures/barrel.bmp");
    if (!gTextures[TEX_BARREL])
    {
        ok = false;
    }
    
    //PS3 dualshock
    gTextures[TEX_PS3] = loadTextureFromFile("/pictures/../pictures/PS3-DualShock.bmp");
    if (!gTextures[TEX_PS3])
    {
        ok = false;
    }
    
    //PS4 dualshock
    gTextures[TEX_PS4] = loadTextureFromFile("/pictures/../pictures/PS4-DualShock.bmp");
    if (!gTextures[TEX_PS4])
    {
        ok = false;
    }

    //XBox 360 controller
    gTextures[TEX_XBOX360] = loadTextureFromFile("/pictures/../pictures/Xbox-360-S-Controller.bmp");
    if (!gTextures[TEX_XBOX360])
    {
        ok = false;
    }
    
    //Wiimote
    gTextures[TEX_WIIMOTE] = loadTextureFromFile("/pictures/../pictures/Wiimote.bmp");
    if (!gTextures[TEX_WIIMOTE])
    {
        ok = false;
    }
    
    //Generic controller
    gTextures[TEX_GENERIC_CTRL] = loadTextureFromFile("/pictures/../pictures/generic-controller.bmp");
    if (!gTextures[TEX_GENERIC_CTRL])
    {
        ok = false;
    }
    
    //rudder to start configuration run
    gTextures[TEX_RUDDER] = loadTextureFromFile("/pictures/../pictures/rudder.bmp");
    if (!gTextures[TEX_RUDDER])
    {
        ok = false;
    }
    
    //rudder to start configuration run
    gTextures[TEX_RUDDER_GREY] = loadTextureFromFile("/pictures/../pictures/rudder_grey.bmp");
    if (!gTextures[TEX_RUDDER_GREY])
    {
        ok = false;
    }
    
    //play icon
    gTextures[TEX_ICON_PLAY] = loadTextureFromFile("/pictures/../pictures/Play.bmp");
    if (!gTextures[TEX_ICON_PLAY])
    {
        ok = false;
    }
    
    //play icon inactive
    gTextures[TEX_ICON_PLAY_IN] = loadTextureFromFile("/pictures/../pictures/Play_inactive.bmp");
    if (!gTextures[TEX_ICON_PLAY_IN])
    {
        ok = false;
    }
    
    //Setup icon
    gTextures[TEX_ICON_SETUP] = loadTextureFromFile("/pictures/../pictures/Setup.bmp");
    if (!gTextures[TEX_ICON_SETUP])
    {
        ok = false;
    }
    
    //Setup icon inactive
    gTextures[TEX_ICON_SETUP_IN] = loadTextureFromFile("/pictures/../pictures/Setup_inactive.bmp");
    if (!gTextures[TEX_ICON_SETUP_IN])
    {
        ok = false;
    }
    
    //Off icon
    gTextures[TEX_ICON_OFF] = loadTextureFromFile("/pictures/../pictures/Off.bmp");
    if (!gTextures[TEX_ICON_OFF])
    {
        ok = false;
    }
    
    //Off icon inactive
    gTextures[TEX_ICON_OFF_IN] = loadTextureFromFile("/pictures/../pictures/Off_inactive.bmp");
    if (!gTextures[TEX_ICON_OFF_IN])
    {
        ok = false;
    }
    
    //no controller
    gTextures[TEX_ICON_NO_CTRL] = loadTextureFromFile("/pictures/../pictures/noController.bmp");
    if (!gTextures[TEX_ICON_NO_CTRL])
    {
        ok = false;
    }  
    
    //no PSX firmware
    gTextures[TEX_ICON_NO_FW_PSX] = loadTextureFromFile("/pictures/../pictures/firmware_PSX.bmp");
    if (!gTextures[TEX_ICON_NO_FW_PSX])
    {
        ok = false;
    }
    
    //no games
    gTextures[TEX_ICON_NO_GAMES] = loadTextureFromFile("/pictures/../pictures/noGames.bmp");
    if (!gTextures[TEX_ICON_NO_GAMES])
    {
        ok = false;
    }

    //games folder
    gTextures[TEX_ICON_GAMES_FLR] = loadTextureFromFile("/pictures/../pictures/path_to_games.bmp");
    if (!gTextures[TEX_ICON_GAMES_FLR])
    {
        ok = false;
    }
    
    //games folder
    gTextures[TEX_ICON_GAMES_FLR_IN] = loadTextureFromFile("/pictures/../pictures/path_to_games_inactive.bmp");
    if (!gTextures[TEX_ICON_GAMES_FLR_IN])
    {
        ok = false;
    }
    
    //firmware folder
    gTextures[TEX_ICON_FW_FLR] = loadTextureFromFile("/pictures/../pictures/path_to_fw.bmp");
    if (!gTextures[TEX_ICON_FW_FLR])
    {
        ok = false;
    }
    
    //firmware folder
    gTextures[TEX_ICON_FW_FLR_IN] = loadTextureFromFile("/pictures/../pictures/path_to_fw_inactive.bmp");
    if (!gTextures[TEX_ICON_FW_FLR_IN])
    {
        ok = false;
    }
    
    //SNES controller
    gTextures[TEX_SNES] = loadTextureFromFile("/pictures/../pictures/SNES-controller.bmp");
    if (!gTextures[TEX_SNES])
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
    SDL_Surface* surf = nullptr;
    SDL_Texture* texture = nullptr;
    
    size_t file_size = 0;
    
    GBytes *mem_access = g_resource_lookup_data(res_get_resource(), str.c_str(), G_RESOURCE_LOOKUP_FLAGS_NONE, nullptr);
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
		printf("Failed to retrieve resource data for %s\n",str.c_str());
	}
	return texture;
}

bool initGUI(void)
{
    bool ok = true;
    Uint32 windowFlags;
    int imgFlags;
    
    SDL_ShowCursor(SDL_DISABLE);
    
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
    
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    WINDOW_WIDTH = current.w / 1.5; 
    WINDOW_HEIGHT = current.h / 1.5;
	
	x_offset_1150 = WINDOW_WIDTH / 1.087;
	x_offset_1068 = WINDOW_WIDTH / 1.170412;
	x_offset_900 = WINDOW_WIDTH / 1.4;
	x_offset_720 = WINDOW_WIDTH / 1.7;
	x_offset_630 = WINDOW_WIDTH / 2;
	x_offset_600 = WINDOW_WIDTH / 2.1;
	x_offset_560 = WINDOW_WIDTH / 2.2;
	x_offset_530 = WINDOW_WIDTH / 2.4;
	x_offset_350 = WINDOW_WIDTH / 3.6;
	x_offset_345 = WINDOW_WIDTH / 3.62;
	x_offset_300 = WINDOW_WIDTH / 4.2;
	x_offset_250 = WINDOW_WIDTH / 5;
	x_offset_200 = WINDOW_WIDTH / 6.25;
	x_offset_150 = WINDOW_WIDTH / 8.3;
	x_offset_132 = WINDOW_WIDTH / 9.5;
	x_offset_100 = WINDOW_WIDTH / 12.5;
	x_offset_80 = WINDOW_WIDTH / 15.6;
	x_offset_50 = WINDOW_WIDTH / 25;
	x_offset_20 = WINDOW_WIDTH / 62.5;
	
	y_offset_675 = WINDOW_HEIGHT / 1.1;
	y_offset_620 = WINDOW_HEIGHT / 1.2;
	y_offset_450 = WINDOW_HEIGHT / 1.7;
	y_offset_390 = WINDOW_HEIGHT / 1.9;
	y_offset_370 = WINDOW_HEIGHT / 2;
	y_offset_250 = WINDOW_HEIGHT / 3;
	y_offset_215 = WINDOW_HEIGHT / 3.5;
	y_offset_200 = WINDOW_HEIGHT / 3.75;
	y_offset_140 = WINDOW_HEIGHT / 5.4;
	y_offset_130 = WINDOW_HEIGHT / 5.8;
	y_offset_80 = WINDOW_HEIGHT / 9.4;
	y_offset_65 = WINDOW_HEIGHT / 11.5;
	y_offset_45 = WINDOW_HEIGHT / 16.7;
	y_offset_36 = WINDOW_HEIGHT / 20.8;
	y_offset_10 = WINDOW_HEIGHT / 75;
    
    xOffset = 0;
    yOffset = 0;
    windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    if (gFullscreen)
    {
        windowFlags = windowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP;
        
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        xOffset = (DM.w-WINDOW_WIDTH)/2;
        yOffset = (DM.h-WINDOW_HEIGHT)/2;
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
    if( gWindow == nullptr )
    {
        printf( "Error creating main window. SDL Error: %s\n", SDL_GetError() );
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
        if (!createRenderer())
        {
            ok =false;
        }
        SDL_DisableScreenSaver();
    }
    return ok;
}


void closeGUI(void)
{
    //Destroy main window    
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;
    gRenderer = nullptr;
}
    
void renderIcons(void)
{
    //render destination 
    SDL_Rect destination;
    SDL_Surface* surfaceMessage = nullptr; 
    SDL_Texture* message = nullptr;     
    
    destination = { x_offset_1068+xOffset, y_offset_10+yOffset, x_offset_132, y_offset_45 };
    if (gState == STATE_OFF)
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_OFF], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
    } 
    else
    {
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_OFF_IN], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
    }
    
    if (gNumDesignatedControllers)
    {
        if (gGamesFound)
        {
            destination = { x_offset_50+xOffset, y_offset_10+yOffset, x_offset_132, y_offset_45 };
            if (gState == STATE_PLAY)
            {
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_PLAY], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            } 
            else
            {
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_PLAY_IN], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            }
        }
        
        if (gGamesFound)
        {
            destination = { x_offset_200+xOffset, y_offset_10+yOffset, x_offset_132, y_offset_45 };
        }
        else
        {
            destination = { x_offset_50+xOffset, y_offset_10+yOffset, x_offset_132, y_offset_45 };
        }
        if (gState == STATE_SETUP)
        {
            SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_SETUP], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
        } 
        else
        {
            SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_SETUP_IN], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
        }
        if (!gSetupIsRunning)
        {
            if (gGamesFound)
            {
                string name_short;
                
                SDL_Color inactive = {125, 46, 115};  
                SDL_Color active = {222, 81, 223};  
                
                destination = { x_offset_50+xOffset, y_offset_675+yOffset, x_offset_1150, y_offset_36 };
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
                SDL_RenderFillRect(gRenderer, &destination);
                
                if(gGame[gCurrentGame].find("/") != string::npos)
                {
                    name_short = gGame[gCurrentGame].substr(gGame[gCurrentGame].find_last_of("/") + 1);
                }
                else
                {
                    name_short=gGame[gCurrentGame];
                }
                
                if(name_short.find(".") != string::npos)
                {
                    name_short = name_short.substr(0,name_short.find_last_of("."));
                }
                
                if(name_short.length()>58)
                {
                    name_short = name_short.substr(0,58);
                }
                
                if (gState == STATE_LAUNCH)
                {
                    surfaceMessage = TTF_RenderText_Solid(gFont, name_short.c_str(), active); 
                } 
                else
                {
                    surfaceMessage = TTF_RenderText_Solid(gFont, name_short.c_str(), inactive); 
                }
                int strLength = name_short.length();
                destination = { x_offset_50+xOffset, y_offset_675+yOffset, strLength*x_offset_20, y_offset_36 };
                message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); 
                SDL_RenderCopyEx( gRenderer, message, nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            }
            else
            {
                
                destination = { x_offset_50+xOffset, y_offset_675+yOffset, x_offset_345, y_offset_45 };
                //if (gState == STATE_SETUP)
                //{
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_NO_GAMES], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                //}
            }
        }
        else  // setup
        {
            
            if (gControllerConf)
            {
                int yConfOffset = y_offset_250*gControllerConfNum;
                
                int strLength = gConfText.length();
                destination = { x_offset_150+xOffset, y_offset_215+yOffset+yConfOffset, strLength*x_offset_20, y_offset_45 };
                SDL_Color active = {222, 81, 223};  
                surfaceMessage = TTF_RenderText_Solid(gFont, gConfText.c_str(), active); 
                message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); 
                SDL_RenderCopyEx( gRenderer, message, nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                
            }
            
            
            destination = { x_offset_50+xOffset, y_offset_620+yOffset, x_offset_530, y_offset_45 };
            if (gState == STATE_FLR_GAMES)
            {
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_GAMES_FLR], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            } 
            else
            {
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_GAMES_FLR_IN], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            }
            
            destination = { x_offset_50+xOffset, y_offset_675+yOffset, x_offset_530, y_offset_45 };
            if (gState == STATE_FLR_FW)
            {
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_FW_FLR], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                
            } 
            else
            {
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_FW_FLR_IN], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            }
            
            if ( (gPathToGames.length()) || (gTextInputForGamingFolder))
            {
                string text;
                if (gTextInputForGamingFolder)
                {
                    text = gText;
                }
                else
                {
                    text = gPathToGames;
                }
                if (text.length()>30)
                {
                    text = text.substr(text.length()-30,30);
                }
                destination = { x_offset_600+xOffset, y_offset_620+yOffset, x_offset_630, y_offset_45 };
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
                SDL_RenderFillRect(gRenderer, &destination);
                
                int strLength = text.length();
                destination = { x_offset_600+xOffset, y_offset_620+yOffset, strLength*x_offset_20, y_offset_45 };
                SDL_Color active = {222, 81, 223};  
                surfaceMessage = TTF_RenderText_Solid(gFont, text.c_str(), active); 
                message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); 
                SDL_RenderCopyEx( gRenderer, message, nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            }
            
            if ( (gPathToFirmwarePSX.length()) || (gTextInputForFirmwareFolder))
            {
                string text;
                if (gTextInputForFirmwareFolder)
                {
                    text = gText;
                }
                else
                {
                    text = gPathToFirmwarePSX;
                }
                if (text.length()>30)
                {
                    text = text.substr(text.length()-30,30);
                }
                destination = { x_offset_600+xOffset, y_offset_675+yOffset, x_offset_630, y_offset_45 };
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
                SDL_RenderFillRect(gRenderer, &destination);
                
                int strLength = text.length();
                destination = { x_offset_600+xOffset, y_offset_675+yOffset, strLength*x_offset_20, y_offset_45 };
                SDL_Color active = {222, 81, 223};  
                surfaceMessage = TTF_RenderText_Solid(gFont, text.c_str(), active); 
                message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage); 
                SDL_RenderCopyEx( gRenderer, message, nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
            }
            
        }
    }
    else
    {
        destination = { x_offset_50+xOffset, y_offset_10+yOffset, x_offset_560, y_offset_45 };
        
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_NO_CTRL], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
    }
    
    if (!gPSX_firmware)
    {
        destination = { x_offset_50+xOffset, y_offset_65+yOffset, x_offset_560, y_offset_45 };
        
        SDL_RenderCopyEx( gRenderer, gTextures[TEX_ICON_NO_FW_PSX], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
    }
}


void setFullscreen(void)
{
    SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    xOffset = (DM.w-WINDOW_WIDTH)/2;
    yOffset = (DM.h-WINDOW_HEIGHT)/2;
}
void setWindowed(void)
{
    SDL_SetWindowFullscreen(gWindow, 0);
    xOffset = 0;
    yOffset = 0;
}

bool createRenderer(void)
{
    bool ok = true;

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
                                    
bool restoreGUI(void)
{
    bool ok = true;
    string str;
    
    if (SDL_GetWindowFlags(gWindow) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN))
    {
        setFullscreen();
    }
    else
    {
        setWindowed();
    }
    
    str = "marley ";
    str += PACKAGE_VERSION;
    SDL_SetWindowTitle(gWindow, str.c_str());    
    
    freeTextures();
    SDL_DestroyRenderer( gRenderer );
    createRenderer();
    SDL_ShowCursor(SDL_DISABLE);
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
    
    int ctrlTex, height;
    //designated controller 0: Load image and render to screen
    if (gDesignatedControllers[0].numberOfDevices != 0)
    {
        string name = gDesignatedControllers[0].name[0];
        string nameDB = gDesignatedControllers[0].nameDB[0];
        string str;
        
        if (gSetupIsRunning)
        {
            destination = { x_offset_50+xOffset, y_offset_140+yOffset, x_offset_720, y_offset_200 };
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
            SDL_RenderFillRect(gRenderer, &destination);
            
            if (gControllerConfNum != 0)
            {
                height=int(amplitude0L/y_offset_200);
                if (height>y_offset_200) height=y_offset_200;
                destination = { x_offset_50+xOffset, y_offset_140+yOffset, x_offset_50, height };
                SDL_SetRenderDrawColor(gRenderer, 120, 162, 219, 128);
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 0 barrel: Set rendering space and render to screen
                destination = { x_offset_100+xOffset, y_offset_140+yOffset, x_offset_200, y_offset_200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_BARREL], nullptr, &destination, angle0L, nullptr, SDL_FLIP_NONE );
                
                height=int(amplitude0R/y_offset_200);
                if (height>y_offset_200) height=y_offset_200;
                destination = { x_offset_300+xOffset, y_offset_140+yOffset, x_offset_50, height };
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 0 barrel: Set rendering space and render to screen
                destination = { x_offset_350+xOffset, y_offset_140+yOffset, x_offset_200, y_offset_200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_BARREL], nullptr, &destination, angle0R, nullptr, SDL_FLIP_NONE );
                
                //icon for configuration run
                destination = { x_offset_600+xOffset, y_offset_200+yOffset, x_offset_80, y_offset_80 };
                
                if (gState == STATE_CONF0)
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                } 
                else
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER_GREY], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                }
            }
        }
        
        ctrlTex = checkType(name,nameDB);
        
        destination = { x_offset_900+xOffset, y_offset_130+yOffset, x_offset_250, y_offset_250 };
        SDL_RenderCopyEx( gRenderer, gTextures[ctrlTex], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
    }
    
    //designated controller 1: load image and render to screen
    if (gDesignatedControllers[1].numberOfDevices != 0)
    {
        string str;
        string name = gDesignatedControllers[1].name[0];
        string nameDB = gDesignatedControllers[1].nameDB[0];
        ctrlTex = TEX_GENERIC_CTRL;
        
        if (gSetupIsRunning)
        {
            
            destination = { x_offset_50+xOffset, y_offset_390+yOffset, x_offset_720, y_offset_200 };
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
            SDL_RenderFillRect(gRenderer, &destination);
            
            if (gControllerConfNum != 1)
            {
            
                height=int(amplitude1L/y_offset_200);
                if (height>y_offset_200) height=y_offset_200;
                destination = { x_offset_50+xOffset, y_offset_390+yOffset, x_offset_50, height };
                SDL_SetRenderDrawColor(gRenderer, 120, 162, 219, 128);
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 1 barrel: Set rendering space and render to screen
                destination = { x_offset_100+xOffset, y_offset_390+yOffset, x_offset_200, y_offset_200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_BARREL], nullptr, &destination, angle1L, nullptr, SDL_FLIP_NONE );
                
                height=int(amplitude1R/y_offset_200);
                if (height>y_offset_200) height=y_offset_200;
                destination = { x_offset_300+xOffset, y_offset_390+yOffset, x_offset_50, height };
                SDL_RenderFillRect(gRenderer, &destination);
                
                //controller 1 barrel: Set rendering space and render to screen
                destination = { x_offset_350+xOffset, y_offset_390+yOffset, x_offset_200, y_offset_200 };
                SDL_RenderCopyEx( gRenderer, gTextures[TEX_BARREL], nullptr, &destination, angle1R, nullptr, SDL_FLIP_NONE );
                            
                //icon for configuration run
                destination = { x_offset_600+xOffset, y_offset_450+yOffset, x_offset_80, y_offset_80 };
                
                if (gState == STATE_CONF1)
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                } 
                else
                {
                    SDL_RenderCopyEx( gRenderer, gTextures[TEX_RUDDER_GREY], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
                }
            }
        }
        
        ctrlTex = checkType(name,nameDB);
        
        destination = { x_offset_900+xOffset, y_offset_370+yOffset, x_offset_250, y_offset_250 };
        SDL_RenderCopyEx( gRenderer, gTextures[ctrlTex], nullptr, &destination, 0, nullptr, SDL_FLIP_NONE );
    }
    renderIcons();
    SDL_RenderPresent( gRenderer );

}
