// PIniFile
// Taken from Dolphin but relicensed by me, Henrik Rydgard, under the MIT
// license as I wrote the whole thing originally and it has barely changed.

#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Common/Data/Format/IniFile.h"
#include "Common/File/VFS/VFS.h"
#include "Common/Data/Text/Parsers.h"

#include "Common/StringUtils.h"

static bool ParseLineKey(const std::string &line, size_t &pos, std::string *keyOut) {
	std::string key = "";

	while (pos < line.size()) {
		size_t next = line.find_first_of("=#", pos);
		if (next == line.npos || next == 0) {
			// Key never ended or empty, invalid.
			return false;
		} else if (line[next] == '#') {
			if (line[next - 1] != '\\') {
				// Value commented out before =, so not valid.
				return false;
			}

			// Escaped.
			key += line.substr(pos, next - pos - 1) + "#";
			pos = next + 1;
		} else if (line[next] == '=') {
			// Hurray, done.
			key += line.substr(pos, next - pos);
			pos = next + 1;
			break;
		}
	}

	if (keyOut) {
		*keyOut = StripSpaces(key);
	}
	return true;
}

static bool ParseLineValue(const std::string &line, size_t &pos, std::string *valueOut) {
	std::string value = "";

	while (pos < line.size()) {
		size_t next = line.find('#', pos);
		if (next == line.npos) {
			value += line.substr(pos);
			pos = line.npos;
			break;
		} else if (line[next - 1] != '\\') {
			// It wasn't escaped, so finish before the #.
			value += line.substr(pos, next - pos);
			// Include the comment's # in pos.
			pos = next;
			break;
		} else {
			// Escaped.
			value += line.substr(pos, next - pos - 1) + "#";
			pos = next + 1;
		}
	}

	if (valueOut) {
		*valueOut = StripQuotes(StripSpaces(value));
	}

	return true;
}

static bool ParseLineComment(const std::string& line, size_t &pos, std::string *commentOut) {
	// Don't bother with anything if we don't need the comment data.
	if (commentOut) {
		// Include any whitespace/formatting in the comment.
		size_t commentStartPos = pos;
		if (commentStartPos != line.npos) {
			while (commentStartPos > 0 && line[commentStartPos - 1] <= ' ') {
				--commentStartPos;
			}

			*commentOut = line.substr(commentStartPos);
		} else {
			// There was no comment.
			commentOut->clear();
		}
	}

	pos = line.npos;
	return true;
}

static bool ParseLine(const std::string& line, std::string* keyOut, std::string* valueOut, std::string* commentOut)
{
	// Rules:
	// 1. A line starting with ; is commented out.
	// 2. A # in a line (and all the space before it) is the comment.
	// 3. A \# in a line is not part of a comment and becomes # in the value.
	// 4. Whitespace around values is removed.
	// 5. Double quotes around values is removed.

	if (line.size() < 2 || line[0] == ';')
		return false;

	size_t pos = 0;
	if (!ParseLineKey(line, pos, keyOut))
		return false;
	if (!ParseLineValue(line, pos, valueOut))
		return false;
	if (!ParseLineComment(line, pos, commentOut))
		return false;

	return true;
}

static std::string EscapeComments(const std::string &value) {
	std::string result = "";

	for (size_t pos = 0; pos < value.size(); ) {
		size_t next = value.find('#', pos);
		if (next == value.npos) {
			result += value.substr(pos);
			pos = value.npos;
		} else {
			result += value.substr(pos, next - pos) + "\\#";
			pos = next + 1;
		}
	}

	return result;
}

void SCREEN_Section::Clear() {
	lines.clear();
}

std::string* SCREEN_Section::GetLine(const char* key, std::string* valueOut, std::string* commentOut)
{
	for (std::vector<std::string>::iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		std::string& line = *iter;
		std::string lineKey;
		ParseLine(line, &lineKey, valueOut, commentOut);
		if (!strcasecmp(lineKey.c_str(), key))
			return &line;
	}
	return 0;
}

void SCREEN_Section::Set(const char* key, uint32_t newValue) {
	Set(key, PStringFromFormat("0x%08x", newValue).c_str());
}

void SCREEN_Section::Set(const char* key, float newValue) {
	Set(key, PStringFromFormat("%f", newValue).c_str());
}

void SCREEN_Section::Set(const char* key, double newValue) {
	Set(key, PStringFromFormat("%f", newValue).c_str());
}

