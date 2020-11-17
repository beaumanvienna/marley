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

#include <string>

#include "ppsspp_config.h"

#include "Common.h"
#include "Common/Log.h"
#include "StringUtils.h"
#include "Common/Data/Encoding/Utf8.h"

#define LOG_BUF_SIZE 2048

bool HandleAssert(const char *function, const char *file, int line, const char *expression, const char* format, ...) {
	// Read message and write it to the log
	char text[LOG_BUF_SIZE];
	const char *caption = "Critical";
	va_list args;
	va_start(args, format);
	vsnprintf(text, sizeof(text), format, args);
	va_end(args);

	char formatted[LOG_BUF_SIZE];
	snprintf(formatted, sizeof(formatted), "(%s:%s:%d) %s: [%s] %s", file, function, line, caption, expression, text);
	
	printf("%s\n", formatted);

	return false;
}
