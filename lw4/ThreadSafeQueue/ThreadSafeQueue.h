#ifndef THREADSAFEQUEUE_THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_THREADSAFEQUEUE_H

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

template <typename T>
class ThreadSafeQueue
{
public:
	explicit ThreadSafeQueue(const size_t capacity = 0)
		: m_capacity(capacity)
	{
	}

	void Push(const T& value)
	{
		std::unique_lock lock(m_queueMutex);
		m_producer.wait(lock, [this] {
			return !IsFull();
		});
		m_queue.push_back(value);

		m_consumer.notify_one();
	}

	void Push(T&& value)
	{
		std::unique_lock lock(m_queueMutex);
		m_producer.wait(lock, [this] {
			return !IsFull();
		});
		m_queue.push_back(std::move(value));

		m_consumer.notify_one();
	}

	[[nodiscard]] bool TryPush(const T& value)
	{
		std::lock_guard lock(m_queueMutex);
		if (IsFull()) {
			return false;
		}
		m_queue.push_back(value);
		m_consumer.notify_one();
		return true;
	}

	[[nodiscard]] bool TryPush(T&& value)
	{
		std::lock_guard lock(m_queueMutex);
		if (IsFull()) {
			return false;
		}
		m_queue.push_back(std::move(value));
		m_consumer.notify_one();
		return true;
	}

	bool TryPop(T& out)
	{
		std::lock_guard lock(m_queueMutex);
		if (m_queue.empty()) {
			return false;
		}
		out = std::move(m_queue.front());
		m_queue.pop_front();

		m_producer.notify_one();
		return true;
	}

	std::unique_ptr<T> TryPop()
	{
		std::lock_guard lock(m_queueMutex);
		if (m_queue.empty()) {
			return nullptr;
		}
		auto ptr = std::make_unique<T>(std::move(m_queue.front()));
		m_queue.pop_front();

		m_producer.notify_one();
		return ptr;
	}

	T WaitAndPop() requires std::is_nothrow_move_constructible_v<T>
	{
		std::unique_lock lock(m_queueMutex);
		m_consumer.wait(lock, [this] {
			return !m_queue.empty();
		});
		T result = std::move(m_queue.front());
		m_queue.pop_front();

		m_producer.notify_one();
		return result;
	}

	void WaitAndPop(T& out)
	{
		std::unique_lock lock(m_queueMutex);
		m_consumer.wait(lock, [this] {
			return !m_queue.empty();
		});
		out = std::move(m_queue.front());
		m_queue.pop_front();

		m_producer.notify_one();
	}

	[[nodiscard]] size_t GetSize() const
	{
		std::lock_guard lock(m_queueMutex);
		return m_queue.size();
	}

	[[nodiscard]] bool IsEmpty() const {
		std::lock_guard lock(m_queueMutex);
		return m_queue.empty();
	}

	void Swap(ThreadSafeQueue& other)
	{
		if (this == &other) {
			return;
		}

		std::scoped_lock lock(m_queueMutex, other.m_queueMutex);

		m_queue.swap(other.m_queue);

		m_consumer.notify_all();
		m_producer.notify_all();

		other.m_consumer.notify_all();
		other.m_producer.notify_all();
	}


	void Swap(std::deque<T>& other)
	{
		std::lock_guard lock(m_queueMutex);
		m_queue.swap(other);

		m_consumer.notify_all();
		m_producer.notify_all();
	}

private:
	[[nodiscard]] bool IsFull() const {
		return m_capacity > 0 && m_queue.size() == m_capacity;
	}

	std::deque<T> m_queue;
	size_t m_capacity;
	mutable std::mutex m_queueMutex;
	std::condition_variable m_producer;
	std::condition_variable m_consumer;
};

#endif // THREADSAFEQUEUE_THREADSAFEQUEUE_H
