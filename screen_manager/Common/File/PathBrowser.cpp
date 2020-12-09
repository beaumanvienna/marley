#include <algorithm>
#include <cstring>
#include <set>

#include "Common/File/PathBrowser.h"
#include "Common/StringUtils.h"
#include "Common/TimeUtil.h"
#include "Common/Log.h"
#include "Common/Thread/ThreadUtil.h"

SCREEN_PathBrowser::~SCREEN_PathBrowser() {
	std::unique_lock<std::mutex> guard(pendingLock_);
	pendingCancel_ = true;
	pendingStop_ = true;
	pendingCond_.notify_all();
	guard.unlock();

	if (pendingThread_.joinable()) {
		pendingThread_.join();
	}
}

// Normalize slashes.
void SCREEN_PathBrowser::SetPath(const std::string &path) {
	if (path[0] == '!') {
		path_ = path;
		HandlePath();
		return;
	}
	path_ = path;
	for (size_t i = 0; i < path_.size(); i++) {
		if (path_[i] == '\\') path_[i] = '/';
	}
	if (!path_.size() || (path_[path_.size() - 1] != '/'))
		path_ += "/";
	HandlePath();
}

void SCREEN_PathBrowser::HandlePath() {
	std::lock_guard<std::mutex> guard(pendingLock_);

	if (!path_.empty() && path_[0] == '!') {
		ready_ = true;
		pendingCancel_ = true;
		pendingPath_.clear();
		return;
	}
	if (!startsWith(path_, "http://") && !startsWith(path_, "https://")) {
		ready_ = true;
		pendingCancel_ = true;
		pendingPath_.clear();
		return;
	}

	ready_ = false;
	pendingCancel_ = false;
	pendingFiles_.clear();
	pendingPath_ = path_;
	pendingCond_.notify_all();

	if (pendingThread_.joinable())
		return;

	pendingThread_ = std::thread([&] {
		setCurrentThreadName("PathBrowser");

		std::unique_lock<std::mutex> guard(pendingLock_);
		std::vector<FileInfo> results;
		std::string lastPath;
		while (!pendingStop_) {
			while (lastPath == pendingPath_ && !pendingCancel_) {
				pendingCond_.wait(guard);
			}
			lastPath = pendingPath_;
			bool success = false;
			if (!lastPath.empty()) {
				guard.unlock();
				results.clear();
				success = false;
				guard.lock();
			}

			if (pendingPath_ == lastPath) {
				if (success && !pendingCancel_) {
					pendingFiles_ = results;
				}
				pendingPath_.clear();
				lastPath.clear();
				ready_ = true;
			}
		}
	});
}

bool SCREEN_PathBrowser::IsListingReady() {
	return ready_;
}

bool SCREEN_PathBrowser::GetListing(std::vector<FileInfo> &fileInfo, const char *filter, bool *cancel) {
	std::unique_lock<std::mutex> guard(pendingLock_);
	while (!IsListingReady() && (!cancel || !*cancel)) {
		// In case cancel changes, just sleep.
		guard.unlock();
		sleep_ms(100);
		guard.lock();
	}

    getFilesInDir(path_.c_str(), &fileInfo, filter);
    return true;

}

// TODO: Support paths like "../../hello"
void SCREEN_PathBrowser::Navigate(const std::string &path) {
	if (path == ".")
		return;
	if (path == "..") {
		// Upwards.
		// Check for windows drives.
		if (path_.size() == 3 && path_[1] == ':') {
			path_ = "/";
		} else {
			size_t slash = path_.rfind('/', path_.size() - 2);
			if (slash != std::string::npos)
				path_ = path_.substr(0, slash + 1);
		}
	} else {
		if (path.size() > 2 && path[1] == ':' && path_ == "/")
			path_ = path;
		else
			path_ = path_ + path;
		if (path_[path_.size() - 1] != '/')
			path_ += "/";
	}
	HandlePath();
}
