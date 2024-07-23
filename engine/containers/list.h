#pragma once

#include "platform/platform.h"

#include <cstring>
#include <cassert>
#include <type_traits>
#include <utility>
#include <new>

static size_t s_ResizeFactor = 2;

template<typename T>
class list {
	using Element = T;
	using ElementPtr = T*;
	using ElementRef = T&;
    using ConstElementRef = const T&;
	using ElementRValue = T&&;
public:
    template<typename Ty>
	class Iterator {
	public:
		Iterator(ElementPtr ptr)
			:
			m_Ptr(ptr)
		{}
		
		ElementRef operator*() { return *m_Ptr; }
		ElementPtr operator->() { return m_Ptr; }
        ElementRef operator[](int index) { return *(m_Ptr + index); }

		Iterator& operator++() { m_Ptr++; return *this; }
		Iterator operator++(int) { Iterator it = *this; ++(*this); return it; }

        Iterator& operator--() { m_Ptr--; return *this; }
		Iterator operator--(int) { Iterator it = *this; --(*this); return it; }

		friend bool operator==(const Iterator& a, const Iterator& b) { return a.m_Ptr == b.m_Ptr; }
		friend bool operator!=(const Iterator& a, const Iterator& b) { return a.m_Ptr != b.m_Ptr; }

        bool operator==(const Iterator& other) { return this->m_Ptr == other.m_Ptr; }
        bool operator!=(const Iterator& other) { return this->m_Ptr != other.m_Ptr; }

	private:
		ElementPtr m_Ptr;
	};

public:
	list(size_t capacity)
		:
		m_Elements(allocate(capacity)),
		m_Size(0),
		m_Capacity(capacity)
	{
		static_assert(std::is_copy_constructible_v<Element>, "The template argument must be copy constructible to be used in the list.");
		static_assert(std::is_move_constructible_v<Element>, "The template argument must be move constructible to be used in the list.");
	}

	list()
		:
		m_Elements(nullptr),
		m_Size(0),
		m_Capacity(0)
	{
		static_assert(std::is_copy_constructible_v<Element>, "The template argument must be copy constructible to be used in the list.");
		static_assert(std::is_move_constructible_v<Element>, "The template argument must be move constructible to be used in the list.");
	}

	list(const list& rhs) 
		:
		m_Elements(allocate(rhs.m_Capacity)),
		m_Size(rhs.m_Size),
		m_Capacity(rhs.m_Capacity) {
		
		assert(m_Elements != nullptr);

		for (size_t i = 0; i < m_Size; i++) {
			ElementPtr ptr = &m_Elements[i];
			new (static_cast<void*>(ptr)) Element(rhs.m_Elements[i]);
		}
	}

	list(list&& rhs) 
		:
		m_Elements(rhs.m_Elements),
		m_Size(rhs.m_Size),
		m_Capacity(rhs.m_Capacity) {
		rhs.m_Elements = nullptr;
		rhs.m_Size = 0;
		rhs.m_Capacity = 0;
	}

    list operator=(const list& rhs) {
        if (rhs.m_Elements == nullptr) {
            m_Elements = nullptr;
            m_Capacity = rhs.m_Capacity;
            m_Size = rhs.m_Size;
            return *this;
        }

        m_Elements = allocate(rhs.m_Capacity);
		m_Size = rhs.m_Size;
		m_Capacity = rhs.m_Capacity;

		for (size_t i = 0; i < m_Size; i++) {
			ElementPtr ptr = &m_Elements[i];
			new (static_cast<void*>(ptr)) Element(rhs.m_Elements[i]);
		}

        return *this;
	}

	~list() {
		delete[] m_Elements;
		m_Size = 0;
		m_Capacity = 0;
	}

	void push_back(ConstElementRef element) {
		if (m_Capacity == 0) {
			reserve(2);
		}
		if (m_Size >= m_Capacity) {
			reserve(size() * s_ResizeFactor);
			m_Elements[m_Size++] = element;
			return;
		}
		if (m_Capacity != 0 && m_Size < m_Capacity) {
			m_Elements[m_Size++] = element;
		}
	}

