#ifndef FAT_SYSTEM_H
#define FAT_SYSTEM_H

#include "FatStructs.h"
#include "ImageReader.h"
#include <memory>
#include <string>
#include <vector>

struct FileInfo
{
	std::string name;
	bool isDir;
	uint32_t size;
	uint32_t startCluster;
};

class FatSystem
{
public:
	explicit FatSystem(const std::string& imagePath);

	void ShowPath(std::string& path) const;

private:
	void ParseBootSector();

	[[nodiscard]] uint32_t GetNextCluster(uint32_t cluster) const;

	[[nodiscard]] off_t ClusterToOffset(uint32_t cluster) const;

	[[nodiscard]] std::vector<uint8_t> ReadCluster(uint32_t cluster) const;

	[[nodiscard]] std::vector<FileInfo> ReadDirectory(uint32_t startCluster) const;

	static void ParseDirEntry(const uint8_t* ptr, std::vector<FileInfo>& results, std::wstring& lfnBuffer);

	static void HandleLFNEntry(const FatLFNEntry* entry, std::wstring& lfnBuffer);

	static void HandleShortEntry(const FatDirEntry* entry, std::vector<FileInfo>& results, std::wstring& lfnBuffer);

	[[nodiscard]] FileInfo FindChild(uint32_t startCluster, const std::string& name) const;

	void PrintDirectory(const FileInfo& dirInfo) const;

	void PrintFile(const FileInfo& fileInfo) const;

	static std::string ProcessLfn(const std::wstring& lfn);

	static std::string ProcessShortName(const uint8_t* name);

	static std::vector<std::string> SplitPath(std::string& path);

	std::unique_ptr<ImageReader> m_reader;
	BootSector m_bootSector{};
	uint32_t m_fatStart;
	uint32_t m_dataStart;
	uint32_t m_bytesPerCluster;
};

#endif // FAT_SYSTEM_H
