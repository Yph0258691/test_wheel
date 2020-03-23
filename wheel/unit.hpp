#ifndef unit_h__
#define unit_h__
#include <string>
#include <unordered_set>
#include <random>
#include <boost/asio.hpp>
#include <memory>
#include <regex>
#include <iomanip>
#include <sstream>
#include <tuple>

namespace wheel {
	namespace unit {

#define PATTERN_IPV4   "^(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}$"
		/* IPv6 pattern */
#define PATTERN_IPV6   "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:)|(([0-9A-Fa-f]{1,4}:)"  \
                                       "{6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}(:[0-9A-Fa-f]{1,4}){1,2})|(([0-9A-Fa-f]{1,4}:)"  \
                                       "{4}(:[0-9A-Fa-f]{1,4}){1,3})|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){1,4})|(([0-9A-Fa-f]"  \
                                       "{1,4}:){2}(:[0-9A-Fa-f]{1,4}){1,5})|([0-9A-Fa-f]{1,4}:(:[0-9A-Fa-f]{1,4}){1,6})|(:(:[0-9A-Fa-f]"  \
                                       "{1,4}){1,7})|(([0-9A-Fa-f]{1,4}:){6}(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}" \
                                       "|2[0-4]\\d|25[0-5])){3})|(([0-9A-Fa-f]{1,4}:){5}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])" \
                                       "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4})" \
                                       "{0,1}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|" \
                                       "(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){0,2}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])" \
                                       "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4})" \
                                       "{0,3}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|" \
                                       "([0-9A-Fa-f]{1,4}:(:[0-9A-Fa-f]{1,4}){0,4}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])" \
                                       "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|(:(:[0-9A-Fa-f]{1,4})" \
                                       "{0,5}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}))$"

		static std::uint32_t float_to_uint32(float value)
		{
			return *(std::uint32_t*)(&value);
		}


		static float string_to_float(std::string str) {
			float res;
			std::stringstream stream(str);
			stream >> res;
			return res;
		}

