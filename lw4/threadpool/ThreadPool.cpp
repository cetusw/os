#include <functional>
#include <mutex>
#include <stdexcept>
#include <iostream>

#include "ThreadPool.h"

ThreadPool::ThreadPool(const unsigned numThreads)
{
	if (numThreads == 0)
	{
		throw std::invalid_argument("Number of threads must be greater than zero.");
	}
	for (unsigned i = 0; i < numThreads; ++i)
	{
		m_workers.emplace_back([this](const std::stop_token& stopToken) {
			while (true)
			{
				Task task;
				{
					std::unique_lock lock(m_queueMutex);
					m_condition.wait(lock, stopToken, [this] {
						return !m_tasks.empty();
					});

					if (stopToken.stop_requested() && m_tasks.empty()) // TODO что будет при return
					{
						return;
					}

					task = std::move(m_tasks.front());
					m_tasks.pop();
				}
				task();
			}
		});
	}
}

ThreadPool::~ThreadPool()
{
	if (!m_stopSource.request_stop())
	{
		std::cout << "Thread pool is already stopped" << std::endl;
	}
	m_condition.notify_all();
}

void ThreadPool::Dispatch(Task task)
{
	if (m_stopSource.stop_requested())
	{
		throw std::runtime_error("Dispatch on stopped thread pool");
	}
	{
		std::unique_lock lock(m_queueMutex);
		m_tasks.emplace(std::move(task));
	}
	m_condition.notify_one();
}