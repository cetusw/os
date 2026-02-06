#include "FileSystemManager.h"

#include <cstring>
#include <iostream>

// TODO два пользователя записывают данные - состояние гонки

FileSystemManager::~FileSystemManager()
{
	CloseImage();
}

bool FileSystemManager::CreateImage(const std::string& path, const uint64_t totalSize, const uint32_t maxFiles)
{
	InitializeEmptyStorage(path, totalSize);

	m_imageStream.open(path, std::ios::in | std::ios::out | std::ios::binary);
	if (!m_imageStream.is_open())
	{
		return false;
	}

	if (!InitializeSuperblock(maxFiles, totalSize))
	{
		return false;
	}

	InitializeFileTable(maxFiles);

	std::cout << "FS created successfully" << std::endl;
	return true;
}

bool FileSystemManager::OpenImage(const std::string& path)
{
	m_imageStream.open(path, std::ios::in | std::ios::out | std::ios::binary);
	if (!m_imageStream.is_open())
	{
		return false;
	}

	ReadSuperblock();

	if (std::memcmp(m_superblock.signature, SIGNATURE, 5) != 0)
	{
		std::cerr << "Error: Invalid FS signature" << std::endl;
		m_imageStream.close();
		return false;
	}

	ReadFileTable();
	return true;
}

void FileSystemManager::CloseImage()
{
	if (m_imageStream.is_open())
	{
		WriteSuperblock();
		WriteFileTable();
		m_imageStream.close();
	}
}

bool FileSystemManager::CreateFile(const std::string& name)
{
	if (name.empty() || name.length() > MAX_FILENAME)
	{
		return false;
	}

	if (GetFileIndex(name) != -1)
	{
		std::cerr << "Error: File already exists" << std::endl;
		return false;
	}

	const int freeEntryIndex = FindFreeFileTableEntry();
	if (freeEntryIndex == -1)
	{
		std::cerr << "Error: File table is full" << std::endl;
		return false;
	}

	InitializeFileEntry(freeEntryIndex, name);

	return true;
}

bool FileSystemManager::RemoveFile(const std::string& name)
{
	const int fileIndex = GetFileIndex(name);
	if (fileIndex == -1)
	{
		return false;
	}
	m_fileTable[fileIndex].isUsed = false;
	WriteFileTable();
	return true;
}

bool FileSystemManager::TruncateFile(const std::string& name, const uint64_t newSize)
{
	if (newSize > MAX_FILE_SIZE)
	{
		return false;
	}

	const int fileIndex = GetFileIndex(name);
	if (fileIndex == -1)
	{
		return false;
	}

	FileEntry& entry = m_fileTable[fileIndex];
	if (!EnsureSufficientBlocks(entry, newSize))
	{
		return false;
	}

	entry.size = static_cast<uint32_t>(newSize);
	WriteFileTable();
	return true;
}

int FileSystemManager::ReadData(const std::string& name, char* buffer, const size_t size, const uint64_t offset)
{
	const int fileIndex = GetFileIndex(name);
	if (fileIndex == -1)
	{
		return -1;
	}

	const FileEntry& entry = m_fileTable[fileIndex];
	if (offset >= entry.size)
	{
		return 0;
	}

	size_t bytesToRead = size;
	if (offset + size > entry.size)
	{
		bytesToRead = entry.size - offset;
	}

	const uint64_t toReadOffset = m_superblock.dataAreaOffset
		+ static_cast<uint64_t>(entry.startBlock) * BLOCK_SIZE + offset;

	m_imageStream.seekg(static_cast<std::streamoff>(toReadOffset), std::ios::beg);
	m_imageStream.read(buffer, static_cast<std::streamsize>(bytesToRead));

	return static_cast<int>(bytesToRead);
}

int FileSystemManager::WriteData(const std::string& name, const char* buffer, const size_t size, const uint64_t offset)
{
	const int fileIndex = GetFileIndex(name);
	if (fileIndex == -1)
	{
		return -1;
	}

	FileEntry& entry = m_fileTable[fileIndex];
	const uint64_t newSize = std::max(static_cast<uint64_t>(entry.size), offset + size);
	if (newSize > MAX_FILE_SIZE || !EnsureSufficientBlocks(entry, newSize))
	{
		return -1;
	}

	const uint64_t toWriteOffset = m_superblock.dataAreaOffset
		+ static_cast<uint64_t>(entry.startBlock) * BLOCK_SIZE + offset;

	m_imageStream.seekp(static_cast<std::streamoff>(toWriteOffset), std::ios::beg);
	m_imageStream.write(buffer, static_cast<std::streamsize>(size));

	entry.size = static_cast<uint32_t>(newSize);
	WriteFileTable();

	return static_cast<int>(size);
}

