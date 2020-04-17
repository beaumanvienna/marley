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

#include <vector>
using namespace std;

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

    #define STATE_ZERO      0
    #define STATE_CONF0     1
    #define STATE_CONF1     2
    #define STATE_PLAY      3
    #define STATE_SETUP     4
    #define STATE_OFF       5
    #define STATE_LAUNCH    6
    #define STATE_FLR_GAMES 7
    #define STATE_FLR_FW    8

    bool statemachine(int cmd);
    
     // statemachine
    extern int gState;
    
    extern int gCurrentGame;
    extern std::vector<string> gGame;
    
    extern bool gQuit;

#endif
