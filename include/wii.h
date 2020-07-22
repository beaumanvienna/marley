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

#include <stdio.h>
#include <string>

#ifdef DOLPHIN_HEADRES_FOR_WIIMOTE

    #include <type_traits>
    #include <string_view>
    #include <algorithm>
    #include <cstdlib>
    #include <mutex>
    #include <queue>
    #include <unordered_set>

    #include "Common/ChunkFile.h"
    #include "Common/CommonTypes.h"
    #include "Common/FileUtil.h"
    #include "Common/IniFile.h"
    #include "Common/Swap.h"
    #include "Common/Thread.h"
    #include "Core/ConfigManager.h"
    #include "Core/Core.h"
    #include "Core/HW/Wiimote.h"
    #include "Core/HW/WiimoteCommon/DataReport.h"
    #include "Core/HW/WiimoteCommon/WiimoteHid.h"
    #include "Core/HW/WiimoteReal/IOAndroid.h"
    #include "Core/HW/WiimoteReal/IOLinux.h"
    #include "Core/HW/WiimoteReal/IOWin.h"
    #include "Core/HW/WiimoteReal/IOdarwin.h"
    #include "Core/HW/WiimoteReal/IOhidapi.h"
    #include "InputCommon/ControllerInterface/Wiimote/Wiimote.h"
    #include "InputCommon/InputConfig.h"

    #include "SFML/Network.hpp" 
    #include "../dolphin/Source/Core/Core/HW/WiimoteReal/WiimoteReal.h"
    #include "../dolphin/Source/Core/Core/HW/WiimoteReal/IOLinux.h"
    #include "UICommon/UICommon.h"

#endif

using namespace std;

#ifndef MARLEY_WII_H
#define MARLEY_WII_H

    bool initWii(void);
    void mainLoopWii(void);
    void shutdownWii(void);

#endif
