#pragma once

#include <string>

template<typename T, typename TASK>
class Scheduler
{
public:
	Scheduler(std::size_t core_count)
	{
	}

	~Scheduler()
	{
	}

	std::size_t add_task(TASK&& task)
	{
		return 0;
	}

	bool is_task_ready(std::size_t index)
	{
		return false;
	}

	T get_task_result(std::size_t index)
	{
		return T();
	}
};

