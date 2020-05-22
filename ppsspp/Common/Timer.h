// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#pragma once

#include "Common.h"
#include <string>

namespace PCommon
{
class PTimer
{
public:
	PTimer();

	void PStart();
	void PStop();
	void PUpdate();

	// The time difference is always returned in milliseconds, regardless of alternative internal representation
	u64 GetTimeDifference() const;
	void PAddTimeDifference();
	void WindBackStartingTime(u64 WindBack);

	static void PIncreaseResolution();
	static void PRestoreResolution();
	static u64 PGetTimeSinceJan1970();
	static u64 PGetLocalTimeSinceJan1970();
	static double PGetDoubleTime();

  static void GetTimeFormatted(char formattedTime[13]);
	std::string PGetTimeElapsedFormatted() const;
	u64 GetTimeElapsed() const;

	static u32 PGetTimeMs();

private:
	u64 m_LastTime;
	u64 m_StartTime;
#ifdef _WIN32
	u64 m_frequency;
#endif
	bool m_Running;
};

} // Namespace PCommon
