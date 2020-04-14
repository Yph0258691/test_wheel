#ifndef encoding_conversion_h__
#define encoding_conversion_h__

#include <codecvt>
#include <locale>
#include <utility>
#include <string>

namespace wheel {
	namespace char_encoding {
		template<class Facet>
		struct deletable_facet : Facet
		{
			using Facet::Facet; // 继承构造函数
			~deletable_facet() {}
		};

		using mbs_facet_t = deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>>;

		class encoding_conversion {
		public:
			encoding_conversion() = delete;
			encoding_conversion(const encoding_conversion&) = delete;
			encoding_conversion(encoding_conversion&&) = delete;
			~encoding_conversion() = delete;
			encoding_conversion& operator=(const encoding_conversion&) = delete;
			encoding_conversion& operator=(encoding_conversion&&) = delete;
			static std::string   to_string(const std::wstring& wstr)
			{
				setlocale(LC_ALL, "");
				//算出代转string字节
				std::int64_t size = wcstombs(NULL, wstr.c_str(), 0);
				std::string desrt;
				desrt.resize(size);
				wcstombs(&desrt[0], wstr.c_str(), size);
				return std::move(desrt);

				//此方法调试时看不见中文
				//setlocale(LC_ALL, "");
				//std::wstring_convert<mbs_facet_t> converter(new mbs_facet_t(std::locale().name()));
				//return std::move(converter.to_bytes(wstr));
			}

			static std::wstring   to_wstring(const std::string& str)
			{
				setlocale(LC_ALL, "");
				std::int64_t size = mbstowcs(NULL, str.c_str(), 0);
				std::wstring w_str;
				w_str.resize(size);

				//算出代转wstring字节
				mbstowcs(&w_str[0], str.c_str(), str.size());
				return std::move(w_str);

				//此方法调试时看不见中文
				//setlocale(LC_ALL, "");
				//std::wstring_convert<mbs_facet_t> converter(new mbs_facet_t(std::locale().name()));
				//return std::move(converter.from_bytes(str));
			}

			static std::string    gbk_to_utf8(const std::string& str)
			{
				return to_utf8(from_gbk(str));
			}

			static std::string   utf8_to_gbk(const std::string& str)
			{
				return to_gbk(from_utf8(str));
			}

			static std::u16string utf8_to_utf16(const std::string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> convert;
				auto tmp = convert.from_bytes(str.data(), str.data() + str.size());
				return std::u16string(tmp.data(), tmp.data() + tmp.size());
#else
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				return convert.from_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::u32string utf8_to_utf32(const std::string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> convert;
				auto tmp = convert.from_bytes(str.data(), str.data() + str.size());
				return std::u32string(tmp.data(), tmp.data() + tmp.size());
#else
				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				return convert.from_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::string    utf16_to_utf8(const std::u16string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> convert;
				return convert.to_bytes((uint16_t*)str.data(), (uint16_t*)str.data() + str.size());
#else
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				return convert.to_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::u32string utf16_to_utf32(const std::u16string& str)
			{
				std::string bytes;
				bytes.reserve(str.size() * 2);

				for (const char16_t ch : str) {
					bytes.push_back((uint8_t)(ch / 256));
					bytes.push_back((uint8_t)(ch % 256));
				}

#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf16<uint32_t>, uint32_t> convert;
				auto tmp = convert.from_bytes(bytes);
				return std::u32string(tmp.data(), tmp.data() + tmp.size());
#else
				std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
				return convert.from_bytes(bytes);
#endif
			}

			static std::string    utf32_to_utf8(const std::u32string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> convert;
				return convert.to_bytes((uint32_t*)str.data(), (uint32_t*)str.data() + str.size());
#else
				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				return convert.to_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::u16string utf32_to_utf16(const std::u32string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf16<uint32_t>, uint32_t> convert;
				std::string bytes = convert.to_bytes((uint32_t*)str.data(), (uint32_t*)str.data() + str.size());
#else
				std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
				std::string bytes = convert.to_bytes(str.data(), str.data() + str.size());
#endif

				std::u16string result;
				result.reserve(bytes.size() / 2);

				for (size_t i = 0; i < bytes.size(); i += 2)
					result.push_back((char16_t)((uint8_t)(bytes[i]) * 256 + (uint8_t)(bytes[i + 1])));

				return result;
			}

			static bool is_valid_utf8(const char* string)
			{
				if (string == nullptr) {
					return false;
				}

				const unsigned char* bytes = (const unsigned char*)string;
				unsigned int cp = 0;
				int num = 0;

				while (*bytes != 0x00) {
					if ((*bytes & 0x80) == 0x00) {
						// U+0000 to U+007F 
						cp = (*bytes & 0x7F);
						num = 1;
					}
					else if ((*bytes & 0xE0) == 0xC0) {
						// U+0080 to U+07FF 
						cp = (*bytes & 0x1F);
						num = 2;
					}
					else if ((*bytes & 0xF0) == 0xE0) {
						// U+0800 to U+FFFF 
						cp = (*bytes & 0x0F);
						num = 3;
					}
					else if ((*bytes & 0xF8) == 0xF0) {
						// U+10000 to U+10FFFF 
						cp = (*bytes & 0x07);
						num = 4;
					}
					else {
						return false;
					}

					bytes += 1;
					for (int i = 1; i < num; ++i) {
						if ((*bytes & 0xC0) != 0x80)
							return false;
						cp = (cp << 6) | (*bytes & 0x3F);
						bytes += 1;
					}

					if ((cp > 0x10FFFF) ||
						((cp >= 0xD800) && (cp <= 0xDFFF)) ||
						((cp <= 0x007F) && (num != 1)) ||
						((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
						((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
						((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4))) {
						return false;
					}

				}

				return true;
			}

		private:
			static std::string    to_gbk(const std::wstring& wstr)
			{
				std::wstring_convert<mbs_facet_t> conv(new mbs_facet_t(gbk_locale_name));
				std::string  str = conv.to_bytes(wstr);
				return str;

			}
			static std::wstring   from_gbk(const std::string& str)
			{
				std::wstring_convert<mbs_facet_t> conv(new mbs_facet_t(gbk_locale_name));
				std::wstring wstr = conv.from_bytes(str);
				return wstr;

			}
			static std::string    to_utf8(const std::wstring& wstr)
			{
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				return convert.to_bytes((char16_t*)wstr.data(), (char16_t*)wstr.data() + wstr.size());
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
				std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				return convert.to_bytes(wstr.data(), wstr.data() + wstr.size());
#elif defined(_WIN32) || defined(_WIN64)
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
				return convert.to_bytes(wstr.data(), wstr.data() + wstr.size());
#endif
			}
			static std::wstring   from_utf8(const std::string& str)
			{
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				auto tmp = convert.from_bytes(str.data(), str.data() + str.size());
				return std::wstring(tmp.data(), tmp.data() + tmp.size());
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
				std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				return convert.from_bytes(str.data(), str.data() + str.size());
#elif defined(_WIN32) || defined(_WIN64)
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
				return convert.from_bytes(str.data(), str.data() + str.size());
#endif
			}
#if defined(_WIN32) || defined(_WIN64)
			constexpr static const char* gbk_locale_name = ".936";
#else
			constexpr static const char* gbk_locale_name = "zh_CN.GBK";
#endif
		};
	}
}

#endif // encoding_conversion_h__
