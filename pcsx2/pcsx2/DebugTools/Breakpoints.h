// Copyright (c) 2012- PPSSPP Project.

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

#include <vector>

#include "DebugInterface.h"
#include "Pcsx2Types.h"

struct BreakPointCond
{
	DebugInterface *debug;
	PostfixExpression expression;
	char expressionString[128];

	BreakPointCond() : debug(NULL)
	{
		expressionString[0] = '\0';
	}

	u32 Evaluate()
	{
		u64 result;
		if (!debug->parseExpression(expression,result) || result == 0) return 0;
		return 1;
	}
};

struct BreakPoint
{
	BreakPoint() : addr(0), enabled(false), temporary(false), hasCond(false)
	{}

	u32	addr;
	bool enabled;
	bool temporary;

	bool hasCond;
	BreakPointCond cond;

	bool operator == (const BreakPoint &other) const {
		return addr == other.addr;
	}
	bool operator < (const BreakPoint &other) const {
		return addr < other.addr;
	}
};

enum MemCheckCondition
{
	MEMCHECK_READ = 0x01,
	MEMCHECK_WRITE = 0x02,
	MEMCHECK_WRITE_ONCHANGE = 0x04,

	MEMCHECK_READWRITE = 0x03,
};

enum MemCheckResult
{
	MEMCHECK_IGNORE = 0x00,
	MEMCHECK_LOG = 0x01,
	MEMCHECK_BREAK = 0x02,

	MEMCHECK_BOTH = 0x03,
};

struct PMemCheck
{
	PMemCheck();
	u32 start;
	u32 end;

	MemCheckCondition cond;
	MemCheckResult result;

	u32 numHits;

	u32 lastPC;
	u32 lastAddr;
	int lastSize;

	void Action(u32 addr, bool write, int size, u32 pc);
	void JitBefore(u32 addr, bool write, int size, u32 pc);
	void JitCleanup();

	void Log(u32 addr, bool write, int size, u32 pc);

	bool operator == (const PMemCheck &other) const {
		return start == other.start && end == other.end;
	}
};

// BreakPoints cannot overlap, only one is allowed per address.
// MemChecks can overlap, as long as their ends are different.
// WARNING: MemChecks are not used in the interpreter or HLE currently.
class PCBreakPoints
{
public:
	static const size_t INVALID_BREAKPOINT = -1;
	static const size_t INVALID_MEMCHECK = -1;

	static bool IsAddressBreakPoint(u32 addr);
	static bool IsAddressBreakPoint(u32 addr, bool* enabled);
	static bool IsTempBreakPoint(u32 addr);
	static void AddBreakPoint(u32 addr, bool temp = false);
	static void RemoveBreakPoint(u32 addr);
	static void ChangeBreakPoint(u32 addr, bool enable);
	static void ClearAllBreakPoints();
	static void ClearTemporaryBreakPoints();

	// Makes a copy.  Temporary breakpoints can't have conditions.
	static void ChangeBreakPointAddCond(u32 addr, const BreakPointCond &cond);
	static void ChangeBreakPointRemoveCond(u32 addr);
	static BreakPointCond *GetBreakPointCondition(u32 addr);

	static void AddMemCheck(u32 start, u32 end, MemCheckCondition cond, MemCheckResult result);
	static void RemoveMemCheck(u32 start, u32 end);
	static void ChangeMemCheck(u32 start, u32 end, MemCheckCondition cond, MemCheckResult result);
	static void ClearAllMemChecks();

	static void SetSkipFirst(u32 pc);
	static u32 CheckSkipFirst(u32 pc);

	// Includes uncached addresses.
	static const std::vector<PMemCheck> GetMemCheckRanges();

	static const std::vector<PMemCheck> GetMemChecks();
	static const std::vector<BreakPoint> GetBreakpoints();
	static size_t GetNumMemchecks() { return memChecks_.size(); }

	static void Update(u32 addr = 0);

	static void SetBreakpointTriggered(bool b) { breakpointTriggered_ = b; };
	static bool GetBreakpointTriggered() { return breakpointTriggered_; };

private:
	static size_t FindBreakpoint(u32 addr, bool matchTemp = false, bool temp = false);
	// Finds exactly, not using a range check.
	static size_t FindMemCheck(u32 start, u32 end);

	static std::vector<BreakPoint> breakPoints_;
	static u32 breakSkipFirstAt_;
	static u64 breakSkipFirstTicks_;
	static bool breakpointTriggered_;

	static std::vector<PMemCheck> memChecks_;
	static std::vector<PMemCheck *> cleanupMemChecks_;
};


// called from the dynarec
u32 __fastcall standardizeBreakpointAddress(u32 addr);
