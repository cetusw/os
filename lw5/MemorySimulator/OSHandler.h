#ifndef MEMORYSIMULATOR_OSHANDLER_H
#define MEMORYSIMULATOR_OSHANDLER_H
#include "VirtualMemory.h"


enum class Access
{
    Read,
    Write,
  };

enum class PageFaultReason {
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
    virtual ~OSHandler() = default;
    virtual bool OnPageFault(VirtualMemory& vm, uint32_t virtualPageNumber,
                             Access access, PageFaultReason reason) = 0;
    // Разместите здесь прочие методы-обработчики, если нужно
};


#endif //MEMORYSIMULATOR_OSHANDLER_H