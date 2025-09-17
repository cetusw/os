#include "SysInfo.h"
#include <thread>
#include <fstream>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

constexpr uint64_t BYTE_PER_KB = 1024;
constexpr uint64_t BYTE_PER_MB = BYTE_PER_KB * 1024;
constexpr uint64_t BYTE_PER_GB = BYTE_PER_MB * 1024;
constexpr std::string OS_RELEASE_FILE = "/etc/os-release";
constexpr std::string NOT_AVAILABLE = "n/a";
constexpr std::string PRETTY_NAME = "PRETTY_NAME";

utsname GetUtsname()
{
    utsname buffer{};
    if (uname(&buffer) != 0)
    {
        throw std::runtime_error("uname() завершился с ошибкой");
    }
    return buffer;
}

std::string SysInfo::GetOSName() const
{
    return GetUtsname().sysname;
}

std::string SysInfo::GetOSVersion() const
{
    utsname buffer{};
    if (uname(&buffer) != 0)
    {
        throw std::runtime_error("uname() завершился с ошибкой");
    }
    return buffer.release;
}

uint64_t SysInfo::GetFreeMemory() const
{
    struct sysinfo info{};
    if (sysinfo(&info) != 0)
    {
        throw std::runtime_error("sysinfo() завершился с ошибкой");
    }

    return info.freeram / BYTE_PER_MB;
}

uint64_t SysInfo::GetTotalMemory() const
{
    struct sysinfo info{};
    if (sysinfo(&info) != 0)
    {
        throw std::runtime_error("sysinfo() завершился с ошибкой");
    }

    return info.totalram / BYTE_PER_MB;
}

unsigned SysInfo::GetProcessorCount() const
{
    return get_nprocs();
}
