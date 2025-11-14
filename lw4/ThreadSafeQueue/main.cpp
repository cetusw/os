#include <iostream>
#include "ThreadSafeQueue.h"

int main()
{
	ThreadSafeQueue<int> q1(5);
	q1.Push(1);
	q1.Push(2);

	ThreadSafeQueue<int> q2(3);
	q2.Push(100);

	std::cout << "\n--- Before swap ---\n";
	std::cout << "q1 before swap" << " size: " << q1.GetSize() << std::endl;
	std::cout << "q2 before swap" << " size: " << q2.GetSize() << std::endl;

	q1.Swap(q2);

	std::cout << "\n--- After swap ---\n";
	std::cout << "q1 after swap" << " size: " << q1.GetSize() << std::endl;
	std::cout << "q2 after swap" << " size: " << q2.GetSize() << std::endl;

	int val;
	q1.TryPop(val);
	std::cout << "Popped from q1: " << val << std::endl;

	return 0;
}