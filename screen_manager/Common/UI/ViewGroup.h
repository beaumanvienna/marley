#pragma once

#include <cfloat>
#include <vector>
#include <set>
#include <mutex>
#include <iostream>
#include "Common/Math/geom2d.h"
#include "Common/UI/View.h"
#include "UI/Scale.h"

namespace SCREEN_UI {

class AnchorTranslateTween;

struct NeighborResult {
	NeighborResult() : view(0), score(0) {}
	NeighborResult(View *v, float s) : view(v), score(s) {}

	View *view;
	float score;
};

class ViewGroup : public View {
public:
	ViewGroup(LayoutParams *layoutParams = 0) : View(layoutParams) {}
	virtual ~ViewGroup();

	// Pass through external events to children.
	virtual bool Key(const SCREEN_KeyInput &input) override;
	virtual void Touch(const SCREEN_TouchInput &input) override;
	virtual void Axis(const SCREEN_AxisInput &input) override;

	// By default, a container will layout to its own bounds.
	virtual void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override = 0;
	virtual void Layout() override = 0;
	virtual void Update() override;
	virtual void Query(float x, float y, std::vector<View *> &list) override;

	virtual void DeviceLost() override;
	virtual void DeviceRestored(SCREEN_Draw::SCREEN_DrawContext *draw) override;

	virtual void Draw(SCREEN_UIContext &dc) override;

	// These should be unused.
	virtual float GetContentWidth() const { return 0.0f; }
	virtual float GetContentHeight() const { return 0.0f; }

	// Takes ownership! DO NOT add a view to multiple parents!
	template <class T>
	T *Add(T *view) {
		std::lock_guard<std::mutex> guard(modifyLock_);
		views_.push_back(view);
		return view;
	}

	virtual bool SetFocus() override;
	virtual bool SubviewFocused(View *view) override;
	virtual void RemoveSubview(View *view);

	void SetDefaultFocusView(View *view) { defaultFocusView_ = view; }
	View *GetDefaultFocusView() { return defaultFocusView_; }

	// Assumes that layout has taken place.
	NeighborResult FindNeighbor(View *view, FocusDirection direction, NeighborResult best);

	virtual bool CanBeFocused() const override { return false; }
	virtual bool IsViewGroup() const override { return true; }

	virtual void SetBG(const Drawable &bg) { bg_ = bg; }

	virtual void Clear();
	void PersistData(PersistStatus status, std::string anonId, PersistMap &storage) override;
	View *GetViewByIndex(int index) { return views_[index]; }
	int GetNumSubviews() const { return (int)views_.size(); }
	void SetHasDropShadow(bool has) { hasDropShadow_ = has; }
	void SetDropShadowExpand(float s) { dropShadowExpand_ = s; }

	void Lock() { modifyLock_.lock(); }
	void Unlock() { modifyLock_.unlock(); }

	void SetClip(bool clip) { clip_ = clip; }
	std::string Describe() const override { return "ViewGroup: " + View::Describe(); }

protected:
	std::mutex modifyLock_;  // Hold this when changing the subviews.
	std::vector<View *> views_;
	View *defaultFocusView_ = nullptr;
	Drawable bg_;
	float dropShadowExpand_ = 0.0f;
	bool hasDropShadow_ = false;
	bool clip_ = false;
};

// A frame layout contains a single child view (normally).
// It simply centers the child view.
class FrameLayout : public ViewGroup {
public:
	void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
	void Layout() override;
};

const float NONE = -FLT_MAX;

class AnchorLayoutParams : public LayoutParams {
public:
	AnchorLayoutParams(Size w, Size h, float l, float t, float r, float b, bool c = false)
		: LayoutParams(w, h, LP_ANCHOR), left(l), top(t), right(r), bottom(b), center(c) {

	}
	AnchorLayoutParams(Size w, Size h, bool c = false)
		: LayoutParams(w, h, LP_ANCHOR), left(0), top(0), right(NONE), bottom(NONE), center(c) {
	}
	AnchorLayoutParams(float l, float t, float r, float b, bool c = false)
		: LayoutParams(WRAP_CONTENT, WRAP_CONTENT, LP_ANCHOR), left(l), top(t), right(r), bottom(b), center(c) {}

	// These are not bounds, but distances from the container edges.
	// Set to NONE to not attach this edge to the container.
	float left, top, right, bottom;
	bool center;  // If set, only two "sides" can be set, and they refer to the center, not the edge, of the view being layouted.

