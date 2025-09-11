#include <iomanip>
#include <iostream>
#include <ostream>

#include "SysInfo.h"

int main() {
    constexpr SysInfo sysInfo;
    std::cout << std::left << std::setw(16) << "OS Name:" << sysInfo.GetOSName() << std::endl;
    std::cout << std::setw(16) << "OS Version:" << sysInfo.GetOSVersion() << std::endl;
    std::cout << std::setw(16) << "RAM:" << sysInfo.GetFreeMemory() << "MB free / " << sysInfo.GetTotalMemory() << "MB total" << std::endl;
    std::cout << std::setw(16) << "Processors:" << sysInfo.GetProcessorCount() << std::endl;
}