void SCREEN_Section::Set(const char* key, int newValue) {
	Set(key, StringFromInt(newValue).c_str());
}

void SCREEN_Section::Set(const char* key, const char* newValue)
{
	std::string value, commented;
	std::string* line = GetLine(key, &value, &commented);
	if (line)
	{
		// Change the value - keep the key and comment
		*line = StripSpaces(key) + " = " + EscapeComments(newValue) + commented;
	}
	else
	{
		// The key did not already exist in this section - let's add it.
		lines.push_back(std::string(key) + " = " + EscapeComments(newValue));
	}
}

void SCREEN_Section::Set(const char* key, const std::string& newValue, const std::string& defaultValue)
{
	if (newValue != defaultValue)
		Set(key, newValue);
	else
		Delete(key);
}

bool SCREEN_Section::Get(const char* key, std::string* value, const char* defaultValue)
{
	const std::string* line = GetLine(key, value, 0);
	if (!line)
	{
		if (defaultValue)
		{
			*value = defaultValue;
		}
		return false;
	}
	return true;
}

void SCREEN_Section::Set(const char* key, const float newValue, const float defaultValue)
{
	if (newValue != defaultValue)
		Set(key, newValue);
	else
		Delete(key);
}

void SCREEN_Section::Set(const char* key, int newValue, int defaultValue)
{
	if (newValue != defaultValue)
		Set(key, newValue);
	else
		Delete(key);
}

void SCREEN_Section::Set(const char* key, bool newValue, bool defaultValue)
{
	if (newValue != defaultValue)
		Set(key, newValue);
	else
		Delete(key);
}

void SCREEN_Section::Set(const char* key, const std::vector<std::string>& newValues) 
{
	std::string temp;
	// Join the strings with , 
	std::vector<std::string>::const_iterator it;
	for (it = newValues.begin(); it != newValues.end(); ++it)
	{
		temp += (*it) + ",";
	}
	// remove last ,
	if (temp.length())
		temp.resize(temp.length() - 1);
	Set(key, temp.c_str());
}

void SCREEN_Section::AddComment(const std::string &comment) {
	lines.push_back("# " + comment);
}

bool SCREEN_Section::Get(const char* key, std::vector<std::string>& values) 
{
	std::string temp;
	bool retval = Get(key, &temp, 0);
	if (!retval || temp.empty())
	{
		return false;
	}
	// ignore starting , if any
	size_t subStart = temp.find_first_not_of(",");
	size_t subEnd;

	// split by , 
	while (subStart != std::string::npos) {
		
		// Find next , 
		subEnd = temp.find_first_of(",", subStart);
		if (subStart != subEnd) 
			// take from first char until next , 
			values.push_back(StripSpaces(temp.substr(subStart, subEnd - subStart)));
	
		// Find the next non , char
		subStart = temp.find_first_not_of(",", subEnd);
	} 
	
	return true;
}

bool SCREEN_Section::Get(const char* key, int* value, int defaultValue)
{
	std::string temp;
	bool retval = Get(key, &temp, 0);
	if (retval && PTryParse(temp.c_str(), value))
		return true;
	*value = defaultValue;
	return false;
}

bool SCREEN_Section::Get(const char* key, uint32_t* value, uint32_t defaultValue)
{
	std::string temp;
	bool retval = Get(key, &temp, 0);
	if (retval && PTryParse(temp, value))
		return true;
	*value = defaultValue;
	return false;
}

bool SCREEN_Section::Get(const char* key, bool* value, bool defaultValue)
{
	std::string temp;
	bool retval = Get(key, &temp, 0);
	if (retval && PTryParse(temp.c_str(), value))
		return true;
	*value = defaultValue;
	return false;
}

bool SCREEN_Section::Get(const char* key, float* value, float defaultValue)
{
	std::string temp;
	bool retval = Get(key, &temp, 0);
	if (retval && PTryParse(temp.c_str(), value))
		return true;
	*value = defaultValue;
	return false;
}

bool SCREEN_Section::Get(const char* key, double* value, double defaultValue)
{
	std::string temp;
	bool retval = Get(key, &temp, 0);
	if (retval && PTryParse(temp.c_str(), value))
		return true;
	*value = defaultValue;
	return false;
}

