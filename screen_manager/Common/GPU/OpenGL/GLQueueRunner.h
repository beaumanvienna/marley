#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "Common/GPU/OpenGL/GLCommon.h"
#include "Common/GPU/DataFormat.h"
#include "Common/Data/Collections/TinySet.h"

struct GLRViewport {
	float x, y, w, h, minZ, maxZ;
};

struct GLRect2D {
	int x, y, w, h;
};

struct GLOffset2D {
	int x, y;
};

enum class SCREEN_GLRAllocType {
	NONE,
	NEW,
	ALIGNED,
};

class SCREEN_GLRShader;
class SCREEN_GLRTexture;
class SCREEN_GLRBuffer;
class SCREEN_GLRFramebuffer;
class SCREEN_GLRProgram;
class SCREEN_GLRInputLayout;

enum class SCREEN_GLRRenderCommand : uint8_t {
	DEPTH,
	STENCILFUNC,
	STENCILOP,
	BLEND,
	BLENDCOLOR,
	LOGICOP,
	UNIFORM4I,
	UNIFORM4F,
	UNIFORMMATRIX,
	TEXTURESAMPLER,
	TEXTURELOD,
	VIEWPORT,
	SCISSOR,
	RASTER,
	CLEAR,
	INVALIDATE,
	BINDPROGRAM,
	BINDTEXTURE,
	BIND_FB_TEXTURE,
	BIND_VERTEX_BUFFER,
	BIND_BUFFER,
	GENMIPS,
	DRAW,
	DRAW_INDEXED,
	PUSH_CONSTANTS,
	TEXTURE_SUBIMAGE,
};

// TODO: Bloated since the biggest struct decides the size. Will need something more efficient (separate structs with shared
// type field, smashed right after each other?)
// Also, all GLenums are really only 16 bits.
struct GLRRenderData {
	SCREEN_GLRRenderCommand cmd;
	union {
		struct {
			GLboolean enabled;
			GLenum srcColor;
			GLenum dstColor;
			GLenum srcAlpha;
			GLenum dstAlpha;
			GLenum funcColor;
			GLenum funcAlpha;
			int mask;
		} blend;
		struct {
			float color[4];
		} blendColor;
		struct {
			GLboolean enabled;
			GLenum logicOp;
		} logic;
		struct {
			GLboolean enabled;
			GLboolean write;
			GLenum func;
		} depth;
		struct {
			GLboolean enabled;
			GLenum func;
			uint8_t ref;
			uint8_t compareMask;
		} stencilFunc;
		struct {
			GLenum sFail;
			GLenum zFail;
			GLenum pass;
			uint8_t writeMask;
		} stencilOp;  // also write mask
		struct {
			GLenum mode;  // primitive
			GLint buffer;
			GLint first;
			GLint count;
		} draw;
		struct {
			GLenum mode;  // primitive
			GLint count;
			GLint instances;
			GLint indexType;
			void *indices;
		} drawIndexed;
		struct {
			const char *name;  // if null, use loc
			const GLint *loc; // NOTE: This is a pointer so we can immediately use things that are "queried" during program creation.
			GLint count;
			float v[4];
		} uniform4;
		struct {
			const char *name;  // if null, use loc
			const GLint *loc;
			float m[16];
		} uniformMatrix4;
		struct {
			uint32_t clearColor;
			float clearZ;
			uint8_t clearStencil;
			uint8_t colorMask; // Like blend, but for the clear.
			GLuint clearMask;   // GL_COLOR_BUFFER_BIT etc
			int16_t scissorX;
			int16_t scissorY;
			int16_t scissorW;
			int16_t scissorH;
		} clear;  // also used for invalidate
		struct {
			int slot;
			SCREEN_GLRTexture *texture;
		} texture;
		struct {
			SCREEN_GLRTexture *texture;
			SCREEN_Draw::SCREEN_DataFormat format;
			int level;
			int x;
			int y;
			int width;
			int height;
			SCREEN_GLRAllocType allocType;
			uint8_t *data;  // owned, delete[]-d
		} texture_subimage;
		struct {
			int slot;
			SCREEN_GLRFramebuffer *framebuffer;
			int aspect;
		} bind_fb_texture;
		struct {
			SCREEN_GLRBuffer *buffer;
			GLuint target;
		} bind_buffer;
		struct {
			SCREEN_GLRProgram *program;
		} program;
		struct {
			SCREEN_GLRInputLayout *inputLayout;
			SCREEN_GLRBuffer *buffer;
			size_t offset;
		} bindVertexBuffer;
		struct {
			int slot;
			GLenum wrapS;
			GLenum wrapT;
			GLenum magFilter;
			GLenum minFilter;  // also includes mip. GL...
			float anisotropy;
		} textureSampler;
		struct {
			int slot;
			float minLod;
			float maxLod;
			float lodBias;
		} textureLod;
		struct {
			GLRViewport vp;
		} viewport;
		struct {
			GLRect2D rc;
		} scissor;
		struct {
			GLboolean cullEnable;
			GLenum frontFace;
			GLenum cullFace;
			GLboolean ditherEnable;
		} raster;
	};
};

