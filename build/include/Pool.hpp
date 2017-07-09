#ifndef INCLUDE__POOL_HPP
#define INCLUDE__POOL_HPP

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>


template <class THING>
class Pool {
	private:
		bool is_empty;
		std::list<THING> pool;
		std::mutex mtx;
		std::condition_variable cond;

	public:
		Pool(): is_empty(true) {};

		THING request() {
			// blocking
			std::unique_lock<std::mutex> lock(mtx);
			
			while(is_empty)
				cond.wait( lock );
			
			THING result = pool.front();
			pool.pop_front();

			is_empty = pool.empty();

			return result;
		};

		void release(THING thing) {
			std::lock_guard<std::mutex> lock(mtx);
			
			pool.push_back( thing );
			is_empty = pool.empty();

			cond.notify_one();
		};
	
		void add(THING thing) {
			std::lock_guard<std::mutex> lock(mtx);
			
			pool.push_front( thing );
			is_empty = pool.empty();

			cond.notify_one();
		};
		
		bool empty() {
			std::lock_guard<std::mutex> lock(mtx);

			return is_empty;
		};
};

#endif
