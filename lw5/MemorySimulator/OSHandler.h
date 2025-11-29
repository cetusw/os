#ifndef MEMORYSIMULATOR_OSHANDLER_H
#define MEMORYSIMULATOR_OSHANDLER_H
#include "VirtualMemory.h"


enum class Access
{
    Read,
    Write,
};

enum class PageFaultReason
{
    NotPresent,
    WriteToReadOnly,
    ExecOnNX,
    UserAccessToSupervisor,
    PhysicalAccessOutOfRange,
    MisalignedAccess,
};

class OSHandler
{
public:
    OSHandler(PhysicalMemory &pm)
        : m_physicalMemory(pm), m_nextFreeFrame(1) // Начинаем с 1-го, 0-й часто зарезервирован
    {
    }

    virtual ~OSHandler() = default;

    virtual bool OnPageFault(VirtualMemory &virtualMemory, uint32_t virtualPageNumber,
                             Access access, PageFaultReason reason) = 0;

    // Разместите здесь прочие методы-обработчики, если нужно
private:
    PhysicalMemory& m_physicalMemory;
    uint32_t m_nextFreeFrame;
};


#endif //MEMORYSIMULATOR_OSHANDLER_H
