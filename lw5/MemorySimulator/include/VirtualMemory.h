#ifndef MEMORYSIMULATOR_VIRTUALMEMORY_H
#define MEMORYSIMULATOR_VIRTUALMEMORY_H
#include "OSHandler.h"
#include "PhysicalMemory.h"

struct PTE
{
    uint32_t raw = 0;

    static constexpr uint32_t P = 1u << 0;
    static constexpr uint32_t RW = 1u << 1;
    static constexpr uint32_t US = 1u << 2;
    static constexpr uint32_t A = 1u << 5;
    static constexpr uint32_t D = 1u << 6;
    static constexpr uint32_t NX = 1u << 31;

    static constexpr uint32_t FRAME_SHIFT = 12;
    static constexpr uint32_t FRAME_MASK = 0xFFFFF000u;

    [[nodiscard]] uint32_t GetFrame() const { return (raw & FRAME_MASK) >> FRAME_SHIFT; }
    void SetFrame(const uint32_t fn) { raw = (raw & ~FRAME_MASK) | (fn << FRAME_SHIFT); }

    [[nodiscard]] bool IsPresent() const { return raw & P; }
    void SetPresent(const bool v) { raw = v ? (raw | P) : (raw & ~P); }

    [[nodiscard]] bool IsWritable() const { return raw & RW; }
    void SetWritable(const bool v) { raw = v ? (raw | RW) : (raw & ~RW); }

    [[nodiscard]] bool IsUser() const { return raw & US; }
    void SetUser(const bool v) { raw = v ? (raw | US) : (raw & ~US); }

    [[nodiscard]] bool IsAccessed() const { return raw & A; }
    void SetAccessed(const bool v) { raw = v ? (raw | A) : (raw & ~A); }

    [[nodiscard]] bool IsDirty() const { return raw & D; }
    void SetDirty(const bool v) { raw = v ? (raw | D) : (raw & ~D); }

    [[nodiscard]] bool IsNX() const { return raw & NX; }
    void SetNX(const bool v) { raw = v ? (raw | NX) : (raw & ~NX); }
};

struct TranslationResult
{
    bool success = false;
    PTE pte;
    uint32_t physicalAddress{};
    PageFaultReason faultReason = PageFaultReason::NotPresent;
};

enum class Privilege { User, Supervisor };

class VirtualMemory
{
public:
    explicit VirtualMemory(PhysicalMemory &physicalMemory, OSHandler &handler);

    void SetPageTableAddress(uint32_t physicalAddress);

    [[nodiscard]] uint32_t GetPageTableAddress() const noexcept;

    [[nodiscard]] uint8_t Read8(uint32_t address, Privilege privilege, bool execute = false) const;

    [[nodiscard]] uint16_t Read16(uint32_t address, Privilege privilege, bool execute = false) const;

    [[nodiscard]] uint32_t Read32(uint32_t address, Privilege privilege, bool execute = false) const;

    [[nodiscard]] uint64_t Read64(uint32_t address, Privilege privilege, bool execute = false) const;

    void Write8(uint32_t address, uint8_t value, Privilege privilege);

    void Write16(uint32_t address, uint16_t value, Privilege privilege);

    void Write32(uint32_t address, uint32_t value, Privilege privilege);

    void Write64(uint32_t address, uint64_t value, Privilege privilege);

private:
    [[nodiscard]] TranslationResult TranslateAddress(uint32_t virtualAddress, Access access,
                                                     Privilege privilege, bool execute) const;

    static bool CheckAccess(TranslationResult &result, const PTE &pte, Access access, Privilege privilege,
                            bool execute);

    PhysicalMemory &m_physicalMemory;
    OSHandler &m_handler;
    uint32_t m_pageTableAddress = 0;

    template<typename T>
    T Read(const uint32_t address, const Privilege privilege, const bool execute) const
    {
        auto result = TranslateAddress(address, Access::Read, privilege, execute);

        if (!result.success)
        {
            const uint32_t vpn = address >> PTE::FRAME_SHIFT;
            const bool shouldRetry = m_handler.OnPageFault(
                *const_cast<VirtualMemory *>(this),
                vpn,
                Access::Read,
                result.faultReason);
            if (!shouldRetry)
            {
                throw std::runtime_error("Unhandled page fault on read");
            }
            result = TranslateAddress(address, Access::Read, privilege, execute);
            if (!result.success)
            {
                throw std::runtime_error("Page fault on read retry");
            }
        }

        PTE updatedPte = result.pte;
        updatedPte.SetAccessed(true);
        if (updatedPte.raw != result.pte.raw)
        {
            const uint32_t vpn = address >> PTE::FRAME_SHIFT;
            const uint32_t pteAddress = m_pageTableAddress + vpn * sizeof(PTE);
            m_physicalMemory.Write32(pteAddress, updatedPte.raw);
        }

        if constexpr (sizeof(T) == 1)
        {
            return m_physicalMemory.Read8(result.physicalAddress);
        }
        if constexpr (sizeof(T) == 2)
        {
            return m_physicalMemory.Read16(result.physicalAddress);
        }
        if constexpr (sizeof(T) == 4)
        {
            return m_physicalMemory.Read32(result.physicalAddress);
        }
        if constexpr (sizeof(T) == 8)
        {
            return m_physicalMemory.Read64(result.physicalAddress);
        }
        return 0;
    }

    template<typename T>
    void Write(const uint32_t address, T value, const Privilege privilege)
    {
        auto result = TranslateAddress(address, Access::Write, privilege, false);

        if (!result.success)
        {
            const uint32_t vpn = address >> PTE::FRAME_SHIFT;
            bool shouldRetry = m_handler.OnPageFault(
                *this,
                vpn,
                Access::Write,
                result.faultReason);
            if (!shouldRetry)
            {
                throw std::runtime_error("Unhandled page fault on write");
            }
            result = TranslateAddress(address, Access::Write, privilege, false);
            if (!result.success)
            {
                throw std::runtime_error("Page fault on write retry");
            }
        }

        PTE updatedPte = result.pte;
        updatedPte.SetAccessed(true);
        updatedPte.SetDirty(true);

        if (updatedPte.raw != result.pte.raw)
        {
            const uint32_t vpn = address >> PTE::FRAME_SHIFT;
            const uint32_t pteAddress = m_pageTableAddress + vpn * sizeof(PTE);
            m_physicalMemory.Write32(pteAddress, updatedPte.raw);
        }

        if constexpr (sizeof(T) == 1)
        {
            m_physicalMemory.Write8(result.physicalAddress, value);
        }
        if constexpr (sizeof(T) == 2)
        {
            m_physicalMemory.Write16(result.physicalAddress, value);
        }
        if constexpr (sizeof(T) == 4)
        {
            m_physicalMemory.Write32(result.physicalAddress, value);
        }
        if constexpr (sizeof(T) == 8)
        {
            m_physicalMemory.Write64(result.physicalAddress, value);
        }
    }
};


#endif //MEMORYSIMULATOR_VIRTUALMEMORY_H
