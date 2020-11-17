#pragma once

#include <set>

#include "Common/Math/lin/vec3.h"
#include "Common/UI/Screen.h"
#include "Common/UI/ViewGroup.h"

using namespace SCREEN_Lin;

class SCREEN_I18NCategory;
namespace SCREEN_Draw {
	class SCREEN_DrawContext;
}

class SCREEN_UIScreen : public SCREEN_Screen {
public:
	SCREEN_UIScreen();
	~SCREEN_UIScreen();

	void update() override;
	void preRender() override;
	void render() override;
	void postRender() override;
	void deviceLost() override;
	void deviceRestored() override;

	bool touch(const TouchInput &touch) override;
	bool key(const KeyInput &touch) override;
	bool axis(const AxisInput &touch) override;

	TouchInput transformTouch(const TouchInput &touch) override;

	virtual void TriggerFinish(DialogResult result);

	// Some useful default event handlers
	SCREEN_UI::EventReturn OnOK(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnCancel(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnBack(SCREEN_UI::EventParams &e);

protected:
	virtual void CreateViews() = 0;
	virtual void DrawBackground(SCREEN_UIContext &dc) {}

	virtual void RecreateViews() override { recreateViews_ = true; }

	SCREEN_UI::ViewGroup *root_ = nullptr;
	SCREEN_Vec3 translation_ = SCREEN_Vec3(0.0f);
	SCREEN_Vec3 scale_ = SCREEN_Vec3(1.0f);
	float alpha_ = 1.0f;
	bool ignoreInsets_ = false;

private:
	void DoRecreateViews();

	bool recreateViews_ = true;
};

class SCREEN_UIDialogScreen : public SCREEN_UIScreen {
public:
	SCREEN_UIDialogScreen() : SCREEN_UIScreen(), finished_(false) {printf("jc: SCREEN_UIDialogScreen() : SCREEN_UIScreen(), finished_(false)\n");}
	bool key(const KeyInput &key) override;
	void sendMessage(const char *msg, const char *value) override;

private:
	bool finished_;
};


class SCREEN_PopupScreen : public SCREEN_UIDialogScreen {
public:
	SCREEN_PopupScreen(std::string title, std::string button1 = "", std::string button2 = "");

	virtual void CreatePopupContents(SCREEN_UI::ViewGroup *parent) = 0;
	virtual void CreateViews() override;
	virtual bool isTransparent() const override { return true; }
	virtual bool touch(const TouchInput &touch) override;
	virtual bool key(const KeyInput &key) override;
	virtual void resized() override;

	virtual void TriggerFinish(DialogResult result) override;

	void SetPopupOrigin(const SCREEN_UI::View *view);

protected:
	virtual bool FillVertical() const { return false; }
	virtual SCREEN_UI::Size PopupWidth() const { return 550; }
	virtual bool ShowButtons() const { return true; }
	virtual bool CanComplete(DialogResult result) { return true; }
	virtual void OnCompleted(DialogResult result) {}

	virtual void update() override;

private:
	SCREEN_UI::ViewGroup *box_;
	SCREEN_UI::Button *defaultButton_;
	std::string title_;
	std::string button1_;
	std::string button2_;

	enum {
		FRAMES_LEAD_IN = 6,
		FRAMES_LEAD_OUT = 4,
	};

	int frames_ = 0;
	int finishFrame_ = -1;
	DialogResult finishResult_;
	bool hasPopupOrigin_ = false;
	Point popupOrigin_;
};

class ListSCREEN_PopupScreen : public SCREEN_PopupScreen {
public:
	ListSCREEN_PopupScreen(std::string title) : SCREEN_PopupScreen(title) {}
	ListSCREEN_PopupScreen(std::string title, const std::vector<std::string> &items, int selected, std::function<void(int)> callback, bool showButtons = false)
		: SCREEN_PopupScreen(title, "OK", "Cancel"), adaptor_(items, selected), callback_(callback), showButtons_(showButtons) {
	}
	ListSCREEN_PopupScreen(std::string title, const std::vector<std::string> &items, int selected, bool showButtons = false)
		: SCREEN_PopupScreen(title, "OK", "Cancel"), adaptor_(items, selected), showButtons_(showButtons) {
	}

	int GetChoice() const {
		return listView_->GetSelected();
	}
	std::string GetChoiceString() const {
		return adaptor_.GetTitle(listView_->GetSelected());
	}
	void SetHiddenChoices(std::set<int> hidden) {
		hidden_ = hidden;
	}
	virtual std::string tag() const override { return std::string("listpopup"); }

	SCREEN_UI::Event OnChoice;

protected:
	virtual bool FillVertical() const override { return false; }
	virtual bool ShowButtons() const override { return showButtons_; }
	virtual void CreatePopupContents(SCREEN_UI::ViewGroup *parent) override;
	SCREEN_UI::StringVectorListAdaptor adaptor_;
	SCREEN_UI::ListView *listView_ = nullptr;

private:
	SCREEN_UI::EventReturn OnListChoice(SCREEN_UI::EventParams &e);

	std::function<void(int)> callback_;
	bool showButtons_ = false;
	std::set<int> hidden_;
};

class MessageSCREEN_PopupScreen : public SCREEN_PopupScreen {
public:
	MessageSCREEN_PopupScreen(std::string title, std::string message, std::string button1, std::string button2, std::function<void(bool)> callback) 
		: SCREEN_PopupScreen(title, button1, button2), message_(message), callback_(callback) {}
	SCREEN_UI::Event OnChoice;

protected:
	virtual bool FillVertical() const override { return false; }
	virtual bool ShowButtons() const override { return true; }
	virtual void CreatePopupContents(SCREEN_UI::ViewGroup *parent) override;

private:
	void OnCompleted(DialogResult result) override;
	std::string message_;
	std::function<void(bool)> callback_;
};

// TODO: Need a way to translate OK and Cancel

namespace SCREEN_UI {

class SliderSCREEN_PopupScreen : public SCREEN_PopupScreen {
public:
	SliderSCREEN_PopupScreen(int *value, int minValue, int maxValue, const std::string &title, int step = 1, const std::string &units = "")
		: SCREEN_PopupScreen(title, "OK", "Cancel"), units_(units), value_(value), minValue_(minValue), maxValue_(maxValue), step_(step) {}
	virtual void CreatePopupContents(ViewGroup *parent) override;

	void SetNegativeDisable(const std::string &str) {
		negativeLabel_ = str;
		disabled_ = *value_ < 0;
	}

	Event OnChange;

private:
	EventReturn OnDecrease(EventParams &params);
	EventReturn OnIncrease(EventParams &params);
	EventReturn OnTextChange(EventParams &params);
	EventReturn OnSliderChange(EventParams &params);
	virtual void OnCompleted(DialogResult result) override;
	Slider *slider_ = nullptr;
	SCREEN_UI::TextEdit *edit_ = nullptr;
	std::string units_;
	std::string negativeLabel_;
	int *value_;
	int sliderValue_ = 0;
	int minValue_;
	int maxValue_;
	int step_;
	bool changing_ = false;
	bool disabled_ = false;
};

class SliderFloatSCREEN_PopupScreen : public SCREEN_PopupScreen {
public:
	SliderFloatSCREEN_PopupScreen(float *value, float minValue, float maxValue, const std::string &title, float step = 1.0f, const std::string &units = "")
	: SCREEN_PopupScreen(title, "OK", "Cancel"), units_(units), value_(value), minValue_(minValue), maxValue_(maxValue), step_(step), changing_(false) {}
	void CreatePopupContents(SCREEN_UI::ViewGroup *parent) override;

	Event OnChange;

private:
	EventReturn OnIncrease(EventParams &params);
	EventReturn OnDecrease(EventParams &params);
	EventReturn OnTextChange(EventParams &params);
	EventReturn OnSliderChange(EventParams &params);
	virtual void OnCompleted(DialogResult result) override;
	SCREEN_UI::SliderFloat *slider_;
	SCREEN_UI::TextEdit *edit_;
	std::string units_;
	float sliderValue_;
	float *value_;
	float minValue_;
	float maxValue_;
	float step_;
	bool changing_;
};

class TextEditSCREEN_PopupScreen : public SCREEN_PopupScreen {
public:
	TextEditSCREEN_PopupScreen(std::string *value, const std::string &placeholder, const std::string &title, int maxLen)
		: SCREEN_PopupScreen(title, "OK", "Cancel"), value_(value), placeholder_(placeholder), maxLen_(maxLen) {}
	virtual void CreatePopupContents(ViewGroup *parent) override;

	Event OnChange;

private:
	virtual void OnCompleted(DialogResult result) override;
	TextEdit *edit_;
	std::string *value_;
	std::string textEditValue_;
	std::string placeholder_;
	int maxLen_;
};

// Reads and writes value to determine the current selection.
class SCREEN_PopupMultiChoice : public SCREEN_UI::Choice {
public:
	SCREEN_PopupMultiChoice(int *value, const std::string &text, const char **choices, int minVal, int numChoices,
		const char *category, SCREEN_ScreenManager *screenManager, SCREEN_UI::LayoutParams *layoutParams = nullptr)
		: SCREEN_UI::Choice(text, "", false, layoutParams), value_(value), choices_(choices), minVal_(minVal), numChoices_(numChoices), 
		category_(category), screenManager_(screenManager) {
		if (*value >= numChoices + minVal)
			*value = numChoices + minVal - 1;
		if (*value < minVal)
			*value = minVal;
		OnClick.Handle(this, &SCREEN_PopupMultiChoice::HandleClick);
		UpdateText();
	}

	virtual void Draw(SCREEN_UIContext &dc) override;
	virtual void Update() override;

	void HideChoice(int c) {
		hidden_.insert(c);
	}

	SCREEN_UI::Event OnChoice;

protected:
	int *value_;
	const char **choices_;
	int minVal_;
	int numChoices_;
	void UpdateText();

private:
	SCREEN_UI::EventReturn HandleClick(SCREEN_UI::EventParams &e);

	void ChoiceCallback(int num);
	virtual void PostChoiceCallback(int num) {}

	const char *category_;
	SCREEN_ScreenManager *screenManager_;
	std::string valueText_;
	bool restoreFocus_ = false;
	std::set<int> hidden_;
};

// Allows passing in a dynamic vector of strings. Saves the string.
class SCREEN_PopupMultiChoiceDynamic : public SCREEN_PopupMultiChoice {
public:
	SCREEN_PopupMultiChoiceDynamic(std::string *value, const std::string &text, std::vector<std::string> choices,
		const char *category, SCREEN_ScreenManager *screenManager, SCREEN_UI::LayoutParams *layoutParams = nullptr)
		: SCREEN_UI::SCREEN_PopupMultiChoice(&valueInt_, text, nullptr, 0, (int)choices.size(), category, screenManager, layoutParams),
		  valueStr_(value) {
		choices_ = new const char *[numChoices_];
		valueInt_ = 0;
		for (int i = 0; i < numChoices_; i++) {
			choices_[i] = new char[choices[i].size() + 1];
			memcpy((char *)choices_[i], choices[i].c_str(), choices[i].size() + 1);
			if (*value == choices_[i])
				valueInt_ = i;
		}
		value_ = &valueInt_;
		UpdateText();
	}
	~SCREEN_PopupMultiChoiceDynamic() {
		for (int i = 0; i < numChoices_; i++) {
			delete[] choices_[i];
		}
		delete[] choices_;
	}

protected:
	void PostChoiceCallback(int num) {
		*valueStr_ = choices_[num];
	}

private:
	int valueInt_;
	std::string *valueStr_;
};

class SCREEN_PopupSliderChoice : public Choice {
public:
	SCREEN_PopupSliderChoice(int *value, int minValue, int maxValue, const std::string &text, SCREEN_ScreenManager *screenManager, const std::string &units = "", LayoutParams *layoutParams = 0);
	SCREEN_PopupSliderChoice(int *value, int minValue, int maxValue, const std::string &text, int step, SCREEN_ScreenManager *screenManager, const std::string &units = "", LayoutParams *layoutParams = 0);

	virtual void Draw(SCREEN_UIContext &dc) override;

	void SetFormat(const char *fmt) {
		fmt_ = fmt;
	}
	void SetZeroLabel(const std::string &str) {
		zeroLabel_ = str;
	}
	void SetNegativeDisable(const std::string &str) {
		negativeLabel_ = str;
	}

	Event OnChange;

private:
	EventReturn HandleClick(EventParams &e);
	EventReturn HandleChange(EventParams &e);

	int *value_;
	int minValue_;
	int maxValue_;
	int step_;
	const char *fmt_;
	std::string zeroLabel_;
	std::string negativeLabel_;
	std::string units_;
	SCREEN_ScreenManager *screenManager_;
	bool restoreFocus_;
};

class SCREEN_PopupSliderChoiceFloat : public Choice {
public:
	SCREEN_PopupSliderChoiceFloat(float *value, float minValue, float maxValue, const std::string &text, SCREEN_ScreenManager *screenManager, const std::string &units = "", LayoutParams *layoutParams = 0);
	SCREEN_PopupSliderChoiceFloat(float *value, float minValue, float maxValue, const std::string &text, float step, SCREEN_ScreenManager *screenManager, const std::string &units = "", LayoutParams *layoutParams = 0);

	virtual void Draw(SCREEN_UIContext &dc) override;

	void SetFormat(const char *fmt) {
		fmt_ = fmt;
	}
	void SetZeroLabel(const std::string &str) {
		zeroLabel_ = str;
	}

	Event OnChange;

private:
	EventReturn HandleClick(EventParams &e);
	EventReturn HandleChange(EventParams &e);
	float *value_;
	float minValue_;
	float maxValue_;
	float step_;
	const char *fmt_;
	std::string zeroLabel_;
	std::string units_;
	SCREEN_ScreenManager *screenManager_;
	bool restoreFocus_;
};

class SCREEN_PopupTextInputChoice: public Choice {
public:
	SCREEN_PopupTextInputChoice(std::string *value, const std::string &title, const std::string &placeholder, int maxLen, SCREEN_ScreenManager *screenManager, LayoutParams *layoutParams = 0);

	virtual void Draw(SCREEN_UIContext &dc) override;

	Event OnChange;

private:
	EventReturn HandleClick(EventParams &e);
	EventReturn HandleChange(EventParams &e);
	SCREEN_ScreenManager *screenManager_;
	std::string *value_;
	std::string placeHolder_;
	std::string defaultText_;
	int maxLen_;
	bool restoreFocus_;
};

class SCREEN_ChoiceWithValueDisplay : public SCREEN_UI::Choice {
public:
	SCREEN_ChoiceWithValueDisplay(int *value, const std::string &text, LayoutParams *layoutParams = 0)
		: Choice(text, layoutParams), iValue_(value) {}

	SCREEN_ChoiceWithValueDisplay(std::string *value, const std::string &text, const char *category, LayoutParams *layoutParams = 0)
		: Choice(text, layoutParams), sValue_(value), category_(category) {}

	SCREEN_ChoiceWithValueDisplay(std::string *value, const std::string &text, std::string (*translateCallback)(const char *value), LayoutParams *layoutParams = 0)
		: Choice(text, layoutParams), sValue_(value), translateCallback_(translateCallback) {
	}

	virtual void Draw(SCREEN_UIContext &dc) override;

private:
	int *iValue_ = nullptr;
	std::string *sValue_ = nullptr;
	const char *category_ = nullptr;
	std::string (*translateCallback_)(const char *value) = nullptr;
};

}  // namespace SCREEN_UI
