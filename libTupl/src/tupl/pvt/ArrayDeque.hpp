/*
Copyright (C) 2014      Pooja Nagpal

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _TUPL_PVT_ARRAYDEQUE_HPP
#define _TUPL_PVT_ARRAYDEQUE_HPP

#include <type_traits>

namespace tupl { namespace pvt {

/**
Fixed size circular deque
*/
template<typename T, std::size_t N>
class ArrayDeque {
public:
	ArrayDeque();
	~ArrayDeque();

	void emplaceFront(T&& val);
	void emplaceBack(T&& val);

	void popFront();
	void popBack();

	const T& front() const;
	const T& back() const;

	T& front();
	T& back();

	bool empty() const;
	bool full() const;

	std::size_t size() const;

	void clear();

	/**
	Disable copy and move operations
	*/
	ArrayDeque(const ArrayDeque&) = delete;
	ArrayDeque& operator=(const ArrayDeque&) = delete;
	ArrayDeque(const ArrayDeque&&) = delete;
	ArrayDeque& operator=(const ArrayDeque&&) = delete;

private:
	std::size_t mSize;
	std::size_t mFrontIdx;
	std::size_t mBackIdx;	

	/**
	Use aligned_storage over type T to avoid default
	construction of N elements
	*/ 
	typename std::aligned_storage<sizeof(T), alignof(T)>::type mElems[N];
};

} } // namespace tupl::pvt

#include "ArrayDeque.ii"

#endif