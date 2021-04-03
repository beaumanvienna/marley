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

#pragma once

#include <memory>

#include "Common/GPU/thin3d.h"
#include "Common/UI/View.h"

enum ImageFileType {
    PNG,
    JPEG,
    ZIM,
    DETECT,
    TYPE_UNKNOWN,
};

class SCREEN_ManagedTexture {
public:
    SCREEN_ManagedTexture(SCREEN_Draw::SCREEN_DrawContext *draw) : draw_(draw) {
    }
    ~SCREEN_ManagedTexture() {
        if (texture_)
            texture_->Release();
    }

    bool LoadFromFile(const std::string &filename, ImageFileType type = ImageFileType::DETECT, bool generateMips = false);
    bool LoadFromFileData(const uint8_t *data, size_t dataSize, ImageFileType type, bool generateMips, const char *name);
    SCREEN_Draw::SCREEN_Texture *GetTexture();  // For immediate use, don't store.
    int Width() const { return texture_->Width(); }
    int Height() const { return texture_->Height(); }

    void DeviceLost();
    void DeviceRestored(SCREEN_Draw::SCREEN_DrawContext *draw);

private:
    SCREEN_Draw::SCREEN_Texture *texture_ = nullptr;
    SCREEN_Draw::SCREEN_DrawContext *draw_;
    std::string filename_;  // Textures that are loaded from files can reload themselves automatically.
    bool generateMips_ = false;
    bool loadPending_ = false;
};

std::unique_ptr<SCREEN_ManagedTexture> CreateTextureFromFile(SCREEN_Draw::SCREEN_DrawContext *draw, const char *filename, ImageFileType fileType, bool generateMips);
std::unique_ptr<SCREEN_ManagedTexture> CreateTextureFromFileData(SCREEN_Draw::SCREEN_DrawContext *draw, const uint8_t *data, int size, ImageFileType fileType, bool generateMips, const char *name);

class SCREEN_GameIconView : public SCREEN_UI::InertView {
public:
    SCREEN_GameIconView(std::string gamePath, float scale, SCREEN_UI::LayoutParams *layoutParams = 0)
        : InertView(layoutParams), gamePath_(gamePath), scale_(scale) {}

    void GetContentDimensions(const SCREEN_UIContext &dc, float &w, float &h) const override;
    void Draw(SCREEN_UIContext &dc) override;

private:
    std::string gamePath_;
    float scale_ = 1.0f;
    int textureWidth_ = 0;
    int textureHeight_ = 0;
};
