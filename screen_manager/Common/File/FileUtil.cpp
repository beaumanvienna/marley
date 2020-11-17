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

#include "ppsspp_config.h"

#include <cstring>
#include <memory>

#include "Common/Log.h"
#include "Common/File/FileUtil.h"
#include "Common/StringUtils.h"
#include "Common/SysError.h"

#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

#include "Common/Data/Encoding/Utf8.h"

#include <sys/stat.h>

#ifndef S_ISDIR
#define S_ISDIR(m)  (((m)&S_IFMT) == S_IFDIR)
#endif

#if !defined(__linux__) && !defined(_WIN32) && !defined(__QNX__)
#define stat64 stat
#define fstat64 fstat
#endif

#define DIR_SEP "/"
#ifdef _WIN32
#define DIR_SEP_CHRS "/\\"
#else
#define DIR_SEP_CHRS "/"
#endif

// This namespace has various generic functions related to files and paths.
// The code still needs a ton of cleanup.
// REMEMBER: strdup considered harmful!
namespace SCREEN_PFile
{

FILE *OpenCFile(const std::string &filename, const char *mode)
{
	return fopen(filename.c_str(), mode);
}

bool OpenCPPFile(std::fstream & stream, const std::string &filename, std::ios::openmode mode)
{
	stream.open(filename.c_str(), mode);

	return stream.is_open();
}


std::string ResolvePath(const std::string &path) {
	if (startsWith(path, "http://") || startsWith(path, "https://")) {
		return path;
	}
	std::unique_ptr<char[]> buf(new char[PATH_MAX + 32768]);
	if (realpath(path.c_str(), buf.get()) == nullptr)
		return path;
	return buf.get();
}

// Remove any ending forward slashes from directory paths
// Modifies argument.
static void StripTailDirSlashes(std::string &fname) {
	if (fname.length() > 1) {
		size_t i = fname.length() - 1;

		while (strchr(DIR_SEP_CHRS, fname[i]))
			fname[i--] = '\0';
	}
	return;
}

// Returns true if file filename exists. Will return true on directories.
bool Exists(const std::string &filename) {
	std::string fn = filename;
	StripTailDirSlashes(fn);

	struct stat file_info;
	return stat(fn.c_str(), &file_info) == 0;

}

// Returns true if filename is a directory
bool IsDirectory(const std::string &filename)
{
	std::string fn = filename;
	StripTailDirSlashes(fn);

#if defined(_WIN32)
	std::wstring copy = ConvertUTF8ToWString(fn);
	WIN32_FILE_ATTRIBUTE_DATA data{};
	if (!GetFileAttributesEx(copy.c_str(), GetFileExInfoStandard, &data) || data.dwFileAttributes == INVALID_FILE_ATTRIBUTES) {
		printf("GetFileAttributes failed on %s: %08x", fn.c_str(), GetLastError());
		return false;
	}
	DWORD result = data.dwFileAttributes;
	return (result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
#else
	std::string copy(fn);
	struct stat file_info;
	int result = stat(copy.c_str(), &file_info);
	if (result < 0) {
		printf("IsDirectory: stat failed on %s: %s", fn.c_str(), GetLastErrorMsg().c_str());
		return false;
	}
	return S_ISDIR(file_info.st_mode);
#endif
}

// Deletes a given filename, return true on success
// Doesn't supports deleting a directory
bool Delete(const std::string &filename) {
	printf("Delete: file %s", filename.c_str());

	// Return true because we care about the file no 
	// being there, not the actual delete.
	if (!Exists(filename)) {
		printf("Delete: %s does not exists", filename.c_str());
		return true;
	}

	// We can't delete a directory
	if (IsDirectory(filename)) {
		printf("Delete failed: %s is a directory", filename.c_str());
		return false;
	}

#ifdef _WIN32
	if (!DeleteFile(ConvertUTF8ToWString(filename).c_str())) {
		printf("Delete: DeleteFile failed on %s: %s", filename.c_str(), GetLastErrorMsg().c_str());
		return false;
	}
#else
	if (unlink(filename.c_str()) == -1) {
		printf("Delete: unlink failed on %s: %s", 
				 filename.c_str(), GetLastErrorMsg().c_str());
		return false;
	}
#endif

	return true;
}

// Returns true if successful, or path already exists.
bool CreateDir(const std::string &path)
{
	std::string fn = path;
	StripTailDirSlashes(fn);
	printf("CreateDir('%s')", fn.c_str());
#ifdef _WIN32
	if (::CreateDirectory(ConvertUTF8ToWString(fn).c_str(), NULL))
		return true;
	DWORD error = GetLastError();
	if (error == ERROR_ALREADY_EXISTS)
	{
		printf("CreateDir: CreateDirectory failed on %s: already exists", path.c_str());
		return true;
	}
	printf("CreateDir: CreateDirectory failed on %s: %i", path.c_str(), error);
	return false;
#else
	if (mkdir(fn.c_str(), 0755) == 0)
		return true;

	int err = errno;

	if (err == EEXIST)
	{
		printf("CreateDir: mkdir failed on %s: already exists", fn.c_str());
		return true;
	}

	printf("CreateDir: mkdir failed on %s: %s", fn.c_str(), strerror(err));
	return false;
#endif
}

// Creates the full path of fullPath returns true on success
bool CreateFullPath(const std::string &path)
{
	std::string fullPath = path;
	StripTailDirSlashes(fullPath);
	int panicCounter = 100;
	VERBOSE_LOG(COMMON, "CreateFullPath: path %s", fullPath.c_str());
		
	if (SCREEN_PFile::Exists(fullPath)) {
		printf("CreateFullPath: path exists %s", fullPath.c_str());
		return true;
	}

	size_t position = 0;

#ifdef _WIN32
	// Skip the drive letter, no need to create C:\.
	position = 3;
#endif

	while (true)
	{
		// Find next sub path
		position = fullPath.find_first_of(DIR_SEP_CHRS, position);

		// we're done, yay!
		if (position == fullPath.npos)
		{
			if (!SCREEN_PFile::Exists(fullPath))
				return SCREEN_PFile::CreateDir(fullPath);
			return true;
		}
		std::string subPath = fullPath.substr(0, position);
		if (position != 0 && !SCREEN_PFile::Exists(subPath))
			SCREEN_PFile::CreateDir(subPath);

		// A safety check
		panicCounter--;
		if (panicCounter <= 0)
		{
			printf("CreateFullPath: directory structure too deep");
			return false;
		}
		position++;
	}
}


// Deletes a directory filename, returns true on success
bool DeleteDir(const std::string &filename)
{
	printf("DeleteDir: directory %s", filename.c_str());

	// check if a directory
	if (!SCREEN_PFile::IsDirectory(filename))
	{
		printf("DeleteDir: Not a directory %s", filename.c_str());
		return false;
	}

#ifdef _WIN32
	if (::RemoveDirectory(ConvertUTF8ToWString(filename).c_str()))
		return true;
#else
	if (rmdir(filename.c_str()) == 0)
		return true;
#endif
	printf("DeleteDir: %s: %s", filename.c_str(), GetLastErrorMsg().c_str());

	return false;
}

// renames file srcFilename to destFilename, returns true on success 
bool Rename(const std::string &srcFilename, const std::string &destFilename)
{
	printf("Rename: %s --> %s", 
			srcFilename.c_str(), destFilename.c_str());
#if defined(_WIN32) && defined(UNICODE)
	std::wstring srcw = ConvertUTF8ToWString(srcFilename);
	std::wstring destw = ConvertUTF8ToWString(destFilename);
	if (_wrename(srcw.c_str(), destw.c_str()) == 0)
		return true;
#else
	if (rename(srcFilename.c_str(), destFilename.c_str()) == 0)
		return true;
#endif
	printf("Rename: failed %s --> %s: %s", 
			  srcFilename.c_str(), destFilename.c_str(), GetLastErrorMsg().c_str());
	return false;
}

// copies file srcFilename to destFilename, returns true on success 
bool Copy(const std::string &srcFilename, const std::string &destFilename)
{
	printf("Copy: %s --> %s", 
			srcFilename.c_str(), destFilename.c_str());
#ifdef _WIN32
#if PPSSPP_PLATFORM(UWP)
	if (CopyFile2(ConvertUTF8ToWString(srcFilename).c_str(), ConvertUTF8ToWString(destFilename).c_str(), nullptr))
		return true;
	return false;
#else
	if (CopyFile(ConvertUTF8ToWString(srcFilename).c_str(), ConvertUTF8ToWString(destFilename).c_str(), FALSE))
		return true;
#endif
	printf("Copy: failed %s --> %s: %s", 
			srcFilename.c_str(), destFilename.c_str(), GetLastErrorMsg().c_str());
	return false;
#else

	// buffer size
#define BSIZE 1024

	char buffer[BSIZE];

	// Open input file
	FILE *input = fopen(srcFilename.c_str(), "rb");
	if (!input)
	{
		printf("Copy: input failed %s --> %s: %s", 
				srcFilename.c_str(), destFilename.c_str(), GetLastErrorMsg().c_str());
		return false;
	}

	// open output file
	FILE *output = fopen(destFilename.c_str(), "wb");
	if (!output)
	{
		fclose(input);
		printf("Copy: output failed %s --> %s: %s", 
				srcFilename.c_str(), destFilename.c_str(), GetLastErrorMsg().c_str());
		return false;
	}

	// copy loop
	while (!feof(input))
	{
		// read input
		int rnum = fread(buffer, sizeof(char), BSIZE, input);
		if (rnum != BSIZE)
		{
			if (ferror(input) != 0)
			{
				printf(
						"Copy: failed reading from source, %s --> %s: %s", 
						srcFilename.c_str(), destFilename.c_str(), GetLastErrorMsg().c_str());
				fclose(input);
				fclose(output);		
				return false;
			}
		}

		// write output
		int wnum = fwrite(buffer, sizeof(char), rnum, output);
		if (wnum != rnum)
		{
			printf(
					"Copy: failed writing to output, %s --> %s: %s", 
					srcFilename.c_str(), destFilename.c_str(), GetLastErrorMsg().c_str());
			fclose(input);
			fclose(output);				
			return false;
		}
	}
	// close flushs
	fclose(input);
	fclose(output);
	return true;
#endif
}

#ifdef _WIN32

static int64_t FiletimeToStatTime(FILETIME ft) {
	const int windowsTickResolution = 10000000;
	const int64_t secToUnixEpoch = 11644473600LL;
	int64_t ticks = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	return (int64_t)(ticks / windowsTickResolution - secToUnixEpoch);
}

#endif

// Returns file attributes.
bool GetFileDetails(const std::string &filename, FileDetails *details) {
#ifdef _WIN32
	WIN32_FILE_ATTRIBUTE_DATA attr;
	if (!GetFileAttributesEx(ConvertUTF8ToWString(filename).c_str(), GetFileExInfoStandard, &attr))
		return false;
	details->isDirectory = (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	details->size = ((uint64_t)attr.nFileSizeHigh << 32) | (uint64_t)attr.nFileSizeLow;
	details->atime = FiletimeToStatTime(attr.ftLastAccessTime);
	details->mtime = FiletimeToStatTime(attr.ftLastWriteTime);
	details->ctime = FiletimeToStatTime(attr.ftCreationTime);
	if (attr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
		details->access = 0444;  // Read
	} else {
		details->access = 0666;  // Read/Write
	}
	if (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		details->access |= 0111;  // Execute
	}
	return true;
#else
	if (!Exists(filename)) {
		return false;
	}
#if __ANDROID__ && __ANDROID_API__ < 21
	struct stat buf;
	if (stat(filename.c_str(), &buf) == 0) {
#else
	struct stat64 buf;
	if (stat64(filename.c_str(), &buf) == 0) {
#endif
		details->size = buf.st_size;
		details->isDirectory = S_ISDIR(buf.st_mode);
		details->atime = buf.st_atime;
		details->mtime = buf.st_mtime;
		details->ctime = buf.st_ctime;
		details->access = buf.st_mode & 0x1ff;
		return true;
	} else {
		return false;
	}
#endif
}

bool GetModifTime(const std::string &filename, tm &return_time) {
	memset(&return_time, 0, sizeof(return_time));
	FileDetails details;
	if (GetFileDetails(filename, &details)) {
		time_t t = details.mtime;
		localtime_r((time_t*)&t, &return_time);
		return true;
	} else {
		return false;
	}
}

std::string GetDir(const std::string &path) {
	if (path == "/")
		return path;
	int n = (int)path.size() - 1;
	while (n >= 0 && path[n] != '\\' && path[n] != '/')
		n--;
	std::string cutpath = n > 0 ? path.substr(0, n) : "";
	for (size_t i = 0; i < cutpath.size(); i++) {
		if (cutpath[i] == '\\') cutpath[i] = '/';
	}
#ifndef _WIN32
	if (!cutpath.size()) {
		return "/";
	}
#endif
	return cutpath;
}

std::string GetFilename(std::string path) {
	size_t off = GetDir(path).size() + 1;
	if (off < path.size())
		return path.substr(off);
	else
		return path;
}

// Returns the size of file (64bit)
// TODO: Add a way to return an error.
uint64_t GetFileSize(const std::string &filename) {
#if defined(_WIN32) && defined(UNICODE)
	WIN32_FILE_ATTRIBUTE_DATA attr;
	if (!GetFileAttributesEx(ConvertUTF8ToWString(filename).c_str(), GetFileExInfoStandard, &attr))
		return 0;
	if (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return 0;
	return ((uint64_t)attr.nFileSizeHigh << 32) | (uint64_t)attr.nFileSizeLow;
#else
#if __ANDROID__ && __ANDROID_API__ < 21
	struct stat file_info;
	int result = stat(filename.c_str(), &file_info);
#else
	struct stat64 file_info;
	int result = stat64(filename.c_str(), &file_info);
#endif
	if (result != 0) {
		printf("GetSize: failed %s: No such file", filename.c_str());
		return 0;
	}
	if (S_ISDIR(file_info.st_mode)) {
		printf("GetSize: failed %s: is a directory", filename.c_str());
		return 0;
	}
	printf("GetSize: %s: %lld", filename.c_str(), (long long)file_info.st_size);
	return file_info.st_size;
#endif
}

uint64_t GetFileSize(FILE *f)
{
	// This will only support 64-bit when large file support is available.
	// That won't be the case on some versions of Android, at least.
#if defined(__ANDROID__) || (defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS < 64)
	int fd = fileno(f);

	off64_t pos = lseek64(fd, 0, SEEK_CUR);
	off64_t size = lseek64(fd, 0, SEEK_END);
	if (size != pos && lseek64(fd, pos, SEEK_SET) != pos) {
		// Should error here.
		return 0;
	}
	return size;
#else
#ifdef _WIN32
	uint64_t pos = _ftelli64(f);
#else
	uint64_t pos = ftello(f);
#endif
	if (fseek(f, 0, SEEK_END) != 0) {
		return 0;
	}
#ifdef _WIN32
	uint64_t size = _ftelli64(f);
	// Reset the seek position to where it was when we started.
	if (size != pos && _fseeki64(f, pos, SEEK_SET) != 0) {
#else
	uint64_t size = ftello(f);
	// Reset the seek position to where it was when we started.
	if (size != pos && fseeko(f, pos, SEEK_SET) != 0) {
#endif
		// Should error here.
		return 0;
	}
	return size;
#endif
}

// creates an empty file filename, returns true on success 
bool CreateEmptyFile(const std::string &filename)
{
	printf("CreateEmptyFile: %s", filename.c_str()); 

	FILE *pFile = OpenCFile(filename, "wb");
	if (!pFile) {
		printf("CreateEmptyFile: failed %s: %s", filename.c_str(), GetLastErrorMsg().c_str());
		return false;
	}
	fclose(pFile);
	return true;
}

// Deletes the given directory and anything under it. Returns true on success.
bool DeleteDirRecursively(const std::string &directory)
{	
	//Removed check, it prevents the UWP from deleting store downloads
	printf("DeleteDirRecursively: %s", directory.c_str());

	struct dirent *result = NULL;
	DIR *dirp = opendir(directory.c_str());
	if (!dirp)
		return false;

	// non windows loop
	while ((result = readdir(dirp)))
	{
		const std::string virtualName = result->d_name;

		// check for "." and ".."
		if (((virtualName[0] == '.') && (virtualName[1] == '\0')) ||
			((virtualName[0] == '.') && (virtualName[1] == '.') && 
			 (virtualName[2] == '\0')))
			continue;

		std::string newPath = directory + DIR_SEP + virtualName;
		if (IsDirectory(newPath))
		{
			if (!DeleteDirRecursively(newPath))
			{

				closedir(dirp);

				return false;
			}
		}
		else
		{
			if (!SCREEN_PFile::Delete(newPath))
			{

				closedir(dirp);

				return false;
			}
		}
	}
	closedir(dirp);

	return SCREEN_PFile::DeleteDir(directory);
}


// Create directory and copy contents (does not overwrite existing files)
void CopyDir(const std::string &source_path, const std::string &dest_path)
{

	if (source_path == dest_path) return;
	if (!SCREEN_PFile::Exists(source_path)) return;
	if (!SCREEN_PFile::Exists(dest_path)) SCREEN_PFile::CreateFullPath(dest_path);

	struct dirent *result = NULL;
	DIR *dirp = opendir(source_path.c_str());
	if (!dirp) return;

	while ((result = readdir(dirp)))
	{
		const std::string virtualName(result->d_name);
		// check for "." and ".."
		if (((virtualName[0] == '.') && (virtualName[1] == '\0')) ||
			((virtualName[0] == '.') && (virtualName[1] == '.') &&
			(virtualName[2] == '\0')))
			continue;

		std::string source, dest;
		source = source_path + virtualName;
		dest = dest_path + virtualName;
		if (IsDirectory(source))
		{
			source += '/';
			dest += '/';
			if (!SCREEN_PFile::Exists(dest)) SCREEN_PFile::CreateFullPath(dest);
			CopyDir(source, dest);
		}
		else if (!SCREEN_PFile::Exists(dest)) SCREEN_PFile::Copy(source, dest);
	}
	closedir(dirp);

}

void openIniFile(const std::string& fileName) {

	std::string iniFile;
	iniFile = "xdg-open ";

	iniFile.append(fileName);
	printf("Launching %s", iniFile.c_str());
	int retval = system(iniFile.c_str());
	if (retval != 0) {
		printf("Failed to launch ini file");
	}
}

const std::string &GetExeDirectory()
{
	static std::string ExePath;

	if (ExePath.empty()) {

		char program_path[4096];
		uint32_t program_path_size = sizeof(program_path) - 1;

		if (readlink("/proc/self/exe", program_path, program_path_size) > 0)
		{
			program_path[sizeof(program_path) - 1] = '\0';
			char *last_slash = strrchr(program_path, '/');
			if (last_slash != NULL)
				*(last_slash + 1) = '\0';
			ExePath = program_path;
		}
	}

	return ExePath;
}


SCREEN_IOFile::SCREEN_IOFile()
	: m_file(NULL), m_good(true)
{}

SCREEN_IOFile::SCREEN_IOFile(std::FILE* file)
	: m_file(file), m_good(true)
{}

SCREEN_IOFile::SCREEN_IOFile(const std::string& filename, const char openmode[])
	: m_file(NULL), m_good(true)
{
	Open(filename, openmode);
}

SCREEN_IOFile::~SCREEN_IOFile()
{
	Close();
}

bool SCREEN_IOFile::Open(const std::string& filename, const char openmode[])
{
	Close();
	m_file = SCREEN_PFile::OpenCFile(filename, openmode);
	m_good = IsOpen();
	return m_good;
}

bool SCREEN_IOFile::Close()
{
	if (!IsOpen() || 0 != std::fclose(m_file))
		m_good = false;

	m_file = NULL;
	return m_good;
}

std::FILE* SCREEN_IOFile::ReleaseHandle()
{
	std::FILE* const ret = m_file;
	m_file = NULL;
	return ret;
}

void SCREEN_IOFile::SetHandle(std::FILE* file)
{
	Close();
	Clear();
	m_file = file;
}

uint64_t SCREEN_IOFile::GetSize()
{
	if (IsOpen())
		return SCREEN_PFile::GetFileSize(m_file);
	else
		return 0;
}

bool SCREEN_IOFile::Seek(int64_t off, int origin)
{
	if (!IsOpen() || 0 != fseeko(m_file, off, origin))
		m_good = false;

	return m_good;
}

uint64_t SCREEN_IOFile::Tell()
{	
	if (IsOpen())
		return ftello(m_file);
	else
		return -1;
}

bool SCREEN_IOFile::Flush()
{
	if (!IsOpen() || 0 != std::fflush(m_file))
		m_good = false;

	return m_good;
}

bool SCREEN_IOFile::Resize(uint64_t size)
{
	if (!IsOpen() || 0 !=

		// TODO: handle 64bit and growing
		ftruncate(fileno(m_file), size)
	)
		m_good = false;

	return m_good;
}

} // namespace

bool readFileToString(bool text_file, const char *filename, std::string & str)
{
	FILE *f = SCREEN_PFile::OpenCFile(filename, text_file ? "r" : "rb");
	if (!f)
		return false;
	size_t len = (size_t)SCREEN_PFile::GetFileSize(f);
	char *buf = new char[len + 1];
	buf[fread(buf, 1, len, f)] = 0;
	str = std::string(buf, len);
	fclose(f);
	delete[] buf;
	return true;
}

uint8_t *ReadLocalFile(const char *filename, size_t * size) {
	FILE *file = SCREEN_PFile::OpenCFile(filename, "rb");
	if (!file) {
		*size = 0;
		return nullptr;
	}
	fseek(file, 0, SEEK_END);
	size_t f_size = ftell(file);
	if ((long)f_size < 0) {
		*size = 0;
		fclose(file);
		return nullptr;
	}
	fseek(file, 0, SEEK_SET);
	uint8_t *contents = new uint8_t[f_size + 1];
	if (fread(contents, 1, f_size, file) != f_size) {
		delete[] contents;
		contents = nullptr;
		*size = 0;
	} else {
		contents[f_size] = 0;
		*size = f_size;
	}
	fclose(file);
	return contents;
}

bool writeStringToFile(bool text_file, const std::string &str, const char *filename)
{
	FILE *f = SCREEN_PFile::OpenCFile(filename, text_file ? "w" : "wb");
	if (!f)
		return false;
	size_t len = str.size();
	if (len != fwrite(str.data(), 1, str.size(), f))
	{
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool writeDataToFile(bool text_file, const void* data, const unsigned int size, const char *filename)
{
	FILE *f = SCREEN_PFile::OpenCFile(filename, text_file ? "w" : "wb");
	if (!f)
		return false;
	size_t len = size;
	if (len != fwrite(data, 1, len, f))
	{
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}
