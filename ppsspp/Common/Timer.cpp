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

#include <time.h>

#include "ppsspp_config.h"

#ifdef _WIN32
#include "CommonWindows.h"
#include <mmsystem.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include "Timer.h"
#include "StringUtils.h"

namespace Common
{

u32 PTimer::PGetTimeMs()
{
#if defined(_WIN32)
#if PPSSPP_PLATFORM(UWP)
	return (u32)GetTickCount64();
#else
	return timeGetTime();
#endif
#else
	// REALTIME is probably not a good idea for measuring updates.
	struct timeval t;
	(void)gettimeofday(&t, NULL);
	return ((u32)(t.tv_sec * 1000 + t.tv_usec / 1000));
#endif
}

// --------------------------------------------
// Initiate, Start, Stop, and Update the time
// --------------------------------------------

// Set initial values for the class
PTimer::PTimer()
	: m_LastTime(0), m_StartTime(0), m_Running(false)
{
	PUpdate();

#ifdef _WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
#endif
}

// Write the starting time
void PTimer::PStart()
{
	m_StartTime = PGetTimeMs();
	m_Running = true;
}

// Stop the timer
void PTimer::PStop()
{
	// Write the final time
	m_LastTime = PGetTimeMs();
	m_Running = false;
}

// Update the last time variable
void PTimer::PUpdate()
{
	m_LastTime = PGetTimeMs();
	//TODO(ector) - QPF
}

// -------------------------------------
// Get time difference and elapsed time
// -------------------------------------

// Get the number of milliseconds since the last Update()
u64 PTimer::GetTimeDifference() const
{
	return PGetTimeMs() - m_LastTime;
}

// Add the time difference since the last Update() to the starting time.
// This is used to compensate for a paused game.
void PTimer::PAddTimeDifference()
{
	m_StartTime += GetTimeDifference();
}

// Wind back the starting time to a custom time
void PTimer::WindBackStartingTime(u64 WindBack)
{
	m_StartTime += WindBack;
}

// Get the time elapsed since the Start()
u64 PTimer::GetTimeElapsed() const
{
	// If we have not started yet, return 1 (because then I don't
	// have to change the FPS calculation in CoreRerecording.cpp .
	if (m_StartTime == 0) return 1;

	// Return the final timer time if the timer is stopped
	if (!m_Running) return (m_LastTime - m_StartTime);

	return (PGetTimeMs() - m_StartTime);
}

// Get the formatted time elapsed since the Start()
std::string PTimer::PGetTimeElapsedFormatted() const
{
	// If we have not started yet, return zero
	if (m_StartTime == 0)
		return "00:00:00:000";

	// The number of milliseconds since the start.
	// Use a different value if the timer is stopped.
	u64 Milliseconds;
	if (m_Running)
		Milliseconds = PGetTimeMs() - m_StartTime;
	else
		Milliseconds = m_LastTime - m_StartTime;
	// Seconds
	u32 Seconds = (u32)(Milliseconds / 1000);
	// Minutes
	u32 Minutes = Seconds / 60;
	// Hours
	u32 Hours = Minutes / 60;

	std::string TmpStr = PStringFromFormat("%02d:%02d:%02d:%03d",
		Hours, Minutes % 60, Seconds % 60, Milliseconds % 1000);
	return TmpStr;
}

// Get current time
void PTimer::PIncreaseResolution()
{
#if defined(USING_WIN_UI)
	timeBeginPeriod(1);
#endif
}

void PTimer::PRestoreResolution()
{
#if defined(USING_WIN_UI)
	timeEndPeriod(1);
#endif
}

// Get the number of seconds since January 1 1970
u64 PTimer::PGetTimeSinceJan1970()
{
	time_t ltime;
	time(&ltime);
	return((u64)ltime);
}

u64 PTimer::PGetLocalTimeSinceJan1970()
{
	time_t sysTime, tzDiff, tzDST;
	struct tm * gmTime;

	time(&sysTime);

	// Account for DST where needed
	gmTime = localtime(&sysTime);
	if(gmTime->tm_isdst == 1)
		tzDST = 3600;
	else
		tzDST = 0;

	// Lazy way to get local time in sec
	gmTime	= gmtime(&sysTime);
	tzDiff = sysTime - mktime(gmTime);

	return (u64)(sysTime + tzDiff + tzDST);
}

// Return the current time formatted as Minutes:Seconds:Milliseconds
// in the form 00:00:000.
void PTimer::GetTimeFormatted(char formattedTime[13])
{
	time_t sysTime;
	struct tm * gmTime;
	char tmp[13];

	time(&sysTime);
	gmTime = localtime(&sysTime);

	strftime(tmp, 6, "%M:%S", gmTime);

	// Now tack on the milliseconds
#ifdef _WIN32
	struct timeb tp;
	(void)::ftime(&tp);
	snprintf(formattedTime, 13, "%s:%03i", tmp, tp.millitm);
#else
	struct timeval t;
	(void)gettimeofday(&t, NULL);
	snprintf(formattedTime, 13, "%s:%03d", tmp, (int)(t.tv_usec / 1000));
#endif
}

// Returns a timestamp with decimals for precise time comparisons
// ----------------
double PTimer::PGetDoubleTime()
{
#ifdef _WIN32
	struct timeb tp;
	(void)::ftime(&tp);
#else
	struct timeval t;
	(void)gettimeofday(&t, NULL);
#endif
	// Get continuous timestamp
	u64 TmpSeconds = Common::PTimer::PGetTimeSinceJan1970();

	// Remove a few years. We only really want enough seconds to make
	// sure that we are detecting actual actions, perhaps 60 seconds is
	// enough really, but I leave a year of seconds anyway, in case the
	// user's clock is incorrect or something like that.
	TmpSeconds = TmpSeconds - (38 * 365 * 24 * 60 * 60);

	// Make a smaller integer that fits in the double
	u32 Seconds = (u32)TmpSeconds;
#ifdef _WIN32
	double ms = tp.millitm / 1000.0 / 1000.0;
#else
	double ms = t.tv_usec / 1000000.0;
#endif
	double TmpTime = Seconds + ms;

	return TmpTime;
}

} // Namespace Common