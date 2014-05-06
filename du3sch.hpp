#pragma once

#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <condition_variable>

template <typename T, typename TASK>
class wrapper
{
	public:
		bool ready;
		TASK task;
		std::promise<T> *prom;

		wrapper (TASK _task)
		{
			ready = false;
			task = std::move(_task);
			prom = new std::promise<T>();
		}

		~wrapper() {}
};

template<typename T, typename TASK>
class Scheduler
{
	private:
		std::mutex m;
		std::condition_variable cv;
		size_t task_count;
		std::atomic<unsigned int> train;
		std::vector<wrapper<T, TASK>> tasks;

	public:
		Scheduler(std::size_t core_count)
		{
			task_count = 0;
			train = 0;

			std::thread **threads = new std::thread*[core_count];
			for (size_t i = 0; i < core_count; i++)
				threads[i] = new std::thread(&Scheduler::worker, this);
		}

		~Scheduler()
		{
		}

		void worker ()
		{
			while (1)
			{
				std::unique_lock<std::mutex> lck(m);
				while (train >= task_count)
					cv.wait(lck);	

				unsigned int local_train_copy = train;
				train++;
				lck.unlock();
				
				tasks[local_train_copy].prom->set_value(tasks[local_train_copy].task());		
				tasks[local_train_copy].ready = true;

				std::this_thread::yield();
			}
		}

		std::size_t add_task(TASK&& task)
		{
			std::unique_lock<std::mutex> lck(m);

			wrapper<T, TASK> w(task);
			tasks.push_back(w);

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
			return tasks[index].prom->get_future().get();	
		}
};

