#ifndef SYS_INFO_LINUX_MNTENTWRAPPER_H
#define SYS_INFO_LINUX_MNTENTWRAPPER_H

#include <string>

class MntentWrapper
{
public:
	MntentWrapper(const char* path, const char* mode);
	~MntentWrapper();

	MntentWrapper(const MntentWrapper&) = delete;
	MntentWrapper& operator=(const MntentWrapper&) = delete;

	[[nodiscard]] FILE* Get() const;

private:
	FILE* handle;
};

#endif // SYS_INFO_LINUX_MNTENTWRAPPER_H
