#include "SysInfo.h"
#include <thread>

constexpr uint64_t BYTE_PER_KB = 1024;
constexpr uint64_t BYTE_PER_MB = BYTE_PER_KB * 1024;
constexpr uint64_t BYTE_PER_GB = BYTE_PER_MB * 1024;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <VersionHelpers.h>

#elif __linux__
#include <fstream>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

constexpr std::string OS_RELEASE_FILE = "/etc/os-release";
constexpr std::string NOT_AVAILABLE = "n/a";
constexpr std::string PRETTY_NAME = "PRETTY_NAME";
#endif

std::string SysInfo::GetOSName() const
{
#ifdef _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#endif
}

std::string SysInfo::GetOSVersion() const
{
#ifdef _WIN32
    if (IsWindows10OrGreater())
    {
        return "10 or Greater";
    }
    if (IsWindows8Point1OrGreater())
    {
        return "8.1 or Greater";
    }
    if (IsWindows8OrGreater())
    {
        return "8 or Greater";
    }
    if (IsWindows7SP1OrGreater())
    {
        return "7 with Service Pack 1 or Greater";
    }
    if (IsWindows7OrGreater())
    {
        return "7 or Greater";
    }
    if (IsWindowsVistaSP2OrGreater())
    {
        return "Vista with Service Pack 2 or Greater";
    }
    if (IsWindowsVistaSP1OrGreater())
    {
        return "Vista with Service Pack 1 or Greater";
    }
    if (IsWindowsVistaOrGreater())
    {
        return "Vista or Greater";
    }
    if (IsWindowsXPSP3OrGreater())
    {
        return "XP with Service Pack 3 or Greater";
    }
    if (IsWindowsXPSP2OrGreater())
    {
        return "XP with Service Pack 2 or Greater";
    }
    if (IsWindowsXPSP1OrGreater())
    {
        return "XP with Service Pack 1 or Greater";
    }
    return "Old version";
#elif __linux__
    utsname buffer{};
    if (uname(&buffer) != 0)
    {
        throw std::runtime_error("uname() завершился с ошибкой");
    }
    return buffer.release;
#endif
}

uint64_t SysInfo::GetFreeMemory() const
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memInfo))
    {
        throw std::runtime_error("Failed to get memory information. Error code: " + std::to_string(GetLastError()));
    }

    return memInfo.ullAvailPhys / BYTE_PER_MB;
#elif __linux__
    struct sysinfo info{};
    if (sysinfo(&info) != 0)
    {
        throw std::runtime_error("sysinfo() завершился с ошибкой");
    }

    return info.freeram / BYTE_PER_MB;
#endif
}

uint64_t SysInfo::GetTotalMemory() const
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memInfo))
    {
        throw std::runtime_error("Failed to get memory information. Error code: " + std::to_string(GetLastError()));
    }

    return memInfo.ullTotalPhys / BYTE_PER_MB;
#elif __linux__
    struct sysinfo info{};
    if (sysinfo(&info) != 0)
    {
        throw std::runtime_error("sysinfo() завершился с ошибкой");
    }

    return info.totalram / BYTE_PER_MB;
#endif
}

unsigned SysInfo::GetProcessorCount() const
{
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#elif __linux__
    return get_nprocs();
#endif
}