	static LayoutParamsType StaticType() {
		return LP_ANCHOR;
	}
};

class AnchorLayout : public ViewGroup {
public:
	AnchorLayout(LayoutParams *layoutParams = 0) : ViewGroup(layoutParams), overflow_(true) {}
	void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
	void Layout() override;
	void Overflow(bool allow) {
		overflow_ = allow;
	}
	std::string Describe() const override { return "AnchorLayout: " + View::Describe(); }

private:
	void MeasureViews(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert);
	bool overflow_;
};

class LinearLayoutParams : public LayoutParams {
public:
	LinearLayoutParams()
		: LayoutParams(LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), hasMargins_(false) {}
	explicit LinearLayoutParams(float wgt, Gravity grav = G_TOPLEFT)
		: LayoutParams(LP_LINEAR), weight(wgt), gravity(grav), hasMargins_(false) {}
	LinearLayoutParams(float wgt, const Margins &mgn)
		: LayoutParams(LP_LINEAR), weight(wgt), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}
	LinearLayoutParams(Size w, Size h, float wgt = 0.0f, Gravity grav = G_TOPLEFT)
		: LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(grav), hasMargins_(false) {}
	LinearLayoutParams(Size w, Size h, float wgt, Gravity grav, const Margins &mgn)
		: LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(grav), margins(mgn), hasMargins_(true) {}
	LinearLayoutParams(Size w, Size h, const Margins &mgn)
		: LayoutParams(w, h, LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}
	LinearLayoutParams(Size w, Size h, float wgt, const Margins &mgn)
		: LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}
	LinearLayoutParams(const Margins &mgn)
		: LayoutParams(WRAP_CONTENT, WRAP_CONTENT, LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true) {}

	float weight;
	Gravity gravity;
	Margins margins;

	bool HasMargins() const { return hasMargins_; }

	static LayoutParamsType StaticType() {
		return LP_LINEAR;
	}

private:
	bool hasMargins_;
};

class LinearLayout : public ViewGroup {
public:
	LinearLayout(Orientation orientation, LayoutParams *layoutParams = 0)
		: ViewGroup(layoutParams), orientation_(orientation), defaultMargins_(0), spacing_(10) {}

	void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
	void Layout() override;
	void SetSpacing(float spacing) {
		spacing_ = spacing;
	}
	std::string Describe() const override { return (orientation_ == ORIENT_HORIZONTAL ? "LinearLayoutHoriz: " : "LinearLayoutVert: ") + View::Describe(); }

protected:
	Orientation orientation_;
private:
	Margins defaultMargins_;
	float spacing_;
};

// GridLayout is a little different from the Android layout. This one has fixed size
// rows and columns. Items are not allowed to deviate from the set sizes.
// Initially, only horizontal layout is supported.
struct GridLayoutSettings {
	GridLayoutSettings() : orientation(ORIENT_HORIZONTAL), columnWidth(100), rowHeight(50), spacing(5), fillCells(false) {}
	GridLayoutSettings(int colW, int colH, int spac = 5)
		: orientation(ORIENT_HORIZONTAL), columnWidth(colW), rowHeight(colH), spacing(spac), fillCells(false) {}

	Orientation orientation;
	int columnWidth;
	int rowHeight;
	int spacing;
	bool fillCells;
};

class GridLayout : public ViewGroup {
public:
	GridLayout(GridLayoutSettings settings, LayoutParams *layoutParams = 0);

	void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
	void Layout() override;
	std::string Describe() const override { return "GridLayout: " + View::Describe(); }

private:
	GridLayoutSettings settings_;
	int numColumns_;
};

// A scrollview usually contains just a single child - a linear layout or similar.
class ScrollView : public ViewGroup {
public:
	ScrollView(Orientation orientation, LayoutParams *layoutParams = 0, bool exactly = false)
		: ViewGroup(layoutParams), orientation_(orientation), vert_type_exactly_(exactly) {}

	void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
	void Layout() override;

	bool Key(const SCREEN_KeyInput &input) override;
	void Touch(const SCREEN_TouchInput &input) override;
	void Draw(SCREEN_UIContext &dc) override;
	std::string Describe() const override { return "ScrollView: " + View::Describe(); }

	void ScrollTo(float newScrollPos);
	void ScrollToBottom();
	void ScrollRelative(float distance);
	float GetScrollPosition();
	bool CanScroll() const;
	void Update() override;

	// Override so that we can scroll to the active one after moving the focus.
	bool SubviewFocused(View *view) override;
	void PersistData(PersistStatus status, std::string anonId, PersistMap &storage) override;
	void SetVisibility(Visibility visibility) override;

	// Quick hack to prevent scrolling to top in some lists
	void SetScrollToTop(bool t) { scrollToTopOnSizeChange_ = t; }

private:
	float ClampedScrollPos(float pos);

	Orientation orientation_;
	float scrollPos_ = 0.0f;
	float scrollStart_ = 0.0f;
	float scrollTarget_ = 0.0f;
	int scrollTouchId_ = -1;
	bool scrollToTarget_ = false;
	float inertia_ = 0.0f;
	float pull_ = 0.0f;
	float lastViewSize_ = 0.0f;
	bool scrollToTopOnSizeChange_ = false;
    bool vert_type_exactly_ = false;
};

class ViewPager : public ScrollView {
public:
};


class ChoiceStrip : public LinearLayout {
public:
	ChoiceStrip(Orientation orientation, LayoutParams *layoutParams = 0);

