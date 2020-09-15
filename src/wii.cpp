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
#include "../include/controller.h"
#include "../include/statemachine.h"
#include "../include/emu.h"

#define DOLPHIN_HEADRES_FOR_WIIMOTE 1
#include "../include/wii.h"

bool up, up_prev;
bool down, down_prev;
bool lft, lft_prev;
bool rght, rght_prev;
bool a,a_prev;
bool bbutton,b_prev;
bool guide,guide_prev;
bool wiimoteOnline, wiimoteOnline_prev;

using namespace std;

bool initWii(void)
{
    bool ok = true;
    
    string user_directory = gBaseDir;
    user_directory += "dolphin-emu";
    UICommon::SetUserDirectory(user_directory);
    UICommon::CreateDirectories();
    UICommon::Init();

    Wiimote::Initialize(Wiimote::InitializeMode::DO_NOT_WAIT_FOR_WIIMOTES);
    UICommon::SaveWiimoteSources();
    
    up = up_prev = down, down_prev = false;
    lft = lft_prev = rght = rght_prev = false;
    a = a_prev = bbutton = b_prev = guide = guide_prev = false;
    wiimoteOnline = wiimoteOnline_prev = false;

    return ok;
}
using namespace WiimoteReal;

int delay_after_shutdown;
void mainLoopWii(void)
{
    u16 buttons;
    
    if (delay_after_shutdown)
    {
        delay_after_shutdown--;
        return;
    }
    
    Core::HostDispatchJobs();
    if (g_wiimotes[0]) 
    {   
        wiimoteOnline = true;
        buttons = g_wiimotes[0]->getWiiButtons(0);
        
        up      = (buttons & 0x0008);
        down    = (buttons & 0x0004);
        lft     = (buttons & 0x0001);
        rght    = (buttons & 0x0002);
        a       = (buttons & 0x0800);
        bbutton       = (buttons & 0x0400);
        guide   = (buttons & 0x8000);
        
        if ((up != up_prev) && (up))            statemachine(SDL_CONTROLLER_BUTTON_DPAD_UP);
        if ((down != down_prev) && (down))      statemachine(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        if ((lft != lft_prev) && (lft))         statemachine(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        if ((rght != rght_prev) && (rght))      statemachine(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        if ((a != a_prev) && (a))               statemachine(SDL_CONTROLLER_BUTTON_A);
        if ((bbutton != b_prev) && (bbutton))   statemachine(SDL_CONTROLLER_BUTTON_A);
        if ((guide != guide_prev) && (guide))   statemachine(SDL_CONTROLLER_BUTTON_GUIDE);
        
        up_prev     = up;
        down_prev   = down;
        lft_prev    = lft;
        rght_prev   = rght;
        a_prev      = a;
        b_prev      = bbutton;
        guide_prev  = guide;
    }
    else
    {
        up = up_prev = down, down_prev = false;
        lft = lft_prev = rght = rght_prev = false;
        a = a_prev = bbutton = b_prev = guide = guide_prev = false;
        wiimoteOnline = false;
    }
    if (wiimoteOnline != wiimoteOnline_prev)
    {
        if ( (wiimoteOnline) && (g_wiimotes[0]) )
        {
                        
            //new wiimote detected
            printf("Wiimote detected\n");
            openWiimote(0);
    
            SDL_Delay(400);
            //send command to request report for buttons
            buttons = g_wiimotes[0]->getWiiButtons(1);
            
        }
        else
        {
            //connection to wiimote lost
            printf  ("Wiimote removed\n");
            closeWiimote(0);
        }
    }
    wiimoteOnline_prev = wiimoteOnline;
}

void shutdownWii(void)
{
    //Core::Stop();
    //Core::Shutdown();
    UICommon::Shutdown();
}

