#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <string>
#include <vector>

constexpr unsigned long BYTE_PER_KB = 1024;
constexpr unsigned long BYTE_PER_MB = BYTE_PER_KB * 1024;
constexpr unsigned long BYTE_PER_GB = BYTE_PER_MB * 1024;
constexpr unsigned long KB_PER_MB = 1024;
constexpr std::string SOURCE_DIR = "/proc";
constexpr std::string PROCESS_NAME_FILE = "/comm";
constexpr std::string CMD_LINE_FILE = "/cmdline";
constexpr std::string STATUS_FILE = "/status";
constexpr std::string SMAPS_ROLLUP_FILE = "/smaps_rollup";
constexpr std::string STAT_FILE = "/stat";
constexpr std::string UID_FIELD = "Uid:";
constexpr std::string SHARED_CLEAN = "Shared_Clean:";
constexpr std::string SHARED_DIRTY = "Shared_Dirty:";
constexpr std::string PRIVATE_CLEAN = "Private_Clean:";
constexpr std::string PRIVATE_DIRTY = "Private_Dirty:";
constexpr int COLUMN_WIDTH_SHIFT = 2;
constexpr std::string UNKNOWN = "unknown";

struct Process
{
	std::string pid;
	std::string name;
	std::string user;
	int sharedMemoryKB = 0;
	int privateMemoryKB = 0;
	float cpu = 0.0f;
};

struct ColumnWidth
{
	int pidWidth = 0;
	int nameWidth = 0;
	int userWidth = 0;
	int sharedWidth = 0;
	int privateWidth = 0;
	int cpuWidth = 0;
};

bool IsProcessAlive(const std::string& pid)
{
	const std::filesystem::path procDir = SOURCE_DIR + "/" + pid;
	return std::filesystem::exists(procDir) && std::filesystem::is_directory(procDir);
}

unsigned long KilobyteToMegabyte(const unsigned long value)
{
	return value / KB_PER_MB;
}

bool isNumber(const std::string& s)
{
	return !s.empty() && std::ranges::all_of(s, ::isdigit);
}

std::string Trim(const std::string& str)
{
	const size_t start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos)
	{
		return "";
	}
	const size_t end = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(start, end - start + 1);
}

std::string ParseValueWithoutUnits(const std::string& str)
{
	std::string numStr;
	for (const char c : str)
	{
		if (std::isdigit(c) || c == '-')
		{
			numStr += c;
		}
	}
	if (numStr.empty())
	{
		return "0";
	}
	return numStr;
}

std::string GetValueFromFileField(const std::string& fileName, const std::string& fieldName)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << fileName << std::endl;
		return UNKNOWN;
	}
	std::string line;
	while (std::getline(file, line))
	{
		if (!line.starts_with(fieldName))
		{
			continue;
		}
		return Trim(line.substr(line.find(':') + 1));
	}

	return UNKNOWN;
}

std::vector<std::string> GetPIDs()
{
	std::vector<std::string> pids;
	for (const auto& entry : std::filesystem::directory_iterator(SOURCE_DIR))
	{
		if (!entry.is_directory())
		{
			continue;
		}
		std::string dirName = entry.path().filename().string();
		if (!isNumber(dirName))
		{
			continue;
		}

		pids.push_back(dirName);
	}

	return pids;
}

std::string GetProcessName(const std::string& pid)
{
	std::ifstream processNameFile(SOURCE_DIR + "/" + pid + PROCESS_NAME_FILE);
	if (!processNameFile.is_open())
	{
		return UNKNOWN;
	}
	std::string processName;
	std::getline(processNameFile, processName);

	return processName;
}

std::string GetUserNameByUid(const int uid)
{
	const passwd* pw = getpwuid(uid);
	if (pw == nullptr)
	{
		return UNKNOWN;
	}
	return pw->pw_name;
}

std::string GetProcessUserName(const std::string& pid)
{
	const std::string uidString = GetValueFromFileField(SOURCE_DIR + "/" + pid + STATUS_FILE, UID_FIELD);
	if (uidString == UNKNOWN)
	{
		return UNKNOWN;
	}

	try
	{
		const int uid = std::stoi(uidString);
		return GetUserNameByUid(uid);
	}
	catch (...)
	{
		return UNKNOWN;
	}
}

int GetSharedMemory(const std::string& pid)
{
	std::string sharedCleanString = GetValueFromFileField(
		SOURCE_DIR + "/" + pid + SMAPS_ROLLUP_FILE, SHARED_CLEAN);
	std::string sharedDirtyString = GetValueFromFileField(
		SOURCE_DIR + "/" + pid + SMAPS_ROLLUP_FILE, SHARED_DIRTY);
	sharedCleanString = ParseValueWithoutUnits(sharedCleanString);
	sharedDirtyString = ParseValueWithoutUnits(sharedDirtyString);

	try
	{
		return std::stoi(sharedCleanString) + std::stoi(sharedDirtyString);
	}
	catch (...)
	{
		return 0;
	}
}

