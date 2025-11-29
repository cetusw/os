#include "VirtualMemory.h"

uint32_t VirtualMemory::GetPageTableAddress() const noexcept
{
    // TODO
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
    const uint32_t vpn = virtualAddress >> PTE::FRAME_SHIFT;
    const uint32_t offset = virtualAddress & 0xFFF;

    const uint32_t pteAddress = m_pageTableAddress + vpn * sizeof(PTE);

    PTE pte;
    pte.raw = m_physicalMemory.Read32(pteAddress);

    TranslationResult result;
    result.pte = pte;
    CheckAccess(result, pte, access, privilege, execute);

    const uint32_t pfn = pte.GetFrame();
    const uint32_t physicalAddress = pfn << PTE::FRAME_SHIFT | offset;

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

    if (execute && pte.IsNX()) {
        result.success = false;
        result.faultReason = PageFaultReason::ExecOnNX;
        return false;
    }

    return true;
}
