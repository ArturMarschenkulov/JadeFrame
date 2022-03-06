#pragma once
#include <algorithm>

namespace JadeFrame {

//template<typename T>
class DynamicArray {

	using T = int;
public:
	DynamicArray() {
		m_size = 0;
		m_capacity = 1;
		m_array = new T[m_capacity];
	}
	DynamicArray(const DynamicArray&) = delete;
	DynamicArray(DynamicArray&&) = delete;
	auto operator=(const DynamicArray& other) -> DynamicArray& {
		//m_size = 0;
		//m_capacity = 0;
		//delete[] m_array;
		this->~DynamicArray();

		for(int i = 0; i < other.m_size; i++) {
			this->push_back(other.at(i));
		}
	}
	auto operator=(DynamicArray&&) -> DynamicArray& = delete;

	DynamicArray(const std::initializer_list<T>& list) {
		m_size = 0;
		m_capacity = 1;
		m_array = new T[m_capacity];

		this->push_back(list);
	}
	~DynamicArray() {
		m_size = 0;
		m_capacity = 0;
		delete[] m_array;
	}
	auto at(const size_t& index) const -> T& {
		return m_array[index];
	}
	auto push_back(const T& elem) -> void {
		const auto growth = m_capacity * 2;
		if (m_size == m_capacity) {
			this->reserve(growth);
		}
		m_array[m_size] = elem;
		m_size++;
	}
	auto push_back(std::initializer_list<T> list) -> void {
		for (const DynamicArray::T& elem : list) {
			this->push_back(elem);
		}
	}
	auto reserve(const size_t new_capacity) -> void {
		T* temp = new T[new_capacity];
		std::copy(m_array, m_array + m_size, temp);
		delete[] m_array;
		m_capacity = new_capacity;
		m_array = temp;
	}

private:
	T* m_array;
	size_t m_size;
	size_t m_capacity;
};

auto dafoo() -> void {
	DynamicArray da = { 1, 2, 3, 4 };
}

}