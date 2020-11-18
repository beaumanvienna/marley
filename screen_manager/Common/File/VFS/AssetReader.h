// TODO: Move much of this code to vfs.cpp
#pragma once

#ifdef __ANDROID__
#include <zip.h>
#endif

#include <string.h>
#include <string>

#include "Common/File/VFS/VFS.h"

// Direct readers. deallocate using delete [].
uint8_t *SCREEN_ReadLocalFile(const char *filename, size_t *size);

class SCREEN_AssetReader {
public:
	virtual ~SCREEN_AssetReader() {}
	// use delete[]
	virtual uint8_t *ReadAsset(const char *path, size_t *size) = 0;
	// Filter support is optional but nice to have
	virtual bool GetFileListing(const char *path, std::vector<FileInfo> *listing, const char *filter = 0) = 0;
	virtual bool GetFileInfo(const char *path, FileInfo *info) = 0;
	virtual std::string toString() const = 0;
};

#ifdef __ANDROID__
uint8_t *ReadFromZip(zip *archive, const char* filename, size_t *size);
class ZipSCREEN_AssetReader : public SCREEN_AssetReader {
public:
	ZipSCREEN_AssetReader(const char *zip_file, const char *in_zip_path);
	~ZipSCREEN_AssetReader();
	// use delete[]
	virtual uint8_t *ReadAsset(const char *path, size_t *size);
	virtual bool GetFileListing(const char *path, std::vector<FileInfo> *listing, const char *filter);
	virtual bool GetFileInfo(const char *path, FileInfo *info);
	virtual std::string toString() const {
		return in_zip_path_;
	}

private:
	zip *zip_file_;
	char in_zip_path_[256];
};
#endif

class DirectorySCREEN_AssetReader : public SCREEN_AssetReader {
public:
	explicit DirectorySCREEN_AssetReader(const char *path);
	// use delete[]
	virtual uint8_t *ReadAsset(const char *path, size_t *size);
	virtual bool GetFileListing(const char *path, std::vector<FileInfo> *listing, const char *filter);
	virtual bool GetFileInfo(const char *path, FileInfo *info);
	virtual std::string toString() const {
		return path_;
	}

private:
	char path_[512]{};
};

