#ifndef FILESYSTEM_MOUNTMANAGER_H
#define FILESYSTEM_MOUNTMANAGER_H

#define FUSE_USE_VERSION 31
#include "FileSystemManager.h"
#include <fuse3/fuse.h>
#include <string>

class MountManager
{
public:
	explicit MountManager(FileSystemManager& fsManager);
	~MountManager() = default;

	int Run(int argc, char* argv[]) const;

private:
	static int GetAttr(const char* path, struct stat* stbuf, fuse_file_info* fi);
	static int ReadDir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi, fuse_readdir_flags flags);
	static int Create(const char* path, mode_t mode, fuse_file_info* fi);
	static int Read(const char* path, char* buf, size_t size, off_t offset, fuse_file_info* fi);
	static int Write(const char* path, const char* buf, size_t size, off_t offset, fuse_file_info* fi);
	static int Unlink(const char* path);
	static int Truncate(const char* path, off_t size, fuse_file_info* fi);
	static int Open(const char* path, fuse_file_info* fi);

	static FileSystemManager* GetFileSystem();
	static std::string ExtractFileName(const char* path);

	FileSystemManager& m_fileSystemManager;
	static fuse_operations m_fuseOperators;
};

#endif // FILESYSTEM_MOUNTMANAGER_H