	void push_back(ElementRValue element) {
		if (m_Size >= m_Capacity) {
			if (m_Capacity == 0) {
				reserve(2);
			} else {
				reserve(m_Capacity * s_ResizeFactor);
			}
		}
		
		if (m_Capacity != 0 && m_Size < m_Capacity) {
			ElementPtr ptr = &m_Elements[m_Size++];
			new (static_cast<void*>(ptr)) Element(std::forward<Element>(element));
		}
	}

	size_t find_index(ConstElementRef element) const {
		if (m_Size != 0) {
			for (size_t i = 0; i < size(); i++) {
				ConstElementRef el = m_Elements[i];
				if (el == element) {
					return i;
				}
			}
		}
		return -1;
	}

	void remove_last() {
		if (m_Size != 0) {
			ElementRef el = m_Elements[m_Size];
			el.~Element();
			Platform::zero_memory(&m_Elements[m_Size], sizeof(Element));
		}
	}

	void remove_at(size_t index) {
		assert(index < m_Size);
		assert(m_Size > 0);
		ElementRef el = m_Elements[index];
		el.~Element();
		Platform::zero_memory(&m_Elements[index], sizeof(Element));
		
		// Yep. memmove on instances. ikr
		if (index < (m_Size - 1)) {
			memmove(&m_Elements[index], &m_Elements[index + 1], (index - size()) * sizeof(Element));
		}

		if (m_Size - 1 > 0) {
			m_Size--;
		}
	}

	void remove_all() {
		for (size_t i = 0; i < size(); i++) {
			m_Elements[i].~Element();
		}
		Platform::zero_memory(m_Elements, sizeof(Element) * size());
	}

	void set_resize_factor(float resize_factor) {
		assert(resize_factor >= 2);
		s_ResizeFactor = resize_factor;
	}

	void resize(size_t new_size) {
        reserve(new_size);
        m_Size = new_size;
	}

	bool is_empty() {
		return m_Capacity == 0;
	}

    void reserve(size_t new_size) {
        ElementPtr new_elements = allocate(new_size);

		for (size_t i = 0; i < size(); i++) {
			ElementPtr ptr = &new_elements[i];
			new (static_cast<void*>(ptr)) Element(std::move(m_Elements[i]));
		}
		delete[] m_Elements;
		m_Elements = new_elements;
		m_Capacity = new_size;
    }
	
	size_t size() const { return m_Size; }
	uint32_t size_u32() const { return (uint32_t)m_Size; }

	ConstElementRef operator[](size_t index) const { 
		assert(index < m_Capacity);
		return m_Elements[index];
	}

	ElementRef operator[](size_t index) { 
		assert(index < m_Capacity);
		return m_Elements[index]; 
	}

	const Iterator<T> begin() const { return Iterator<T>(m_Elements); }
	const Iterator<T> end() const { return Iterator<T>(m_Elements + m_Size); }

	Iterator<T> begin() { return Iterator<T>(m_Elements); }
	Iterator<T> end() { return Iterator<T>(m_Elements + m_Size); }

    const Iterator<T> begin(const ElementPtr elements) const { return Iterator<T>(elements); }
	const Iterator<T> end(const ElementPtr elements) const { return Iterator<T>(elements); }

	Iterator<T> begin(ElementPtr elements) { return Iterator<T>(elements); }
	Iterator<T> end(ElementPtr elements) { return Iterator<T>(elements); }

    const ElementPtr data() const {
        return m_Elements;
    }

    ElementPtr data() {
        return m_Elements;
    }

private:
	ElementPtr allocate(size_t element_count) {
		ElementPtr elements = new Element[element_count];

		// memsetting the array to 0 if Element == number
		if (std::is_arithmetic_v<Element>) {
			Platform::zero_memory(elements, sizeof(Element) * element_count);
		}
		
		return elements;
	}

private:
	ElementPtr m_Elements;
	size_t m_Size;
	size_t m_Capacity;
};