bool SCREEN_Section::Exists(const char *key) const
{
	for (std::vector<std::string>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		std::string lineKey;
		ParseLine(*iter, &lineKey, NULL, NULL);
		if (!strcasecmp(lineKey.c_str(), key))
			return true;
	}
	return false;
}

std::map<std::string, std::string> SCREEN_Section::ToMap() const
{
	std::map<std::string, std::string> outMap;
	for (std::vector<std::string>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		std::string lineKey, lineValue;
		if (ParseLine(*iter, &lineKey, &lineValue, NULL)) {
			outMap[lineKey] = lineValue;
		}
	}
	return outMap;
}


bool SCREEN_Section::Delete(const char *key)
{
	std::string* line = GetLine(key, 0, 0);
	for (std::vector<std::string>::iterator liter = lines.begin(); liter != lines.end(); ++liter)
	{
		if (line == &*liter)
		{
			lines.erase(liter);
			return true;
		}
	}
	return false;
}

// PIniFile

const SCREEN_Section* SCREEN_IniFile::GetSection(const char* sectionName) const
{
	for (std::vector<SCREEN_Section>::const_iterator iter = sections.begin(); iter != sections.end(); ++iter)
		if (!strcasecmp(iter->name().c_str(), sectionName))
			return (&(*iter));
	return 0;
}

