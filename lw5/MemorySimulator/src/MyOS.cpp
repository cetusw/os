#include "MyOS.h"
#include "VirtualMemory.h"
#include <iostream>

MyOS::MyOS(PhysicalMemory &physicalMemory)
    : OSHandler(),
      m_physicalMemory(physicalMemory),
      m_nextFreeFrame(1)
{
}

bool MyOS::OnPageFault(
    VirtualMemory &virtualMemory,
    const uint32_t virtualPageNumber,
    Access access,
    PageFaultReason reason)
{
    std::cout << "[OS HANDLER]: Page Fault! Reason: " << static_cast<int>(reason)
            << " on virtual page " << virtualPageNumber << std::endl;

    if (reason != PageFaultReason::NotPresent)
    {
        std::cout << "[OS HANDLER]: Cannot fix this fault. Aborting." << std::endl;
        return false;
    }

    const uint32_t newFrame = m_nextFreeFrame++;

    const uint32_t newFrameAddress = newFrame * (1u << PTE::FRAME_SHIFT);
    if (newFrameAddress >= m_physicalMemory.GetSize())
    {
        std::cout << "[OS HANDLER]: Out of physical memory! Aborting." << std::endl;
        m_nextFreeFrame--;
        return false;
    }

    std::cout << "[OS HANDLER]: Allocating physical frame " << newFrame
            << " for virtual page " << virtualPageNumber << std::endl;

    PTE newPte;
    newPte.SetPresent(true);
    newPte.SetWritable(true);
    newPte.SetUser(true);
    newPte.SetFrame(newFrame);

    const uint32_t pageTableAddress = virtualMemory.GetPageTableAddress();
    const uint32_t pteAddress = pageTableAddress + virtualPageNumber * sizeof(PTE);
    m_physicalMemory.Write32(pteAddress, newPte.raw);

    std::cout << "[OS HANDLER]: PTE updated. Retrying the operation..." << std::endl;
    return true;
}
