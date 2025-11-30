#include "MemoryManager.h"
#include <memory>

MemoryManager::MemoryManager(void* start, const size_t size) noexcept
{
	if (size < sizeof(BlockHeader))
	{
		m_blockHeader = nullptr;
		return;
	}

	size_t space = size;
	void* ptr = start;
	if (!std::align(alignof(BlockHeader), sizeof(BlockHeader), ptr, space))
	{
		m_blockHeader = nullptr;
		return;
	}

	m_blockHeader = static_cast<BlockHeader*>(ptr);
	m_blockHeader->Prev = nullptr;
	m_blockHeader->Next = nullptr;
	m_blockHeader->Size = space - sizeof(BlockHeader);
	m_blockHeader->IsFree = true;
}

void* MemoryManager::Allocate(const size_t size, const size_t align) noexcept
{
	if (!m_blockHeader || size == 0)
	{
		return nullptr;
	}

	std::unique_lock lock(m_mutex);
	BlockHeader* block = FindFreeBlock(size, align);

	if (!block)
	{
		return nullptr;
	}

	const uintptr_t alignedHeader = CalculateAlignedAddress(block, align);
	BlockHeader* usedBlock = SplitBlock(block, alignedHeader, size);
	lock.unlock();
	usedBlock->IsFree = false;

	return reinterpret_cast<char*>(usedBlock) + sizeof(BlockHeader);
}

void MemoryManager::Free(void* addr) noexcept
{
	if (!addr)
	{
		return;
	}

	std::unique_lock lock(m_mutex);
	BlockHeader* header = GetHeader(addr);
	if (!header)
	{
		return;
	}
	header->IsFree = true;
	Coalesce(header);
	lock.unlock();
}

MemoryManager::BlockHeader* MemoryManager::FindFreeBlock(const size_t size, const size_t align) const noexcept
{
	BlockHeader* curr = m_blockHeader;
	while (curr)
	{
		if (curr->IsFree)
		{
			const uintptr_t alignedHeader = CalculateAlignedAddress(curr, align);
			const uintptr_t dataStart = alignedHeader + sizeof(BlockHeader);
			const uintptr_t blockEnd = reinterpret_cast<uintptr_t>(curr) + sizeof(BlockHeader) + curr->Size;

			if (dataStart + size <= blockEnd)
			{
				return curr;
			}
		}
		curr = curr->Next;
	}
	return nullptr;
}

uintptr_t MemoryManager::CalculateAlignedAddress(BlockHeader* block, const size_t align) noexcept
{
	const uintptr_t rawAddr = reinterpret_cast<uintptr_t>(block) + sizeof(BlockHeader);
	const uintptr_t mask = align - 1;
	uintptr_t alignedData = rawAddr + mask & ~mask;

	uintptr_t alignedHeader = alignedData - sizeof(BlockHeader);

	while (alignedHeader < reinterpret_cast<uintptr_t>(block) || (alignedHeader > reinterpret_cast<uintptr_t>(block) && alignedHeader - reinterpret_cast<uintptr_t>(block) < sizeof(BlockHeader)))
	{
		alignedData += align;
		alignedHeader = alignedData - sizeof(BlockHeader);
	}

	return alignedHeader;
}

MemoryManager::BlockHeader* MemoryManager::SplitBlock(BlockHeader* block, const uintptr_t alignedHeaderAddr, const size_t requestedSize) noexcept
{
	if (alignedHeaderAddr > reinterpret_cast<uintptr_t>(block))
	{
		auto* newBlock = reinterpret_cast<BlockHeader*>(alignedHeaderAddr);
		const size_t prevSize = alignedHeaderAddr - reinterpret_cast<uintptr_t>(block) - sizeof(BlockHeader);
		const size_t totalSize = block->Size;

		newBlock->Size = totalSize - (prevSize + sizeof(BlockHeader));
		newBlock->Prev = block;
		newBlock->Next = block->Next;
		newBlock->IsFree = true;

		if (block->Next)
		{
			block->Next->Prev = newBlock;
		}
		block->Next = newBlock;
		block->Size = prevSize;

		block = newBlock;
	}

	const size_t requiredTotal = requestedSize;
	if (block->Size >= requiredTotal + sizeof(BlockHeader))
	{
		const uintptr_t splitAddr = reinterpret_cast<uintptr_t>(block) + sizeof(BlockHeader) + requiredTotal;
		auto* nextBlock = reinterpret_cast<BlockHeader*>(splitAddr);

		nextBlock->Size = block->Size - requiredTotal - sizeof(BlockHeader);
		nextBlock->Prev = block;
		nextBlock->Next = block->Next;
		nextBlock->IsFree = true;

		if (block->Next)
			block->Next->Prev = nextBlock;
		block->Next = nextBlock;
		block->Size = requestedSize;
	}

	return block;
}

void MemoryManager::Coalesce(BlockHeader* block) noexcept
{
	if (block->Next && block->Next->IsFree)
	{
		const BlockHeader* next = block->Next;
		block->Size += sizeof(BlockHeader) + next->Size;
		block->Next = next->Next;
		if (block->Next)
		{
			block->Next->Prev = block;
		}
	}

	if (block->Prev && block->Prev->IsFree)
	{
		BlockHeader* prev = block->Prev;
		prev->Size += sizeof(BlockHeader) + block->Size;
		prev->Next = block->Next;
		if (prev->Next)
		{
			prev->Next->Prev = prev;
		}
	}
}

MemoryManager::BlockHeader* MemoryManager::GetHeader(void* addr) noexcept
{
	return reinterpret_cast<BlockHeader*>(static_cast<char*>(addr) - sizeof(BlockHeader));
}