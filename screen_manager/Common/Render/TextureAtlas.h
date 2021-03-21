#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>
#include "Common/Render/Sprite_Sheet.h"

#define ATLAS_MAGIC ('A' + ('T' << 8) + ('L' << 16) | ('A' << 24))

// Metadata file structure v0:
//
// AtlasHeader
// For each image:
//   AtlasImage
// For each font:
//   AtlasFontHeader
//   For each range:
//     AtlasRange
//   For each char:
//     AtlasChar

struct SCREEN_Atlas;
struct AtlasImage {
	float u1, v1, u2, v2;
	int w, h;
	char name[32];
};

struct ImageID {
public:
	ImageID() : id(nullptr) {}
    ~ImageID() {}
	explicit ImageID(const char *_id) : id(_id) {}
    explicit ImageID(const char *_id, int currentFrame) : id(_id), currentFrame_(currentFrame)
    {
        isSpriteSheet = true;
    }

	static inline ImageID invalid() {
		return ImageID{ nullptr };
	}

	bool isValid() const {
		return id != nullptr;
	}

	bool isInvalid() const {
		return id == nullptr;
	}

	bool operator ==(const ImageID &other) {
		return (id == other.id) || !strcmp(id, other.id);
	}

	bool operator !=(const ImageID &other) {
		if (id == other.id) {
			return false;
		}
		return strcmp(id, other.id) != 0;
	}

    bool isSpriteSheet = false;
    int currentFrame_ = 0; // requested frame
    
private:
	const char *id;

	friend struct SCREEN_Atlas;
};

struct FontID {
public:
	explicit FontID(const char *_id) : id(_id) {}

	static inline FontID invalid() {
		return FontID{ nullptr };
	}

	bool isInvalid() const {
		return id == nullptr;
	}

private:
	const char *id;
	friend struct SCREEN_Atlas;
};

struct AtlasChar {
	// texcoords
	float sx, sy, ex, ey;
	// offset from the origin
	float ox, oy;
	// distance to move the origin forward
	float wx;
	// size in pixels
	unsigned short pw, ph;
};

struct AtlasCharRange {
	int start;
	int end;
	int result_index;
};

struct AtlasFontHeader {
	float padding;
	float height;
	float ascend;
	float distslope;
	int numRanges;
	int numChars;
	char name[32];
};

struct SCREEN_AtlasFont {
	~SCREEN_AtlasFont();

	float padding;
	float height;
	float ascend;
	float distslope;
	const AtlasChar *charData;
	const AtlasCharRange *ranges;
	int numRanges;
	int numChars;
	char name[32];

	// Returns 0 on no match.
	const AtlasChar *getChar(int utf32) const ;
};

struct AtlasHeader {
	int magic;
	int version;
	int numFonts;
	int numImages;
};

typedef std::vector<AtlasImage> SpriteSheetAllFrames;
struct SpriteSheet 
{
    std::string id;
    int numberOfFrames;
    SpriteSheetAllFrames frames;
};
typedef std::vector<SpriteSheet> SpriteSheetArray;

struct SCREEN_Atlas {
    SCREEN_Atlas() {}
	~SCREEN_Atlas();
	bool Load(const uint8_t *data, size_t data_size);
	bool IsMetadataLoaded() {
		return images != nullptr;
	}

	SCREEN_AtlasFont *fonts = nullptr;
	int num_fonts = 0;
	AtlasImage *images = nullptr;
	int num_images = 0;

	// These are inefficient linear searches, try not to call every frame.
	const SCREEN_AtlasFont *getFont(FontID id) const;
	const AtlasImage *getImage(ImageID id) const;
    bool registerSpriteSheet(std::string id, int numberOfFrames);

	bool measureImage(ImageID id, float *w, float *h) const;
    
    SpriteSheetArray sprite_sheets;
};
