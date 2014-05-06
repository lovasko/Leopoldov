#pragma once

#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <condition_variable>

template <typename T, typename TASK>
class Wrapper
{
	public:
		bool ready;
		TASK task;
		T result;

		Wrapper (TASK _task)
		{
			ready = false;
			task = std::move(_task);
		}

		~Wrapper() 
		{
		}
};

template<typename T, typename TASK>
class Scheduler
{
	private:
		std::mutex m, m2;
		std::condition_variable cv, cv2;
		size_t task_count;
		std::atomic<unsigned int> train;
		std::vector<Wrapper<T, TASK>> tasks;
		size_t _core_count;
		std::thread **threads;
		bool should_finish;

	public:
		Scheduler(std::size_t core_count)
		{
			task_count = 0;
			train = 0;
			_core_count = core_count;

			threads = new std::thread*[core_count];
			for (size_t i = 0; i < _core_count; i++)
			{
				threads[i] = new std::thread(&Scheduler::worker, this);
			}
		}

		~Scheduler()
		{
			// This variable is checked in the ::worker after condvar unlock
			should_finish = true;

			// Add fake task so the condvar would unblock and notify all threads
			task_count += 1;
			cv.notify_all();

			// Join all threads
			for (size_t i = 0; i < _core_count; i++)
			{
				threads[i]->join();
			}
		}

		void worker ()
		{
			while (1)
			{
				std::unique_lock<std::mutex> lck(m);
				while (train >= task_count)
					cv.wait(lck);	

				if (should_finish)
					break;

				unsigned int local_train_copy = train;
				train++;
				lck.unlock();
				
				tasks[local_train_copy].result = tasks[local_train_copy].task();
				tasks[local_train_copy].ready = true;
				cv2.notify_one();

				std::this_thread::yield();
			}
		}

		std::size_t add_task(TASK&& task)
		{
			std::unique_lock<std::mutex> lck(m);

			Wrapper<T, TASK> w(task);
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
			if (is_task_ready(index))
				return tasks[index].result;
			else
			{
				std::unique_lock<std::mutex> lock(m2);	
				while (!is_task_ready(index))
					cv2.wait(lock);

				return tasks[index].result;
			}
		}
};

