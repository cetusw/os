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

	void ShowPath(std::string& path);

private:
	void ParseBootSector();
	[[nodiscard]] uint32_t GetNextCluster(uint32_t cluster) const;
	[[nodiscard]] off_t ClusterToOffset(uint32_t cluster) const;
	[[nodiscard]] std::vector<uint8_t> ReadCluster(uint32_t cluster) const;
	[[nodiscard]] std::vector<FileInfo> ReadDirectory(uint32_t startCluster);
	void ParseDirEntry(const uint8_t* entry, std::vector<FileInfo>& results);
	void HandleLFNEntry(const FatLFNEntry* entry);
	static void AppendLFNSegment(std::wstring& part, const uint16_t* source, size_t length);
	void HandleRegularEntry(const FatDirEntry* entry, std::vector<FileInfo>& results);
	[[nodiscard]] FileInfo FindChild(uint32_t startCluster, const std::string& name);
	void PrintDirectory(const FileInfo& dirInfo);
	void PrintFile(const FileInfo& fileInfo) const;
	[[nodiscard]] std::string ProcessLfn() const;
	static std::string ProcessShortName(const uint8_t* name);
	static std::vector<std::string> SplitPath(std::string& path);
	static uint8_t CalculateChecksum(const uint8_t* shortName);

	std::unique_ptr<ImageReader> m_reader;
	BootSector m_bootSector{};
	uint32_t m_fatStart;
	uint32_t m_dataStart;
	uint32_t m_bytesPerCluster;

	std::wstring m_lfnBuffer;
	uint8_t m_expectedLfnChecksum = 0;
	bool m_lfnValid = false;
};

#endif // FAT_SYSTEM_H
