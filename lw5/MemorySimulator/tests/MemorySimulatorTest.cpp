#include "gtest/gtest.h"

#include "../include/PhysicalMemory.h"
#include "../include/MyOS.h"
#include "../include/VirtualMemory.h"

class VirtualMemoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        PhysicalMemoryConfig config;
        config.numFrames = 256;
        physicalMemory = std::make_unique<PhysicalMemory>(config);
        osHandler = std::make_unique<MyOS>(*physicalMemory);
        virtualMemory = std::make_unique<VirtualMemory>(*physicalMemory, *osHandler);
        virtualMemory->SetPageTableAddress(0);
    }

    std::unique_ptr<PhysicalMemory> physicalMemory;
    std::unique_ptr<MyOS> osHandler;
    std::unique_ptr<VirtualMemory> virtualMemory;

    PTE CreatePage(const uint32_t vpn, const uint32_t pfn, const bool isWritable, const bool isUser) const
    {
        PTE pte;
        pte.SetPresent(true);
        pte.SetWritable(isWritable);
        pte.SetUser(isUser);
        pte.SetFrame(pfn);

        const uint32_t pte_address = virtualMemory->GetPageTableAddress() + vpn * sizeof(PTE);
        physicalMemory->Write32(pte_address, pte.raw);
        return pte;
    }
};

TEST_F(VirtualMemoryTest, HandlesPageFaultOnFirstWrite) {
    constexpr uint32_t addr = 0x1000;
    constexpr uint32_t value = 0xDEADBEEF;

    ASSERT_NO_THROW(virtualMemory->Write32(addr, value, Privilege::User));
    ASSERT_EQ(virtualMemory->Read32(addr, Privilege::User), value);
}

TEST_F(VirtualMemoryTest, ThrowsOnWriteToReadOnlyPage) {
    constexpr uint32_t vpn = 10;
    constexpr uint32_t pfn = 2;
    constexpr uint32_t addr = vpn << PTE::FRAME_SHIFT;

    CreatePage(vpn, pfn, false, true);

    ASSERT_THROW(virtualMemory->Write32(addr, 123, Privilege::User), std::runtime_error);
}

TEST_F(VirtualMemoryTest, ThrowsOnUserAccessToSupervisorPage) {
    constexpr uint32_t vpn = 20;
    constexpr uint32_t pfn = 3;
    constexpr uint32_t addr = vpn << PTE::FRAME_SHIFT;

    CreatePage(vpn, pfn, true, false);

    ASSERT_THROW(virtualMemory->Read32(addr, Privilege::User), std::runtime_error);
    ASSERT_THROW(virtualMemory->Write32(addr, 123, Privilege::User), std::runtime_error);
}

TEST_F(VirtualMemoryTest, AllowsSupervisorAccessToAnyPage) {
    constexpr uint32_t userVpn = 30;
    constexpr uint32_t superVpn = 31;
    constexpr uint32_t userPfn = 4;
    constexpr uint32_t superPfn = 5;
    constexpr uint32_t userAddr = userVpn << PTE::FRAME_SHIFT;
    constexpr uint32_t superAddr = superVpn << PTE::FRAME_SHIFT;
    constexpr uint32_t value1 = 1;
    constexpr uint32_t value2 = 2;

    CreatePage(userVpn, userPfn, true, true);
    CreatePage(superVpn, superPfn, true, false);

    ASSERT_NO_THROW(virtualMemory->Write32(userAddr, value1, Privilege::Supervisor));
    ASSERT_NO_THROW(virtualMemory->Write32(superAddr, value2, Privilege::Supervisor));

    ASSERT_EQ(virtualMemory->Read32(userAddr, Privilege::Supervisor), value1);
    ASSERT_EQ(virtualMemory->Read32(superAddr, Privilege::Supervisor), value2);
}

TEST_F(VirtualMemoryTest, ThrowsOnExecuteFromNXPage) {
    constexpr uint32_t vpn = 40;
    constexpr uint32_t pfn = 6;
    constexpr uint32_t addr = vpn << PTE::FRAME_SHIFT;

    PTE pte = CreatePage(vpn, pfn, true, true);
    pte.SetNX(true);
    physicalMemory->Write32(virtualMemory->GetPageTableAddress() + vpn * sizeof(PTE), pte.raw);

    ASSERT_THROW(virtualMemory->Read32(addr, Privilege::User, true), std::runtime_error);
}

TEST_F(VirtualMemoryTest, SetsAccessedBitOnRead) {
    constexpr uint32_t vpn = 50;
    constexpr uint32_t pfn = 7;
    constexpr uint32_t addr = vpn << PTE::FRAME_SHIFT;
    const uint32_t pteAddr = virtualMemory->GetPageTableAddress() + vpn * sizeof(PTE);

    CreatePage(vpn, pfn, true, true);

    virtualMemory->Read32(addr, Privilege::User);

    PTE finalPte;
    finalPte.raw = physicalMemory->Read32(pteAddr);

    ASSERT_TRUE(finalPte.IsAccessed());
    ASSERT_FALSE(finalPte.IsDirty());
}

TEST_F(VirtualMemoryTest, SetsAccessedAndDirtyBitsOnWrite) {
    constexpr uint32_t vpn = 60;
    constexpr uint32_t pfn = 8;
    constexpr uint32_t addr = vpn << PTE::FRAME_SHIFT;
    const uint32_t pteAddr = virtualMemory->GetPageTableAddress() + vpn * sizeof(PTE);

    CreatePage(vpn, pfn, true, true);

    virtualMemory->Write32(addr, 123, Privilege::User);

    PTE finalPte;
    finalPte.raw = physicalMemory->Read32(pteAddr);

    ASSERT_TRUE(finalPte.IsAccessed());
    ASSERT_TRUE(finalPte.IsDirty());
}

TEST_F(VirtualMemoryTest, ThrowsOnMisalignedPageTableAddress) {
    ASSERT_THROW(virtualMemory->SetPageTableAddress(1), std::invalid_argument);
    ASSERT_THROW(virtualMemory->SetPageTableAddress(4095), std::invalid_argument);

    ASSERT_NO_THROW(virtualMemory->SetPageTableAddress(0));
    ASSERT_NO_THROW(virtualMemory->SetPageTableAddress(4096));
    ASSERT_NO_THROW(virtualMemory->SetPageTableAddress(8192));
}