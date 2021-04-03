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
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include "UI/OnScreenDisplay.h"

#include "Common/Data/Color/RGBAUtil.h"
#include "Common/Render/TextureAtlas.h"
#include "Common/Render/DrawBuffer.h"

#include "Common/UI/Context.h"

#include "Common/TimeUtil.h"

SCREEN_OnScreenMessages osm;

void SCREEN_OnScreenMessagesView::Draw(SCREEN_UIContext &dc) {
    // First, clean out old messages.
    osm.Lock();
    osm.Clean();

    // Get height
    float w, h;
    dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, "Wg", &w, &h);

    float y = 10.0f;
    // Then draw them all. 
    const std::list<SCREEN_OnScreenMessages::Message> &messages = osm.Messages();
    double now = time_now_d();
    for (auto iter = messages.begin(); iter != messages.end(); ++iter) {
        float alpha = (iter->endTime - now) * 4.0f;
        if (alpha > 1.0) alpha = 1.0f;
        if (alpha < 0.0) alpha = 0.0f;
        // Messages that are wider than the screen are left-aligned instead of centered.
        float tw, th;
        dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, iter->text.c_str(), &tw, &th);
        float x = bounds_.centerX();
        int align = ALIGN_TOP | ALIGN_HCENTER;
        if (tw > bounds_.w) {
            align = ALIGN_TOP | ALIGN_LEFT;
            x = 2;
        }
        dc.SetFontStyle(dc.theme->uiFont);
        dc.DrawTextShadow(iter->text.c_str(), x, y, colorAlpha(iter->color, alpha), align);
        y += h;
    }

    osm.Unlock();
}

void SCREEN_OnScreenMessages::Clean() {
restart:
    double now = time_now_d();
    for (auto iter = messages_.begin(); iter != messages_.end(); iter++) {
        if (iter->endTime < now) {
            messages_.erase(iter);
            goto restart;
        }
    }
}

void SCREEN_OnScreenMessages::Show(const std::string &text, float duration_s, uint32_t color, int icon, bool checkUnique, const char *id) {
    double now = time_now_d();
    std::lock_guard<std::mutex> guard(mutex_);
    if (checkUnique) {
        for (auto iter = messages_.begin(); iter != messages_.end(); ++iter) {
            if (iter->text == text || (id && iter->id && !strcmp(iter->id, id))) {
                Message msg = *iter;
                msg.endTime = now + duration_s;
                msg.text = text;
                msg.color = color;
                messages_.erase(iter);
                messages_.insert(messages_.begin(), msg);
                return;
            }
        }
    }
    Message msg;
    msg.text = text;
    msg.color = color;
    msg.endTime = now + duration_s;
    msg.icon = icon;
    msg.id = id;
    messages_.insert(messages_.begin(), msg);
}

void SCREEN_OnScreenMessages::ShowOnOff(const std::string &message, bool b, float duration_s, uint32_t color, int icon) {
    Show(message + (b ? ": on" : ": off"), duration_s, color, icon);
}
