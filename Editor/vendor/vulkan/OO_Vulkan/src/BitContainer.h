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
	BitContainer();
	~BitContainer();

	int32_t Add(const T& obj);
	void Remove(int32_t id);
	T& Get(int32_t id);
	void Clear();

	auto Raw();

	T& operator[](size_t i);

private:
	std::bitset<MAX_OBJECTS> m_bits;
	std::vector<T> m_data;
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
inline int32_t BitContainer<T, MAX_OBJECTS>::Add(const T& obj)
{
	int32_t id = -1;

	for (size_t i = 0; i < m_bits.size(); i++)
	{
		if (m_bits[i] == false)
		{
			m_bits[i] = true;
			id = i;

			m_data[id] = obj;
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
		m_bits[i] == false;
	}
}

template<typename T, int32_t MAX_OBJECTS>
inline auto BitContainer<T, MAX_OBJECTS>::Raw()
{
	return std::tuple<std::bitset<MAX_OBJECTS>&, std::vector<T>&>{ m_bits,m_data };
}

template<typename T, int32_t MAX_OBJECTS>
inline T& BitContainer<T, MAX_OBJECTS>::operator[](size_t i)
{
	return Get(i);
}
