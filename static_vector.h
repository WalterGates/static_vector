#pragma once
#include <array>
#include <exception>
#include <algorithm>
#include <concepts>
#include <compare>


template <typename Tp, std::size_t Capacity>
class static_vector : public std::array<Tp, Capacity> {
private:
	using base_type = std::array<Tp, Capacity>;
	
public:
	// Inhereted typedefs
    using value_type             = base_type::value_type;
    using size_type              = base_type::size_type;
    using difference_type        = base_type::difference_type;
    using reference              = base_type::reference;
    using const_reference        = base_type::const_reference;
    using pointer                = base_type::pointer;
    using const_pointer          = base_type::const_pointer;
    using iterator               = base_type::iterator;
    using const_iterator         = base_type::const_iterator;
    using reverse_iterator       = base_type::reverse_iterator;
    using const_reverse_iterator = base_type::const_reverse_iterator;

	// Inhereted member functions
	using base_type::operator[];
	using base_type::front;
	using base_type::data;
	using base_type::begin;
	using base_type::cbegin;
	using base_type::rbegin;
	using base_type::crbegin;
	using base_type::max_size;

	// HACK: What I actually want to write, but don't know how, is:
	// template <std::size_t Capacity_>
	// friend class static_vector<Tp, Capacity_>;	// Same type but different capacity
	template <typename Tp_, std::size_t Capacity_>
	friend class static_vector;

private:
	size_type m_size = 0;

public:
	constexpr static_vector() = default;

	constexpr static_vector(size_type count) {
		assign(count, Tp());
	}

	constexpr static_vector(size_type count, const Tp& value) {
		assign(count, value);
	}

	template <std::input_iterator InputIt>
	constexpr static_vector(InputIt first, InputIt last) {
		assign(first, last);
	}

	// If attempting to copy more elements from 'other' than is allowed by the current capacity, the function
	// will silently stop after copying this->capacity() elements
	template <size_type Capacity_>
	constexpr static_vector(const static_vector<Tp, Capacity_>& other) {
		m_size = std::min(Capacity, other.size());
		std::copy(other.begin(), other.begin() + m_size, begin());
	}
	
	// If attempting to copy more elements from 'other' than is allowed by the current capacity, the function
	// will silently stop after copying this->capacity() elements
	template <size_type Capacity_>
	constexpr static_vector(static_vector<Tp, Capacity_>&& other) noexcept {
		swap(*this, other);
	}

	constexpr static_vector(std::initializer_list<Tp> list) {
		assign(list);
	}
	
	// If attempting to copy more elements from 'other' than is allowed by the current capacity, the function
	// will silently stop after copying this->capacity() elements
	template <size_type Capacity_>
	constexpr static_vector& operator=(const static_vector<Tp, Capacity_>& other) {
		m_size = std::min(Capacity, other.size());
		std::copy(other.begin(), other.begin() + m_size, begin());
		return *this;
	}
	
