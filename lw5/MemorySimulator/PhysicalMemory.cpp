#include "PhysicalMemory.h"

PhysicalMemory::PhysicalMemory(const PhysicalMemoryConfig cfg)
{
    m_memory.resize(static_cast<size_t>(cfg.numFrames) * cfg.frameSize, 0);
}

uint32_t PhysicalMemory::GetSize() const noexcept
{
    return m_memory.size();
}

uint8_t PhysicalMemory::Read8(const uint32_t address) const
{
    return Read<uint8_t>(address);
}

uint16_t PhysicalMemory::Read16(const uint32_t address) const
{
    return Read<uint16_t>(address);
}

uint32_t PhysicalMemory::Read32(const uint32_t address) const
{
    return Read<uint32_t>(address);
}

uint64_t PhysicalMemory::Read64(const uint32_t address) const
{
    return Read<uint64_t>(address);
}

void PhysicalMemory::Write8(const uint32_t address, const uint8_t value)
{
    Write<uint8_t>(address, value);
}

void PhysicalMemory::Write16(const uint32_t address, const uint16_t value)
{
    Write<uint16_t>(address, value);
}

void PhysicalMemory::Write32(const uint32_t address, const uint32_t value)
{
    Write<uint32_t>(address, value);
}

void PhysicalMemory::Write64(const uint32_t address, const uint64_t value)
{
    Write<uint64_t>(address, value);
}
