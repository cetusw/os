#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include <string>

class ImageReader
{
public:
	explicit ImageReader(const std::string& path);

	~ImageReader();

	void Read(void* buffer, size_t size, off_t offset) const;

	[[nodiscard]] bool IsOpen() const;

private:
	int m_fileDescriptor;
};

#endif // IMAGE_READER_H
