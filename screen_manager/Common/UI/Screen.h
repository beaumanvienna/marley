// Super basic screen manager. Let's you, well, switch between screens. Can also be used
// to pop one screen in front for a bit while keeping another one running, it's basically
// a native "activity stack". Well actually that part is still a TODO.
//
// Semantics
//
// switchScreen: When you call this, on a newed screen, the SCREEN_ScreenManager takes ownership.
// On the next update, it switches to the new screen and deletes the previous screen.
//
// TODO: A way to do smooth transitions between screens. Will probably involve screenshotting
// the previous screen and then animating it on top of the current screen with transparency
// and/or other similar effects.

#pragma once

#include <vector>
#include <mutex>
#include <string>

#include "Common/Common.h"
#include "Common/Input/InputState.h"

namespace SCREEN_UI {
	class View;
}

enum DialogResult {
	DR_OK,
	DR_CANCEL,
	DR_YES,
	DR_NO,
	DR_BACK,
};

class SCREEN_ScreenManager;
class SCREEN_UIContext;

namespace SCREEN_Draw {
	class SCREEN_DrawContext;
}

class SCREEN_Screen {
public:
	SCREEN_Screen() : screenManager_(nullptr) { }
	virtual ~SCREEN_Screen() {
		screenManager_ = nullptr;
	}

	virtual void onFinish(DialogResult reason) {}
	virtual void update() {}
	virtual void preRender() {}
	virtual void render() {}
	virtual void postRender() {}
	virtual void resized() {}
	virtual void dialogFinished(const SCREEN_Screen *dialog, DialogResult result) {}
	virtual bool touch(const SCREEN_TouchInput &touch) { return false;  }
	virtual bool key(const SCREEN_KeyInput &key) { return false; }
	virtual bool axis(const SCREEN_AxisInput &touch) { return false; }
	virtual void sendMessage(const char *msg, const char *value) {}
	virtual void deviceLost() {}
	virtual void deviceRestored() {}

	virtual void RecreateViews() {}

	SCREEN_ScreenManager *screenManager() { return screenManager_; }
	void setSCREEN_ScreenManager(SCREEN_ScreenManager *sm) { screenManager_ = sm; }

	// This one is icky to use because you can't know what's in it until you know
	// what screen it is.
	virtual void *dialogData() { return 0; }

	virtual std::string tag() const { return std::string(""); }

	virtual bool isTransparent() const { return false; }
	virtual bool isTopLevel() const { return false; }

	virtual SCREEN_TouchInput transformTouch(const SCREEN_TouchInput &touch) { return touch; }

private:
	SCREEN_ScreenManager *screenManager_;
	DISALLOW_COPY_AND_ASSIGN(SCREEN_Screen);
};

class SCREEN_Transition {
public:
	SCREEN_Transition() {}
};

enum {
	LAYER_SIDEMENU = 1,
	LAYER_TRANSPARENT = 2,
};

typedef void(*PostRenderCallback)(SCREEN_UIContext *ui, void *userdata);

class SCREEN_ScreenManager {
public:
	SCREEN_ScreenManager();
	virtual ~SCREEN_ScreenManager();

	void switchScreen(SCREEN_Screen *screen);
	void update();

	void setUIContext(SCREEN_UIContext *context) { uiContext_ = context; }
	SCREEN_UIContext *getUIContext() { return uiContext_; }

	void setSCREEN_DrawContext(SCREEN_Draw::SCREEN_DrawContext *context) { thin3DContext_ = context; }
	SCREEN_Draw::SCREEN_DrawContext *getSCREEN_DrawContext() { return thin3DContext_; }

	void setPostRenderCallback(PostRenderCallback cb, void *userdata) {
		postRenderCb_ = cb;
		postRenderUserdata_ = userdata;
	}

	void render();
	void resized();
	void shutdown();

	void deviceLost();
	void deviceRestored();

	// Push a dialog box in front. Currently 1-level only.
	void push(SCREEN_Screen *screen, int layerFlags = 0);

	// Recreate all views
	void RecreateAllViews();

	// Pops the dialog away.
	void finishDialog(SCREEN_Screen *dialog, DialogResult result = DR_OK);
	SCREEN_Screen *dialogParent(const SCREEN_Screen *dialog) const;

	// Instant touch, separate from the update() mechanism.
	bool touch(const SCREEN_TouchInput &touch);
	bool key(const SCREEN_KeyInput &key);
	bool axis(const SCREEN_AxisInput &touch);

	// Generic facility for gross hacks :P
	void sendMessage(const char *msg, const char *value);

	SCREEN_Screen *topScreen() const;

	std::recursive_mutex inputLock_;

private:
	void pop();
	void switchToNext();
	void processFinishDialog();

	SCREEN_UIContext *uiContext_;
	SCREEN_Draw::SCREEN_DrawContext *thin3DContext_;

	PostRenderCallback postRenderCb_ = nullptr;
	void *postRenderUserdata_ = nullptr;

	const SCREEN_Screen *dialogFinished_;
	DialogResult dialogResult_;

	struct Layer {
		SCREEN_Screen *screen;
		int flags;  // From LAYER_ enum above
		SCREEN_UI::View *focusedView;  // TODO: save focus here. Going for quick solution now to reset focus.
	};

	// Dialog stack. These are shown "on top" of base screens and the Android back button works as expected.
	// Used for options, in-game menus and other things you expect to be able to back out from onto something.
	std::vector<Layer> stack_;
	std::vector<Layer> nextStack_;
};
