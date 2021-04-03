// Copyright (c) 2013-2020 PPSSPP project
// Copyright (c) 2020 - 2021 Marley project

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.#pragma once

#include <string>
#include <list>
#include <mutex>

#include "Common/Math/geom2d.h"
#include "Common/UI/View.h"

class SCREEN_DrawBuffer;

class SCREEN_OnScreenMessages {
public:
    void Show(const std::string &message, float duration_s = 1.0f, uint32_t color = 0xFFFFFF, int icon = -1, bool checkUnique = true, const char *id = nullptr);
    void ShowOnOff(const std::string &message, bool b, float duration_s = 1.0f, uint32_t color = 0xFFFFFF, int icon = -1);
    bool IsEmpty() const { return messages_.empty(); }

    void Lock() {
        mutex_.lock();
    }
    void Unlock() {
        mutex_.unlock();
    }

    void Clean();

    struct Message {
        int icon;
        uint32_t color;
        std::string text;
        const char *id;
        double endTime;
        double duration;
    };
    const std::list<Message> &Messages() { return messages_; }

private:
    std::list<Message> messages_;
    std::mutex mutex_;
};

class SCREEN_OnScreenMessagesView : public SCREEN_UI::InertView {
public:
    SCREEN_OnScreenMessagesView(SCREEN_UI::LayoutParams *layoutParams = nullptr) : SCREEN_UI::InertView(layoutParams) {}
    void Draw(SCREEN_UIContext &dc);
};

extern SCREEN_OnScreenMessages osm;
