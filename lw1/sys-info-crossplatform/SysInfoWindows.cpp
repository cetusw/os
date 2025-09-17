#include <stdexcept>

#include "SysInfo.h"
#include <windows.h>
#include <VersionHelpers.h>

constexpr uint64_t BYTE_PER_KB = 1024;
constexpr uint64_t BYTE_PER_MB = BYTE_PER_KB * 1024;
constexpr uint64_t BYTE_PER_GB = BYTE_PER_MB * 1024;

std::string SysInfo::GetOSName() const
{
    return "Windows";
}

std::string SysInfo::GetOSVersion() const
{
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
}

uint64_t SysInfo::GetFreeMemory() const
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memInfo))
    {
        throw std::runtime_error("Failed to get memory information. Error code: " + std::to_string(GetLastError()));
    }

    return memInfo.ullAvailPhys / BYTE_PER_MB;
}

uint64_t SysInfo::GetTotalMemory() const
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&memInfo))
    {
        throw std::runtime_error("Failed to get memory information. Error code: " + std::to_string(GetLastError()));
    }

    return memInfo.ullTotalPhys / BYTE_PER_MB;
}

unsigned SysInfo::GetProcessorCount() const
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}
