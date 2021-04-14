#include <iostream>
#include "Common/System/Display.h"
#include "Common/Input/InputState.h"
#include "Common/UI/Root.h"
#include "Common/UI/Screen.h"
#include "Common/UI/UI.h"
#include "Common/UI/View.h"

#include "Common/Log.h"
#include "Common/TimeUtil.h"

SCREEN_ScreenManager::SCREEN_ScreenManager() {
	uiContext_ = 0;
	dialogFinished_ = 0;
}

SCREEN_ScreenManager::~SCREEN_ScreenManager() {
	shutdown();
}

void SCREEN_ScreenManager::switchScreen(SCREEN_Screen *screen) {
	if (!nextStack_.empty() && screen == nextStack_.front().screen) {
		printf("Already switching to this screen");
		return;
	}
	// Note that if a dialog is found, this will be a silent background switch that
	// will only become apparent if the dialog is closed. The previous screen will stick around
	// until that switch.
	// TODO: is this still true?
	if (!nextStack_.empty()) {
		printf("Already had a nextStack_! Asynchronous open while doing something? Deleting the new screen.");
		delete screen;
		return;
	}
	if (screen == nullptr) {
		printf("Switching to a zero screen, this can't be good");
	}
	if (stack_.empty() || screen != stack_.back().screen) {
		screen->setSCREEN_ScreenManager(this);
		nextStack_.push_back({ screen, 0 });
	}
}

void SCREEN_ScreenManager::update() {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	if (!nextStack_.empty()) {
		switchToNext();
	}

	if (stack_.size()) {
		stack_.back().screen->update();
	}
}

void SCREEN_ScreenManager::switchToNext() {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	if (nextStack_.empty()) {
		printf("switchToNext: No nextStack_!");
	}

	Layer temp = {nullptr, 0};
	if (!stack_.empty()) {
		temp = stack_.back();
		stack_.pop_back();
	}
	stack_.push_back(nextStack_.front());
	if (temp.screen) {
		delete temp.screen;
	}
	SCREEN_UI::SetFocusedView(nullptr);

	for (size_t i = 1; i < nextStack_.size(); ++i) {
		stack_.push_back(nextStack_[i]);
	}
	nextStack_.clear();
}

bool SCREEN_ScreenManager::touch(const SCREEN_TouchInput &touch) {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	bool result = false;
	// Send release all events to every screen layer.
	if (touch.flags & TOUCH_RELEASE_ALL) {
		for (auto &layer : stack_) {
			SCREEN_Screen *screen = layer.screen;
			result = layer.screen->touch(screen->transformTouch(touch));
		}
	} else if (!stack_.empty()) {
		SCREEN_Screen *screen = stack_.back().screen;
		result = stack_.back().screen->touch(screen->transformTouch(touch));
	}
	return result;
}

bool SCREEN_ScreenManager::key(const SCREEN_KeyInput &key) {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	bool result = false;
	// Send key up to every screen layer.
	if (key.flags & KEY_UP) {
		for (auto &layer : stack_) {
			result = layer.screen->key(key);
		}
	} else if (!stack_.empty()) {
		result = stack_.back().screen->key(key);
	}
	return result;
}

bool SCREEN_ScreenManager::axis(const SCREEN_AxisInput &axis) {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	bool result = false;
	// Send center axis to every screen layer.
	if (axis.value == 0) {
		for (auto &layer : stack_) {
			result = layer.screen->axis(axis);
		}
	} else if (!stack_.empty()) {
		result = stack_.back().screen->axis(axis);
	}
	return result;
}

void SCREEN_ScreenManager::deviceLost() {
	for (auto &iter : stack_)
		iter.screen->deviceLost();
}

void SCREEN_ScreenManager::deviceRestored() {
	for (auto &iter : stack_)
		iter.screen->deviceRestored();
}

void SCREEN_ScreenManager::resized() {
	printf("SCREEN_ScreenManager::resized(dp: %dx%d)\n", dp_xres, dp_yres);
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	// Have to notify the whole stack, otherwise there will be problems when going back
	// to non-top screens.
	for (auto iter = stack_.begin(); iter != stack_.end(); ++iter) {
		iter->screen->resized();
	}
}

