#include <windows.h>
#include <VersionHelpers.h>
#include <iomanip>
#include <iostream>
#include <string>

constexpr double BYTE_PER_MB = 1024 * 1024;
constexpr double BYTE_PER_GB = 1024 * 1024 * 1024;
constexpr std::string SLASH_SEPARATOR = "/";
constexpr std::string MB_FREE = "MB free";
constexpr std::string MB_TOTAL = "MB total";
constexpr std::string GB_FREE = "GB free";
constexpr std::string GB_TOTAL = "GB total";


std::string GetOS()
{
	if (IsWindows10OrGreater())
	{
		return "Windows 10 or Greater";
	}
	if (IsWindows8Point1OrGreater())
	{
		return "Windows 8.1 or Greater";
	}
	if (IsWindows8OrGreater())
	{
		return "Windows 8 or Greater";
	}
	if (IsWindows7SP1OrGreater())
	{
		return "Windows 7 with Service Pack 1 or Greater";
	}
	if (IsWindows7OrGreater())
	{
		return "Windows 7 or Greater";
	}
	if (IsWindowsVistaSP2OrGreater())
	{
		return "Windows Vista with Service Pack 2 or Greater";
	}
	if (IsWindowsVistaSP1OrGreater())
	{
		return "Windows Vista with Service Pack 1 or Greater";
	}
	if (IsWindowsVistaOrGreater())
	{
		return "Windows Vista or Greater";
	}
	if (IsWindowsXPSP3OrGreater())
	{
		return "Windows XP with Service Pack 3 or Greater";
	}
	if (IsWindowsXPSP2OrGreater())
	{
		return "Windows XP with Service Pack 2 or Greater";
	}
	if (IsWindowsXPSP1OrGreater())
	{
		return "Windows XP with Service Pack 1 or Greater";
	}
	return "Old version";
}

std::wstring GetCompName()
{
	DWORD bufferSize = 0;
	std::wstring computerName;

	if (!GetComputerNameExW(ComputerNameDnsHostname, nullptr, &bufferSize))
	{
		if (GetLastError() == ERROR_MORE_DATA)
		{
			computerName.resize(bufferSize);
			if (GetComputerNameExW(ComputerNameDnsHostname, computerName.data(), &bufferSize))
			{
				return computerName;
			}
			return L"Error getting computer name";
		}
		return L"Error determining buffer size: " + GetLastError();
	}
	return L"";
}

std::wstring GetUsrName()
{
	DWORD bufferSize = 0;
	std::wstring userName;

	if (!GetUserNameW(nullptr, &bufferSize))
	{
		DWORD lastError = GetLastError();
		if (lastError == ERROR_MORE_DATA)
		{
			userName.resize(bufferSize);
			if (GetUserNameW(userName.data(), &bufferSize))
			{
				if (bufferSize > 1)
				{
					userName.resize(bufferSize - 1);
				}
				else
				{
					userName.clear();
				}
				return userName;
			}
			throw std::runtime_error("Failed to get user name. Error code: " + std::to_string(GetLastError()));
		}
		throw std::runtime_error("Failed to determine buffer size. Error code: " + std::to_string(lastError));
	}
	return L"Unlikely success with null buffer";
}

std::string GetProcessorArchitecture()
{
	SYSTEM_INFO sysInfo;
	GetNativeSystemInfo(&sysInfo);

	switch (sysInfo.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
		return "x64 (AMD or Intel)";
	case PROCESSOR_ARCHITECTURE_ARM:
		return "ARM";
	case PROCESSOR_ARCHITECTURE_ARM64:
		return "ARM64";
	case PROCESSOR_ARCHITECTURE_IA64:
		return "Intel Itanium-based";
	case PROCESSOR_ARCHITECTURE_INTEL:
		return "x86";
	default:
		return "Unknown architecture";
	}
}

MEMORYSTATUSEX GetMemoryInfo()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	if (!GlobalMemoryStatusEx(&memInfo))
	{
		throw std::runtime_error("Failed to get memory information. Error code: " + std::to_string(GetLastError()));
	}

	// const double totalPhysGB = static_cast<double>(memInfo.ullTotalPhys) / BYTE_PER_MB;
	// const double availPhysGB = static_cast<double>(memInfo.ullAvailPhys) / BYTE_PER_MB;
	//
	// std::stringstream ss;
	// ss << std::fixed << std::setprecision(2);
	// ss << totalPhysGB << MB_TOTAL << SLASH_SEPARATOR << availPhysGB << MB_FREE;

	return memInfo;
}

double ByteToMegabyte(const DWORDLONG memory)
{
	std::cout << memory << std::endl;
	return static_cast<double>(memory) / BYTE_PER_MB;
}

int main()
{
	try
	{
		MEMORYSTATUSEX memoryInfo = GetMemoryInfo();

		std::cout << std::left << std::setw(16) << "OS:" << GetOS() << std::endl;
		std::wcout << std::left << std::setw(16) << "Computer Name:" << GetCompName() << std::endl;
		// std::wcout << std::left << std::setw(16) << "User:" << GetUsrName() << std::endl;
		std::cout << std::setw(16) << "Architecture:" << GetProcessorArchitecture() << std::endl;
		std::cout << std::setw(16) << "RAM:"
				  << ByteToMegabyte(memoryInfo.ullTotalPhys) << MB_TOTAL << " " << SLASH_SEPARATOR << " "
				  << ByteToMegabyte(memoryInfo.ullAvailPhys) << MB_FREE << std::endl;

		std::cout << std::setw(16) << "Virtual memory:" << ByteToMegabyte(memoryInfo.ullTotalVirtual) << "MB" << std::endl;
		// std::cout << std::setw(16) << "Memory load:" << virtualMemory << std::endl;
		// std::cout << std::setw(16) << "Pagefile:" << loadAverage << std::endl;
		// std::cout << std::setw(16) << "Processors:" << processorCount << std::endl;
		// std::cout << std::setw(16) << "Drives:" << drives << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}
