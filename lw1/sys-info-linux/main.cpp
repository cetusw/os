#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <string>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

// TODO: выяснить posix

constexpr std::string NOT_AVAILABLE = "n/a";
constexpr std::string OS_RELEASE_FILE = "/etc/os-release";
constexpr auto MOUNTS_FILE = "/proc/mounts";
constexpr std::string MEMINFO_FILE = "/proc/meminfo";
constexpr std::string PRETTY_NAME = "PRETTY_NAME";
constexpr int BYTE_PER_MB = 1024 * 1024;
constexpr int BYTE_PER_GB = 1024 * 1024 * 1024;
constexpr std::string SLASH_SEPARATOR = "/";
constexpr std::string MB_FREE = "MB free";
constexpr std::string MB_TOTAL = "MB total";
constexpr std::string GB_FREE = "GB free";
constexpr std::string GB_TOTAL = "GB total";
constexpr std::string VMALLOC_TOTAL = "VmallocTotal";

// TODO: получить информацию о памяти из командной строки

std::string GetOS()
{
	std::ifstream file(OS_RELEASE_FILE);
	if (!file.is_open())
	{
		return NOT_AVAILABLE + ": не удалось открыть " + OS_RELEASE_FILE;
	}
	std::string line;
	while (std::getline(file, line))
	{
		if (!line.starts_with(PRETTY_NAME))
		{
			continue;
		}
		std::string value = line.substr(line.find('=') + 1);
		if (value.length() > 1 && value.front() == '"' && value.back() == '"')
		{
			return value.substr(1, value.length() - 2);
		}
		return value;
	}
	return NOT_AVAILABLE;
}

utsname GetUtsname()
{
	utsname buffer{};
	if (uname(&buffer) != 0)
	{
		throw std::runtime_error("uname() завершился с ошибкой");
	}
	return buffer;
}

// использовать функции с w

// если метод возвращает указатель, то нужно убедиться, что его не нужно уничтожать
std::string GetUser()
{
	const passwd* pw = getpwuid(getuid());
	if (pw == nullptr)
	{
		return NOT_AVAILABLE;
	}
	return pw->pw_name;
}

struct sysinfo GetSystemInfo()
{
	struct sysinfo info{};
	if (sysinfo(&info) != 0)
	{
		throw std::runtime_error("sysinfo() завершился с ошибкой");
	}

	return info;
}

unsigned long ByteToMegabyte(const unsigned long value)
{
	return value / BYTE_PER_MB;
}

std::string GetVirtualMemory()
{
	std::ifstream file(MEMINFO_FILE);
	if (!file.is_open())
	{
		return NOT_AVAILABLE + ": не удалось открыть " + MEMINFO_FILE;
	}
	std::string line;
	while (std::getline(file, line))
	{
		if (!line.starts_with(VMALLOC_TOTAL))
		{
			continue;
		}
		std::string value = line.substr(line.find(':') + 1);
		const size_t first = value.find_first_not_of(" \t");
		value = value.substr(first, line.find(' ') + 1 - first);
		return std::stod(value) / 1024;
		// const size_t first = value.find_first_not_of(" \t");
		// const size_t last = value.find_last_not_of(" \t");
		// return value.substr(first, last - first + 1);
	}
	return NOT_AVAILABLE;
}

std::string GetProcessorCount()
{
	return std::to_string(get_nprocs());
}

std::string GetLoadAverage(const struct sysinfo& info)
{
	std::stringstream ss;
	const double scale = std::pow(2, SI_LOAD_SHIFT);
	ss << std::fixed << std::setprecision(2)
	   << static_cast<double>(info.loads[0]) / scale << ", "
	   << static_cast<double>(info.loads[1]) / scale << ", "
	   << static_cast<double>(info.loads[2]) / scale;
	return ss.str();
}

std::string GetDrives()
{
	std::stringstream ss;
	std::ifstream file(MOUNTS_FILE);

	if (!file.is_open())
	{
		return NOT_AVAILABLE + ": не удалось открыть " + MOUNTS_FILE;
	}

	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream ls(line);
		std::string fsname, dir, type, opts;
		int freq, passno;

		if (!(ls >> fsname >> dir >> type >> opts >> freq >> passno))
		{
			continue;
		}

		struct statvfs vfsStats{};
		if (statvfs(dir.c_str(), &vfsStats) != 0)
		{
			continue;
		}

		const unsigned long totalSpace = vfsStats.f_blocks * vfsStats.f_frsize / BYTE_PER_GB;
		if (totalSpace == 0)
		{
			continue;
		}

		const unsigned long freeSpace = vfsStats.f_bavail * vfsStats.f_frsize / BYTE_PER_GB;

		ss << "\n\t" << std::left << std::setw(25) << dir
		   << std::setw(10) << type
		   << std::right << std::setw(12) << freeSpace << GB_FREE << " " << SLASH_SEPARATOR << " "
		   << std::left << totalSpace << GB_TOTAL;
	}

	return ss.str();
}

int main()
{
	try
	{
		const utsname utsname = GetUtsname();
		const std::string OSInfo = GetOS();
		const std::string user = GetUser();
		const struct sysinfo systemInfo = GetSystemInfo();
		const std::string virtualMemory = GetVirtualMemory();
		const std::string processorCount = GetProcessorCount();
		const std::string loadAverage = GetLoadAverage(systemInfo);
		const std::string drives = GetDrives();

		std::cout << std::left << std::setw(16) << "OS:" << OSInfo << std::endl;
		std::cout << std::setw(16) << "Kernel:" << utsname.sysname << " " << utsname.release << std::endl;
		std::cout << std::setw(16) << "Architecture:" << utsname.machine << std::endl;
		std::cout << std::setw(16) << "Hostname:" << utsname.nodename << std::endl;
		std::cout << std::setw(16) << "User:" << user << std::endl;
		std::cout << std::setw(16) << "RAM:"
				  << ByteToMegabyte(systemInfo.freeram) << MB_FREE << " " << SLASH_SEPARATOR
				  << ByteToMegabyte(systemInfo.totalram) << MB_TOTAL << std::endl;
		std::cout << std::setw(16) << "Swap:"
				  << ByteToMegabyte(systemInfo.freeswap) << MB_FREE << " " << SLASH_SEPARATOR
				  << ByteToMegabyte(systemInfo.totalswap) << MB_TOTAL << std::endl;
		std::cout << std::setw(16) << "Virtual memory:" << virtualMemory << std::endl;
		std::cout << std::setw(16) << "Processors:" << processorCount << std::endl;
		std::cout << std::setw(16) << "Load average:" << loadAverage << std::endl;
		std::cout << std::setw(16) << "Drives:" << drives << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}