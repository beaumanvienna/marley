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

#include <algorithm>

#include "Common/GPU/thin3d.h"
#include "Common/UI/View.h"
#include "Common/UI/Context.h"
#include "Common/Render/DrawBuffer.h"

#include "Common/Data/Color/RGBAUtil.h"
#include "Common/Data/Format/ZIMLoad.h"
#include "Common/Data/Format/PNGLoad.h"
#include "Common/Math/math_util.h"
#include "Common/Math/curves.h"
#include "Common/File/VFS/VFS.h"
#include "Common/Log.h"
#include "Common/TimeUtil.h"
#include "UI/TextureUtil.h"

static SCREEN_Draw::SCREEN_DataFormat ZimToT3DFormat(int zim) {
    switch (zim) {
    case ZIM_RGBA8888: return SCREEN_Draw::SCREEN_DataFormat::R8G8B8A8_UNORM;
    default: return SCREEN_Draw::SCREEN_DataFormat::R8G8B8A8_UNORM;
    }
}

static ImageFileType DetectImageFileType(const uint8_t *data, size_t size) {
    if (size < 4) {
        return TYPE_UNKNOWN;
    }
    if (!memcmp(data, "ZIMG", 4)) {
        return ZIM;
    }
    else if (!memcmp(data, "\x89\x50\x4E\x47", 4)) {
        return PNG;
    }
    else if (!memcmp(data, "\xff\xd8\xff\xe0", 4) || !memcmp(data, "\xff\xd8\xff\xe1", 4)) {
        return JPEG;
    }
    else {
        return TYPE_UNKNOWN;
    }
}

static bool LoadTextureLevels(const uint8_t *data, size_t size, ImageFileType type, int width[16], int height[16], int *num_levels, SCREEN_Draw::SCREEN_DataFormat *fmt, uint8_t *image[16], int *zim_flags) {
    if (type == DETECT) {
        type = DetectImageFileType(data, size);
    }
    if (type == TYPE_UNKNOWN) {
        printf("File (size: %d) has unknown format", (int)size);
        return false;
    }

    *num_levels = 0;
    *zim_flags = 0;

    switch (type) {
    case ZIM:
    {
        *num_levels = LoadZIMPtr((const uint8_t *)data, size, width, height, zim_flags, image);
        *fmt = ZimToT3DFormat(*zim_flags & ZIM_FORMAT_MASK);
    }
    break;

    case PNG:
        if (1 == pngLoadPtr((const unsigned char *)data, size, &width[0], &height[0], &image[0])) {
            *num_levels = 1;
            *fmt = SCREEN_Draw::SCREEN_DataFormat::R8G8B8A8_UNORM;
            if (!image[0]) {
                printf("if (!image[0]) {");
                return false;
            }
        } else {
            printf("PNG load failed");
            return false;
        }
        break;

    case JPEG:
    //fall through
    default:
        printf("Unsupported image format %d", (int)type);
        return false;
    }

    return *num_levels > 0;
}

bool SCREEN_ManagedTexture::LoadFromFileData(const uint8_t *data, size_t dataSize, ImageFileType type, bool generateMips, const char *name) {
    generateMips_ = generateMips;
    using namespace SCREEN_Draw;

    int width[16]{}, height[16]{};
    uint8_t *image[16]{};

    int num_levels = 0;
    int zim_flags = 0;
    SCREEN_DataFormat fmt;
    if (!LoadTextureLevels(data, dataSize, type, width, height, &num_levels, &fmt, image, &zim_flags)) {
        return false;
    }

    _assert_(image[0] != nullptr);

    if (num_levels < 0 || num_levels >= 16) {
        printf("Invalid num_levels: %d. Falling back to one. Image: %dx%d", num_levels, width[0], height[0]);
        num_levels = 1;
    }

    // Free the old texture, if any.
    if (texture_) {
        delete texture_;
        texture_ = nullptr;
    }

    int potentialLevels = std::min(log2i(width[0]), log2i(height[0]));

    TextureDesc desc{};
    desc.type = SCREEN_TextureType::LINEAR2D;
    desc.format = fmt;
    desc.width = width[0];
    desc.height = height[0];
    desc.depth = 1;
    desc.mipLevels = generateMips ? potentialLevels : num_levels;
    desc.generateMips = generateMips && potentialLevels > num_levels;
    desc.tag = name;
    for (int i = 0; i < num_levels; i++) {
        desc.initData.push_back(image[i]);
    }
    texture_ = draw_->CreateTexture(desc);
    for (int i = 0; i < num_levels; i++) {
        if (image[i])
            free(image[i]);
    }
    return texture_;
}

