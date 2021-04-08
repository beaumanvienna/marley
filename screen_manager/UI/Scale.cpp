/* Marley Copyright (c) 2020 - 2021 Marley Development Team 
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
#include "UI/Scale.h"

float gScale = 1.0f;

float f500   = 500.0f;
float f476   = 476.0f;
float f410   = 410.0f;
float f273   = 273.0f;
float f243   = 243.0f;
float f266   = 266.0f;
float f256   = 266.0f;
float f204   = 204.0f;
float f200   = 200.0f;
float f160   = 160.0f;
float f150   = 150.0f;
float f140   = 140.0f;
float f128   = 128.0f;
float f102   = 102.0f;
float f100   = 100.0f;
float f85    = 85.0f;
float f80    = 80.0f;
float f64    = 64.0f;
float f54    = 54.0f;
float f50    = 50.0f;
float f48    = 48.0f;
float f44    = 44.0f;
float f40    = 40.0f;
float f32    = 32.0f;
float f30    = 30.0f;
float f24    = 24.0f;
float f20    = 20.0f;
float f16    = 16.0f;
float f12    = 12.0f;
float f10    = 10.0f;
float f5     = 5.0f;
float f4     = 4.0f;
float f2     = 2.0f;
float f1     = 1.0f;

extern int WINDOW_WIDTH;

void setGlobalScaling(void)
{
    float window_width = WINDOW_WIDTH; // convert to float
    gScale =  window_width / 1365;
    
    std::cout << "gScale = " << gScale << std::endl;
    
    f500   = 500.0f * gScale;
    f476   = 476.0f * gScale;
    f410   = 410.0f * gScale;
    f273   = 273.0f * gScale;
    f243   = 243.0f * gScale;
    f266   = 266.0f * gScale;
    f256   = 256.0f * gScale;
    f204   = 204.0f * gScale;
    f200   = 200.0f * gScale;
    f160   = 160.0f * gScale;
    f150   = 150.0f * gScale;
    f140   = 140.0f * gScale;
    f128   = 128.0f * gScale;
    f102   = 102.0f * gScale;
    f100   = 100.0f * gScale;
    f85    = 85.0f * gScale;
    f80    = 80.0f * gScale;
    f64    = 64.0f * gScale;
    f54    = 54.0f * gScale;
    f50    = 50.0f * gScale;
    f48    = 48.0f * gScale;
    f44    = 48.0f * gScale;
    f40    = 40.0f * gScale;
    f32    = 32.0f * gScale;
    f30    = 30.0f * gScale;
    f24    = 24.0f * gScale;
    f20    = 20.0f * gScale;
    f16    = 16.0f * gScale;
    f12    = 12.0f * gScale;
    f10    = 10.0f * gScale;
    f5     = 5.0f * gScale;
    f4     = 4.0f * gScale;
    f2     = 2.0f * gScale;
    f1     = 1.0f * gScale;
}
