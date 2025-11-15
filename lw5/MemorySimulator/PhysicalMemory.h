#ifndef MEMORYSIMULATOR_PHYSICALMEMORY_H
#define MEMORYSIMULATOR_PHYSICALMEMORY_H
#include <cstdint>
#include <stdexcept>
#include <vector>


struct PhysicalMemoryConfig
{
    uint32_t numFrames = 1024;
    uint32_t frameSize = 4096;
};

class PhysicalMemory
{
public:
    explicit PhysicalMemory(PhysicalMemoryConfig cfg);

    [[nodiscard]] uint32_t GetSize() const noexcept;

    [[nodiscard]] uint8_t Read8(uint32_t address) const;

    [[nodiscard]] uint16_t Read16(uint32_t address) const;

    [[nodiscard]] uint32_t Read32(uint32_t address) const;

    [[nodiscard]] uint64_t Read64(uint32_t address) const;

    void Write8(uint32_t address, uint8_t value);

    void Write16(uint32_t address, uint16_t value);

    void Write32(uint32_t address, uint32_t value);

    void Write64(uint32_t address, uint64_t value);

private:
    template<typename T>
    T Read(const uint32_t address) const
    {
        if (address % sizeof(T) != 0)
        {
            throw std::runtime_error("Misaligned memory access");
        }

        if (address + sizeof(T) > m_memory.size())
        {
            throw std::out_of_range("Physical memory access out of range");
        }

        const T *ptr = reinterpret_cast<const T *>(m_memory.data() + address);
        return *ptr;
    }

    template<typename T>
    void Write(const uint32_t address, T value)
    {
        if (address % sizeof(T) != 0)
        {
            throw std::runtime_error("Misaligned memory access");
        }
        if (address + sizeof(T) > m_memory.size())
        {
            throw std::out_of_range("Physical memory access out of range");
        }

        T *ptr = reinterpret_cast<T *>(m_memory.data() + address);
        *ptr = value;
    }

    std::vector<uint8_t> m_memory;
};


#endif //MEMORYSIMULATOR_PHYSICALMEMORY_H