std::vector<FileEntry> FileSystemManager::GetFileList() const
{
	std::vector<FileEntry> result;
	for (const auto& entry : m_fileTable)
	{
		if (entry.isUsed)
		{
			result.push_back(entry);
		}
	}
	return result;
}

bool FileSystemManager::GetFileStat(const std::string& name, FileEntry& entry) const
{
	const int fileIndex = GetFileIndex(name);
	if (fileIndex == -1)
	{
		return false;
	}
	entry = m_fileTable[fileIndex];
	return true;
}

void FileSystemManager::WriteSuperblock()
{
	m_imageStream.seekp(0, std::ios::beg);
	m_imageStream.write(reinterpret_cast<const char*>(&m_superblock), sizeof(Superblock));
	m_imageStream.flush();
}

void FileSystemManager::ReadSuperblock()
{
	m_imageStream.seekg(0, std::ios::beg);
	m_imageStream.read(reinterpret_cast<char*>(&m_superblock), sizeof(Superblock));
}

void FileSystemManager::WriteFileTable()
{
	m_imageStream.seekp(m_superblock.fileTableOffset, std::ios::beg);
	m_imageStream.write(reinterpret_cast<const char*>(m_fileTable.data()),
		static_cast<std::streamsize>(m_fileTable.size() * sizeof(FileEntry)));
	m_imageStream.flush();
}

void FileSystemManager::ReadFileTable()
{
	m_fileTable.resize(m_superblock.maxFiles);
	m_imageStream.seekg(m_superblock.fileTableOffset, std::ios::beg);
	m_imageStream.read(reinterpret_cast<char*>(m_fileTable.data()),
		static_cast<std::streamsize>(m_fileTable.size() * sizeof(FileEntry)));
}

void FileSystemManager::InitializeEmptyStorage(const std::string& path, const uint64_t size)
{
	std::ofstream ofs(path, std::ios::binary | std::ios::out);
	constexpr char zeroBlock[BLOCK_SIZE] = {};

	const uint64_t blocksNeeded = size / BLOCK_SIZE;
	for (uint64_t i = 0; i < blocksNeeded; ++i)
	{
		ofs.write(zeroBlock, BLOCK_SIZE);
	}

	const uint64_t remainder = size % BLOCK_SIZE;
	if (remainder > 0)
	{
		ofs.write(zeroBlock, static_cast<std::streamsize>(remainder));
	}
	ofs.close();
}

bool FileSystemManager::InitializeSuperblock(const uint32_t maxFiles, const uint64_t totalSize)
{
	std::memcpy(m_superblock.signature, SIGNATURE, sizeof(SIGNATURE));
	m_superblock.version = 1;
	m_superblock.blockSize = BLOCK_SIZE;
	m_superblock.maxFiles = maxFiles;
	m_superblock.fileTableOffset = sizeof(Superblock);
	m_superblock.dataAreaOffset = AlignUpToBlockSize(maxFiles);
	if (totalSize < m_superblock.dataAreaOffset)
	{
		std::cerr << "Error: Image size is too small for metadata" << std::endl;
		return false;
	}
	m_superblock.totalBlocks = (totalSize - m_superblock.dataAreaOffset) / BLOCK_SIZE;
	WriteSuperblock();

	return true;
}
void FileSystemManager::InitializeFileTable(const uint32_t maxFiles)
{
	m_fileTable.assign(maxFiles, FileEntry{});
	for (auto& entry : m_fileTable)
	{
		entry.isUsed = false;
	}
	WriteFileTable();
}
void FileSystemManager::InitializeFileEntry(const int freeEntryIndex, const std::string& name)
{
	FileEntry& entry = m_fileTable[freeEntryIndex];
	std::memset(entry.name, 0, MAX_FILENAME + 1);
	std::strncpy(entry.name, name.c_str(), MAX_FILENAME);

	entry.size = 0;
	entry.startBlock = 0;
	entry.blocksCount = 0;
	entry.isUsed = true;

	WriteFileTable();
}

int FileSystemManager::FindConsecutiveBlocks(const uint32_t neededBlocks) const
{
	if (neededBlocks == 0)
	{
		return 0;
	}
	std::vector<bool> blockMap = GetBlockMap();
	uint32_t consecutiveFound = 0;
	for (uint32_t blockIndex = 0; blockIndex < m_superblock.totalBlocks; ++blockIndex)
	{
		if (!blockMap[blockIndex])
		{
			consecutiveFound++;
			if (consecutiveFound == neededBlocks)
			{
				return static_cast<int>(blockIndex - neededBlocks + 1);
			}
		}
		else
		{
			consecutiveFound = 0;
		}
	}

	return -1;
}