	// If attempting to copy more elements from 'other' than is allowed by the current capacity, the function
	// will silently stop after copying this->capacity() elements
	template <size_type Capacity_>
	constexpr static_vector& operator=(static_vector<Tp, Capacity_>&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	constexpr static_vector& operator=(std::initializer_list<Tp> list) {
		assign(list);
		return *this;
	}

	constexpr void assign(size_type count, const Tp& value) {
		if (count > Capacity) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		std::fill(begin(), begin() + count, value);
		m_size = count;
	}

	template <std::input_iterator InputIt>
	constexpr void assign(InputIt first, InputIt last) {
		m_size = 0;

		for (auto it = begin(); first != last; ++it, ++first) {
			if (it == end()) {
				throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
			}

			*it = *first;
			++m_size;
		}
	}

	constexpr void assign(std::initializer_list<Tp> list) {
		if (list.size() > Capacity) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		std::copy(list.begin(), list.end(), begin());
		m_size = list.size();
	}

	constexpr reference at(size_type pos) {
		if (pos >= size()) {
			throw std::out_of_range{ "Attempted to retrive a value outside the range of the container." };
		}

		return operator[](pos);
	}

	constexpr const_reference at(size_type pos) const {
		if (pos >= size()) {
			throw std::out_of_range{ "Attempted to retrive a value outside the range of the container." };
		}

		return operator[](pos);
	}

	constexpr reference back() {
		return operator[](size() - 1);
	}

	constexpr const_reference back() const {
		return operator[](size() - 1);
	}

	constexpr iterator end() noexcept {
		return begin() + m_size;
	}

	constexpr const_iterator end() const noexcept {
		return begin() + m_size;
	}

	constexpr const_iterator cend() const noexcept {
		return cbegin() + m_size;
	}

	constexpr reverse_iterator rend() noexcept {
		return rbegin() + m_size;
	}

	constexpr const_reverse_iterator rend() const noexcept {
		return rbegin() + m_size;
	}

	constexpr const_reverse_iterator crend() const noexcept {
		return crbegin() + m_size;
	}

	[[nodiscard]]
	constexpr bool empty() const noexcept {
		return (m_size == 0);
	}

	constexpr size_type size() const noexcept {
		return m_size;
	}
	
	constexpr void reserve(size_type new_cap) { /* NOTE: NoOp */ }

	constexpr size_type capacity() const noexcept {
		return Capacity;
	}

	constexpr void shrink_to_fit() { /* NOTE: NoOp */ }

	// FIXME: RAII resources aren't released
	constexpr void clear() noexcept {
		m_size = 0;
	}

private:
	constexpr iterator insert_impl(const_iterator pos, size_type insert_count, auto insert_func) {
		if (size() + insert_count > capacity()) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		const auto mut_pos = begin() + (pos - cbegin());
		std::move_backward(mut_pos, end(), end() + insert_count);
		m_size += insert_count;
		insert_func(mut_pos);

		return mut_pos;
	}

public:
	constexpr iterator insert(const_iterator pos, const Tp& value) {
		return insert_impl(pos, 1, [&](iterator mut_pos) {
			*mut_pos = value;
		});
	}

	constexpr iterator insert(const_iterator pos, Tp&& value) {
		return insert_impl(pos, 1, [&](iterator mut_pos) {
			*mut_pos = std::move(value);
		});
	}

	constexpr iterator insert(const_iterator pos, size_type count, const Tp& value) {
		return insert_impl(pos, count, [&](iterator mut_pos) {
			std::fill(mut_pos, mut_pos + count, value);
		});
	}

	template <std::input_iterator InputIt>
	constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
		// TODO: std::distance() increases runtime complexity, consider trading time for space.
		// That is the aproach of the standard library
		return insert_impl(pos, std::distance(first, last), [&](iterator mut_pos) {
			std::copy(first, last, mut_pos);
		});
	}

	constexpr iterator insert(const_iterator pos, std::initializer_list<Tp> list) {
		return insert_impl(pos, list.size(), [&](iterator mut_pos) {
			std::copy(list.begin(), list.end(), mut_pos);
		});
	}

	template <typename... Args>
	constexpr iterator emplace(const_iterator pos, Args&&... args) {
		return insert_impl(pos, 1, [&](iterator mut_pos) {
			*mut_pos = Tp(std::forward<Args>(args)...);
		});
	}

	// FIXME: RAII resources aren't released when erasing the last element
	constexpr iterator erase(const_iterator pos) {
		const auto mut_pos = begin() + (pos - cbegin());
		const auto count = 1;
		std::move(mut_pos + count, end(), mut_pos);
		m_size -= count;

		return mut_pos;
	}
	
	// FIXME: RAII resources aren't released when the to-be-erased range is at the end of the container
	constexpr iterator erase(const_iterator first, const_iterator last) {
		const auto mut_pos = begin() + (first - cbegin());
		const auto count = (last - first);
		std::move(mut_pos + count, end(), mut_pos);
		m_size -= count;

		return mut_pos;
	}

	constexpr void push_back(const Tp& value) {
		if (m_size >= Capacity) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		operator[](m_size++) = value;
	}

