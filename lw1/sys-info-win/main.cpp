#include <VersionHelpers.h>
#include <iomanip>
#include <iostream>
#include <psapi.h>
#include <string>
#include <vector>
#include <windows.h>

constexpr double BYTE_PER_KB = 1024;
constexpr double BYTE_PER_MB = BYTE_PER_KB * 1024;
constexpr double BYTE_PER_GB = BYTE_PER_MB * 1024;
constexpr std::string SEPARATOR = "/";
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

SYSTEM_INFO GetSysInfo()
{
	SYSTEM_INFO sysInfo;
	GetNativeSystemInfo(&sysInfo);
	return sysInfo;
}

std::string GetProcessorArchitecture(const SYSTEM_INFO& sysInfo)
{
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

	return memInfo;
}

double ByteToMegabyte(const DWORDLONG memory)
{
	return static_cast<double>(memory) / BYTE_PER_MB;
}

PERFORMANCE_INFORMATION GetPerfInfo()
{
	PERFORMANCE_INFORMATION perfInfo;
	perfInfo.cb = sizeof(PERFORMANCE_INFORMATION);

	if (!GetPerformanceInfo(&perfInfo, sizeof(PERFORMANCE_INFORMATION)))
	{
		std::cout << "Failed to get performance information." << std::endl;
		return {};
	}

	return perfInfo;
}

std::wstring GetDrivesInfo()
{
	std::wstringstream drivesInfo;
	const DWORD bufferSize = GetLogicalDriveStringsW(0, nullptr);
	if (bufferSize == 0)
	{
		throw std::runtime_error("Failed to get logical drive strings size. Error: " + GetLastError());
	}

	std::vector<wchar_t> buffer(bufferSize);
	if (GetLogicalDriveStringsW(bufferSize, buffer.data()) == 0)
	{
		throw std::runtime_error("Failed to get logical drive strings. Error: " + GetLastError());
	}

	const wchar_t* currentDrive = buffer.data();
	while (*currentDrive)
	{
		ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes;

		if (!GetDiskFreeSpaceExW(currentDrive, &freeBytesAvailable, &totalNumberOfBytes, nullptr))
		{
			drivesInfo << L"  - " << currentDrive << L" : Failed to get disk info." << std::endl;
			currentDrive += wcslen(currentDrive) + 1;
			continue;
		}

		wchar_t fileSystemNameBuffer[MAX_PATH + 1] = {};
		if (!GetVolumeInformationW(currentDrive, nullptr, 0, nullptr, nullptr, nullptr, fileSystemNameBuffer, MAX_PATH + 1))
		{
			wcscpy_s(fileSystemNameBuffer, L"N/A");
		}

		const double freeGB = static_cast<double>(freeBytesAvailable.QuadPart) / BYTE_PER_GB;
		const double totalGB = static_cast<double>(totalNumberOfBytes.QuadPart) / BYTE_PER_GB;

		drivesInfo << L"  - " << currentDrive << L"  (" << fileSystemNameBuffer << L"): "
				   << std::fixed << std::setprecision(0) << freeGB << L" GB free / "
				   << totalGB << L" GB total" << std::endl;

		currentDrive += wcslen(currentDrive) + 1;
	}

	return drivesInfo.str();
}

int main()
{
	try
	{
		const MEMORYSTATUSEX memoryInfo = GetMemoryInfo();
		const PERFORMACE_INFORMATION perfInfo = GetPerfInfo();
		const SYSTEM_INFO sysInfo = GetSysInfo();

		const double virtualMemoryMB = ByteToMegabyte(perfInfo.CommitLimit * perfInfo.PageSize);
		const double pageFileMB = virtualMemoryMB - ByteToMegabyte(memoryInfo.ullTotalPhys);

		std::cout << std::left << std::setw(16) << "OS:" << GetOS() << std::endl;
		std::wcout << std::left << std::setw(16) << "Computer Name:" << GetCompName() << std::endl;
		// std::wcout << std::left << std::setw(16) << "User:" << GetUsrName() << std::endl;
		std::cout << std::setw(16) << "Architecture:" << GetProcessorArchitecture(sysInfo) << std::endl;
		std::cout << std::setw(16) << "RAM:"
				  << ByteToMegabyte(memoryInfo.ullAvailPhys) << MB_FREE << " " << SEPARATOR << " "
				  << ByteToMegabyte(memoryInfo.ullTotalPhys) << MB_TOTAL << std::endl;
		std::cout << std::setw(16) << "Virtual memory:" << virtualMemoryMB << "MB" << std::endl;
		std::cout << std::setw(16) << "Memory load:" << memoryInfo.dwMemoryLoad << "%" << std::endl;
		std::cout << std::setw(16) << "Pagefile:"
				  << pageFileMB << "MB" << " " << SEPARATOR << " "
				  << virtualMemoryMB << "MB" << std::endl;
		std::cout << std::setw(16) << "Processors:" << sysInfo.dwNumberOfProcessors << std::endl;
		std::wcout << std::setw(16) << L"Drives:" << std::endl
				   << GetDrivesInfo() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}
