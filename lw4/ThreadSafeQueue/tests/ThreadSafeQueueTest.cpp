#include "../ThreadSafeQueue.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <thread>

TEST(ThreadSafeQueueTest, InitiallyEmpty)
{
	const ThreadSafeQueue<int> q;
	EXPECT_TRUE(q.IsEmpty());
	EXPECT_EQ(q.GetSize(), 0);
}

TEST(ThreadSafeQueueTest, SinglePushAndPop)
{
	ThreadSafeQueue<int> q;
	q.Push(42);

	EXPECT_FALSE(q.IsEmpty());
	EXPECT_EQ(q.GetSize(), 1);

	int value;
	EXPECT_TRUE(q.TryPop(value));
	EXPECT_EQ(value, 42);

	EXPECT_TRUE(q.IsEmpty());
	EXPECT_EQ(q.GetSize(), 0);
}

TEST(ThreadSafeQueueTest, FIFOOrder)
{
	ThreadSafeQueue<int> q;
	q.Push(1);
	q.Push(2);
	q.Push(3);

	int v1, v2, v3;
	EXPECT_TRUE(q.TryPop(v1));
	EXPECT_EQ(v1, 1);

	EXPECT_TRUE(q.TryPop(v2));
	EXPECT_EQ(v2, 2);

	EXPECT_TRUE(q.TryPop(v3));
	EXPECT_EQ(v3, 3);

	EXPECT_TRUE(q.IsEmpty());
}

TEST(ThreadSafeQueueTest, TryPopOnEmpty)
{
	ThreadSafeQueue<int> q;
	int value;

	EXPECT_FALSE(q.TryPop(value));

	const auto ptr = q.TryPop();
	EXPECT_EQ(ptr, nullptr);
}

TEST(ThreadSafeQueueTest, BoundedQueueTryPushFull)
{
	ThreadSafeQueue<int> q(2);

	EXPECT_TRUE(q.TryPush(1));
	EXPECT_TRUE(q.TryPush(2));
	EXPECT_FALSE(q.TryPush(3));
	EXPECT_EQ(q.GetSize(), 2);

	int value;
	q.TryPop(value);

	EXPECT_TRUE(q.TryPush(3));
	EXPECT_EQ(q.GetSize(), 2);
}

TEST(ThreadSafeQueueTest, WaitAndPopByValue)
{
	ThreadSafeQueue<int> q;
	q.Push(123);

	const int value = q.WaitAndPop();
	EXPECT_EQ(value, 123);
	EXPECT_TRUE(q.IsEmpty());
}

TEST(ThreadSafeQueueTest, SwapWithOtherQueue)
{
	ThreadSafeQueue<int> q1(5);
	q1.Push(1);
	q1.Push(2);

	ThreadSafeQueue<int> q2(3);
	q2.Push(100);

	q1.Swap(q2);

	EXPECT_EQ(q1.GetSize(), 1);
	EXPECT_EQ(q2.GetSize(), 2);

	int val;
	q1.WaitAndPop(val);
	EXPECT_EQ(val, 100);

	q2.WaitAndPop(val);
	EXPECT_EQ(val, 1);
}

TEST(ThreadSafeQueueTest, MPMCStressTest)
{
	constexpr int numProducers = 4;
	constexpr int numConsumers = 4;
	constexpr int itemsPerProducer = 10000;
	constexpr int totalItems = numProducers * itemsPerProducer;

	ThreadSafeQueue<int> q;
	std::atomic itemsProduced = 0;
	std::atomic itemsConsumed = 0;

	std::vector<int> consumedData;
	std::mutex consumedDataMutex;
	consumedData.reserve(totalItems);

	std::vector<std::jthread> producers;
	for (int i = 0; i < numProducers; ++i)
	{
		producers.emplace_back([&] {
			for (int j = 0; j < itemsPerProducer; ++j)
			{
				int value = i;
				q.Push(value);
				++itemsProduced;
			}
		});
	}

	std::vector<std::jthread> consumers;
	for (int i = 0; i < numConsumers; ++i)
	{
		consumers.emplace_back([&] {
			int value;
			while (itemsConsumed.load() < totalItems)
			{
				if (q.TryPop(value))
				{
					++itemsConsumed;
					std::lock_guard lock(consumedDataMutex);
					consumedData.push_back(value);
				}
			}
		});
	}

	producers.clear();
	consumers.clear();

	EXPECT_EQ(itemsProduced, totalItems);
	EXPECT_EQ(itemsConsumed, totalItems);
	EXPECT_EQ(q.IsEmpty(), true);
}

TEST(ThreadSafeQueueTest, BoundedQueueBlocksOnPush)
{
	ThreadSafeQueue<int> q(1);
	q.Push(1);

	std::atomic pushFinished = false;

	std::jthread producerThread([&] {
		q.Push(2);
		pushFinished = true;
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_FALSE(pushFinished.load());

	int value;
	q.WaitAndPop(value);
	EXPECT_EQ(value, 1);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(pushFinished.load());

	q.WaitAndPop(value);
	EXPECT_EQ(value, 2);
}

TEST(ThreadSafeQueueTest, SwapWithDeque)
{
	ThreadSafeQueue<int> q(5);
	q.Push(1);
	q.Push(2);

	std::deque d = { 100, 200, 300 };

	q.Swap(d);

	EXPECT_EQ(q.GetSize(), 3);
	EXPECT_EQ(d.size(), 2);

	int val;
	q.WaitAndPop(val);
	EXPECT_EQ(val, 100);
	EXPECT_EQ(d.front(), 1);
}

TEST(ThreadSafeQueueTest, ConcurrentSwapNoDeadlock)
{
	ThreadSafeQueue<int> q1;
	ThreadSafeQueue<int> q2;
	constexpr int iterations = 5000;

	std::jthread thread1([&] {
		for (int i = 0; i < iterations; ++i)
		{
			q1.Swap(q2);
		}
	});

	std::jthread thread2([&] {
		for (int i = 0; i < iterations; ++i)
		{
			q2.Swap(q1);
		}
	});
}

struct Bomb
{
	Bomb() = default;

	Bomb(const Bomb& other)
	{
		throw std::runtime_error("Copy failed!");
	}
	Bomb(Bomb&& other) noexcept(false)
	{
		throw std::runtime_error("Move failed!");
	}

	Bomb& operator=(const Bomb& other) = default;
	Bomb& operator=(Bomb&& other) = default;
};

TEST(ThreadSafeQueueTest, ExceptionSafetyOnPush)
{
	ThreadSafeQueue<Bomb> q(5);
	Bomb b;

	EXPECT_THROW(q.Push(b), std::runtime_error);

	EXPECT_EQ(q.GetSize(), 0);

	Bomb out;
	EXPECT_THROW(q.Push(out), std::runtime_error);
	EXPECT_EQ(q.GetSize(), 0);
}

// TODO: стресс тесты