bool SCREEN_ManagedTexture::LoadFromFile(const std::string &filename, ImageFileType type, bool generateMips) {
    generateMips_ = generateMips;
    size_t fileSize;
    uint8_t *buffer = SCREEN_VFSReadFile(filename.c_str(), &fileSize);
    if (!buffer) {
        filename_ = "";
        printf("Failed to read file '%s'", filename.c_str());
        return false;
    }
    bool retval = LoadFromFileData(buffer, fileSize, type, generateMips, filename.c_str());
    if (retval) {
        filename_ = filename;
    } else {
        filename_ = "";
        printf("Failed to load texture '%s'", filename.c_str());
    }
    delete[] buffer;
    return retval;
}

std::unique_ptr<SCREEN_ManagedTexture> CreateTextureFromFile(SCREEN_Draw::SCREEN_DrawContext *draw, const char *filename, ImageFileType type, bool generateMips) {
    if (!draw)
        return std::unique_ptr<SCREEN_ManagedTexture>();
    // TODO: Load the texture on a background thread.
    SCREEN_ManagedTexture *mtex = new SCREEN_ManagedTexture(draw);
    if (!mtex->LoadFromFile(filename, type, generateMips)) {
        delete mtex;
        return std::unique_ptr<SCREEN_ManagedTexture>();
    }
    return std::unique_ptr<SCREEN_ManagedTexture>(mtex);
}

void SCREEN_ManagedTexture::DeviceLost() {
    printf("SCREEN_ManagedTexture::DeviceLost(%s)", filename_.c_str());
    if (texture_)
        texture_->Release();
    texture_ = nullptr;
}

void SCREEN_ManagedTexture::DeviceRestored(SCREEN_Draw::SCREEN_DrawContext *draw) {
    printf("SCREEN_ManagedTexture::DeviceRestored(%s)", filename_.c_str());
    _assert_(!texture_);
    draw_ = draw;
    // Vulkan: Can't load textures before the first frame has started.
    // Should probably try to lift that restriction again someday..
    loadPending_ = true;
}

SCREEN_Draw::SCREEN_Texture *SCREEN_ManagedTexture::GetTexture() {
    if (loadPending_) {
        if (!LoadFromFile(filename_, ImageFileType::DETECT, generateMips_)) {
            printf("SCREEN_ManagedTexture failed: '%s'", filename_.c_str());
        }
        loadPending_ = false;
    }
    return texture_;
}

// TODO: Remove the code duplication between this and LoadFromFileData
std::unique_ptr<SCREEN_ManagedTexture> CreateTextureFromFileData(SCREEN_Draw::SCREEN_DrawContext *draw, const uint8_t *data, int size, ImageFileType type, bool generateMips, const char *name) {
    if (!draw)
        return std::unique_ptr<SCREEN_ManagedTexture>();
    SCREEN_ManagedTexture *mtex = new SCREEN_ManagedTexture(draw);
    if (mtex->LoadFromFileData(data, size, type, generateMips, name)) {
        return std::unique_ptr<SCREEN_ManagedTexture>(mtex);
    } else {
        // Best to return a null pointer if we fail!
        delete mtex;
        return std::unique_ptr<SCREEN_ManagedTexture>();
    }
}

void SCREEN_GameIconView::GetContentDimensions(const SCREEN_UIContext &dc, float &w, float &h) const {
    w = textureWidth_;
    h = textureHeight_;
}

void SCREEN_GameIconView::Draw(SCREEN_UIContext &dc) {
}
