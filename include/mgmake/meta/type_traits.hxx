#pragma once

#ifndef MGMAKE_META_TYPE_TRAITS_HXX
#define MGMAKE_META_TYPE_TRAITS_HXX

#include <array>
#include <type_traits>
#include <vector>

namespace mgmake::meta {
	template<typename T>
	struct array_traits : std::false_type {};

	template<typename T, std::size_t size_v>
	struct array_traits<std::array<T, size_v>> : std::true_type {
		using type = std::array<T, size_v>;
		using value_type = T;

		static inline constexpr std::size_t size = size_v;

		using size_type = typename type::size_type;
		using difference_type = typename type::difference_type;
		using reference = typename type::reference;
		using const_reference = typename type::const_reference;
		using pointer = typename type::pointer;
		using const_pointer = typename type::const_pointer;
		using iterator = typename type::iterator;
		using const_iterator = typename type::const_iterator;
	};

	template<typename T>
	using array_traits_t = array_traits<std::remove_cvref_t<T>>;

	template<typename T>
	concept array_type = array_traits_t<T>::value;

	template<typename T>
	inline constexpr bool is_array_v = array_traits<std::remove_cvref_t<T>>::value;

	template<typename T>
	struct vector_traits : std::false_type {};

	template<typename T, typename allocator_t>
	struct vector_traits<std::vector<T, allocator_t>> : std::true_type {
		using type = std::vector<T, allocator_t>;
		using value_type = T;
		using allocator_type = allocator_t;

		using size_type = typename type::size_type;
		using difference_type = typename type::difference_type;
		using reference = typename type::reference;
		using const_reference = typename type::const_reference;
		using pointer = typename type::pointer;
		using const_pointer = typename type::const_pointer;
		using iterator = typename type::iterator;
		using const_iterator = typename type::const_iterator;
	};

	template<typename T>
	using vector_traits_t = vector_traits<std::remove_cvref_t<T>>;

	template<typename T>
	concept vector_type = vector_traits_t<T>::value;

	template<typename T>
	inline constexpr bool is_vector_v = vector_traits<std::remove_cvref_t<T>>::value;
}

#endif // MGMAKE_META_TYPE_TRAITS_HXX