#pragma once
#include <memory>
#include <compare>
#include <utility>
#include <concepts>
#include <iterator>
#include <algorithm>
#include <exception>


// TODO: Provide natvis vizualizer file

template <typename Tp, std::size_t Capacity = 32>
class static_vector {
public:
    using value_type             = Tp;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = Tp&;
    using const_reference        = const Tp&;
    using pointer                = Tp*;
    using const_pointer          = const Tp*;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// HACK: What I actually want to write, but don't know how, is:
	// template <std::size_t Capacity_>
	// friend class static_vector<Tp, Capacity_>;	// Same type but different capacity
	template <typename Tp_, std::size_t Capacity_>
	friend class static_vector;

private:
	alignas(Tp) std::byte m_buffer[sizeof(Tp) * Capacity];
	size_type m_size = 0;

public:
	constexpr static_vector() = default;

	constexpr static_vector(size_type count) {
		resize(count);
	}

	constexpr static_vector(size_type count, const Tp& value) {
		assign(count, value);
	}

	template <std::input_iterator InputIt>
	constexpr static_vector(InputIt first, InputIt last) {
		assign(first, last);
	}

	constexpr static_vector(const static_vector& other) {
		assign(other.begin(), other.begin() + other.size());
	}
	
	constexpr static_vector(static_vector&& other) noexcept {
		swap(other);
	}

	constexpr static_vector(std::initializer_list<Tp> list) {
		assign(list);
	}

	// TODO: This works but seems kinda sketchy, make sure it's not undefined behavior
	template <size_type Capacity_>
	constexpr static_vector(const static_vector<Tp, Capacity_>& other) {
		assign(other.begin(), other.begin() + other.size());
	}

	template <size_type Capacity_>
	constexpr static_vector(static_vector<Tp, Capacity_>&& other) noexcept {
		swap(other);
	}

	constexpr ~static_vector() {
		std::destroy(begin(), end());
	}

	constexpr static_vector& operator=(const static_vector& other) {
		assign(other.begin(), other.begin() + other.size());
		return *this;
	}
	
	// FIXME: Elements are pointlessly moved before being destroyed
	constexpr static_vector& operator=(static_vector&& other) noexcept {
		swap(other);
		return *this;
	}
	
	constexpr static_vector& operator=(std::initializer_list<Tp> list) {
		assign(list);
		return *this;
	}
	
	// TODO: This works but seems kinda sketchy, make sure it's not undefined behavior
	template <size_type Capacity_>
	constexpr static_vector& operator=(const static_vector<Tp, Capacity_>& other) {
		assign(other.begin(), other.begin() + other.size());
		return *this;
	}

	// FIXME: Elements are pointlessly moved before being destroyed
	template <size_type Capacity_>
	constexpr static_vector& operator=(static_vector<Tp, Capacity_>&& other) noexcept {
		swap(other);
		return *this;
	}

private:
	constexpr void assign_impl(size_type assign_count, auto func) {
		if (assign_count > Capacity) {
			throw std::bad_alloc{};
		}
		
		for (size_type i = 0; i < assign_count; ++i) {
			if (i < size()) {
				get(i) = func(i);
			} else {
				std::construct_at(data() + i, func(i));
			}
		}

		if (assign_count < size()) {
			std::destroy(begin() + assign_count, end());
		}

		m_size = assign_count;
	}

public:
	constexpr void assign(size_type count, const Tp& value) {
		assign_impl(count, [&](size_type) -> const auto& {
			return value;
		});
	}

	template <std::input_iterator InputIt>
	constexpr void assign(InputIt first, InputIt last) {
		// TODO: Complexity could be improved
		assign_impl(std::distance(first, last), [&](size_type) -> const auto& {
			return *first++;
		});
	}

	constexpr void assign(std::initializer_list<Tp> list) {
		auto it = list.begin();
		assign_impl(list.size(), [&](size_type i) -> const auto& {
			return *it++;
		});
	}

private:
	constexpr reference get(size_type pos) {
		return data()[pos];
	}

	constexpr const_reference get(size_type pos) const {
		return data()[pos];
	}

public:
	constexpr reference at(size_type pos) {
		if (pos >= size()) {
			throw std::out_of_range{ "Attempted to retrive a value outside the range of the container." };
		}

		return get(pos);
	}

	constexpr const_reference at(size_type pos) const {
		if (pos >= size()) {
			throw std::out_of_range{ "Attempted to retrive a value outside the range of the container." };
		}

		return get(pos);
	}

	constexpr reference operator[](size_type pos) {
		return get(pos);
	}

	constexpr const reference operator[](size_type pos) const {
		return get(pos);
	}

	constexpr reference front() {
		return *data();
	}

	constexpr const_reference front() const {
		return *data();
	}

	constexpr reference back() {
		return get(size() - 1);
	}

	constexpr const_reference back() const {
		return get(size() - 1);
	}

