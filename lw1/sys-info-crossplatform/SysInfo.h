#ifndef SYSINFO_H
#define SYSINFO_H
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



#endif //SYSINFO_H
