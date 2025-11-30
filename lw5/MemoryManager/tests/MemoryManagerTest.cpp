#include "MemoryManager.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

TEST(MemoryManagerTest, BasicAllocation)
{
	alignas(std::max_align_t) char buffer[1024];
	MemoryManager mm(buffer, sizeof(buffer));

	void* p1 = mm.Allocate(100);
	EXPECT_NE(p1, nullptr);

	void* p2 = mm.Allocate(200);
	EXPECT_NE(p2, nullptr);
	EXPECT_NE(p1, p2);

	mm.Free(p1);
	mm.Free(p2);
}

TEST(MemoryManagerTest, AlignmentCheck)
{
	alignas(std::max_align_t) char buffer[2048];
	MemoryManager mm(buffer, sizeof(buffer));

	void* p1 = mm.Allocate(10, 16);
	EXPECT_NE(p1, nullptr);
	EXPECT_EQ(reinterpret_cast<uintptr_t>(p1) % 16, 0);

	void* p2 = mm.Allocate(10, 128);
	EXPECT_NE(p2, nullptr);
	EXPECT_EQ(reinterpret_cast<uintptr_t>(p2) % 128, 0);

	mm.Free(p1);
	mm.Free(p2);
}

TEST(MemoryManagerTest, Coalescing)
{
	alignas(std::max_align_t) char buffer[512];
	MemoryManager mm(buffer, sizeof(buffer));

	void* p1 = mm.Allocate(64);
	void* p2 = mm.Allocate(64);
	void* p3 = mm.Allocate(64);

	ASSERT_NE(p1, nullptr);
	ASSERT_NE(p2, nullptr);
	ASSERT_NE(p3, nullptr);

	mm.Free(p2);
	mm.Free(p1);

	void* pBig = mm.Allocate(128 + sizeof(void*) * 4);
	EXPECT_NE(pBig, nullptr);

	mm.Free(p3);
	mm.Free(pBig);
}

TEST(MemoryManagerTest, OutOfMemory)
{
	alignas(std::max_align_t) char buffer[128];
	MemoryManager mm(buffer, sizeof(buffer));

	void* p1 = mm.Allocate(80);
	void* p2 = mm.Allocate(100);
	EXPECT_EQ(p2, nullptr);

	mm.Free(p1);
}

TEST(MemoryManagerTest, ThreadSafety)
{
	alignas(std::max_align_t) char buffer[4096 * 4];
	MemoryManager mm(buffer, sizeof(buffer));

	auto threadFunc = [&mm]() {
		for (int i = 0; i < 100; ++i)
		{
			void* p = mm.Allocate(32);
			if (p)
			{
				std::this_thread::yield();
				mm.Free(p);
			}
		}
	};

	std::vector<std::thread> threads;
	for (int i = 0; i < 10; ++i)
	{
		threads.emplace_back(threadFunc);
	}

	for (auto& t : threads)
	{
		t.join();
	}

	void* pAll = mm.Allocate(sizeof(buffer));
	EXPECT_NE(pAll, nullptr);
	mm.Free(pAll);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}