		static std::string to_string_with_precision(const float a_value, int precison)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precison) << a_value;
			return out.str();
		}

		static std::string to_string_with_precision(const double a_value, int precison)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precison) << a_value;
			return out.str();

		}
		static float uint32_to_float(std::uint32_t value, int precison)
		{
			return  *(float*)(&value);
		}

		static std::int32_t float_to_int32(float value)
		{
			return *(float*)(&value);
		}

		static float int32_to_float(int32_t value)
		{
			return *(std::int32_t*)(&value);
		}

		static void bubble_sort_big(int array[], int n)
		{
			if (array == nullptr || n<=1){
				return;
			}

			bool flag = false;
			for (int i = 1; i < n; i++) {
				flag = false;
				for (int j = 0; j < n - i; j++) {
					if (array[j] < array[j + 1]) {
						flag = true;
						array[j] ^= array[j + 1];
						array[j + 1] ^= array[j];
						array[j] ^= array[j + 1];
					}
				}

				if (!flag) {
					break;
				}
			}
		}

		//从小到大，插入排序
		static void insert_sort(int* arr, int n) {
			int temp = -1;
			for (int i = 1;i < n;++i) {
				temp = arr[i];

				int j = i - 1;
				//从后往前搬动数据
				for (;j >= 0;--j) {
					if (arr[j] <= temp) {
						break;
					}

					arr[j + 1] = arr[j];
				}

				//当前的后一个位置，放入数据
				arr[j + 1] = temp;
			}
		}

		//选择排序
		static void selection_sort(int* ptr, int len)
		{
			if (ptr == NULL || len <= 1) {
				return;
			}

			int minindex = -1;
			//i是次数，也即排好的个数;j是继续排
			for (int i = 0;i < len - 1;++i) {
				minindex = i;
				for (int j = i + 1;j < len;++j) {
					//从小到大
					if (ptr[j] < ptr[minindex]) {
						minindex = j;
					}
				}

				//这里一定要加上,比如(5,8,5,2,9,2,1,10)
				if (i == minindex) {
					continue;
				}

				ptr[i] ^= ptr[minindex];
				ptr[minindex] ^= ptr[i];
				ptr[i] ^= ptr[minindex];

			}
		}

		static void trim(std::string& s)
		{
			if (s.empty()) {
				return;
			}

			s.erase(0, s.find_first_not_of(" "));
			s.erase(s.find_last_not_of(" ") + 1);
		}

		static std::string find_substr(const std::string& str, const std::string key, const std::string& diml) {
			std::string value;
			std::size_t begin = str.find(key, 0);
			if (begin != std::string::npos) {
				++begin;
				std::size_t falg_pos = str.find(diml, begin);
				if (falg_pos != std::string::npos) {
					++falg_pos;
					std::size_t end = str.find("\r\n", falg_pos);
					if (end != std::string::npos) {
						value = str.substr(falg_pos, end - falg_pos);
					}
				}
			}

			trim(value);

			return value;
		}

		//单个tuple去索引
		template <typename Tuple, typename F, std::size_t...Is>
		void tuple_switch(const std::size_t i, Tuple&& t, F&& f, std::index_sequence<Is...>) {
			[](...) {}(
				(i == Is && (
				(void)std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))), false))...
				);
		}

		template <typename Tuple, typename F>
		void tuple_switch(const std::size_t i, Tuple&& t, F&& f) {
			static constexpr auto N =
				std::tuple_size<std::remove_reference_t<Tuple>>::value;

			tuple_switch(i, std::forward<Tuple>(t), std::forward<F>(f),
				std::make_index_sequence<N>{});
		}

		/**********使用例子********/

		//auto const t = std::make_tuple(42, 'z', 3.14, 13, 0, "Hello, World!");

		//for (std::size_t i = 0; i < std::tuple_size<decltype(t)>::value; ++i) {
		//	wheel::unit::tuple_switch(i, t, [](const auto& v) {
		//		std::cout << v << std::endl;
		//		});


		template<typename F, typename...Ts, std::size_t...Is>
		void for_each_tuple_front(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>) {
			using expander = int[];
			(void)expander {
				((void)std::forward<F>(func)(std::get<Is>(tuple), std::integral_constant<size_t, Is>{}), false)...
			};
		}

		template<typename F, typename...Ts>
		void for_each_tuple_front(const std::tuple<Ts...>& tuple, F func) {
			for_each_tuple_front(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
		}

		template<typename F, typename...Ts, std::size_t...Is>
		void for_each_tuple_back(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>) {
			//匿名构造函数调用
			[](...) {}(0,
				((void)std::forward<F>(func)(std::get<Is>(tuple), std::integral_constant<size_t, Is>{}), false)...
				);
		}

		template<typename F, typename...Ts>
		void for_each_tuple_back(std::tuple<Ts...>& tuple, F func) {
			for_each_tuple_back(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
		}


		/***************使用列子*****************/
		//auto some = std::make_tuple("I am good", 255, 2.1);
		//for_each_tuple(some, [](const auto& x, auto index) {
		//	constexpr auto Idx = decltype(index)::value;
		//	std::cout << x << std::endl;
		//	}
		//);

		static 	int domain_name_query(const std::string& domain_name, std::vector< std::string >& ips)
		{
			boost::asio::io_service io_service;

			boost::asio::ip::tcp::resolver resolver(io_service);

			boost::asio::ip::tcp::resolver::query q(boost::asio::ip::tcp::v4(), domain_name);

			boost::system::error_code ec;
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(q, ec);

			if (ec.value() == 0)
			{
				boost::asio::ip::tcp::resolver::iterator end;
				for (; endpoint_iterator != end; ++endpoint_iterator)
				{
					ips.push_back((*endpoint_iterator).endpoint().address().to_string());
				}
			}

			return ec.value();
		}

		static 	bool ipAddr_check(const std::string& ip_addr_dot_format)
		{

			std::regex expression(PATTERN_IPV4);
			return (std::regex_match(ip_addr_dot_format, expression));

			//std::regex expression("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
			//return (std::regex_match(ip_addr_dot_format, expression));

			//std::regex pattern(("((([01]?\\d\\d?)|(2[0-4]\\d)|(25[0-5]))\\.){3}(([01]?\\d\\d?)|(2[0-4]\\d)|(25[0-5]))"));
			//return std::regex_match(ip_addr_dot_format, pattern);
		}

		static 	bool ipV6_check(const std::string& ip_addr_dot_format)
		{
			std::regex expression(PATTERN_IPV6);
			return (std::regex_match(ip_addr_dot_format, expression));
		}

		static void random_str(std::string& random_str, int len) {
			std::unordered_set<char> set;
			for (int i = 0;i < len;i++) {
				static std::default_random_engine e(time(0));
				static std::uniform_int_distribution<unsigned int> u(1, 3);
				uint32_t ret = u(e);
				char c = 0;
				std::unordered_set<char>::const_iterator iter_find;
				switch (ret)
				{
				case 1:
					if (!set.empty()) {
						while (1) {
							c = 'A' + rand() % 26;
							iter_find = set.find(c);
							if (iter_find != set.end()) {
								break;
							}

							continue;
						}

					}
					else {
						c = 'A' + rand() % 26;
					}

					random_str.push_back(c);
					break;
				case 2:
					if (!set.empty()) {
						while (1) {
							c = 'a' + rand() % 26;
							iter_find = set.find(c);
							if (iter_find != set.end()) {
								break;
							}

							continue;
						}
					}
					else {
						c = 'a' + rand() % 26;
					}

					random_str.push_back(c);
					break;
				case 3:
					if (!set.empty()) {
						while (1) {
							c = 0 + rand() % 10;
							iter_find = set.find(c);
							if (iter_find != set.end()) {
								break;
							}

							continue;
						}
					}
					else {
						c = 0 + rand() % 10;
					}

					random_str.push_back(c);
					break;
				default:
					break;
				}
			}


		}
	}
}

#endif // unit_h__
