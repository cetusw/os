#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(unsigned numThreads);

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	~ThreadPool();

	void Dispatch(Task task);

private:
	std::queue<Task> m_tasks;
	std::mutex m_queueMutex;
	std::condition_variable_any m_condition;
	std::stop_source m_stopSource;
	std::vector<std::jthread> m_workers;
};

#endif // THREADPOOL_THREADPOOL_H