SCREEN_Section* SCREEN_IniFile::GetSection(const char* sectionName)
{
	for (std::vector<SCREEN_Section>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
		if (!strcasecmp(iter->name().c_str(), sectionName))
			return (&(*iter));
	return 0;
}

SCREEN_Section* SCREEN_IniFile::GetOrCreateSection(const char* sectionName)
{
	SCREEN_Section* section = GetSection(sectionName);
	if (!section)
	{
		sections.push_back(SCREEN_Section(sectionName));
		section = &sections[sections.size() - 1];
	}
	return section;
}

bool SCREEN_IniFile::DeleteSection(const char* sectionName)
{
	SCREEN_Section* s = GetSection(sectionName);
	if (!s)
		return false;
	for (std::vector<SCREEN_Section>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
	{
		if (&(*iter) == s)
		{
			sections.erase(iter);
			return true;
		}
	}
	return false;
}

bool SCREEN_IniFile::Exists(const char* sectionName, const char* key) const
{
	const SCREEN_Section* section = GetSection(sectionName);
	if (!section)
		return false;
	return section->Exists(key);
}

void SCREEN_IniFile::SetLines(const char* sectionName, const std::vector<std::string> &lines)
{
	SCREEN_Section* section = GetOrCreateSection(sectionName);
	section->lines.clear();
	for (std::vector<std::string>::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		section->lines.push_back(*iter);
	}
}

bool SCREEN_IniFile::DeleteKey(const char* sectionName, const char* key)
{
	SCREEN_Section* section = GetSection(sectionName);
	if (!section)
		return false;
	std::string* line = section->GetLine(key, 0, 0);
	for (std::vector<std::string>::iterator liter = section->lines.begin(); liter != section->lines.end(); ++liter)
	{
		if (line == &(*liter))
		{
			section->lines.erase(liter);
			return true;
		}
	}
	return false; //shouldn't happen
}

// Return a list of all keys in a section
bool SCREEN_IniFile::GetKeys(const char* sectionName, std::vector<std::string>& keys) const
{
	const SCREEN_Section* section = GetSection(sectionName);
	if (!section)
		return false;
	keys.clear();
	for (std::vector<std::string>::const_iterator liter = section->lines.begin(); liter != section->lines.end(); ++liter)
	{
		std::string key;
		ParseLine(*liter, &key, 0, 0);
		if (!key.empty())
			keys.push_back(key);
	}
	return true;
}

// Return a list of all lines in a section
bool SCREEN_IniFile::GetLines(const char* sectionName, std::vector<std::string>& lines, const bool remove_comments) const
{
	const SCREEN_Section* section = GetSection(sectionName);
	if (!section)
		return false;

	lines.clear();
	for (std::vector<std::string>::const_iterator iter = section->lines.begin(); iter != section->lines.end(); ++iter)
	{
		std::string line = StripSpaces(*iter);

		if (remove_comments)
		{
			int commentPos = (int)line.find('#');
			if (commentPos == 0)
			{
				continue;
			}

			if (commentPos != (int)std::string::npos)
			{
				line = StripSpaces(line.substr(0, commentPos));
			}
		}

		lines.push_back(line);
	}

	return true;
}


void SCREEN_IniFile::SortSections()
{
	std::sort(sections.begin(), sections.end());
}

bool SCREEN_IniFile::Load(const char* filename)
{
	sections.clear();
	sections.push_back(SCREEN_Section(""));
	// first section consists of the comments before the first real section

	// Open file
	std::ifstream in;
#if defined(_WIN32) && !defined(__MINGW32__)
	in.open(ConvertUTF8ToWString(filename), std::ios::in);
#else
	in.open(filename, std::ios::in);
#endif
	if (in.fail()) return false;

	bool success = Load(in);
	in.close();
	return success;
}

bool SCREEN_IniFile::LoadFromVFS(const std::string &filename) {
	size_t size;
	uint8_t *data = VFSReadFile(filename.c_str(), &size);
	if (!data)
		return false;
	std::string str((const char*)data, size);
	delete [] data;

	std::stringstream sstream(str);
	return Load(sstream);
}

bool SCREEN_IniFile::Load(std::istream &in) {
	// Maximum number of letters in a line
	static const int MAX_BYTES = 1024*32;

	while (!(in.eof() || in.fail()))
	{
		char templine[MAX_BYTES];
		in.getline(templine, MAX_BYTES);
		std::string line = templine;

		// Remove UTF-8 byte order marks.
		if (line.substr(0, 3) == "\xEF\xBB\xBF")
			line = line.substr(3);
		 
		// Check for CRLF eol and convert it to LF
		if (!line.empty() && line.at(line.size()-1) == '\r')
		{
			line.erase(line.size()-1);
		}

		if (!line.empty()) {
			size_t sectionNameEnd = std::string::npos;
			if (line[0] == '[') {
				sectionNameEnd = line.find(']');
			}

			if (sectionNameEnd != std::string::npos) {
				// New section!
				std::string sub = line.substr(1, sectionNameEnd - 1);
				sections.push_back(SCREEN_Section(sub));

				if (sectionNameEnd + 1 < line.size()) {
					sections[sections.size() - 1].comment = line.substr(sectionNameEnd + 1);
				}
			} else {
				if (sections.empty()) {
					sections.push_back(SCREEN_Section(""));
				}
				sections[sections.size() - 1].lines.push_back(line);
			}
		}
	}

	return true;
}

bool SCREEN_IniFile::Save(const char* filename)
{
	std::ofstream out;

	out.open(filename, std::ios::out);

	if (out.fail())
	{
		return false;
	}

	// UTF-8 byte order mark. To make sure notepad doesn't go nuts.
	out << "\xEF\xBB\xBF";

	for (const SCREEN_Section &section : sections) {
		if (!section.name().empty() && (!section.lines.empty() || !section.comment.empty())) {
			out << "[" << section.name() << "]" << section.comment << std::endl;
		}

		for (const std::string &s : section.lines) {
			out << s << std::endl;
		}
	}

	out.close();
	return true;
}

bool SCREEN_IniFile::Get(const char* sectionName, const char* key, std::string* value, const char* defaultValue)
{
	SCREEN_Section* section = GetSection(sectionName);
	if (!section) {
		if (defaultValue) {
			*value = defaultValue;
		}
		return false;
	}
	return section->Get(key, value, defaultValue);
}

bool SCREEN_IniFile::Get(const char *sectionName, const char* key, std::vector<std::string>& values) 
{
	SCREEN_Section *section = GetSection(sectionName);
	if (!section)
		return false;
	return section->Get(key, values);
}

bool SCREEN_IniFile::Get(const char* sectionName, const char* key, int* value, int defaultValue)
{
	SCREEN_Section *section = GetSection(sectionName);
	if (!section) {
		*value = defaultValue;
		return false;
	} else {
		return section->Get(key, value, defaultValue);
	}
}

bool SCREEN_IniFile::Get(const char* sectionName, const char* key, uint32_t* value, uint32_t defaultValue)
{
	SCREEN_Section *section = GetSection(sectionName);
	if (!section) {
		*value = defaultValue;
		return false;
	} else {
		return section->Get(key, value, defaultValue);
	}
}

bool SCREEN_IniFile::Get(const char* sectionName, const char* key, bool* value, bool defaultValue)
{
	SCREEN_Section *section = GetSection(sectionName);
	if (!section) {
		*value = defaultValue;
		return false;
	} else {
		return section->Get(key, value, defaultValue);
	}
}
