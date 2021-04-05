#pragma once

#include <functional>

#include "Common/UI/Context.h"
#include "Common/Input/InputState.h"

namespace SCREEN_UI {

// The ONLY global is the currently focused item.
// Can be and often is null.
void EnableFocusMovement(bool enable);
bool IsFocusMovementEnabled();
View *GetFocusedView();
void SetFocusedView(View *view, bool force = false);
void RemoveQueuedEventsByEvent(Event *e);
void RemoveQueuedEventsByView(View * v);

void EventTriggered(Event *e, EventParams params);
void DispatchEvents();

class ViewGroup;

void LayoutViewHierarchy(const SCREEN_UIContext &dc, ViewGroup *root, bool ignoreInsets);
void UpdateViewHierarchy(ViewGroup *root);
// Hooks arrow keys for navigation
bool KeyEvent(const SCREEN_KeyInput &key, ViewGroup *root);
bool TouchEvent(const SCREEN_TouchInput &touch, ViewGroup *root);
bool AxisEvent(const SCREEN_AxisInput &axis, ViewGroup *root);

enum class SCREEN_UISound {
	SELECT = 0,
	BACK,
	CONFIRM,
	TOGGLE_ON,
	TOGGLE_OFF,
	COUNT,
};

void SetSoundEnabled(bool enabled);
void SetSoundCallback(std::function<void(SCREEN_UISound)> func);

void PlayUISound(SCREEN_UISound sound);

}  // namespace SCREEN_UI