	constexpr void push_back(Tp&& value) {
		if (m_size >= Capacity) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		operator[](m_size++) = std::move(value);
	}

	template <typename... Args>
	constexpr iterator emplace_back(Args&&... args) {
		if (m_size >= Capacity) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		operator[](m_size++) = Tp(std::forward<Args>(args)...);
	}

	// FIXME: RAII resources aren't released
	constexpr void pop_back() {
		--m_size;
	}

	constexpr void resize(size_type count) {
		// FIXME: Call the default constructor multiple times
		resize(count, Tp());
	}

	constexpr void resize(size_type count, const value_type& value) {
		if (count > Capacity) {
			throw std::out_of_range{ "Attempted to insert a value past the capacity of the container." };
		}

		if (count > size()) {
			std::fill(end(), begin() + count, value);
		} else {
			// FIXME: RAII resources aren't released when reducing the size
		}
		
		m_size = count;
	}

	// If one vector's size is greater than the other's vector capacity, all those extra elements will be silently discarded
	template <size_type Capacity_>
	constexpr void swap(static_vector<Tp, Capacity_>& other) noexcept {
		auto it1 = begin();
		auto it2 = other.begin();
		std::swap(m_size, other.m_size);

		for (size_type i = 0; i < std::min(size(), other.size()); ++i) {
			std::iter_swap(it1++, it2++);
		}

		// FIXME: Call destructors for the elements that didn't fit in the vector with smaller capacity
		if (size() > other.size()) {
			std::move(it1, begin() + Capacity_, it2);
		} 
		else if (size() < other.size()) {
			std::move(it2, other.begin() + Capacity, it2);
		}
	}
};

template <typename Tp, std::size_t Capacity>
constexpr bool operator==(
	const static_vector<Tp, Capacity>& rhs,
	const static_vector<Tp, Capacity>& lhs
) {
	return std::equal(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Tp, std::size_t Capacity1, std::size_t Capacity2>
constexpr bool operator==(
	const static_vector<Tp, Capacity1>& rhs,
	const static_vector<Tp, Capacity1>& lhs
) {
	return std::equal(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Tp, std::size_t Capacity>
constexpr auto operator<=>(
	const static_vector<Tp, Capacity>& rhs,
	const static_vector<Tp, Capacity>& lhs
) {
	return std::lexicographical_compare_three_way(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Tp, std::size_t Capacity1, std::size_t Capacity2>
constexpr auto operator<=>(
	const static_vector<Tp, Capacity1>& rhs,
	const static_vector<Tp, Capacity1>& lhs
) {
	return std::lexicographical_compare_three_way(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

namespace std {

	// If one vector's size is greater than the other's vector capacity, all those extra elements will be silently discarded
	template <typename Tp, std::size_t Capacity>
	constexpr void swap(static_vector<Tp, Capacity>& lhs, static_vector<Tp, Capacity>& rhs) noexcept {
		lhs.swap(rhs);
	}

	// If one vector's size is greater than the other's vector capacity, all those extra elements will be silently discarded
	template <typename Tp, std::size_t Capacity1, std::size_t Capacity2>
	constexpr void swap(static_vector<Tp, Capacity1>& lhs, static_vector<Tp, Capacity2>& rhs) noexcept {
		lhs.swap(rhs);
	}

	template <typename Tp, std::size_t Capacity, typename U>
	constexpr auto erase(static_vector<Tp, Capacity>& vec, const U& value) {
		const auto it = std::remove(vec.begin(), vec.end(), value);
		const auto removed_count = std::distance(it, vec.end());
		vec.erase(it, vec.end());
		
		return removed_count;
	}

	template <typename Tp, std::size_t Capacity, typename Pred>
	constexpr auto erase_if(static_vector<Tp, Capacity>& vec, Pred pred) {
		const auto it = std::remove_if(vec.begin(), vec.end(), pred);
		const auto removed_count = std::distance(it, vec.end());
		vec.erase(it, vec.end());
		
		return removed_count;
	}
}

template <typename T, typename... U>
static_vector(T, U...) -> static_vector<T, 1 + sizeof...(U)>;
