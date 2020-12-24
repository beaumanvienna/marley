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

#ifndef GLOBAL_H
#define GLOBAL_H

enum SCREEN_UI_theme {
    THEME_RETRO,
    THEME_PLAIN
};

enum SCREEN_UI_colors {           //0xAABBGGRR
    RETRO_COLOR_FONT_FOCUSED      = 0xFFFFFFFF,
    RETRO_COLOR_FONT_NOT_FOCUSED  = 0xFFDE51E0,
    RETRO_COLOR_BLUE              = 0xFFFF0000,
    RETRO_COLOR_GREEN             = 0xFF00FF00,
    RETRO_COLOR_RED               = 0xFF0000FF,
    RETRO_COLOR_YELLOW            = 0xFF00FFFF
};

enum SCREEN_page {
    SCREEN_GENERIC,
    SCREEN_DOLPHIN,
    SCREEN_GENERAL,
    SCREEN_MAIN
};

#endif
