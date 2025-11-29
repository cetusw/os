#include <iostream>
#include <cassert>

#include "MyOS.h"
#include "PhysicalMemory.h"
#include "VirtualMemory.h"

void RunSimulation()
{
    std::cout << "--- Starting Simulation ---" << std::endl;

    PhysicalMemoryConfig config;
    config.numFrames = 256;
    PhysicalMemory physicalMemory(config);

    MyOS osHandler(physicalMemory);
    VirtualMemory virtualMemory(physicalMemory, osHandler);

    constexpr uint32_t pageTableAddress = 0;
    virtualMemory.SetPageTableAddress(pageTableAddress);

    constexpr uint32_t virtualAddressToTest = 0x12345678;
    constexpr uint32_t valueToWrite = 0xDEADBEEF;

    std::cout << "\nAttempting to write " << std::hex << valueToWrite
              << " to virtual address " << virtualAddressToTest << "..." << std::endl;

    virtualMemory.Write32(virtualAddressToTest, valueToWrite, Privilege::User);

    std::cout << "Write operation completed." << std::endl;

    std::cout << "\nAttempting to read from virtual address "
              << virtualAddressToTest << "..." << std::endl;

    const uint32_t readValue = virtualMemory.Read32(virtualAddressToTest, Privilege::User);

    std::cout << "Read value: " << std::hex << readValue << std::endl;

    assert(readValue == valueToWrite);

    std::cout << "\n--- Ending Simulation ---" << std::endl;

    constexpr uint32_t vpn = virtualAddressToTest >> PTE::FRAME_SHIFT;
    constexpr uint32_t pteAddress = pageTableAddress + vpn * sizeof(PTE);
    PTE finalPte;
    finalPte.raw = physicalMemory.Read32(pteAddress);

    std::cout << "\nFinal PTE raw value: " << std::hex << finalPte.raw << std::endl;
    assert(finalPte.IsPresent());
    assert(finalPte.IsAccessed());
    assert(finalPte.IsDirty());
    std::cout << "PTE flags (P, A, D) are correctly set." << std::endl;
}


int main()
{
    try
    {
        RunSimulation();
    }
    catch (const std::exception& e)
    {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}