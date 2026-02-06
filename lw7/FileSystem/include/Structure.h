#ifndef FILESYSTEM_STRUCTURE_H
#define FILESYSTEM_STRUCTURE_H

#include <cstdint>

constexpr char SIGNATURE[] = "MYFS1";
constexpr uint32_t BLOCK_SIZE = 4096;
constexpr uint32_t MAX_FILENAME = 255;
constexpr uint64_t MAX_FILE_SIZE = 2ULL * 1024 * 1024 * 1024;

#pragma pack(push, 1)
struct Superblock
{
	char signature[8]{};
	uint32_t version{};
	uint32_t maxFiles{};
	uint32_t blockSize{};
	uint32_t totalBlocks{};
	uint32_t fileTableOffset{};
	uint32_t dataAreaOffset{};
};

struct FileEntry
{
	char name[MAX_FILENAME + 1];
	uint32_t size;
	uint32_t startBlock;
	uint32_t blocksCount;
	bool isUsed;
};
#pragma pack(pop)

#endif // FILESYSTEM_STRUCTURE_H
