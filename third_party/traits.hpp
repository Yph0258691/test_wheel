#ifndef traits_h__
#define traits_h__

#include <type_traits>
#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <queue>
#include <list>

namespace wheel {
	namespace traits {
		template< class T >
		struct is_signed_intergral_like : std::integral_constant < bool,
			(std::is_integral<T>::value) &&
			std::is_signed<T>::value
		> {};

		template< class T >
		struct is_unsigned_intergral_like : std::integral_constant < bool,
			(std::is_integral<T>::value) &&
			std::is_unsigned<T>::value
		> {};

		template < template <typename...> class U, typename T >
		struct is_template_instant_of : std::false_type {};

		template < template <typename...> class U, typename... args >
		struct is_template_instant_of< U, U<args...> > : std::true_type {};

		template<typename T>
		struct is_stdstring : is_template_instant_of < std::basic_string, T >
		{};

		template<typename T>
		struct is_tuple : is_template_instant_of < std::tuple, T >
		{};

		template< class T >
		struct is_sequence_container : std::integral_constant < bool,
			is_template_instant_of<std::deque, T>::value ||
			is_template_instant_of<std::list, T>::value ||
			is_template_instant_of<std::vector, T>::value ||
			is_template_instant_of<std::queue, T>::value
		> {};

		template< class T >
		struct is_associat_container : std::integral_constant < bool,
			is_template_instant_of<std::map, T>::value ||
			is_template_instant_of<std::unordered_map, T>::value
		> {};

		template< class T >
		struct is_emplace_back_able : std::integral_constant < bool,
			is_template_instant_of<std::deque, T>::value ||
			is_template_instant_of<std::list, T>::value ||
			is_template_instant_of<std::vector, T>::value
		> {};

		template<class...> struct disjunction : std::false_type { };
		template<class B1> struct disjunction<B1> : B1 { };
		template<class B1, class... Bn>
		struct disjunction<B1, Bn...>
			: std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> { };

		template <typename T, typename Tuple>
		struct has_type;

		template <typename T, typename... Us>
		struct has_type<T, std::tuple<Us...>> : disjunction<std::is_same<T, Us>...> {};

		struct nonesuch {
			nonesuch() = delete;
			~nonesuch() = delete;
			nonesuch(const nonesuch&) = delete;
			void operator=(const nonesuch&) = delete;
		};

		template<class Default, class AlwaysVoid,
			template<class...> class Op, class... Args>
		struct detector {
			using value_t = std::false_type;
			using type = Default;
		};

		template <typename ...Ts> struct make_void
		{
			using type = void;
		};
		template <typename ...Ts> using void_t = typename make_void<Ts...>::type;

		template<class Default, template<class...> class Op, class... Args>
		struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
			using value_t = std::true_type;
			using type = Op<Args...>;
		};

		template<template<class...> class Op, class... Args>
		using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

		template<template<class...> class Op, class... Args>
		using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

		template<class T, typename... Args>
		using has_before_t = decltype(std::declval<T>().before(std::declval<Args>()...));

		template<class T, typename... Args>
		using has_after_t = decltype(std::declval<T>().after(std::declval<Args>()...));
		template<typename T, typename... Args>
		using has_before = is_detected<has_before_t, T, Args...>;

		template<typename T, typename... Args>
		using has_after = is_detected<has_after_t, T, Args...>;


		//c++17写法
		//template <typename T, typename... Us>
		//struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {}

		template<typename T>
		constexpr bool  is_int64_v = std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value;

		template<class T>
		constexpr bool is_char_array_v = std::is_array<T>::value && std::is_same<char, std::remove_pointer_t<std::decay_t<T>>>::value;

		template<typename T>
		struct is_string
			: public disjunction<std::is_same<char*, typename std::decay<T>::type>,
			std::is_same<const char*,typename std::decay<T>::type>,
			std::is_same<std::string, typename std::decay<T>::type>
			> {

			};

	}//traits
}//wheel

#endif // traits_h__
