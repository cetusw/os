#ifndef MEMORYSIMULATOR_OSHANDLER_H
#define MEMORYSIMULATOR_OSHANDLER_H
#include <cstdint>
class VirtualMemory;


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
    virtual ~OSHandler() = default;

    virtual bool OnPageFault(VirtualMemory &virtualMemory, uint32_t virtualPageNumber,
                             Access access, PageFaultReason reason) = 0;
};


#endif //MEMORYSIMULATOR_OSHANDLER_H