int GetPrivateMemory(const std::string& pid)
{
	std::string privateCleanString = GetValueFromFileField(
		SOURCE_DIR + "/" + pid + SMAPS_ROLLUP_FILE, PRIVATE_CLEAN);
	std::string privateDirtyString = GetValueFromFileField(
		SOURCE_DIR + "/" + pid + SMAPS_ROLLUP_FILE, PRIVATE_DIRTY);
	privateCleanString = ParseValueWithoutUnits(privateCleanString);
	privateDirtyString = ParseValueWithoutUnits(privateDirtyString);

	try
	{
		return std::stoi(privateCleanString) + std::stoi(privateDirtyString);
	}
	catch (...)
	{
		return 0;
	}
}

ColumnWidth GetMaxWidths(const std::vector<Process>& processes)
{
	int maxPidLength = 0;
	int maxNameLength = 0;
	int maxUserLength = 0;
	int maxSharedLength = 0;
	int maxPrivateLength = 0;

	for (const Process& p : processes)
	{
		maxPidLength = std::max(maxPidLength, static_cast<int>(p.pid.length()));
		maxNameLength = std::max(maxNameLength, static_cast<int>(p.name.length()));
		maxUserLength = std::max(maxUserLength, static_cast<int>(p.user.length()));
		maxSharedLength = std::max(maxSharedLength, static_cast<int>(std::to_string(p.sharedMemoryKB).length()));
		maxPrivateLength = std::max(maxPrivateLength, static_cast<int>(std::to_string(p.privateMemoryKB).length()));
	}

	ColumnWidth columnWidth;
	columnWidth.pidWidth = maxPidLength + COLUMN_WIDTH_SHIFT;
	columnWidth.nameWidth = maxNameLength + COLUMN_WIDTH_SHIFT;
	columnWidth.userWidth = maxUserLength + COLUMN_WIDTH_SHIFT;
	columnWidth.sharedWidth = maxSharedLength + COLUMN_WIDTH_SHIFT;
	columnWidth.privateWidth = maxPrivateLength + COLUMN_WIDTH_SHIFT;

	return columnWidth;
}

std::vector<Process> GetProcesses()
{
	const std::vector<std::string> pids = GetPIDs();
	std::vector<Process> processes;
	for (const std::string& pid : pids)
	{
		if (!IsProcessAlive(pid))
		{
			continue;
		}

		Process process;
		process.pid = pid;
		process.name = GetProcessName(pid);
		process.user = GetProcessUserName(pid);
		process.sharedMemoryKB = GetSharedMemory(pid);
		process.privateMemoryKB = GetPrivateMemory(pid);

		processes.push_back(process);
	}

	return processes;
}

void SortProcesses(std::vector<Process>& processes)
{
	std::ranges::sort(processes, [](const Process& a, const Process& b) {
		const int totalA = a.sharedMemoryKB + a.privateMemoryKB;
		const int totalB = b.sharedMemoryKB + b.privateMemoryKB;
		return totalA > totalB;
	});
}

void PrintHeadline(const ColumnWidth& columnWidth)
{
	std::cout << std::left
			  << std::setw(columnWidth.pidWidth) << "PID"
			  << std::setw(columnWidth.nameWidth) << "NAME"
			  << std::setw(columnWidth.userWidth) << "USER"
			  << std::setw(columnWidth.sharedWidth) << "SHARED"
			  << std::setw(columnWidth.privateWidth) << "PRIVATE"
			  << std::endl;
}

void PrintProcesses(const std::vector<Process>& processes, const ColumnWidth& columnWidth)
{
	for (const Process& process : processes)
	{
		std::cout << std::left
				  << std::setw(columnWidth.pidWidth) << process.pid
				  << std::setw(columnWidth.nameWidth) << process.name
				  << std::setw(columnWidth.userWidth) << process.user
				  << std::setw(columnWidth.sharedWidth) << process.sharedMemoryKB
				  << std::setw(columnWidth.privateWidth) << process.privateMemoryKB
				  << std::endl;
	}
	std::cout << std::endl;
}

void PrintTotalProcessesCount(const std::vector<Process>& processes)
{
	std::cout << "Общее количество процессов: " << processes.size() << std::endl;
}

void PrintSumOfSharedAndPrivateMemory(const std::vector<Process>& processes)
{
	unsigned long sum = 0;
	for (const Process& process : processes)
	{
		sum += process.sharedMemoryKB + process.privateMemoryKB;
	}

	std::cout << "Суммарный объём Private и Shared памяти по всем процессам: "
			  << KilobyteToMegabyte(sum) << " MB"
			  << std::endl;
}

int main()
{
	try
	{
		std::vector<Process> processes = GetProcesses();
		SortProcesses(processes);
		const ColumnWidth columnWidth = GetMaxWidths(processes);
		PrintHeadline(columnWidth);
		PrintProcesses(processes, columnWidth);
		PrintTotalProcessesCount(processes);
		PrintSumOfSharedAndPrivateMemory(processes);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
