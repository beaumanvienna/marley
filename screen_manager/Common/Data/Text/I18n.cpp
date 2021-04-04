#include "Common/Data/Text/I18n.h"
#include "Common/Data/Format/IniFile.h"
#include "Common/File/VFS/VFS.h"

#include "Common/StringUtils.h"

SCREEN_I18NRepo SCREEN_i18nrepo;

SCREEN_I18NRepo::~SCREEN_I18NRepo() {
	Clear();
}

std::string SCREEN_I18NRepo::LanguageID() {
	return languageID_;
}

void SCREEN_I18NRepo::Clear() {
	std::lock_guard<std::mutex> guard(catsLock_);
	for (auto iter = cats_.begin(); iter != cats_.end(); ++iter) {
		iter->second.reset();
	}
	cats_.clear();
}

const char *SCREEN_I18NCategory::T(const char *key, const char *def) {
	if (!key) {
		return "ERROR";
	}
	// Replace the \n's with \\n's so that key values with newlines will be found correctly.
	std::string modifiedKey = key;
	modifiedKey = SCREEN_ReplaceAll(modifiedKey, "\n", "\\n");

	auto iter = map_.find(modifiedKey);
	if (iter != map_.end()) {
//		INFO_LOG(SYSTEM, "translation key found in %s: %s", name_.c_str(), key);
		return iter->second.text.c_str();
	} else {
		std::lock_guard<std::mutex> guard(missedKeyLock_);
		if (def)
			missedKeyLog_[key] = def;
		else
			missedKeyLog_[key] = modifiedKey.c_str();
//		INFO_LOG(SYSTEM, "Missed translation key in %s: %s", name_.c_str(), key);
		return def ? def : key;
	}
}

void SCREEN_I18NCategory::SetMap(const std::map<std::string, std::string> &m) {
	for (auto iter = m.begin(); iter != m.end(); ++iter) {
		if (map_.find(iter->first) == map_.end()) {
			std::string text = SCREEN_ReplaceAll(iter->second, "\\n", "\n");
			map_[iter->first] = I18NEntry(text);
//			INFO_LOG(SYSTEM, "Language entry: %s -> %s", iter->first.c_str(), text.c_str());
		}
	}
}

std::shared_ptr<SCREEN_I18NCategory> SCREEN_I18NRepo::GetCategory(const char *category) {
	std::lock_guard<std::mutex> guard(catsLock_);
	auto iter = cats_.find(category);
	if (iter != cats_.end()) {
		return iter->second;
	} else {
		SCREEN_I18NCategory *c = new SCREEN_I18NCategory(this, category);
		cats_[category].reset(c);
		return cats_[category];
	}
}

std::string SCREEN_I18NRepo::GetIniPath(const std::string &languageID) const {
	return "lang/" + languageID + ".ini";
}

bool SCREEN_I18NRepo::IniExists(const std::string &languageID) const {
	FileInfo info;
	if (!SCREEN_VFSGetFileInfo(GetIniPath(languageID).c_str(), &info))
		return false;
	if (!info.exists)
		return false;
	return true;
}

bool SCREEN_I18NRepo::LoadIni(const std::string &languageID, const std::string &overridePath) {
	SCREEN_IniFile ini;
	std::string iniPath;

//	INFO_LOG(SYSTEM, "Loading lang ini %s", iniPath.c_str());
	if (!overridePath.empty()) {
		iniPath = overridePath + languageID + ".ini";
	} else {
		iniPath = GetIniPath(languageID);
	}

	if (!ini.LoadFromVFS(iniPath))
		return false;

	Clear();

	const std::vector<SCREEN_Section> &sections = ini.Sections();

	std::lock_guard<std::mutex> guard(catsLock_);
	for (auto iter = sections.begin(); iter != sections.end(); ++iter) {
		if (iter->name() != "") {
			cats_[iter->name()].reset(LoadSection(&(*iter), iter->name().c_str()));
		}
	}

	languageID_ = languageID;
	return true;
}

SCREEN_I18NCategory *SCREEN_I18NRepo::LoadSection(const SCREEN_Section *section, const char *name) {
	SCREEN_I18NCategory *cat = new SCREEN_I18NCategory(this, name);
	std::map<std::string, std::string> sectionMap = section->ToMap();
	cat->SetMap(sectionMap);
	return cat;
}

// This is a very light touched save variant - it won't overwrite 
// anything, only create new entries.
void SCREEN_I18NRepo::SaveIni(const std::string &languageID) {
	SCREEN_IniFile ini;
	ini.Load(GetIniPath(languageID));
	std::lock_guard<std::mutex> guard(catsLock_);
	for (auto iter = cats_.begin(); iter != cats_.end(); ++iter) {
		std::string categoryName = iter->first;
		SCREEN_Section *section = ini.GetOrCreateSection(categoryName.c_str());
		SaveSection(ini, section, iter->second);
	}
	ini.Save(GetIniPath(languageID));
}

void SCREEN_I18NRepo::SaveSection(SCREEN_IniFile &ini, SCREEN_Section *section, std::shared_ptr<SCREEN_I18NCategory> cat) {
	const std::map<std::string, std::string> &missed = cat->Missed();

	for (auto iter = missed.begin(); iter != missed.end(); ++iter) {
		if (!section->Exists(iter->first.c_str())) {
			std::string text = SCREEN_ReplaceAll(iter->second, "\n", "\\n");
			section->Set(iter->first, text);
		}
	}

	const std::map<std::string, I18NEntry> &entries = cat->GetMap();
	for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
		std::string text = SCREEN_ReplaceAll(iter->second.text, "\n", "\\n");
		section->Set(iter->first, text);
	}

	cat->ClearMissed();
}
