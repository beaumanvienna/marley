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

#include <iostream>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>

using namespace std;

#ifndef GUI_H
#define GUI_H

    const int WINDOW_WIDTH = 1024;
    const int WINDOW_HEIGHT = 768;

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
    
    
    //rendering window 
    extern SDL_Window* gWindow;

    //window renderer
    extern SDL_Renderer* gRenderer;

    //Scene textures
    extern LTexture gArrowTexture;
    
    
    
    
#endif
