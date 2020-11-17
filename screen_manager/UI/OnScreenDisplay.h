#pragma once

#include <string>
#include <list>
#include <mutex>

#include "Common/Math/geom2d.h"
#include "Common/UI/View.h"

class SCREEN_DrawBuffer;

class SCREEN_OnScreenMessages {
public:
	void Show(const std::string &message, float duration_s = 1.0f, uint32_t color = 0xFFFFFF, int icon = -1, bool checkUnique = true, const char *id = nullptr);
	void ShowOnOff(const std::string &message, bool b, float duration_s = 1.0f, uint32_t color = 0xFFFFFF, int icon = -1);
	bool IsEmpty() const { return messages_.empty(); }

	void Lock() {
		mutex_.lock();
	}
	void Unlock() {
		mutex_.unlock();
	}

	void Clean();

	struct Message {
		int icon;
		uint32_t color;
		std::string text;
		const char *id;
		double endTime;
		double duration;
	};
	const std::list<Message> &Messages() { return messages_; }

private:
	std::list<Message> messages_;
	std::mutex mutex_;
};

class SCREEN_OnScreenMessagesView : public SCREEN_UI::InertView {
public:
	SCREEN_OnScreenMessagesView(SCREEN_UI::LayoutParams *layoutParams = nullptr) : SCREEN_UI::InertView(layoutParams) {}
	void Draw(SCREEN_UIContext &dc);
};

extern SCREEN_OnScreenMessages osm;
