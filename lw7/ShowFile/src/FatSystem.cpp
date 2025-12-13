#include "FatSystem.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <locale>

constexpr uint8_t START_OF_DIR = 2;
constexpr uint32_t END_OF_DIR = 0x0FFFFFF8;
constexpr uint8_t DIR_ENTRY_SIZE = 32;
constexpr uint8_t DELETED_FILE = 0xE5;
constexpr uint8_t LONG_FILE_NAME = 0x0F;
constexpr unsigned short PADDING = 0xFFFF;
constexpr uint8_t VOLUME_ID = 0x08;
constexpr uint8_t DIR_FLAG = 0x10;

FatSystem::FatSystem(const std::string &imagePath)
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

    const auto dataSec = m_bootSector.totalSectors32 - (
                             m_bootSector.reservedSectors + m_bootSector.numFATs * m_bootSector.sectorsPerFAT32);
    if (dataSec / m_bootSector.sectorsPerCluster < 65525)
    {
        throw std::runtime_error("Error: not a FAT32 image.");
    }

    m_bytesPerCluster = m_bootSector.bytesPerSector * m_bootSector.sectorsPerCluster;
    m_fatStart = m_bootSector.reservedSectors * m_bootSector.bytesPerSector;

    const size_t fatSize = m_bootSector.numFATs * m_bootSector.sectorsPerFAT32 * m_bootSector.bytesPerSector;
    m_dataStart = m_fatStart + fatSize;
}

off_t FatSystem::ClusterToOffset(const uint32_t cluster) const
{
    return m_dataStart + static_cast<off_t>(cluster - 2) * m_bytesPerCluster;
}

uint32_t FatSystem::GetNextCluster(const uint32_t cluster) const
{
    uint32_t nextCluster = 0;
    const off_t offset = m_fatStart + static_cast<off_t>(cluster) * 4;
    m_reader->Read(&nextCluster, 4, offset);
    return nextCluster & 0x0FFFFFFF;
}

std::vector<uint8_t> FatSystem::ReadCluster(const uint32_t cluster) const
{
    std::vector<uint8_t> buffer(m_bytesPerCluster);
    m_reader->Read(buffer.data(), m_bytesPerCluster, ClusterToOffset(cluster));
    return buffer;
}

void FatSystem::ShowPath(std::string &path) const
{
    const auto tokens = SplitPath(path);
    FileInfo current = {
        "/",
        true,
        0,
        m_bootSector.rootCluster
    };

    for (const auto &token: tokens)
    {
        if (!current.isDir)
        {
            throw std::runtime_error("Error: not a directory: " + current.name);
        }

        current = FindChild(current.firstCluster, token);
    }

    if (current.isDir)
    {
        PrintDirectory(current);
    } else
    {
        PrintFile(current);
    }
}

FileInfo FatSystem::FindChild(const uint32_t dirCluster, const std::string &name) const
{
    const auto contents = ReadDirectory(dirCluster);
    for (const auto &item: contents)
    {
        if (strcasecmp(item.name.c_str(), name.c_str()) == 0)
        {
            return item;
        }
    }
    throw std::runtime_error("Error: path not found (" + name + ").");
}

std::vector<FileInfo> FatSystem::ReadDirectory(const uint32_t startCluster) const
{
    std::vector<FileInfo> results;
    std::wstring lfnBuffer;
    uint32_t cluster = startCluster;

    while (cluster >= START_OF_DIR && cluster < END_OF_DIR)
    {
        auto data = ReadCluster(cluster);
        for (size_t i = 0; i < data.size(); i += DIR_ENTRY_SIZE)
        {
            ParseDirEntry(data.data() + i, results, lfnBuffer);
        }
        cluster = GetNextCluster(cluster);
    }
    return results;
}

