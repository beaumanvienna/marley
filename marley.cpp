
//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>

#define MAXCONTROLLER 128

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#if defined(_SDL_TTF_H) || defined(SDL_TTF_H)
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gArrowTexture;

//Game Controller array
SDL_Joystick* gGameController[128];


SDL_JoystickGUID guid;
char guid_str[1024];

SDL_GameController *ctrl;


LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#if defined(_SDL_TTF_H) || defined(SDL_TTF_H)
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool closeJoy(int i)
{
    printf("closeJoy(int %i)\n",i);
    //Close game controller
    SDL_JoystickClose( gGameController[i] );
    gGameController[i] = NULL;
    
}

bool openJoy(int i)
{
    printf("openJoy(int %i)\n",i);
    if (i<MAXCONTROLLER)
    {
        SDL_Joystick *joy;
        joy = SDL_JoystickOpen(i);
        if (joy) {
            
            
            printf("******************************************************************************************\n");
            
            
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
            char guid_str[1024];
            SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
            const char* name = SDL_JoystickName(joy);

            int num_axes = SDL_JoystickNumAxes(joy);
            int num_buttons = SDL_JoystickNumButtons(joy);
            int num_hats = SDL_JoystickNumHats(joy);
            int num_balls = SDL_JoystickNumBalls(joy);

            printf("%s \"%s\" axes:%d buttons:%d hats:%d balls:%d\n", guid_str, name, num_axes, num_buttons, num_hats, num_balls);
            
            
            char *mapping;
            SDL_Log("Index \'%i\' is a compatible controller, named \'%s\'", i, SDL_GameControllerNameForIndex(i));
            ctrl = SDL_GameControllerOpen(i);
            mapping = SDL_GameControllerMapping(ctrl);
            SDL_Log("Controller %i is mapped as \"%s\".", i, mapping);
            SDL_free(mapping);
            
            
            printf("Opened Joystick %i\n",i);
            printf("Name: %s\n", SDL_JoystickNameForIndex(i));
            printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
            printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
            printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
            
            //Load joystick
            gGameController[i] = SDL_JoystickOpen( i );
            if( gGameController[i] == NULL )
            {
                printf( "Warning: Unable to open 1st game controller! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                printf("opened %i\n",i);
            }
            
            
            printf("******************************************************************************************\n");
            printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");
            
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

bool init()
{
	//Initialization flag
	bool success = true;
	int i,j;
	SDL_Joystick *joy;
	
	const char* c;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
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
		
        
        for (int i=0; i< MAXCONTROLLER; i++)
        {
            gGameController[i] = NULL;
        }
		
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load arrow texture
	if( !gArrowTexture.loadFromFile( "pictures/arrow.png" ) )
	{
		printf( "Failed to load arrow texture!\n" );
		success = false;
	}
	
	return success;
}

void close()
{
	//Free loaded images
	gArrowTexture.free();

	   
    int j = SDL_NumJoysticks();
    for (int i=0; i<j; i++)
    {
        closeJoy(i);
    }

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main( int argc, char* args[] )
{
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

			//Event handler
			SDL_Event e;

			//Normalized direction
			int xDir = 0;
			int yDir = 0;

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( (e.type != SDL_QUIT) && ( e.type != SDL_JOYAXISMOTION ))
					{
						//printf( "other event on %i\n", e.jaxis.which);
					}
					
					switch (e.type)
                    {
                        case SDL_JOYDEVICEADDED: 
                            printf("************* New device ************* \n");
                            openJoy(e.jdevice.which);
                            break;
                        case SDL_JOYDEVICEREMOVED: 
                            printf("xxxxxxxxxxxxxxx device removed xxxxxxxxxxxxxxx \n");
                            closeJoy(e.jdevice.which);
                            break;
                        case SDL_JOYAXISMOTION: // code to be executed if n = 2;
                            //Motion on controller x
                            if( e.jaxis.which == 0 )
                            {						
                                //X axis motion
                                if( e.jaxis.axis == 0 )
                                {
                                    printf( "X axis motion\n" );
                                    //Left of dead zone
                                    if( e.jaxis.value < -JOYSTICK_DEAD_ZONE )
                                    {
                                        xDir = -1;
                                    }
                                    //Right of dead zone
                                    else if( e.jaxis.value > JOYSTICK_DEAD_ZONE )
                                    {
                                        xDir =  1;
                                    }
                                    else
                                    {
                                        xDir = 0;
                                    }
                                }
                                //Y axis motion
                                else if( e.jaxis.axis == 1 )
                                {
                                    printf( "Y axis motion\n" );
                                    //Below of dead zone
                                    if( e.jaxis.value < -JOYSTICK_DEAD_ZONE )
                                    {
                                        yDir = -1;
                                    }
                                    //Above of dead zone
                                    else if( e.jaxis.value > JOYSTICK_DEAD_ZONE )
                                    {
                                        yDir =  1;
                                    }
                                    else
                                    {
                                        yDir = 0;
                                    }
                                }
                            }
                            
                            //Motion on controller 1
                            if( e.jaxis.which == 1 )
                            {						
                                //X axis motion
                                if( e.jaxis.axis == 0 )
                                {
                                    printf( "ctrl 1: X axis motion\n" );
                                    //Left of dead zone
                                    if( e.jaxis.value < -JOYSTICK_DEAD_ZONE )
                                    {
                                        xDir = -1;
                                    }
                                    //Right of dead zone
                                    else if( e.jaxis.value > JOYSTICK_DEAD_ZONE )
                                    {
                                        xDir =  1;
                                    }
                                    else
                                    {
                                        xDir = 0;
                                    }
                                }
                                //Y axis motion
                                else if( e.jaxis.axis == 1 )
                                {
                                    printf( "ctrl 2: Y axis motion\n" );
                                    //Below of dead zone
                                    if( e.jaxis.value < -JOYSTICK_DEAD_ZONE )
                                    {
                                        yDir = -1;
                                    }
                                    //Above of dead zone
                                    else if( e.jaxis.value > JOYSTICK_DEAD_ZONE )
                                    {
                                        yDir =  1;
                                    }
                                    else
                                    {
                                        yDir = 0;
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

				//Calculate angle
				double joystickAngle = atan2( (double)yDir, (double)xDir ) * ( 180.0 / M_PI );
				
				//Correct angle
				if( xDir == 0 && yDir == 0 )
				{
					joystickAngle = 0;
				}

				//Render joystick 8 way angle
				gArrowTexture.render( ( SCREEN_WIDTH - gArrowTexture.getWidth() ) / 2, ( SCREEN_HEIGHT - gArrowTexture.getHeight() ) / 2, NULL, joystickAngle );

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
