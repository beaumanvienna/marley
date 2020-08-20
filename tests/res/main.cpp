
#include <stdio.h>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <gtk/gtk.h>
#include "res.h"

using namespace std;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 750

    #define NUM_TEXTURES    			1
    #define TEX_BACKGROUND          	0

bool loadMedia();

//rendering window 
SDL_Window* gWindow = NULL;

//window renderer
SDL_Renderer* gRenderer = NULL;

//textures
SDL_Texture* gTextures[NUM_TEXTURES];

bool createRenderer(void)
{
    bool ok = true;
    SDL_GL_ResetAttributes();
    //Create renderer for main window
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( gRenderer == NULL )
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
        SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],NULL,NULL);
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
    if( gWindow == NULL )
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
    SDL_RenderCopy(gRenderer,gTextures[TEX_BACKGROUND],NULL,NULL);
    
    SDL_RenderPresent( gRenderer );

}


int main(int argc, char* argv[])
{

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
				case SDL_KEYDOWN: 

					switch( event.key.keysym.sym )
					{							
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
