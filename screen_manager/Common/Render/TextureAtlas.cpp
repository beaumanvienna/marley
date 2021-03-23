#include <cstring>
#include <cstdint>
#include <cstdio>
#include <string>

#include "Common/Render/TextureAtlas.h"

class SCREEN_ByteReader {
public:
	SCREEN_ByteReader(const uint8_t *data, size_t size) : data_(data), offset_(0), size_(size) {}

	template<class T>
	T Read() {
		T x;
		memcpy(&x, data_ + offset_, sizeof(T));
		offset_ += sizeof(T);
		return x;
	}

	template<class T>
	void ReadInto(T *t) {
		memcpy(t, data_ + offset_, sizeof(T));
		offset_ += sizeof(T);
	}

	template<class T>
	T *ReadMultipleAlloc(size_t count) {
		T *t = new T[count];
		memcpy(t, data_ + offset_, sizeof(T) * count);
		offset_ += sizeof(T) * count;
		return t;
	}

private:
	const uint8_t *data_;
	size_t offset_;
	size_t size_;
};

bool SCREEN_Atlas::registerSpriteSheet(std::string id, int numberOfFrames)
{
    // check if sprite sheet already exists 
    // search in sprite sheet array
    
    SpriteSheetArray::iterator it;
    
    for ( it = sprite_sheets.begin(); it != sprite_sheets.end(); it++)
    {
        SpriteSheet element = *it;
        
        if (element.id == id) 
        {
            // sprite sheet found in map
            return true;
        }
    }
    
    // search the atlas image
    for (int i = 0; i < num_images; i++) {
        std::string name = images[i].name;
        if (id == name)
        {
            // id tag found in atlas image
            AtlasImage * ptr = &images[i];
            
            SpriteSheet newSpriteSheet;
            
            newSpriteSheet.id = id;
            newSpriteSheet.numberOfFrames = numberOfFrames;
            
            float width = ptr->u2 - ptr->u1;
            float width_per_frame = width / numberOfFrames;
            float framestart;
            
            for (int frame_n = 0; frame_n < numberOfFrames; frame_n++)
            {
                framestart = ptr->u1 + width_per_frame * frame_n;
            
                AtlasImage frame;
                frame.u1 = framestart; 
                frame.v1 = ptr->v1;
                frame.u2 = framestart + width_per_frame;
                frame.v2 = ptr->v2;
                frame.w = images[i].w/numberOfFrames;
                frame.h = images[i].h;
                
                newSpriteSheet.frames.push_back(frame);
                    
            }
            sprite_sheets.push_back(newSpriteSheet);
            return true;
        }
    } 
    return false;
}

bool SCREEN_Atlas::Load(const uint8_t *data, size_t data_size) {
	SCREEN_ByteReader reader(data, data_size);

	AtlasHeader header = reader.Read<AtlasHeader>();
	num_images = header.numImages;
	num_fonts = header.numFonts;
	if (header.magic != ATLAS_MAGIC) {
		return false;
	}

	images = reader.ReadMultipleAlloc<AtlasImage>(num_images);
	fonts = new SCREEN_AtlasFont[num_fonts];
	for (int i = 0; i < num_fonts; i++) {
		AtlasFontHeader font_header = reader.Read<AtlasFontHeader>();
		fonts[i].padding = font_header.padding;
		fonts[i].height = font_header.height;
		fonts[i].ascend = font_header.ascend;
		fonts[i].distslope = font_header.distslope;
		fonts[i].numRanges = font_header.numRanges;
		fonts[i].numChars = font_header.numChars;
		fonts[i].ranges = reader.ReadMultipleAlloc<AtlasCharRange>(font_header.numRanges);
		fonts[i].charData = reader.ReadMultipleAlloc<AtlasChar>(font_header.numChars);
		memcpy(fonts[i].name, font_header.name, sizeof(font_header.name));
	}
    
    registerSpriteSheet("I_BACK_R", 4); // sprite sheet with four frames
    registerSpriteSheet("I_GEAR_R", 4); // sprite sheet with four frames
    registerSpriteSheet("I_GRID_R", 4); // sprite sheet with four frames
    registerSpriteSheet("I_HOME_R", 4); // sprite sheet with four frames
    registerSpriteSheet("I_LINES_R", 4); // sprite sheet with four frames
    registerSpriteSheet("I_OFF_R", 4); // sprite sheet with four frames
    registerSpriteSheet("I_TAB_R", 2); // sprite sheet with four frames
    
	return true;
}

const SCREEN_AtlasFont *SCREEN_Atlas::getFont(FontID id) const {
	if (id.isInvalid())
		return nullptr;

	for (int i = 0; i < num_fonts; i++) {
		if (!strcmp(id.id, fonts[i].name))
			return &fonts[i];
	}
	return nullptr;
}

const AtlasImage *SCREEN_Atlas::getImage(ImageID name) const 
{

    if (name.isInvalid())
      return nullptr;

    // if the image is a sprite sheet
    if (name.isSpriteSheet)
    {
        // search in sprite sheet array for the id tag
        std::string id(name.id);
        SpriteSheetArray::const_iterator it;
        int i = 0;

        for (it = sprite_sheets.begin(); it != sprite_sheets.end(); it++, i++)
        {
            SpriteSheet element = *it;
            
            if (element.id == id)
            {
                // sprite sheet found in map
                return &sprite_sheets[i].frames[name.currentFrame_];
            }
        } 
    }

    for (int i = 0; i < num_images; i++) 
    {
        if (!strcmp(name.id, images[i].name))
        {
            return &images[i];
        }
    }
    return nullptr;
}


bool SCREEN_Atlas::measureImage(ImageID id, float *w, float *h) const {
    bool ok = false;
    const AtlasImage *image = getImage(id);
    if (image) {
        *w = (float)image->w;
        *h = (float)image->h;
        ok = true;
    } else {
        *w = 0.0f;
        *h = 0.0f;
    }
    return ok;
}

const AtlasChar *SCREEN_AtlasFont::getChar(int utf32) const {
	for (int i = 0; i < numRanges; i++) {
		if (utf32 >= ranges[i].start && utf32 < ranges[i].end) {
			const AtlasChar *c = &charData[ranges[i].result_index + utf32 - ranges[i].start];
			if (c->ex == 0 && c->ey == 0)
				return nullptr;
			else
				return c;
		}
	}
	return nullptr;
}

SCREEN_Atlas::~SCREEN_Atlas() {
	delete[] images;
	delete[] fonts;
}

SCREEN_AtlasFont::~SCREEN_AtlasFont() {
	delete[] ranges;
	delete[] charData;
}
