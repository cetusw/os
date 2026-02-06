#ifndef FILESYSTEM_FILESYSTEMMANAGER_H
#define FILESYSTEM_FILESYSTEMMANAGER_H

#include "Structure.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class FileSystemManager
{
public:
	FileSystemManager() = default;
	~FileSystemManager();

	bool CreateImage(const std::string& path, uint64_t totalSize, uint32_t maxFiles);
	bool OpenImage(const std::string& path);
	void CloseImage();

	bool CreateFile(const std::string& name);
	bool RemoveFile(const std::string& name);
	bool TruncateFile(const std::string& name, uint64_t newSize);

	int ReadData(const std::string& name, char* buffer, size_t size, uint64_t offset);
	int WriteData(const std::string& name, const char* buffer, size_t size, uint64_t offset);

	std::vector<FileEntry> GetFileList() const;
	bool GetFileStat(const std::string& name, FileEntry& entry) const;

private:
	void ReadSuperblock();
	void WriteSuperblock();
	void ReadFileTable();
	void WriteFileTable();

	static void InitializeEmptyStorage(const std::string& path, uint64_t size);
	bool InitializeSuperblock(uint32_t maxFiles, uint64_t totalSize);
	void InitializeFileTable(uint32_t maxFiles);
	void InitializeFileEntry(int freeEntryIndex, const std::string& name);

	int FindConsecutiveBlocks(uint32_t neededBlocks) const;
	int FindFreeFileTableEntry() const;
	int GetFileIndex(const std::string& name) const;
	uint32_t AlignUpToBlockSize(uint32_t maxFiles) const;
	std::vector<bool> GetBlockMap() const;
	bool EnsureSufficientBlocks(FileEntry& entry, uint64_t newSize);
	void MoveFileData(const FileEntry& entry, uint64_t newStartBlock);
	static uint32_t CalculateTargetBlockCount(const FileEntry& entry, uint32_t neededBlocks);

	std::fstream m_imageStream;
	Superblock m_superblock;
	std::vector<FileEntry> m_fileTable;
};

#endif // FILESYSTEM_FILESYSTEMMANAGER_H