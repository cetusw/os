#include "../ThreadSafeQueue.h"
#include "benchmark/benchmark.h"
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <numeric>
#include <thread>
#include <vector>

constexpr int ITEMS_PER_PRODUCER = 10000;

template <typename T>
static void QueueBenchmark(benchmark::State& state)
{
	const int numProducers = static_cast<int>(state.range(0));
	const int numConsumers = static_cast<int>(state.range(1));

	for (auto _ : state)
	{
		state.PauseTiming();

		T queue(numProducers * ITEMS_PER_PRODUCER);
		std::atomic consumedCount = 0;
		const int totalItems = numProducers * ITEMS_PER_PRODUCER;

		std::vector<std::jthread> producers;
		producers.reserve(numProducers);
		std::vector<std::jthread> consumers;
		consumers.reserve(numConsumers);

		state.ResumeTiming();

		for (int i = 0; i < numProducers; ++i)
		{
			producers.emplace_back([&]() {
				for (int j = 0; j < ITEMS_PER_PRODUCER; ++j)
				{
					int value = j;
					if constexpr (std::is_same_v<T, boost::lockfree::queue<int>>)
					{
						do
						{
						} while (!queue.push(value));
					}
					else
					{
						queue.Push(value);
					}
				}
			});
		}

		for (int i = 0; i < numConsumers; ++i)
		{
			consumers.emplace_back([&]() {
				int value;
				while (consumedCount.load(std::memory_order_acquire) < totalItems)
				{
					bool success = false;
					if constexpr (std::is_same_v<T, boost::lockfree::queue<int>>)
					{
						success = queue.pop(value);
					}
					else
					{
						success = queue.TryPop(value);
					}

					if (success)
					{
						consumedCount.fetch_add(1, std::memory_order_release);
					}
				}
			});
		}
	}
}

BENCHMARK_TEMPLATE(QueueBenchmark, ThreadSafeQueue<int>)
	->Args({ 1, 1 })
	->Args({ 2, 2 })
	->Args({ 4, 4 })
	->Args({ 8, 8 })
	->UseRealTime();

BENCHMARK_TEMPLATE(QueueBenchmark, boost::lockfree::queue<int>)
	->Args({ 1, 1 })
	->Args({ 2, 2 })
	->Args({ 4, 4 })
	->Args({ 8, 8 })
	->UseRealTime();

BENCHMARK_MAIN();