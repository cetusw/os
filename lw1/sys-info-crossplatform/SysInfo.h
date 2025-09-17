#ifndef SYS_INFO_CROSSPLATFORM_SYSINFO_H
#define SYS_INFO_CROSSPLATFORM_SYSINFO_H
#include <cstdint>
#include <string>

class SysInfo
{
public:
    [[nodiscard]] std::string GetOSName() const;
    [[nodiscard]] std::string GetOSVersion() const;
    [[nodiscard]] uint64_t GetFreeMemory() const;
    [[nodiscard]] uint64_t GetTotalMemory() const;
    [[nodiscard]] unsigned GetProcessorCount() const;
};

#endif //SYS_INFO_CROSSPLATFORM_SYSINFO_H