	// TODO: reinterpret_cast isn't allowed in constant expression evaluation,
	// but I don't know if there is any way of fixing this
	constexpr pointer data() noexcept {
		return std::launder(reinterpret_cast<Tp*>(m_buffer));
	}

	constexpr const_pointer data() const noexcept {
		return std::launder(reinterpret_cast<const Tp*>(m_buffer));
	}
	
	constexpr iterator begin() noexcept {
		return data();
	}

	constexpr const_iterator begin() const noexcept {
		return data();
	}

	constexpr const_iterator cbegin() const noexcept {
		return data();
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

	constexpr auto rbegin() noexcept {
		return reverse_iterator{ end() };
	}

	constexpr auto rbegin() const noexcept {
		return const_reverse_iterator{ end() };
	}

	constexpr auto crbegin() const noexcept {
		return const_reverse_iterator{ cend() };
	}

	constexpr auto rend() noexcept {
		return reverse_iterator{ begin() };
	}

	constexpr auto rend() const noexcept {
		return const_reverse_iterator{ begin() };
	}

	constexpr auto crend() const noexcept {
		return const_reverse_iterator{ cbegin() };
	}

	[[nodiscard]]
	constexpr bool empty() const noexcept {
		return (m_size == 0);
	}

	constexpr size_type size() const noexcept {
		return m_size;
	}

	constexpr size_type max_size() const noexcept {
		return Capacity;
	}
	
	constexpr void reserve(size_type new_cap) { /* NOTE: NoOp */ }

	constexpr size_type capacity() const noexcept {
		return Capacity;
	}

	constexpr void shrink_to_fit() { /* NOTE: NoOp */ }

	constexpr void clear() noexcept {
		std::destroy(begin(), end());
		m_size = 0;
	}

private:
	template <typename Func>
	constexpr iterator insert_impl(const_iterator pos, size_type count, Func func) {
		if (size() + count > capacity()) {
			throw std::bad_alloc{};
		}

		const auto mut_pos = begin() + (pos - cbegin());
		std::move_backward(mut_pos, end(), end() + count);
		func(mut_pos);
		m_size += count;

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

	constexpr iterator erase(const_iterator pos) {
		return erase(pos, pos + 1);
	}
	
	constexpr iterator erase(const_iterator first, const_iterator last) {
		const auto mut_pos = begin() + (first - cbegin());
		const auto count = std::distance(first, last);
		std::move(mut_pos + count, end(), mut_pos);
		std::destroy(end() - count, end());

		m_size -= count;
		return mut_pos;
	}

	constexpr void push_back(const Tp& value) {
		if (m_size >= Capacity) {
			throw std::bad_alloc{};
		}

		std::construct_at(data() + (m_size++), value);
	}

	constexpr void push_back(Tp&& value) {
		if (m_size >= Capacity) {
			throw std::bad_alloc{};
		}

		std::construct_at(data() + (m_size++), std::move(value));
	}

	template <typename... Args>
	constexpr reference emplace_back(Args&&... args) {
		if (m_size >= Capacity) {
			throw std::bad_alloc{};
		}

		return *std::construct_at(data() + (m_size++), std::forward<Args>(args)...);
	}

	constexpr void pop_back() {
		std::destroy_at(data() + (--m_size));
	}

	constexpr void resize(size_type count) {
		if (count > Capacity) {
			throw std::bad_alloc{};
		}

		for (size_type i = size(); i < count; ++i) {
			std::construct_at(data() + i);
		}

		if (count < size()) {
			std::destroy(begin() + count, end());
		}

		m_size = count;
	}

	constexpr void resize(size_type count, const value_type& value) {
		assign(count, value);
	}

	// If one vector's size is greater than the other's vector capacity, all the extra elements will be silently discarded
	template <size_type Capacity_>
	constexpr void swap(static_vector<Tp, Capacity_>& other) noexcept {
		if ((void*)this == (void*)&other) {
			return;
		}

		const auto overlap_count = std::min(size(), other.size());
		auto it1 = begin() + overlap_count;
		auto it2 = std::swap_ranges(begin(), begin() + overlap_count, other.begin());

		if (size() > other.size()) {
			const auto smaller_size = std::min(Capacity_, size());
			for (; it1 != begin() + smaller_size; ++it1, ++it2) {
				std::construct_at(std::addressof(*it2), std::move(*it1));
			}

			std::destroy(begin() + other.size(), end());
			m_size = std::exchange(other.m_size, smaller_size);
		} 
		else if (size() < other.size()) {
			const auto smaller_size = std::min(Capacity, other.size());
			for (; it2 != other.begin() + smaller_size; ++it1, ++it2) {
				std::construct_at(std::addressof(*it1), std::move(*it2));
			}

			std::destroy(other.begin() + size(), other.end());
			other.m_size = std::exchange(m_size, smaller_size);
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

template <std::input_iterator InputIt>
static_vector(InputIt, InputIt) -> static_vector<typename std::iterator_traits<InputIt>::value_type>;
