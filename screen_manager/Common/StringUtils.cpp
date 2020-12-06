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

#include <cstring>
#include "ppsspp_config.h"
#include <cstdarg>
#include <errno.h>
#include <string>
#include <sstream>
#include <limits.h>
#include <algorithm>
#include <iomanip>

#include "Common/Common.h"
#include "Common/Buffer.h"
#include "Common/StringUtils.h"

void truncate_cpy(char *dest, size_t destSize, const char *src) {
	size_t len = strlen(src);
	if (len >= destSize - 1) {
		memcpy(dest, src, destSize - 1);
		dest[destSize - 1] = '\0';
	} else {
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
}

long parseHexLong(std::string s) {
	long value = 0;

	if (s.substr(0,2) == "0x") {
		//s = s.substr(2);
	}
	value = strtoul(s.c_str(),0, 0);
	return value;
}

long parseLong(std::string s) {
	long value = 0;
	if (s.substr(0,2) == "0x") {
		s = s.substr(2);
		value = strtol(s.c_str(),NULL, 16);
	} else {
		value = strtol(s.c_str(),NULL, 10);
	}
	return value;
}

bool PCharArrayFromFormatV(char* out, int outsize, const char* format, va_list args)
{
	int writtenCount = vsnprintf(out, outsize, format, args);

	if (writtenCount > 0 && writtenCount < outsize)
	{
		out[writtenCount] = '\0';
		return true;
	}
	else
	{
		out[outsize - 1] = '\0';
		return false;
	}
}

bool PSplitPath(const std::string& full_path, std::string* _pPath, std::string* _pFilename, std::string* _pExtension)
{
	if (full_path.empty())
		return false;

	size_t dir_end = full_path.find_last_of("/");
	if (std::string::npos == dir_end)
		dir_end = 0;
	else
		dir_end += 1;

	size_t fname_end = full_path.rfind('.');
	if (fname_end < dir_end || std::string::npos == fname_end)
		fname_end = full_path.size();

	if (_pPath)
		*_pPath = full_path.substr(0, dir_end);

	if (_pFilename)
		*_pFilename = full_path.substr(dir_end, fname_end - dir_end);

	if (_pExtension)
		*_pExtension = full_path.substr(fname_end);

	return true;
}

std::string GetFilenameFromPath(std::string full_path) {
	size_t pos;
#ifdef _WIN32
	pos = full_path.rfind('\\');
	if (pos != std::string::npos) {
		return full_path.substr(pos + 1);
	}
#endif
	pos = full_path.rfind('/');
	if (pos != std::string::npos) {
		return full_path.substr(pos + 1);
	}
	// No directory components, just return the full path.
	return full_path;
}

std::string LineNumberString(const std::string &str) {
	std::stringstream input(str);
	std::stringstream output;
	std::string line;

	int lineNumber = 1;
	while (std::getline(input, line)) {
		output << std::setw(4) << lineNumber++ << ":  " << line << std::endl;
	}

	return output.str();
}

void SkipSpace(const char **ptr) {
	while (**ptr && isspace(**ptr)) {
		(*ptr)++;
	}
}

void DataToHexString(const uint8_t *data, size_t size, std::string *output) {
	Buffer buffer;
	for (size_t i = 0; i < size; i++) {
		if (i && !(i & 15))
			buffer.Printf("\n");
		buffer.Printf("%02x ", data[i]);
	}
	buffer.TakeAll(output);
}

void DataToHexString(const char* prefix, uint32_t startAddr, const uint8_t* data, size_t size, std::string* output) {
	Buffer buffer;
	size_t i = 0;
	for (; i < size; i++) {
		if (i && !(i & 15)) {
			buffer.Printf(" ");
			for (size_t j = i - 16; j < i; j++) {
				buffer.Printf("%c", ((data[j] < 0x20) || (data[j] > 0x7e)) ? 0x2e : data[j]);
			}
			buffer.Printf("\n");
		}
		if (!(i & 15))
			buffer.Printf("%s%08x  ", prefix, startAddr + i);
		buffer.Printf("%02x ", data[i]);
	}
	if (size & 15) {
		size_t padded_size = ((size - 1) | 15) + 1;
		for (size_t j = size; j < padded_size; j++) {
			buffer.Printf("   ");
		}
		buffer.Printf(" ");
		for (size_t j = size & ~UINT64_C(0xF); j < size; j++) {
			buffer.Printf("%c", ((data[j] < 0x20) || (data[j] > 0x7e)) ? 0x2e : data[j]);
		}
	}
	buffer.TakeAll(output);
}

std::string PStringFromFormat(const char* format, ...)
{
	va_list args;
	std::string temp = "";

	char *buf = nullptr;

	va_start(args, format);
	if (vasprintf(&buf, format, args) < 0)
		buf = nullptr;
	va_end(args);

	if (buf != nullptr) {
		temp = buf;
		free(buf);
	}

	return temp;
}

std::string StringFromInt(int value)
{
	char temp[16];
	sprintf(temp, "%i", value);
	return temp;
}

std::string StripSpaces(const std::string &str)
{
	const size_t s = str.find_first_not_of(" \t\r\n");

	if (str.npos != s)
		return str.substr(s, str.find_last_not_of(" \t\r\n") - s + 1);
	else
		return "";
}

std::string StripQuotes(const std::string& s)
{
	if (s.size() && '\"' == s[0] && '\"' == *s.rbegin())
		return s.substr(1, s.size() - 2);
	else
		return s;
}

void PSplitString(const std::string& str, const char delim, std::vector<std::string>& output)
{
	size_t next = 0;
	for (size_t pos = 0, len = str.length(); pos < len; ++pos) {
		if (str[pos] == delim) {
			output.push_back(str.substr(next, pos - next));
			// Skip the delimiter itself.
			next = pos + 1;
		}
	}

	if (next == 0) {
		output.push_back(str);
	} else if (next < str.length()) {
		output.push_back(str.substr(next));
	}
}

void GetQuotedStrings(const std::string& str, std::vector<std::string>& output)
{
	size_t next = 0;
	bool even = 0;
	for (size_t pos = 0, len = str.length(); pos < len; ++pos) {
		if (str[pos] == '\"' || str[pos] == '\'') {
			if (even) {
				//quoted text
				output.push_back(str.substr(next, pos - next));
				even = 0;
			} else {
				//non quoted text
				even = 1;
			}
			// Skip the delimiter itself.
			next = pos + 1;
		}
	}
}

std::string ReplaceAll(std::string result, const std::string& src, const std::string& dest)
{
	size_t pos = 0;

	if (src == dest)
		return result;

	while (1)
	{
		pos = result.find(src, pos);
		if (pos == result.npos)
			break;
		result.replace(pos, src.size(), dest);
		pos += dest.size();
	}
	return result;
}