void FatSystem::ParseDirEntry(const uint8_t *ptr, std::vector<FileInfo> &results, std::wstring &lfnBuffer)
{
    if (ptr[0] == 0x00)
    {
        return;
    }
    if (ptr[0] == DELETED_FILE)
    {
        lfnBuffer.clear();
        return;
    }

    const uint8_t attr = ptr[11];
    if (attr == LONG_FILE_NAME)
    {
        HandleLfnEntry(reinterpret_cast<const FatLfnEntry *>(ptr), lfnBuffer);
    } else
    {
        HandleShortEntry(reinterpret_cast<const FatDirEntry *>(ptr), results, lfnBuffer);
    }
}

void FatSystem::HandleLfnEntry(const FatLfnEntry *entry, std::wstring &lfnBuffer)
{
    std::wstring part;
    for (const unsigned short code: entry->name1)
    {
        if (code != PADDING && code != 0x0000)
        {
            part += code;
        }
    }
    for (const unsigned short code: entry->name2)
    {
        if (code != PADDING && code != 0x0000)
        {
            part += code;
        }
    }

    for (const unsigned short code: entry->name3)
    {
        if (code != PADDING && code != 0x0000)
        {
            part += code;
        }
    }

    lfnBuffer.insert(0, part);
}

void FatSystem::HandleShortEntry(const FatDirEntry *entry, std::vector<FileInfo> &results,
                                 std::wstring &lfnBuffer)
{
    if (entry->attr & VOLUME_ID)
    {
        lfnBuffer.clear();
        return;
    }

    FileInfo info;
    info.isDir = (entry->attr & DIR_FLAG) != 0;
    info.size = entry->fileSize;
    info.firstCluster = static_cast<uint32_t>(entry->fstClusHI) << 16 | entry->fstClusLO;

    if (!lfnBuffer.empty())
    {
        info.name = ProcessLfn(lfnBuffer);
    } else
    {
        info.name = ProcessShortName(entry->name);
    }

    results.push_back(info);
    lfnBuffer.clear();
}

std::string FatSystem::ProcessLfn(const std::wstring &lfn)
{
    std::string result;
    for (const wchar_t code: lfn)
    {
        if (code > 127)
        {
            result += '?';
        } else
        {
            result += static_cast<char>(code);
        }
    }
    return result;
}

std::string FatSystem::ProcessShortName(const uint8_t *name)
{
    std::string base;
    std::string ext;
    for (int i = 0; i < 8 && name[i] != ' '; ++i)
    {
        base += static_cast<char>(name[i]);
    }
    for (int i = 8; i < 11 && name[i] != ' '; ++i)
    {
        ext += static_cast<char>(name[i]);
    }
    if (!ext.empty())
    {
        return base + "." + ext;
    }
    return base;
}

std::vector<std::string> FatSystem::SplitPath(std::string &path)
{
    std::ranges::replace(path, '\\', '/');

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(path);

    while (std::getline(stream, token, '/'))
    {
        if (!token.empty() && token != ".")
        {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void FatSystem::PrintDirectory(const FileInfo &dirInfo) const
{
    std::cout << "D .\nD ..\n";

    const auto children = ReadDirectory(dirInfo.firstCluster);
    for (const auto &child: children)
    {
        if (child.name == "." || child.name == "..")
        {
            continue;
        }
        if (child.isDir)
        {
            std::cout << "D " << child.name << "\n";
        } else
        {
            std::cout << "F " << child.name << " " << child.size << "\n";
        }
    }
}

void FatSystem::PrintFile(const FileInfo &fileInfo) const
{
    std::cout << "FILE " << fileInfo.name << " " << fileInfo.size << "\n";

    uint32_t cluster = fileInfo.firstCluster;
    size_t remaining = fileInfo.size;

    while (remaining > 0 && cluster >= 2 && cluster < END_OF_DIR)
    {
        auto data = ReadCluster(cluster);
        const size_t toWrite = std::min(remaining, data.size());
        std::cout.write(reinterpret_cast<char *>(data.data()), static_cast<long int>(toWrite));

        remaining -= toWrite;
        cluster = GetNextCluster(cluster);
    }
}