int FileSystemManager::FindFreeFileTableEntry() const
{
	int freeEntryIndex = -1;
	for (uint32_t i = 0; i < m_superblock.maxFiles; ++i)
	{
		if (!m_fileTable[i].isUsed)
		{
			freeEntryIndex = static_cast<int>(i);
			break;
		}
	}

	return freeEntryIndex;
}

int FileSystemManager::GetFileIndex(const std::string& name) const
{
	for (uint32_t i = 0; i < m_superblock.maxFiles; ++i)
	{
		if (m_fileTable[i].isUsed && name == m_fileTable[i].name)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

uint32_t FileSystemManager::AlignUpToBlockSize(const uint32_t maxFiles) const
{
	const uint32_t fileTableSize = maxFiles * sizeof(FileEntry);
	const uint64_t rawDataOffset = m_superblock.fileTableOffset + fileTableSize;
	return ((rawDataOffset + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
}

std::vector<bool> FileSystemManager::GetBlockMap() const
{
	std::vector blockMap(m_superblock.totalBlocks, false);

	for (const auto& entry : m_fileTable)
	{
		if (entry.isUsed && entry.blocksCount > 0)
		{
			for (uint32_t i = 0; i < entry.blocksCount; ++i)
			{
				const uint32_t blockIndex = entry.startBlock + i;
				if (blockIndex < blockMap.size())
				{
					blockMap[blockIndex] = true;
				}
			}
		}
	}

	return blockMap;
}

bool FileSystemManager::EnsureSufficientBlocks(FileEntry& entry, const uint64_t newSize)
{
	// TODO разделить метод на методы с единственной ответственностью
	const auto neededBlocks = static_cast<uint32_t>((newSize + BLOCK_SIZE - 1) / BLOCK_SIZE);
	if (neededBlocks <= entry.blocksCount)
	{
		return true;
	}

	// TODO увеличивать размер дырки в два раза. сделать более эффективное распределение памяти
	uint32_t targetBlocks = CalculateTargetBlockCount(entry, neededBlocks);

	int newStartBlock = FindConsecutiveBlocks(targetBlocks);
	if (newStartBlock == -1 && targetBlocks > neededBlocks)
	{
		targetBlocks = neededBlocks;
		newStartBlock = FindConsecutiveBlocks(targetBlocks);
	}

	if (newStartBlock == -1)
	{
		return false;
	}

	if (entry.size > 0)
	{
		MoveFileData(entry, newStartBlock);
	}

	entry.startBlock = static_cast<uint32_t>(newStartBlock);
	entry.blocksCount = targetBlocks;

	return true;
}

void FileSystemManager::MoveFileData(const FileEntry& entry, const uint64_t newStartBlock)
{
	constexpr size_t bufferSize = 1024 * 1024;
	std::vector<char> tempBuffer(bufferSize);

	const uint64_t oldBaseOffset = m_superblock.dataAreaOffset + (static_cast<uint64_t>(entry.startBlock) * BLOCK_SIZE);
	const uint64_t newBaseOffset = m_superblock.dataAreaOffset + (static_cast<uint64_t>(newStartBlock) * BLOCK_SIZE);

	for (uint64_t moved = 0; moved < entry.size;)
	{
		const uint64_t chunk = std::min(static_cast<uint64_t>(bufferSize), static_cast<uint64_t>(entry.size) - moved);

		m_imageStream.seekg(static_cast<std::streamoff>(oldBaseOffset + moved), std::ios::beg);
		m_imageStream.read(tempBuffer.data(), static_cast<std::streamsize>(chunk));

		m_imageStream.seekp(static_cast<std::streamoff>(newBaseOffset + moved), std::ios::beg);
		m_imageStream.write(tempBuffer.data(), static_cast<std::streamsize>(chunk));

		moved += chunk;
	}
}
uint32_t FileSystemManager::CalculateTargetBlockCount(const FileEntry& entry, const uint32_t neededBlocks)
{
	uint32_t targetBlocks;
	if (entry.blocksCount == 0)
	{
		targetBlocks = 1;
	}
	else
	{
		targetBlocks = entry.blocksCount * 2;
	}
	if (targetBlocks < neededBlocks)
	{
		targetBlocks = neededBlocks;
	}

	return targetBlocks;
}