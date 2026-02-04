#include "FatSystem.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <locale>
#include <sstream>

// TODO записываем сначала файл больше 4кб - он разбивается на 2 части.
// что будет, если дописать данные в первую часть, куда сдвинется/перезапишутся данные из второго кластера

constexpr char DEFAULT_DIVIDER = '/';
constexpr uint32_t START_OF_CLUSTER = 0x0000002;
constexpr uint32_t END_OF_CLUSTER = 0x0FFFFFF8;
constexpr uint8_t DIR_ENTRY_SIZE = 32;
constexpr uint8_t DELETED_FILE = 0xE5;
constexpr uint8_t LONG_FILE_NAME = 0x0F;
constexpr uint8_t END_OF_LINE = 0x0000;
constexpr uint16_t PADDING = 0xFFFF;
constexpr uint8_t VOLUME_ID = 0x08;
constexpr uint8_t DIR_FLAG = 0x10;
constexpr int RESERVED_CLUSTERS = 2;
constexpr uint32_t FAT_ENTRY_SIZE = 4;
constexpr uint8_t LFN_LAST_ENTRY = 0x40;

FatSystem::FatSystem(const std::string& imagePath)
	: m_reader(new ImageReader(imagePath))
	, m_fatStart(0)
	, m_dataStart(0)
	, m_bytesPerCluster(0)
{
	ParseBootSector();
}

void FatSystem::ParseBootSector()
{
	m_reader->Read(&m_bootSector, sizeof(m_bootSector), 0);

	if (m_bootSector.bytesPerSector == 0)
	{
		throw std::runtime_error("Error: invalid FAT32 structure.");
	}

	const auto fatSectors = m_bootSector.numFATs * m_bootSector.sectorsPerFAT32;
	const auto dataSectors = m_bootSector.totalSectors32 - (m_bootSector.reservedSectors + fatSectors);
	const auto dataClusters = dataSectors / m_bootSector.sectorsPerCluster;
	if (dataClusters < 65525)
	{
		throw std::runtime_error("Error: not a FAT32 image.");
	}

	m_bytesPerCluster = m_bootSector.bytesPerSector * m_bootSector.sectorsPerCluster;
	m_fatStart = m_bootSector.reservedSectors * m_bootSector.bytesPerSector;

	const size_t fatSize = fatSectors * m_bootSector.bytesPerSector;
	m_dataStart = m_fatStart + fatSize;
}

void FatSystem::ShowPath(std::string& path)
{
	const auto tokens = SplitPath(path);
	FileInfo current = {
		"/",
		true,
		0,
		m_bootSector.rootCluster
	};

	for (const auto& token : tokens)
	{
		if (!current.isDir)
		{
			throw std::runtime_error("Error: not a directory: " + current.name);
		}

		current = FindChild(current.startCluster, token);
	}

	if (current.isDir)
	{
		PrintDirectory(current);
	}
	else
	{
		PrintFile(current);
	}
}

std::vector<std::string> FatSystem::SplitPath(std::string& path)
{
	std::ranges::replace(path, '\\', DEFAULT_DIVIDER);

	std::vector<std::string> tokens;
	std::string token;
	std::istringstream stream(path);

	while (std::getline(stream, token, DEFAULT_DIVIDER))
	{
		if (!token.empty() && token != ".")
		{
			tokens.push_back(token);
		}
	}
	return tokens;
}

FileInfo FatSystem::FindChild(const uint32_t startCluster, const std::string& name)
{
	const auto children = ReadDirectory(startCluster);
	for (const auto& child : children)
	{
		if (strcasecmp(child.name.c_str(), name.c_str()) == 0)
		{
			return child;
		}
	}
	throw std::runtime_error("Error: path not found: " + name);
}

std::vector<FileInfo> FatSystem::ReadDirectory(const uint32_t startCluster)
{
	std::vector<FileInfo> results;
	uint32_t currentCluster = startCluster;

	m_lfnBuffer.clear();
	m_lfnValid = false;

	while (currentCluster >= START_OF_CLUSTER && currentCluster < END_OF_CLUSTER)
	{
		auto clusterData = ReadCluster(currentCluster);
		for (size_t i = 0; i < clusterData.size(); i += DIR_ENTRY_SIZE)
		{
			ParseDirEntry(clusterData.data() + i, results);
		}
		currentCluster = GetNextCluster(currentCluster);
	}
	return results;
}

std::vector<uint8_t> FatSystem::ReadCluster(const uint32_t cluster) const
{
	std::vector<uint8_t> buffer(m_bytesPerCluster);
	m_reader->Read(buffer.data(), m_bytesPerCluster, ClusterToOffset(cluster));
	return buffer;
}

off_t FatSystem::ClusterToOffset(const uint32_t cluster) const
{
	return m_dataStart + static_cast<off_t>(cluster - RESERVED_CLUSTERS) * m_bytesPerCluster;
}

