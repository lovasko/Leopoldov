#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T, typename TASK>
class Scheduler
{
	private:
		std::mutex m;
		std::condition_variable cv;
		size_t task_count;
		std::atomic<unsigned int> train;
		std::vector<TASK> tasks;

	public:
		Scheduler(std::size_t core_count)
		{
			task_count = 0;
			train = 0;

		}

		~Scheduler()
		{
		}

		void worker ()
		{
			if (train < task_count)
			{
				// make the task with std::async but deferred.	
			}
			else
			{
				// wait on condition variable		
			}
		}

		std::size_t add_task(TASK&& task)
		{
			std::lock_guard<std::mutex>(m);
			tasks.push_back(task);
			task_count++;
			cv.notify_one();

			return (task_count-1);
		}

		bool is_task_ready(std::size_t index)
		{
			return tasks[index].ready;
		}

		T get_task_result(std::size_t index)
		{
			return tasks[index].fut.get();	
		}
};

