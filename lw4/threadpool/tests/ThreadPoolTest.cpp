#include "../ThreadPool.h"
#include "gtest/gtest.h"
#include <atomic>
#include <chrono>

TEST(ThreadPoolTest, BasicTest)
{
	ThreadPool pool(4);
	std::atomic counter{ 0 };

	pool.Dispatch([&counter] {
		++counter;
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_EQ(counter.load(), 1);
}

TEST(ThreadPoolTest, MultipleTasksTest)
{
	ThreadPool pool(4);
	std::atomic counter{ 0 };
	constexpr int numTasks = 100;

	for (int i = 0; i < numTasks; ++i)
	{
		pool.Dispatch([&counter] {
			++counter;
		});
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	ASSERT_EQ(counter.load(), numTasks);
}

TEST(ThreadPoolTest, DestructorTest)
{
	ThreadPool pool(4);
	std::atomic counter{ 0 };
	constexpr int numTasks = 100;

	for (int i = 0; i < numTasks; ++i)
	{
		pool.Dispatch([&counter] {
			++counter;
		});
	}

	ASSERT_EQ(counter.load(), numTasks);
}