// Copyright (c) 2014- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <functional>
#include <memory>

#include "Common/UI/UIScreen.h"
#include "Common/UI/ViewGroup.h"
#include "UI/MiscScreens.h"
#include "UI/TextureUtil.h"

class GamePauseScreenPCSX2 : public SCREEN_UIDialogScreenWithBackground {
public:
	GamePauseScreenPCSX2() {printf("jc: GamePauseScreenPCSX2()\n");}
	virtual ~GamePauseScreenPCSX2();

	virtual void dialogFinished(const SCREEN_Screen *dialog, DialogResult dr) override;

protected:
	virtual void CreateViews() override;
	virtual void update() override;

private:
	SCREEN_UI::EventReturn OnGameSettings(SCREEN_UI::EventParams &e);
	SCREEN_UI::EventReturn OnExitToMarley(SCREEN_UI::EventParams &e);

	bool finishNextFrame_ = false;
	std::string gamePath_;
};

