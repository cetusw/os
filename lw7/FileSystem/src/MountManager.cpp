#include "MountManager.h"
#include <cstring>
#include <iostream>

fuse_operations MountManager::m_fuseOperators = {
	.getattr = GetAttr,
	.unlink = Unlink,
	.truncate = Truncate,
	.open = Open,
	.read = Read,
	.write = Write,
	.readdir = ReadDir,
	.create = Create
};

MountManager::MountManager(FileSystemManager& fsManager)
	: m_fileSystemManager(fsManager)
{
}

int MountManager::Run(const int argc, char* argv[]) const
{
	if (argc < 3)
	{
		std::cerr << "Usage: ./myfs-mount <image_path> <mount_point>" << std::endl;
		return 1;
	}

	const std::string imagePath = argv[1];
	if (!m_fileSystemManager.OpenImage(imagePath))
	{
		return 1;
	}

	std::vector<char*> fuseArgs;
	fuseArgs.push_back(argv[0]);
	for (int i = 2; i < argc; ++i)
	{
		fuseArgs.push_back(argv[i]);
	}

	return fuse_main(static_cast<int>(fuseArgs.size()), fuseArgs.data(), &m_fuseOperators, &m_fileSystemManager);
}

FileSystemManager* MountManager::GetFileSystem()
{
	return static_cast<FileSystemManager*>(fuse_get_context()->private_data);
}

std::string MountManager::ExtractFileName(const char* path)
{
	if (std::strcmp(path, "/") == 0)
	{
		return "/";
	}
	return path + 1;
}

int MountManager::GetAttr(const char* path, struct stat* stbuf, fuse_file_info* fi)
{
	(void)fi;
	std::memset(stbuf, 0, sizeof(struct stat));
	const std::string fileName = ExtractFileName(path);

	if (fileName == "/")
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	FileEntry entry{};
	if (GetFileSystem()->GetFileStat(fileName, entry))
	{
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
		stbuf->st_size = entry.size;
		return 0;
	}
	return -ENOENT;
}

int MountManager::Read(
	const char* path,
	char* buf,
	const size_t size,
	const off_t offset,
	fuse_file_info* fi)
{
	(void)fi;
	const int result = GetFileSystem()->ReadData(ExtractFileName(path), buf, size, static_cast<uint64_t>(offset));
	if (result == -1)
	{
		return -ENOENT;
	}
	return result;
}

int MountManager::Write(
	const char* path,
	const char* buf,
	const size_t size,
	const off_t offset,
	fuse_file_info* fi)
{
	(void)fi;
	const int result = GetFileSystem()->WriteData(ExtractFileName(path), buf, size, static_cast<uint64_t>(offset));
	if (result == -1)
	{
		return -ENOSPC;
	}
	return result;
}

int MountManager::ReadDir(
	const char* path,
	void* buf,
	const fuse_fill_dir_t filler,
	const off_t offset,
	fuse_file_info* fi,
	const fuse_readdir_flags flags)
{
	(void)offset;
	(void)fi;
	(void)flags;
	if (ExtractFileName(path) != "/")
	{
		return -ENOENT;
	}

	filler(buf, ".", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
	filler(buf, "..", nullptr, 0, static_cast<fuse_fill_dir_flags>(0));

	for (const auto& file : GetFileSystem()->GetFileList())
	{
		filler(buf, file.name, nullptr, 0, static_cast<fuse_fill_dir_flags>(0));
	}
	return 0;
}

int MountManager::Create(const char* path, mode_t mode, fuse_file_info* fi)
{
	(void)mode;
	(void)fi;
	if (!GetFileSystem()->CreateFile(ExtractFileName(path)))
	{
		return -EIO;
	}
	return 0;
}

int MountManager::Unlink(const char* path)
{
	if (!GetFileSystem()->RemoveFile(ExtractFileName(path)))
	{
		return -ENOENT;
	}
	return 0;
}

int MountManager::Truncate(const char* path, const off_t size, fuse_file_info* fi)
{
	(void)fi;
	if (!GetFileSystem()->TruncateFile(ExtractFileName(path), static_cast<uint64_t>(size)))
	{
		return -EIO;
	}
	return 0;
}

int MountManager::Open(const char* path, fuse_file_info* fi)
{
	(void)fi;
	FileEntry entry{};
	if (!GetFileSystem()->GetFileStat(ExtractFileName(path), entry))
	{
		return -ENOENT;
	}
	return 0;
}