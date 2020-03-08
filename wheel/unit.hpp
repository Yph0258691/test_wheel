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

namespace wheel {
	namespace unit {
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
			std::regex expression("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
			return (std::regex_match(ip_addr_dot_format, expression));
		}

		static void random_str(std::string& random_str,int len) {
			std::unordered_set<char> set;
			for (int i = 0;i < len;i++) {
				static std::default_random_engine e(time(0));
				static std::uniform_int_distribution<unsigned int> u(1,3);
				uint32_t ret = u(e);
				char c =0;
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

					}else {
						c = 'A' + rand() % 26;
					}

					random_str.push_back(c);
					break;
				case 2:
					if (!set.empty()){
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
					if (!set.empty()){
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
