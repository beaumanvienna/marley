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

#include "../include/marley.h"
#ifdef DOLPHIN
#include "../include/gui.h"
#include "../include/controller.h"
#include "../include/statemachine.h"
#include "../include/emu.h"
#include "../include/global.h"

#define DOLPHIN_HEADRES_FOR_WIIMOTE 1
#include "../include/wii.h"
void SCREEN_wiimoteInput(int button);
u16 buttons, buttons_prev;
bool up, up_prev;
bool down, down_prev;
bool lft, lft_prev;
bool rght, rght_prev;
bool a,a_prev;
bool bbutton,b_prev;
bool guide,guide_prev;
bool wiimoteOnline, wiimoteOnline_prev;

u16 buttons2, buttons_prev2;
bool up2, up_prev2;
bool down2, down_prev2;
bool lft2, lft_prev2;
bool rght2, rght_prev2;
bool a2,a_prev2;
bool bbutton2,b_prev2;
bool guide2,guide_prev2;
bool wiimoteOnline2, wiimoteOnline_prev2;

bool marley_wiimote;
using namespace std;

bool initWii(void)
{
    bool ok = true;
    marley_wiimote = true;
    
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
    if (delay_after_shutdown)
    {
        delay_after_shutdown--;
        return;
    }
    
    Core::HostDispatchJobs();
    if (g_wiimotes[0]) 
    {   
        wiimoteOnline = true;
        buttons = g_wiimotes[0]->buttons_hex;
        
        up      = (buttons & 0x0008);
        down    = (buttons & 0x0004);
        lft     = (buttons & 0x0001);
        rght    = (buttons & 0x0002);
        a       = (buttons & 0x0800);
        bbutton = (buttons & 0x0400);
        guide   = (buttons & 0x8000);
        
        if ((up != up_prev) && (up))                 SCREEN_wiimoteInput(BUTTON_DPAD_UP);
        if ((down != down_prev) && (down))           SCREEN_wiimoteInput(BUTTON_DPAD_DOWN);
        if ((lft != lft_prev) && (lft))              SCREEN_wiimoteInput(BUTTON_DPAD_LEFT);
        if ((rght != rght_prev) && (rght))           SCREEN_wiimoteInput(BUTTON_DPAD_RIGHT);
        if ((a != a_prev) && (a))                    SCREEN_wiimoteInput(BUTTON_A);
        if ((bbutton != b_prev) && (bbutton))        SCREEN_wiimoteInput(BUTTON_B);
        if ((guide != guide_prev) && (guide))        SCREEN_wiimoteInput(BUTTON_GUIDE);
        if ((!buttons) || (buttons == buttons_prev)) SCREEN_wiimoteInput(BUTTON_NO_BUTTON);
        
        up_prev      = up;
        down_prev    = down;
        lft_prev     = lft;
        rght_prev    = rght;
        a_prev       = a;
        b_prev       = bbutton;
        guide_prev   = guide;
        buttons_prev = buttons;
    }
    else
    {
        buttons = 0;
        up = up_prev = down = down_prev = false;
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

        }
        else
        {
            //connection to wiimote lost
            printf  ("Wiimote removed\n");
            closeWiimote(0);
        }
    }
    wiimoteOnline_prev = wiimoteOnline;
    
    
    if (g_wiimotes[1]) 
    {   
        wiimoteOnline2 = true;
        buttons2 = g_wiimotes[1]->buttons_hex;
        
        up2      = (buttons2 & 0x0008);
        down2    = (buttons2 & 0x0004);
        lft2     = (buttons2 & 0x0001);
        rght2    = (buttons2 & 0x0002);
        a2       = (buttons2 & 0x0800);
        bbutton2 = (buttons2 & 0x0400);
        guide2   = (buttons2 & 0x8000);
        
        if ((up2 != up_prev2) && (up2))                 SCREEN_wiimoteInput(BUTTON_DPAD_UP);
        if ((down2 != down_prev2) && (down2))           SCREEN_wiimoteInput(BUTTON_DPAD_DOWN);
        if ((lft2 != lft_prev2) && (lft2))              SCREEN_wiimoteInput(BUTTON_DPAD_LEFT);
        if ((rght2 != rght_prev2) && (rght2))           SCREEN_wiimoteInput(BUTTON_DPAD_RIGHT);
        if ((a2 != a_prev2) && (a2))                    SCREEN_wiimoteInput(BUTTON_A);
        if ((bbutton2 != b_prev2) && (bbutton2))        SCREEN_wiimoteInput(BUTTON_B);
        if ((guide2 != guide_prev2) && (guide2))        SCREEN_wiimoteInput(BUTTON_GUIDE);
        if ((!buttons2) || (buttons2 == buttons_prev2)) SCREEN_wiimoteInput(BUTTON_NO_BUTTON);
        
        up_prev2      = up2;
        down_prev2    = down2;
        lft_prev2     = lft2;
        rght_prev2    = rght2;
        a_prev2       = a2;
        b_prev2       = bbutton2;
        guide_prev2   = guide2;
        buttons_prev2 = buttons2;
    }
    else
    {
        buttons2 = 0;
        up2 = up_prev2 = down2 = down_prev2 = false;
        lft2 = lft_prev2 = rght2 = rght_prev2 = false;
        a2 = a_prev2 = bbutton2 = b_prev2 = guide2 = guide_prev2 = false;
        wiimoteOnline2 = false;
    }
    if (wiimoteOnline2 != wiimoteOnline_prev2)
    {
        if ( (wiimoteOnline2) && (g_wiimotes[1]) )
        {

            //new wiimote detected
            printf("Wiimote detected\n");
            openWiimote(1);

        }
        else
        {
            //connection to wiimote lost
            printf  ("Wiimote removed\n");
            closeWiimote(1);
        }
    }
    wiimoteOnline_prev2 = wiimoteOnline2;
}

void shutdownWii(void)
{
    UICommon::Shutdown();
}

#endif