	void AddChoice(const std::string &title);
	void AddChoice(SCREEN_ImageID buttonImage, std::string tooltip = "", bool * toolTipShown = nullptr);
    void AddChoice(const std::string &title, SCREEN_ImageID icon, SCREEN_ImageID icon_active, SCREEN_ImageID icon_depressed, SCREEN_ImageID icon_depressed_inactive, const std::string &text);

	int GetSelection() const { return selected_; }
	void SetSelection(int sel);

	void HighlightChoice(unsigned int choice);

	bool Key(const SCREEN_KeyInput &input) override;

	void SetTopTabs(bool tabs) { topTabs_ = tabs; }
	void Draw(SCREEN_UIContext &dc) override;

	std::string Describe() const override { return "ChoiceStrip: " + View::Describe(); }

	Event OnChoice;

private:
	StickyChoice *Choice(int index);
	EventReturn OnChoiceClick(EventParams &e);
	int selected_;
	bool topTabs_;  // Can be controlled with L/R.
};


class TabHolder : public LinearLayout {
public:
	TabHolder(Orientation orientation, float stripSize, LayoutParams *layoutParams = 0, float leftMargin = 0.0f);

	template <class T>
	T *AddTab(const std::string &title, T *tabContents) {
		AddTabContents(title, (View *)tabContents);
		return tabContents;
	}

	void SetCurrentTab(int tab, bool skipTween = false);

	int GetCurrentTab() const { return currentTab_; }
	std::string Describe() const override { return "TabHolder: " + View::Describe(); }

	void PersistData(PersistStatus status, std::string anonId, PersistMap &storage) override;
    void SetIcon(SCREEN_ImageID icon, SCREEN_ImageID icon_active, SCREEN_ImageID icon_depressed, SCREEN_ImageID icon_depressed_inactive) {
        icon_ = icon; 
        icon_active_ = icon_active; 
        icon_depressed_ = icon_depressed;
        icon_depressed_inactive_ = icon_depressed_inactive;
        useIcons_ = true;
    }

private:
    bool useIcons_ = false;
    SCREEN_ImageID icon_, icon_active_, icon_depressed_, icon_depressed_inactive_;
	void AddTabContents(const std::string &title, View *tabContents);
	EventReturn OnTabClick(EventParams &e);

	ChoiceStrip *tabStrip_ = nullptr;
	ScrollView *tabScroll_ = nullptr;
	AnchorLayout *contents_ = nullptr;

	float stripSize_;
	int currentTab_ = 0;
	std::vector<View *> tabs_;
	std::vector<AnchorTranslateTween *> tabTweens_;
};

class ListAdaptor {
public:
	virtual ~ListAdaptor() {}
	virtual View *CreateItemView(int index) = 0;
	virtual int GetNumItems() = 0;
	virtual bool AddEventCallback(View *view, std::function<EventReturn(EventParams&)> callback) { return false; }
	virtual std::string GetTitle(int index) const { return ""; }
	virtual void SetSelected(int sel) { }
	virtual int GetSelected() { return -1; }
};

class ChoiceListAdaptor : public ListAdaptor {
public:
	ChoiceListAdaptor(const char *items[], int numItems) : items_(items), numItems_(numItems) {}
	virtual View *CreateItemView(int index);
	virtual int GetNumItems() { return numItems_; }
	virtual bool AddEventCallback(View *view, std::function<EventReturn(EventParams&)> callback);

private:
	const char **items_;
	int numItems_;
};


// The "selected" item is what was previously selected (optional). This items will be drawn differently.
class StringVectorListAdaptor : public ListAdaptor {
public:
	StringVectorListAdaptor() : selected_(-1) {}
	StringVectorListAdaptor(const std::vector<std::string> &items, int selected = -1) : items_(items), selected_(selected) {}
	virtual View *CreateItemView(int index) override;
	virtual int GetNumItems() override { return (int)items_.size(); }
	virtual bool AddEventCallback(View *view, std::function<EventReturn(EventParams&)> callback) override;
	void SetSelected(int sel) override { selected_ = sel; }
	virtual std::string GetTitle(int index) const override { return items_[index]; }
	virtual int GetSelected() override { return selected_; }

private:
	std::vector<std::string> items_;
	int selected_;
};

// A list view is a scroll view with autogenerated items.
class ListView : public ScrollView {
public:
	ListView(ListAdaptor *a, std::set<int> hidden = std::set<int>(), LayoutParams *layoutParams = 0);

	int GetSelected() { return adaptor_->GetSelected(); }
	virtual void Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert) override;
	virtual void SetMaxHeight(float mh) { maxHeight_ = mh; }
	Event OnChoice;
	std::string Describe() const override { return "ListView: " + View::Describe(); }

private:
	void CreateAllItems();
	EventReturn OnItemCallback(int num, EventParams &e);
	ListAdaptor *adaptor_;
	LinearLayout *linLayout_;
	float maxHeight_;
	std::set<int> hidden_;
};

}  // namespace SCREEN_UI
