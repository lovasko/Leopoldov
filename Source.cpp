
#include <iostream>
#include <future>
#include <ctime>
#include <map>
#include <vector>
#include <algorithm>
#include "Stopwatch.h"
#include "du3sch.hpp"

static std::size_t Count = 1000;

static std::size_t MaxNum = 10000000;

static std::size_t CoreCount = 8;

typedef std::map<std::size_t, bool> PrimeMap;

bool IsPrime(std::size_t num)
{
	for (std::size_t i = 2; i < num; i++)
	{
		if (num % i == 0)
			return false;
	}
	return true;
}

template<typename SCHEDULER>
void TestScheduler(const char* msg, const PrimeMap& value, time_sec serialTime)
{
	ticks_t start = now();
	SCHEDULER scheduler(CoreCount);
	std::map<std::size_t, std::size_t> tasks;
	for (std::size_t i = 0; i < Count; i++)
	{
		tasks[i] = scheduler.add_task([i]() -> bool { return IsPrime(i); });
		tasks[MaxNum - i] = scheduler.add_task([i]() -> bool { return IsPrime(MaxNum - i); });
	}

	for (std::map<std::size_t, std::size_t>::iterator i = tasks.begin(); i != tasks.end(); ++i)
	{
		if (value.at(i->first) != scheduler.get_task_result(i->second))
			throw 0;
	}
	time_sec parallelTime = ticks_to_time(now() - start);
	std::cout << msg << " time = " << parallelTime << " [s]  -  Serial time portion = " << parallelTime / (serialTime / 100.0) << " [%]" << std::endl;
	std::cout << msg << " calculation correct" << std::endl;
}

int main()
{
	ticks_t start = now();
	PrimeMap value;
	for (std::size_t i = 0; i < Count; i++)
	{
		value[i] = IsPrime(i);
		value[MaxNum - i] = IsPrime(MaxNum - i);
	}
	time_sec serialTime = ticks_to_time(now() - start);
	std::cout << "Serial time = " << serialTime << std::endl;

	TestScheduler<Scheduler<bool, std::function<bool(void)> > >("Scheduler", value, serialTime);

	system("pause");
	return 0;
}