// Unlike in Vulkan, we can't create stuff on the main thread, but need to
// defer this too. A big benefit will be that we'll be able to do all creation
// at the start of the frame.
enum class SCREEN_GLRInitStepType : uint8_t {
	CREATE_TEXTURE,
	CREATE_SHADER,
	CREATE_PROGRAM,
	CREATE_BUFFER,
	CREATE_INPUT_LAYOUT,
	CREATE_FRAMEBUFFER,

	TEXTURE_IMAGE,
	TEXTURE_FINALIZE,
	BUFFER_SUBDATA,
};

struct GLRInitStep {
	GLRInitStep(SCREEN_GLRInitStepType _type) : stepType(_type) {}
	SCREEN_GLRInitStepType stepType;
	union {
		struct {
			SCREEN_GLRTexture *texture;
			GLenum target;
		} create_texture;
		struct {
			SCREEN_GLRShader *shader;
			// This char arrays needs to be allocated with new[].
			char *code;
			GLuint stage;
		} create_shader;
		struct {
			SCREEN_GLRProgram *program;
			SCREEN_GLRShader *shaders[3];
			int num_shaders;
			bool support_dual_source;
		} create_program;
		struct {
			SCREEN_GLRBuffer *buffer;
			int size;
			GLuint usage;
		} create_buffer;
		struct {
			SCREEN_GLRInputLayout *inputLayout;
		} create_input_layout;
		struct {
			SCREEN_GLRFramebuffer *framebuffer;
		} create_framebuffer;
		struct {
			SCREEN_GLRBuffer *buffer;
			int offset;
			int size;
			uint8_t *data;  // owned, delete[]-d
			bool deleteData;
		} buffer_subdata;
		struct {
			SCREEN_GLRTexture *texture;
			SCREEN_Draw::SCREEN_DataFormat format;
			int level;
			int width;
			int height;
			SCREEN_GLRAllocType allocType;
			bool linearFilter;
			uint8_t *data;  // owned, delete[]-d
		} texture_image;
		struct {
			SCREEN_GLRTexture *texture;
			int maxLevel;
			bool genMips;
		} texture_finalize;
	};
};

enum class SCREEN_GLRStepType : uint8_t {
	RENDER,
	COPY,
	BLIT,
	READBACK,
	READBACK_IMAGE,
	RENDER_SKIP,
};

enum class SCREEN_GLRRenderPassAction {
	DONT_CARE,
	CLEAR,
	KEEP,
};

class SCREEN_GLRFramebuffer;

enum {
	GLR_ASPECT_COLOR = 1,
	GLR_ASPECT_DEPTH = 2,
	GLR_ASPECT_STENCIL = 3,
};

