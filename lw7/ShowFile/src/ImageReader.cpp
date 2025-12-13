#include "ImageReader.h"
#include <fcntl.h>
#include <stdexcept>
#include <iostream>

ImageReader::ImageReader(const std::string& path)
    : m_fileDescriptor(-1)
{
    m_fileDescriptor = open(path.c_str(), O_RDONLY);
    if (m_fileDescriptor < 0)
    {
        throw std::runtime_error("Error: cannot open image file.");
    }
}

ImageReader::~ImageReader()
{
    if (m_fileDescriptor >= 0)
    {
        close(m_fileDescriptor);
    }
}

void ImageReader::Read(void* buffer, const size_t size, const off_t offset) const
{
    const ssize_t bytesRead = pread(m_fileDescriptor, buffer, size, offset);
    if (bytesRead < 0 || static_cast<size_t>(bytesRead) != size)
    {
        throw std::runtime_error("Error: failed to read from image.");
    }
}

bool ImageReader::IsOpen() const
{
    return m_fileDescriptor >= 0;
}