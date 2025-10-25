#include <benchmark/benchmark.h>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include "ThreadPool.h"

constexpr int NUM_TASKS = 10000;

static void ThreadPoolBenchmark(benchmark::State& state)
{
	for (auto _ : state)
	{
		state.PauseTiming();
		ThreadPool pool(state.range(0));
		std::atomic counter{ 0 };
		state.ResumeTiming();

		for (int i = 0; i < NUM_TASKS; ++i)
		{
			pool.Dispatch([&counter] {
				++counter;
			});
		}
	}
}
BENCHMARK(ThreadPoolBenchmark)->DenseRange(1, 16, 1);

static void BoostThreadPool(benchmark::State& state)
{
	for (auto _ : state)
	{
		state.PauseTiming();
		boost::asio::thread_pool pool(state.range(0));
		std::atomic counter{ 0 };
		state.ResumeTiming();

		for (int i = 0; i < NUM_TASKS; ++i)
		{
			boost::asio::post(pool, [&counter] {
				++counter;
			});
		}
		pool.join();
	}
}
BENCHMARK(BoostThreadPool)->DenseRange(1, 16, 1);

BENCHMARK_MAIN();