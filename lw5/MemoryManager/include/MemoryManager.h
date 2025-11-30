#pragma once

#include <cstddef>
#include <mutex>

class MemoryManager
{
public:
	MemoryManager(void* start, size_t size) noexcept;

	MemoryManager(const MemoryManager&) = delete;

	MemoryManager& operator=(const MemoryManager&) = delete;

	void* Allocate(size_t size, size_t align = sizeof(std::max_align_t)) noexcept;

	void Free(void* addr) noexcept;

private:
	struct BlockHeader
	{
		BlockHeader* Prev;
		BlockHeader* Next;
		size_t Size;
		bool IsFree;
	};

	BlockHeader* m_blockHeader;
	std::mutex m_mutex;

	[[nodiscard]] BlockHeader* FindFreeBlock(size_t size, size_t align) const noexcept;

	static uintptr_t CalculateAlignedAddress(BlockHeader* block, size_t align) noexcept;

	static BlockHeader* SplitBlock(BlockHeader* block, uintptr_t alignedHeaderAddr, size_t requestedSize) noexcept;

	static void Coalesce(BlockHeader* block) noexcept;

	static BlockHeader* GetHeader(void* addr) noexcept;
};
