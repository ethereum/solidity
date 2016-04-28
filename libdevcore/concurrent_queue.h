/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <utility>
#include <queue>
#include <condition_variable>
#include <mutex>


namespace dev
{

/// Concurrent queue.
/// You can push and pop elements to/from the queue. Pop will block until the queue is not empty.
/// The default backend (_QueueT) is std::queue. It can be changed to any type that has
/// proper push(), pop(), empty() and front() methods.
template<typename _T, typename _QueueT = std::queue<_T>>
class concurrent_queue
{
public:
	template<typename _U>
	void push(_U&& _elem)
	{
		{
			std::lock_guard<decltype(x_mutex)> guard{x_mutex};
			m_queue.push(std::forward<_U>(_elem));
		}
		m_cv.notify_one();
	}

	_T pop()
	{
		std::unique_lock<std::mutex> lock{x_mutex};
		m_cv.wait(lock, [this]{ return !m_queue.empty(); });
		auto item = std::move(m_queue.front());
		m_queue.pop();
		return item;
	}

private:
	_QueueT m_queue;
	std::mutex x_mutex;
	std::condition_variable m_cv;
};

}