void SCREEN_ScreenManager::render() {
	if (!stack_.empty()) {
		switch (stack_.back().flags) {
		case LAYER_SIDEMENU:
		case LAYER_TRANSPARENT:
			if (stack_.size() == 1) {
				printf("Can't have sidemenu over nothing");
				break;
			} else {
				auto iter = stack_.end();
				iter--;
				iter--;
				Layer backback = *iter;

				// TODO: Make really sure that this "mismatched" pre/post only happens
				// when screens are "compatible" (both are SCREEN_UIScreens, for example).
				backback.screen->preRender();
				backback.screen->render();
				stack_.back().screen->render();
				if (postRenderCb_)
					postRenderCb_(getUIContext(), postRenderUserdata_);
				backback.screen->postRender();
				break;
			}
		default:
			stack_.back().screen->preRender();
			stack_.back().screen->render();
			if (postRenderCb_)
				postRenderCb_(getUIContext(), postRenderUserdata_);
			stack_.back().screen->postRender();
			break;
		}
	} else {
		printf("No current screen!");
	}

	processFinishDialog();
}

void SCREEN_ScreenManager::sendMessage(const char *msg, const char *value) {
	if (!strcmp(msg, "recreateviews"))
		RecreateAllViews();
	if (!strcmp(msg, "lost_focus")) {
		SCREEN_TouchInput input;
		input.flags = TOUCH_RELEASE_ALL;
		input.timestamp = time_now_d();
		input.id = 0;
		touch(input);
	}
	if (!stack_.empty())
		stack_.back().screen->sendMessage(msg, value);
}

SCREEN_Screen *SCREEN_ScreenManager::topScreen() const {
	if (!stack_.empty())
		return stack_.back().screen;
	else
		return 0;
}

void SCREEN_ScreenManager::shutdown() {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	for (auto layer : stack_)
		delete layer.screen;
	stack_.clear();
	for (auto layer : nextStack_)
		delete layer.screen;
	nextStack_.clear();
}

void SCREEN_ScreenManager::push(SCREEN_Screen *screen, int layerFlags) {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	screen->setSCREEN_ScreenManager(this);
	if (screen->isTransparent()) {
		layerFlags |= LAYER_TRANSPARENT;
	}

	// Release touches and unfocus.
	lastFocusView.push(SCREEN_UI::GetFocusedView());
	SCREEN_UI::SetFocusedView(nullptr);
	SCREEN_TouchInput input;
	input.flags = TOUCH_RELEASE_ALL;
	input.timestamp = time_now_d();
	input.id = 0;
	touch(input);

	Layer layer = {screen, layerFlags};
	if (nextStack_.empty())
		stack_.push_back(layer);
	else
		nextStack_.push_back(layer);
}

void SCREEN_ScreenManager::pop() {
	std::lock_guard<std::recursive_mutex> guard(inputLock_);
	if (stack_.size()) {
		delete stack_.back().screen;
		stack_.pop_back();
	} else {
		printf("Can't pop when stack empty");
	}
}

void SCREEN_ScreenManager::RecreateAllViews() {
	for (auto it = stack_.begin(); it != stack_.end(); ++it) {
		it->screen->RecreateViews();
	}
}

void SCREEN_ScreenManager::finishDialog(SCREEN_Screen *dialog, DialogResult result) {
	if (stack_.empty()) {
		printf("Must be in a dialog to finishDialog");
		return;
	}
	if (dialog != stack_.back().screen) {
		printf("Wrong dialog being finished!");
		return;
	}
	dialog->onFinish(result);
	dialogFinished_ = dialog;
	dialogResult_ = result;
}

SCREEN_Screen *SCREEN_ScreenManager::dialogParent(const SCREEN_Screen *dialog) const {
	for (size_t i = 1; i < stack_.size(); ++i) {
		if (stack_[i].screen == dialog) {
			// The previous screen was the caller (not necessarily the topmost.)
			return stack_[i - 1].screen;
		}
	}

	return nullptr;
}

void SCREEN_ScreenManager::processFinishDialog() {
	if (dialogFinished_) {
		{
			std::lock_guard<std::recursive_mutex> guard(inputLock_);
			// Another dialog may have been pushed before the render, so search for it.
			SCREEN_Screen *caller = dialogParent(dialogFinished_);
			for (size_t i = 0; i < stack_.size(); ++i) {
				if (stack_[i].screen == dialogFinished_) {
					stack_.erase(stack_.begin() + i);
				}
			}

			if (!caller) {
				printf("Settings dialog finished\n");
			} else if (caller != topScreen()) {
				// The caller may get confused if we call dialogFinished() now.
				printf("Skipping non-top dialog when finishing dialog.\n");
			} else {
				caller->dialogFinished(dialogFinished_, dialogResult_);
			}
		}
		delete dialogFinished_;
		dialogFinished_ = nullptr;

		SCREEN_UI::SetFocusedView(lastFocusView.top());
		lastFocusView.pop();
	}
}
