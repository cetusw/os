#ifndef MEMORYSIMULATOR_MYOS_H
#define MEMORYSIMULATOR_MYOS_H
#include "OSHandler.h"
#include "PhysicalMemory.h"

class MyOS : public OSHandler
{
public:
    explicit MyOS(PhysicalMemory &physicalMemory);

    bool OnPageFault(VirtualMemory &virtualMemory, uint32_t virtualPageNumber,
                             Access access, PageFaultReason reason) override;
private:
    PhysicalMemory& m_physicalMemory;
    uint32_t m_nextFreeFrame;
};


#endif //MEMORYSIMULATOR_MYOS_H