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

#include "ppsspp_config.h"

#include <fstream>
#include <mutex>
#include <vector>

#include "Common/Data/Format/IniFile.h"
#include "Common/CommonFuncs.h"
#include "Common/Log.h"

#define	MAX_MESSAGES 8000   

extern const char *hleCurrentThreadName;

// Struct that listeners can output how they want. For example, on Android we don't want to add
// timestamp or write the level as a string, those already exist.
struct LogMessage {
	char timestamp[16];
	char header[64];  // Filename/thread/etc. in front.
	SCREEN_LogTypes::LOG_LEVELS level;
	const char *log;
	std::string msg;  // The actual log message.
};

// pure virtual interface
class SCREEN_LogListener {
public:
	virtual ~SCREEN_LogListener() {}

	virtual void Log(const LogMessage &msg) = 0;
};

class SCREEN_FileLogListener : public SCREEN_LogListener {
public:
	SCREEN_FileLogListener(const char *filename);

	void Log(const LogMessage &msg);

	bool IsValid() { if (!m_logfile) return false; else return true; }
	bool IsEnabled() const { return m_enable; }
	void SetEnabled(bool enable) { m_enable = enable; }

	const char* GetName() const { return "file"; }

private:
	std::mutex m_log_lock;
	std::ofstream m_logfile;
	bool m_enable;
};

class SCREEN_OutputDebugStringLogListener : public SCREEN_LogListener {
public:
	void Log(const LogMessage &msg);
};

class SCREEN_RingbufferLogListener : public SCREEN_LogListener {
public:
	SCREEN_RingbufferLogListener() : curMessage_(0), count_(0), enabled_(false) {}
	void Log(const LogMessage &msg);

	bool IsEnabled() const { return enabled_; }
	void SetEnabled(bool enable) { enabled_ = enable; }

	int GetCount() const { return count_ < MAX_LOGS ? count_ : MAX_LOGS; }
	const char *TextAt(int i) const { return messages_[(curMessage_ - i - 1) & (MAX_LOGS - 1)].msg.c_str(); }
	SCREEN_LogTypes::LOG_LEVELS LevelAt(int i) const { return messages_[(curMessage_ - i - 1) & (MAX_LOGS - 1)].level; }

private:
	enum { MAX_LOGS = 128 };
	LogMessage messages_[MAX_LOGS];
	int curMessage_;
	int count_;
	bool enabled_;
};

// TODO: A simple buffered log that can be used to display the log in-window
// on Android etc.
// class BufferedLogListener { ... }

struct LogChannel {
	char m_shortName[32]{};
	SCREEN_LogTypes::LOG_LEVELS level;
	bool enabled;
};

class SCREEN_ConsoleListener;

class SCREEN_LogManager {
private:
	SCREEN_LogManager(bool *enabledSetting);
	~SCREEN_LogManager();

	// Prevent copies.
	SCREEN_LogManager(const SCREEN_LogManager &) = delete;
	void operator=(const SCREEN_LogManager &) = delete;

	LogChannel log_[SCREEN_LogTypes::NUMBER_OF_LOGS];
	SCREEN_FileLogListener *fileLog_ = nullptr;
	SCREEN_ConsoleListener *consoleLog_ = nullptr;
	SCREEN_OutputDebugStringLogListener *debuggerLog_ = nullptr;
	SCREEN_RingbufferLogListener *ringLog_ = nullptr;
	static SCREEN_LogManager *logManager_;  // Singleton. Ugh.

	std::mutex log_lock_;
	std::mutex listeners_lock_;
	std::vector<SCREEN_LogListener*> listeners_;

public:
	void AddListener(SCREEN_LogListener *listener);
	void RemoveListener(SCREEN_LogListener *listener);

	static u32 GetMaxLevel() { return MAX_LOGLEVEL;	}
	static int GetNumChannels() { return SCREEN_LogTypes::NUMBER_OF_LOGS; }

	void Log(SCREEN_LogTypes::LOG_LEVELS level, SCREEN_LogTypes::LOG_TYPE type, 
			 const char *file, int line, const char *fmt, va_list args);
	bool IsEnabled(SCREEN_LogTypes::LOG_LEVELS level, SCREEN_LogTypes::LOG_TYPE type);

	LogChannel *GetLogChannel(SCREEN_LogTypes::LOG_TYPE type) {
		return &log_[type];
	}

	void SetLogLevel(SCREEN_LogTypes::LOG_TYPE type, SCREEN_LogTypes::LOG_LEVELS level) {
		log_[type].level = level;
	}

	void SetAllLogLevels(SCREEN_LogTypes::LOG_LEVELS level) {
		for (int i = 0; i < SCREEN_LogTypes::NUMBER_OF_LOGS; ++i) {
			log_[i].level = level;
		}
	}

	void SetEnabled(SCREEN_LogTypes::LOG_TYPE type, bool enable) {
		log_[type].enabled = enable;
	}

	SCREEN_LogTypes::LOG_LEVELS GetLogLevel(SCREEN_LogTypes::LOG_TYPE type) {
		return log_[type].level;
	}

	SCREEN_ConsoleListener *GetPConsoleListener() const {
		return consoleLog_;
	}

	SCREEN_OutputDebugStringLogListener *GetDebuggerListener() const {
		return debuggerLog_;
	}

	SCREEN_RingbufferLogListener *GetRingbufferListener() const {
		return ringLog_;
	}

	static inline SCREEN_LogManager* GetInstance() {
		return logManager_;
	}

	static void SetInstance(SCREEN_LogManager *logManager) {
		logManager_ = logManager;
	}

	static void Init(bool *enabledSetting);
	static void Shutdown();

	void ChangeFileLog(const char *filename);

	void SaveConfig(SCREEN_Section *section);
	void LoadConfig(SCREEN_Section *section, bool debugDefaults);
};
