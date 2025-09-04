#include "MntentWrapper.h"

#include <mntent.h>
#include <stdexcept>

MntentWrapper::MntentWrapper(const char* path, const char* mode)
{
	handle = setmntent(path, mode);
	if (handle == nullptr)
	{
		throw std::runtime_error("Не удалось открыть " + std::string(path));
	}
}

MntentWrapper::~MntentWrapper()
{
	if (handle != nullptr)
	{
		endmntent(handle);
	}
}

FILE* MntentWrapper::Get() const
{
	return handle;
}
