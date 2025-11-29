#include "VirtualMemory.h"

VirtualMemory::VirtualMemory(PhysicalMemory &physicalMemory, OSHandler &handler)
    : m_physicalMemory(physicalMemory),
      m_handler(handler)
{
}

void VirtualMemory::SetPageTableAddress(const uint32_t physicalAddress)
{
    if ((physicalAddress & 0xFFF) != 0)
    {
        throw std::invalid_argument("Page table address must be aligned to 4096 bytes.");
    }
    m_pageTableAddress = physicalAddress;
}

uint32_t VirtualMemory::GetPageTableAddress() const noexcept
{
    return m_pageTableAddress;
}

uint8_t VirtualMemory::Read8(const uint32_t address, const Privilege privilege, const bool execute) const
{
    return Read<uint8_t>(address, privilege, execute);
}

uint16_t VirtualMemory::Read16(const uint32_t address, const Privilege privilege, const bool execute) const
{
    return Read<uint16_t>(address, privilege, execute);
}

uint32_t VirtualMemory::Read32(const uint32_t address, const Privilege privilege, const bool execute) const
{
    return Read<uint32_t>(address, privilege, execute);
}

uint64_t VirtualMemory::Read64(const uint32_t address, const Privilege privilege, const bool execute) const
{
    return Read<uint64_t>(address, privilege, execute);
}

void VirtualMemory::Write8(const uint32_t address, const uint8_t value, const Privilege privilege)
{
    Write<uint8_t>(address, value, privilege);
}

void VirtualMemory::Write16(const uint32_t address, const uint16_t value, const Privilege privilege)
{
    Write<uint16_t>(address, value, privilege);
}

void VirtualMemory::Write32(const uint32_t address, const uint32_t value, const Privilege privilege)
{
    Write<uint32_t>(address, value, privilege);
}

void VirtualMemory::Write64(const uint32_t address, const uint64_t value, const Privilege privilege)
{
    Write<uint64_t>(address, value, privilege);
}

TranslationResult VirtualMemory::TranslateAddress(
    const uint32_t virtualAddress,
    const Access access,
    const Privilege privilege,
    const bool execute) const
{
    const uint32_t virtualPageNumber = virtualAddress >> PTE::FRAME_SHIFT;
    const uint32_t offset = virtualAddress & 0xFFF;

    const uint32_t pteAddress = m_pageTableAddress + virtualPageNumber * sizeof(PTE);

    PTE pageTableEntry;
    pageTableEntry.raw = m_physicalMemory.Read32(pteAddress);

    TranslationResult result;
    result.pte = pageTableEntry;
    if (!CheckAccess(result, pageTableEntry, access, privilege, execute)) {
        return result;
    }

    const uint32_t pageFrameNumber = pageTableEntry.GetFrame();
    const uint32_t physicalAddress = pageFrameNumber << PTE::FRAME_SHIFT | offset;

    result.success = true;
    result.physicalAddress = physicalAddress;

    return result;
}

bool VirtualMemory::CheckAccess(
    TranslationResult &result,
    const PTE &pte,
    const Access access,
    const Privilege privilege,
    const bool execute)
{
    if (!pte.IsPresent())
    {
        result.success = false;
        result.faultReason = PageFaultReason::NotPresent;
        return false;
    }

    if (privilege == Privilege::User && !pte.IsUser())
    {
        result.success = false;
        result.faultReason = PageFaultReason::UserAccessToSupervisor;
        return false;
    }

    if (access == Access::Write && !pte.IsWritable())
    {
        result.success = false;
        result.faultReason = PageFaultReason::WriteToReadOnly;
        return false;
    }

    if (execute && pte.IsNX())
    {
        result.success = false;
        result.faultReason = PageFaultReason::ExecOnNX;
        return false;
    }

    return true;
}