struct GLRStep {
	GLRStep(SCREEN_GLRStepType _type) : stepType(_type) {}
	SCREEN_GLRStepType stepType;
	std::vector<GLRRenderData> commands;
	TinySet<const SCREEN_GLRFramebuffer *, 8> dependencies;
	const char *tag;
	union {
		struct {
			SCREEN_GLRFramebuffer *framebuffer;
			SCREEN_GLRRenderPassAction color;
			SCREEN_GLRRenderPassAction depth;
			SCREEN_GLRRenderPassAction stencil;
			// Note: not accurate.
			int numDraws;
		} render;
		struct {
			SCREEN_GLRFramebuffer *src;
			SCREEN_GLRFramebuffer *dst;
			GLRect2D srcRect;
			GLOffset2D dstPos;
			int aspectMask;
		} copy;
		struct {
			SCREEN_GLRFramebuffer *src;
			SCREEN_GLRFramebuffer *dst;
			GLRect2D srcRect;
			GLRect2D dstRect;
			int aspectMask;
			GLboolean filter;
		} blit;
		struct {
			int aspectMask;
			SCREEN_GLRFramebuffer *src;
			GLRect2D srcRect;
			SCREEN_Draw::SCREEN_DataFormat dstFormat;
		} readback;
		struct {
			SCREEN_GLRTexture *texture;
			GLRect2D srcRect;
			int mipLevel;
		} readback_image;
	};
};

class SCREEN_GLQueueRunner {
public:
	SCREEN_GLQueueRunner() {}

	void RunInitSteps(const std::vector<GLRInitStep> &steps, bool skipGLCalls);

	void RunSteps(const std::vector<GLRStep *> &steps, bool skipGLCalls);
	void LogSteps(const std::vector<GLRStep *> &steps);

	void CreateDeviceObjects();
	void DestroyDeviceObjects();

	inline int RPIndex(SCREEN_GLRRenderPassAction color, SCREEN_GLRRenderPassAction depth) {
		return (int)depth * 3 + (int)color;
	}

	void CopyReadbackBuffer(int width, int height, SCREEN_Draw::SCREEN_DataFormat srcFormat, SCREEN_Draw::SCREEN_DataFormat destFormat, int pixelStride, uint8_t *pixels);

	void Resize(int width, int height) {
		targetWidth_ = width;
		targetHeight_ = height;
	}

	bool SawOutOfMemory() {
		return sawOutOfMemory_;
	}

	std::string GetGLString(int name) const {
		auto it = glStrings_.find(name);
		return it != glStrings_.end() ? it->second : "";
	}

private:
	void InitCreateFramebuffer(const GLRInitStep &step);

	void PerformBindFramebufferAsRenderTarget(const GLRStep &pass);
	void PerformRenderPass(const GLRStep &pass, bool first, bool last);
	void PerformCopy(const GLRStep &pass);
	void PerformBlit(const GLRStep &pass);
	void PerformReadback(const GLRStep &pass);
	void PerformReadbackImage(const GLRStep &pass);

	void LogRenderPass(const GLRStep &pass);
	void LogCopy(const GLRStep &pass);
	void LogBlit(const GLRStep &pass);
	void LogReadback(const GLRStep &pass);
	void LogReadbackImage(const GLRStep &pass);

	void ResizeReadbackBuffer(size_t requiredSize);

	void fbo_ext_create(const GLRInitStep &step);
	void fbo_bind_fb_target(bool read, GLuint name);
	GLenum fbo_get_fb_target(bool read, GLuint **cached);
	void fbo_unbind();

	SCREEN_GLRFramebuffer *curFB_ = nullptr;

	GLuint globalVAO_ = 0;

	int curFBWidth_ = 0;
	int curFBHeight_ = 0;
	int targetWidth_ = 0;
	int targetHeight_ = 0;

	// Readback buffer. Currently we only support synchronous readback, so we only really need one.
	// We size it generously.
	uint8_t *readbackBuffer_ = nullptr;
	int readbackBufferSize_ = 0;
	// Temp buffer for color conversion
	uint8_t *tempBuffer_ = nullptr;
	int tempBufferSize_ = 0;

	float maxAnisotropyLevel_ = 0.0f;

	// Framebuffer state?
	GLuint currentDrawHandle_ = 0;
	GLuint currentReadHandle_ = 0;

	GLuint AllocTextureName();

	// Texture name cache. Ripped straight from TextureCacheGLES.
	std::vector<GLuint> nameCache_;
	std::unordered_map<int, std::string> glStrings_;

	bool sawOutOfMemory_ = false;
	bool useDebugGroups_ = false;
};
