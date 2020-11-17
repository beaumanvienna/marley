#include <algorithm>
#include <ctype.h>
#include <set>
#include <stdio.h>

#include "Common/Common.h"
#include "Common/Log.h"
#include "Common/File/VFS/AssetReader.h"


DirectorySCREEN_AssetReader::DirectorySCREEN_AssetReader(const char *path) {
	strncpy(path_, path, ARRAY_SIZE(path_));
	path_[ARRAY_SIZE(path_) - 1] = '\0';
}

uint8_t *DirectorySCREEN_AssetReader::ReadAsset(const char *path, size_t *size) {
	char new_path[2048];
	new_path[0] = '\0';
	// Check if it already contains the path
	if (strlen(path) > strlen(path_) && 0 == memcmp(path, path_, strlen(path_))) {
	}
	else {
		strcpy(new_path, path_);
	}
	strcat(new_path, path);
	return ReadLocalFile(new_path, size);
}

bool DirectorySCREEN_AssetReader::GetFileListing(const char *path, std::vector<FileInfo> *listing, const char *filter = 0)
{
	char new_path[2048];
	new_path[0] = '\0';
	// Check if it already contains the path
	if (strlen(path) > strlen(path_) && 0 == memcmp(path, path_, strlen(path_))) {
	}
	else {
		strcpy(new_path, path_);
	}
	strcat(new_path, path);
	FileInfo info;
	if (!getFileInfo(new_path, &info))
		return false;

	if (info.isDirectory)
	{
		getFilesInDir(new_path, listing, filter);
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectorySCREEN_AssetReader::GetFileInfo(const char *path, FileInfo *info) 
{
	char new_path[2048];
	new_path[0] = '\0';
	// Check if it already contains the path
	if (strlen(path) > strlen(path_) && 0 == memcmp(path, path_, strlen(path_))) {
	}
	else {
		strcpy(new_path, path_);
	}
	strcat(new_path, path);
	return getFileInfo(new_path, info);	
}
