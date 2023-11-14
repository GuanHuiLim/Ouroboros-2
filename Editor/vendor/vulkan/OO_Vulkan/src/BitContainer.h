/************************************************************************************//*!
\file           BitContainer.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Bit container class which holds static data and indexes to objects

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "VulkanUtils.h" // this is probably bad
#include <memory>
#include <bitset>
#include <vector>
#include <tuple>

template <typename T, int32_t MAX_OBJECTS = 2048>
class BitContainer
{
public:
	struct Iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = T;
		using pointer           = T*;  // or also value_type*
		using reference         = T&;  // or also value_type&

		Iterator(pointer ptr, pointer begin, pointer end, std::bitset<MAX_OBJECTS>* bits) 
			: m_ptr(ptr),
				m_begin{begin},
				m_end{end},
				m_b{bits}
		{}
		reference operator*() const { assert("Dereferenced Bitcontainer end ptr!" && m_ptr < m_end);  return *m_ptr; }
		pointer operator->() { assert("Dereferenced Bitcontainer end ptr!" && m_ptr < m_end); return m_ptr; }

		// Prefix increment
		Iterator& operator++();

		// Postfix increment
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

		size_t index() const;

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; }; 
		friend bool operator- (const Iterator& a, const Iterator& b) { return a.m_ptr - b.m_ptr; }; 
	private:
		pointer m_ptr;
		pointer m_begin;
		pointer m_end;
		std::bitset<MAX_OBJECTS>* m_b;
		
	};

	BitContainer();
	~BitContainer();

	Iterator begin();
	Iterator end();

	int32_t Add(const T& obj);
	void Remove(int32_t id);
	T& Get(int32_t id);
	void Clear();
	size_t size();

	auto Raw();
	std::vector<T>& buffer();

	T& operator[](size_t i);



private:
	std::bitset<MAX_OBJECTS> m_bits{};
	std::vector<T> m_data{};

	size_t m_size{};
};

template <typename T, int32_t MAX_OBJECTS>
BitContainer<T,MAX_OBJECTS>::BitContainer()
{
	m_data.resize(m_bits.size());
}

template <typename T, int32_t MAX_OBJECTS>
BitContainer<T,MAX_OBJECTS>::~BitContainer()
{
}

template<typename T, int32_t MAX_OBJECTS>
inline typename BitContainer<T, MAX_OBJECTS>::Iterator BitContainer<T, MAX_OBJECTS>::begin()
{	
	int i = 0;
	while (i < m_bits.size())
	{
		if (m_bits[i] == true)
		{
			return Iterator(m_data.data() + i,m_data.data(),m_data.data() + m_bits.size(),&m_bits);
		}
		else
		{
			++i;
		}
	};
	return end();
}

template<typename T, int32_t MAX_OBJECTS>
inline typename BitContainer<T, MAX_OBJECTS>::Iterator BitContainer<T, MAX_OBJECTS>::end()
{
	return Iterator(m_data.data() + m_bits.size(), m_data.data(), m_data.data() + m_bits.size(), &m_bits);
}

template<typename T, int32_t MAX_OBJECTS>
inline int32_t BitContainer<T, MAX_OBJECTS>::Add(const T& obj)
{
	int32_t id = -1;

	for (size_t i = 0; i < m_bits.size(); i++)
	{
		if (m_bits[i] == false)
		{
			m_bits[i] = true;
			id = int32_t(i);

			m_data[id] = obj;
			++m_size;
			return id;
		}
	}
	
	assert(false); // shouldnt hit here. TODO: proper errors
	return id;
}

template<typename T, int32_t MAX_OBJECTS>
inline void BitContainer<T, MAX_OBJECTS>::Remove(int32_t id)
{
	if (m_bits[id] == true)
	{
		m_bits[id] = false;
		--m_size;
		return;
	}
	assert(false); // removed invalid object
}

template<typename T, int32_t MAX_OBJECTS>
inline T& BitContainer<T, MAX_OBJECTS>::Get(int32_t id)
{
	if (m_bits[id] == true)
	{		
		return m_data[id];
	}
	assert(false); // invalid access
	return m_data[0];
}

template<typename T, int32_t MAX_OBJECTS>
inline void BitContainer<T, MAX_OBJECTS>::Clear()
{
	for (size_t i = 0; i < m_bits.size(); i++)
	{
		m_bits[i] = false;
	}
	m_size = 0;
}

template<typename T, int32_t MAX_OBJECTS>
inline size_t BitContainer<T, MAX_OBJECTS>::size()
{
	return m_size;
}

template<typename T, int32_t MAX_OBJECTS>
inline auto BitContainer<T, MAX_OBJECTS>::Raw()
{
	return std::tuple<std::bitset<MAX_OBJECTS>&, std::vector<T>&>{ m_bits,m_data };
}

template<typename T, int32_t MAX_OBJECTS>
inline std::vector<T>& BitContainer<T, MAX_OBJECTS>::buffer()
{
	return m_data;
}

template<typename T, int32_t MAX_OBJECTS>
inline T& BitContainer<T, MAX_OBJECTS>::operator[](size_t i)
{
	return Get(i);
}

template<typename T, int32_t MAX_OBJECTS>
inline typename BitContainer<T, MAX_OBJECTS>::Iterator& BitContainer<T, MAX_OBJECTS>::Iterator::operator++()
{
	size_t i = m_ptr - m_begin;
	while (++i < m_b->size())
	{
		if ((*m_b)[i] == true)
		{
			m_ptr = m_begin + i;
			return *this;
		}
	};
	m_ptr = m_end;
	return *this;
}

template<typename T, int32_t MAX_OBJECTS>
inline size_t BitContainer<T, MAX_OBJECTS>::Iterator::index() const
{
	return m_ptr - m_begin;
}