void FatSystem::ParseDirEntry(const uint8_t* entry, std::vector<FileInfo>& results)
{
	if (entry[0] == 0x00)
	{
		return;
	}
	if (entry[0] == DELETED_FILE)
	{
		m_lfnBuffer.clear();
		m_lfnValid = false;
		return;
	}

	const uint8_t attr = entry[11];
	if (attr == LONG_FILE_NAME)
	{
		HandleLFNEntry(reinterpret_cast<const FatLFNEntry*>(entry));
	}
	else
	{
		HandleRegularEntry(reinterpret_cast<const FatDirEntry*>(entry), results);
	}
}

void FatSystem::HandleLFNEntry(const FatLFNEntry* entry)
{
	if (entry->order & LFN_LAST_ENTRY)
	{
		m_lfnBuffer.clear();
		m_expectedLfnChecksum = entry->checkSum;
		m_lfnValid = true;
	}
	else
	{
		if (!m_lfnValid || entry->checkSum != m_expectedLfnChecksum)
		{
			m_lfnValid = false;
			m_lfnBuffer.clear();
			return;
		}
	}

	std::wstring part;
	AppendLFNSegment(part, entry->name1, 5);
	AppendLFNSegment(part, entry->name2, 6);
	AppendLFNSegment(part, entry->name3, 2);

	m_lfnBuffer.insert(0, part);
}

void FatSystem::AppendLFNSegment(std::wstring& part, const uint16_t* source, const size_t length)
{
	for (size_t i = 0; i < length; ++i)
	{
		const uint16_t code = source[i];
		if (code == END_OF_LINE || code == PADDING)
		{
			break;
		}
		part += code;
	}
}

void FatSystem::HandleRegularEntry(const FatDirEntry* entry, std::vector<FileInfo>& results)
{
	if (entry->attr & VOLUME_ID)
	{
		m_lfnBuffer.clear();
		m_lfnValid = false;
		return;
	}

	if (m_lfnValid && m_expectedLfnChecksum != CalculateChecksum(entry->name))
	{
		m_lfnValid = false;
		m_lfnBuffer.clear();
	}

	FileInfo info;
	info.isDir = (entry->attr & DIR_FLAG) != 0;
	info.size = entry->fileSize;
	info.startCluster = entry->firstClusterHigh << 16 | entry->firstClusterLow;

	if (!m_lfnBuffer.empty())
	{
		info.name = ProcessLfn();
	}
	else
	{
		info.name = ProcessShortName(entry->name);
	}

	results.push_back(info);
	m_lfnBuffer.clear();
	m_lfnValid = false;
}

uint32_t FatSystem::GetNextCluster(const uint32_t cluster) const
{
	uint32_t nextCluster = 0;
	const off_t offset = m_fatStart + static_cast<off_t>(cluster) * FAT_ENTRY_SIZE;
	m_reader->Read(&nextCluster, FAT_ENTRY_SIZE, offset);
	return nextCluster & 0x0FFFFFFF;
}

std::string FatSystem::ProcessLfn() const
{
	std::string result;
	for (const wchar_t code : m_lfnBuffer)
	{
		if (code > 127)
		{
			result += '?';
		}
		else
		{
			result += static_cast<char>(code);
		}
	}
	return result;
}

std::string FatSystem::ProcessShortName(const uint8_t* name)
{
	std::string base;
	std::string ext;
	for (int i = 0; i < 8 && name[i] != ' '; ++i)
	{
		base += static_cast<char>(name[i]);
	}
	for (int i = 0; i < 3 && name[i] != ' '; ++i)
	{
		ext += static_cast<char>(name[i]);
	}
	if (!ext.empty())
	{
		return base + "." + ext;
	}
	return base;
}

void FatSystem::PrintDirectory(const FileInfo& dirInfo)
{
	std::cout << "D .\nD ..\n";

	const auto children = ReadDirectory(dirInfo.startCluster);
	for (const auto& child : children)
	{
		if (child.name == "." || child.name == "..")
		{
			continue;
		}
		if (child.isDir)
		{
			std::cout << "D " << child.name << "\n";
		}
		else
		{
			std::cout << "F " << child.name << " " << child.size << "\n";
		}
	}
}

void FatSystem::PrintFile(const FileInfo& fileInfo) const
{
	std::cout << "FILE " << fileInfo.name << " " << fileInfo.size << "\n";

	uint32_t cluster = fileInfo.startCluster;
	size_t remaining = fileInfo.size;

	while (remaining > 0 && cluster >= START_OF_CLUSTER && cluster < END_OF_CLUSTER)
	{
		auto data = ReadCluster(cluster);
		const size_t toWrite = std::min(remaining, data.size());
		std::cout.write(reinterpret_cast<char*>(data.data()), static_cast<long int>(toWrite));

		remaining -= toWrite;
		cluster = GetNextCluster(cluster);
	}
}

uint8_t FatSystem::CalculateChecksum(const uint8_t* shortName)
{
	uint8_t sum = 0;
	for (int i = 11; i > 0; i--)
	{
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *shortName++;
	}
	return sum;
}
