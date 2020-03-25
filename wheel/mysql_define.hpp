#ifndef mysql_define_h__
#define mysql_define_h__
#include "reflection.hpp"

namespace wheel {
	namespace mysql {
		template <class T>
		struct identity {
		};

#define REGISTER_TYPE(Type, Index)                                              \
    inline constexpr int type_to_id(identity<Type>) noexcept { return Index; } \
    inline constexpr auto id_to_type( std::integral_constant<std::size_t, Index > ) noexcept { Type res{}; return res; }

		REGISTER_TYPE(char, MYSQL_TYPE_TINY)
			REGISTER_TYPE(short, MYSQL_TYPE_SHORT)
			REGISTER_TYPE(int, MYSQL_TYPE_LONG)
			REGISTER_TYPE(float, MYSQL_TYPE_FLOAT)
			REGISTER_TYPE(double, MYSQL_TYPE_DOUBLE)
			REGISTER_TYPE(int64_t, MYSQL_TYPE_LONGLONG)

			inline int type_to_id(identity<std::string>) noexcept { return MYSQL_TYPE_VAR_STRING; }
		inline std::string id_to_type(std::integral_constant<std::size_t, MYSQL_TYPE_VAR_STRING >) noexcept { std::string res{}; return res; }

		inline constexpr auto type_to_name(identity<char>) noexcept { return "TINYINT"; }
		inline constexpr auto type_to_name(identity<short>) noexcept { return "SMALLINT"; }
		inline constexpr auto type_to_name(identity<int>) noexcept { return "INTEGER"; }
		inline constexpr auto type_to_name(identity<float>) noexcept { return "FLOAT"; }
		inline constexpr auto type_to_name(identity<double>) noexcept { return "DOUBLE"; }
		inline constexpr auto type_to_name(identity<int64_t>) noexcept { return "BIGINT"; }
		inline auto type_to_name(identity<std::string>) noexcept { return "TEXT"; }
		template<size_t N>
		inline constexpr auto type_to_name(identity<std::array<char, N>>) noexcept {
			std::string s = "varchar(" + std::to_string(N) + ")";
			return s;
		}

		template <typename ... Args>
		struct value_of;

		template <typename T>
		struct value_of<T> {
			static  constexpr auto value = (wheel::reflector::get_size<T>());
		};

		template < typename T, typename ... Rest >
		struct value_of < T, Rest ... > {
			static  constexpr auto value = (value_of<T>::value + value_of<Rest...>::value);
		};

		template<typename List>
		struct result_size;

		template<template<class...> class List, class... T>
		struct result_size<List<T...>> {
			constexpr static const size_t value = value_of<T...>::value;// (wheel::reflector::get_size<T>() + ...);
		};

	}
}//wheel

#endif // mysql_define_h__
