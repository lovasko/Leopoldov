// DU3sch.hpp
// Daniel Lovasko NPRG051 2013/2014

#pragma once

#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <condition_variable>

// Small wrapper around the task
template <typename T, typename TASK>
class Wrapper
{
	public:
		bool ready;
		TASK task;
		T result;

		// Create the wrapper with default ready set to false
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
		// Mutexes used with conditional variables
		std::mutex m, m2;

		// cv is used as 
		std::condition_variable cv, cv2;

		// Total task cound, increases only in ::add_task
		size_t task_count;

		// Most significant variable, this represents the actual task where we are
		// Only the ::worker function increments this variable
		std::atomic<unsigned int> train;

		// Stored tasks. Index of the task is simply index in this array.
		std::vector<Wrapper<T, TASK>> tasks;

		// Internal copy of the Core Count
		size_t cc;

		// Threads. Stored for later joining in the destructor
		std::thread **threads;

		// Denotes whether threads should finish
		bool should_finish;

	public:

		// Create core_count threads and run member function ::worker in each 
		Scheduler(std::size_t core_count)
		{
			// Initial values
			task_count = 0;
			train = 0;
			cc = core_count;

			// Create a thread for each processor core
			threads = new std::thread*[cc];
			for (size_t i = 0; i < cc; i++)
			{
				threads[i] = new std::thread(&Scheduler::worker, this);
			}
		}

		// Destructor takes care of thread finishing
		~Scheduler()
		{
			// This variable is checked in the ::worker after condvar unlock
			should_finish = true;

			// Add fake task so the condvar would unblock and notify all threads
			task_count += 1;
			cv.notify_all();

			// Join all threads
			for (size_t i = 0; i < cc; i++)
			{
				threads[i]->join();
			}
		}

		// Main thread function
		void worker ()
		{
			while (1)
			{
				// Try to get the lock and satisfy the condvar
				std::unique_lock<std::mutex> lck(m);
				while (train >= task_count)
					cv.wait(lck);	

				// First thing after the condvar is satistief, check whether we should
				// finish. This variable is by default false and only the destructor
				// modifies it.
				if (should_finish)
					break;

				// While in the safe zone, make a by-value copy of the actual train
				unsigned int local_train_copy = train;
				train++;
				lck.unlock();
				
				// Do the hard work
				tasks[local_train_copy].result = tasks[local_train_copy].task();

				// Set the ready variable to be true, since we finished the task
				tasks[local_train_copy].ready = true;

				// Notify the ::get_task_result that we finished _some_ task, so it can
				// check whether it is the one it is waiting for
				cv2.notify_one();

				std::this_thread::yield();
			}
		}

		std::size_t add_task(TASK&& task)
		{
			std::unique_lock<std::mutex> lck(m);

			// Create new wrapper around the task
			Wrapper<T, TASK> w(task);
			tasks.push_back(w);

			// Create a new task and notify one thread that can take it
			// Only threads that are waiting for a job will get this task
			task_count++;
			cv.notify_one();

			return (task_count-1);
		}

		// Check if the indexed task is ready
		bool is_task_ready(std::size_t index)
		{
			return tasks[index].ready;
		}

		// Gets the task result immediately if it is already computed
		// otherwise waits for its completion
		T get_task_result(std::size_t index)
		{
			// This two branches of the if could be merged, but I prefer to have the
			// already-done one alone, without creating the lock, which is a
			// time-consuming action.
			// Important thing to note is, that while we are waiting for the task
			// with index to be done, other tasks are being worked on in the
			// parallel. So even while it may seem as time-wasting, it is not.

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

