//
//  tsqueue.h
//  GameServerVK
//
//  Created by Veronika Kotckovich on 2021-05-30.
//

#ifndef tsqueue_h
#define tsqueue_h
#include "common.h"

namespace server
{
	template <typename T>
	class tsqueue
	{
	public:
		tsqueue() = default;
		tsqueue(const tsqueue<T>&) = delete;
		virtual ~tsqueue() { clear(); }
	
		// Returns and maintains item at front of Queue
		const T& front()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}
	
		// Returns and maintains item at back of Queue
		const T& back()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}
	
		// Adds items to back of Queue
		void push_back(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_back(std::move(item));
		}
	
		// Adds items to front of Queue
		void push_front(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_front(std::move(item));
		}
	
		// Returns true if Queue has no items
		bool isEmpty()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.empty();
		}
	
		// Returns number of the items
		size_t conut()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.size();
		}
	
		// Clear Queue
		void clear()
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.clear();
		}
	
		void wait()
		{
			while (isEmpty())
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.wait(ul);
			}
		}
	
		// Removes and retunrns item from front of Queue
		T pop_front(){
			std::scoped_lock lock(muxQueue);
			auto t = std::move(deqQueue.front());
			deqQueue.pop_front();
			return t;
		}
	
		// Removes and retunrns item from back of Queue
		T pop_back(){
			std::scoped_lock lock(muxQueue);
			auto t = std::move(deqQueue.back());
			deqQueue.pop_back();
			return t;
		}
			
	protected:
		std::mutex muxQueue;
		std::deque<T> deqQueue;
		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}
#endif /* tsqueue_h */
