#include "SDL_syswm.h"
#include "SDL.h"

#include "Common/GPU/OpenGL/GLRenderManager.h"
#include "Common/GPU/OpenGL/GLCommon.h"
#include "Common/GraphicsContext.h"

class SDLGLSCREEN_GraphicsContext : public SCREEN_GraphicsContext {
public:
	SDLGLSCREEN_GraphicsContext() {
	}

	// Returns 0 on success.
	int Init(SDL_Window *&window, int x, int y, int mode, std::string *error_message);

	void Shutdown() override;
	void ShutdownFromRenderThread() override;

	void SwapBuffers() override {
		// Do nothing, the render thread takes care of this.
	}

	// Gets forwarded to the render thread.
	void SwapInterval(int interval) override;

	void Resize() override {}

	SCREEN_Draw::SCREEN_DrawContext *GetSCREEN_DrawContext() override {
		return draw_;
	}

	void ThreadStart() override {
		renderManager_->ThreadStart(draw_);
	}

	bool ThreadFrame() override {
		return renderManager_->ThreadFrame();
	}

	void ThreadEnd() override {
		renderManager_->ThreadEnd();
	}

	void StopThread() override {
		renderManager_->WaitUntilQueueIdle();
		renderManager_->StopThread();
	}

private:
	SCREEN_Draw::SCREEN_DrawContext *draw_ = nullptr;
	SDL_Window *window_;
	SDL_GLContext glContext = nullptr;
	SCREEN_GLRenderManager *renderManager_ = nullptr;
};
