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

#include "../include/emu.h"
#include "../include/gui.h"
#include "../include/statemachine.h"
#include "../include/controller.h"
#include <string>
#include <iostream>
#include <fstream>

bool gPSX_firmware;
string gPathToFirnwarePSX;
string gPathToGames;
string gBaseDir;

bool checkFirmwarePSX(void)
{
    if (gPathToFirnwarePSX!="")
    {
        int count = 0;
        string jp = gPathToFirnwarePSX + "scph5500.bin";
        string na = gPathToFirnwarePSX + "scph5501.bin";
        string eu = gPathToFirnwarePSX + "scph5502.bin";
    
        ifstream jpF (jp.c_str());
        ifstream naF (na.c_str());
        ifstream euF (eu.c_str());
        
        gPSX_firmware = false;
        
        if (jpF.is_open())
        {
            gPSX_firmware = true;
            count++;
        }
        else
        {
            printf("%s not found\n",jp.c_str());
        }
        
        if (naF.is_open())
        {
            gPSX_firmware = true;
            count++;
        }
        else
        {
            printf("%s not found\n",na.c_str());
        }
        
        if (euF.is_open())
        {
            gPSX_firmware = true;
            count++;
        }
        else
        {
            printf("%s not found\n",eu.c_str());
        }
        
        if (gPSX_firmware)
        {
            if (count<3)
            {
                printf("Not all PSX firmware files found. You might not be able to play games from all regions.\n");
            }
        }
        else
        {
            gPathToFirnwarePSX="";
        }
    }
}

bool initEMU(void)
{
    //check for PSX firmware
    checkFirmwarePSX();
}
