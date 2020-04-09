#ifndef http_response_h__
#define http_response_h__

#include "unit.hpp"
#include <chrono>
#include "nlohmann_json.hpp"
#include <string>
#include "htpp_define.hpp"
#include "picohttpparser.hpp"
#include "itoa.hpp"

namespace wheel {
	namespace http_servers {
		class response {
		public:
			response() {
				std::string mbstr;
				mbstr.resize(50);
				std::time_t tm = std::chrono::system_clock::to_time_t(last_time_);
				std::strftime(&mbstr[0], mbstr.size(), "%a, %d %b %Y %T GMT", std::localtime(&tm));
				last_date_str_ = mbstr;
			}

			~response() = default;
			std::string& response_str() {
				return rep_str_;
			}

			template<status_type status, res_content_type content_type, size_t N>
			constexpr auto set_status_and_content(const char(&content)[N], content_encoding encoding = content_encoding::none) {
				constexpr auto status_str = to_rep_string(status);
				constexpr auto type_str = to_content_type_str(content_type);
				constexpr auto len_str = num_to_string<N - 1>::value;

				rep_str_.append(status_str).append(len_str.data(), len_str.size()).append(type_str).append(rep_server);

				using namespace std::chrono_literals;

				auto t = std::chrono::system_clock::now();
				if (t - last_time_ > 1s) {
					char mbstr[50];
					std::time_t tm = std::chrono::system_clock::to_time_t(t);
					std::strftime(mbstr, sizeof(mbstr), "%a, %d %b %Y %T GMT", std::localtime(&tm));
					last_date_str_ = mbstr;
					rep_str_.append("Date: ").append(mbstr).append("\r\n\r\n");
					last_time_ = t;
				}
				else {
					rep_str_.append("Date: ").append(last_date_str_).append("\r\n\r\n");
				}

				rep_str_.append(content);
			}

			void build_response_str() {
				rep_str_.append(to_rep_string(status_));

				if (!headers_.empty()) {
					for (auto& header : headers_) {
						rep_str_.append(header.first).append(":").append(header.second).append("\r\n");
					}
					headers_.clear();
				}

				char temp[20] = {};
				itoa_fwd((int)content_.size(), temp);
				rep_str_.append("Content-Length: ").append(temp).append("\r\n");
				if (res_type_ != res_content_type::none) {
					rep_str_.append(get_content_type(res_type_));
				}
				rep_str_.append("Server: http_server\r\n");

				using namespace std::chrono_literals;

				auto t = std::chrono::system_clock::now();
				if (t - last_time_ > 1s) {
					char mbstr[50];
					std::time_t tm = std::chrono::system_clock::to_time_t(t);
					std::strftime(mbstr, sizeof(mbstr), "%a, %d %b %Y %T GMT", std::localtime(&tm));
					last_date_str_ = mbstr;
					rep_str_.append("Date: ").append(mbstr).append("\r\n\r\n");
					last_time_ = t;
				}
				else {
					rep_str_.append("Date: ").append(last_date_str_).append("\r\n\r\n");
				}

				rep_str_.append(std::move(content_));
			}

			std::vector<boost::asio::const_buffer> to_buffers() {
				std::vector<boost::asio::const_buffer> buffers;
				add_header("Host", "http_server");

				buffers.reserve(headers_.size() * 4 + 5);
				buffers.emplace_back(to_buffer(status_));
				for (auto const& h : headers_) {
					buffers.emplace_back(boost::asio::buffer(h.first));
					buffers.emplace_back(boost::asio::buffer(name_value_separator));
					buffers.emplace_back(boost::asio::buffer(h.second));
					buffers.emplace_back(boost::asio::buffer(crlf));
				}

				buffers.push_back(boost::asio::buffer(crlf));

				if (body_type_ == content_type::string) {
					buffers.emplace_back(boost::asio::buffer(content_.data(), content_.size()));
				}

				return buffers;
			}

			void add_header(std::string&& key, std::string&& value) {
				headers_.emplace_back(std::move(key), std::move(value));
			}

			void set_status(status_type status) {
				status_ = status;
			}

			status_type get_status() const {
				return status_;
			}

			void set_delay(bool delay) {
				delay_ = delay;
			}

			void set_status_and_content(status_type status) {
				status_ = status;
				set_content(to_string(status).data());
				build_response_str();
			}

			void set_status_and_content(status_type status, std::string&& content, res_content_type res_type = res_content_type::none, content_encoding encoding = content_encoding::none) {
				status_ = status;
				res_type_ = res_type;

				set_content(std::move(content));

				build_response_str();
			}

			std::string get_content_type(res_content_type type) {
				switch (type) {
				case res_content_type::html:
					return rep_html;
				case res_content_type::json:
					return rep_json;
				case res_content_type::string:
					return rep_string;
				case res_content_type::multipart:
					return rep_multipart;
				case res_content_type::none:
				default:
					return "";
				}
			}

			bool need_delay() const {
				return delay_;
			}

			void set_continue(bool con) {
				proc_continue_ = con;
			}

			bool need_continue() const {
				return proc_continue_;
			}

			void set_content(std::string&& content) {
				body_type_ = content_type::string;
				content_ = std::move(content);
			}

			void set_chunked() {
				add_header("Transfer-Encoding", "chunked");
			}

			std::vector<boost::asio::const_buffer> to_chunked_buffers(const char* chunk_data, size_t length, bool eof) {
				std::vector<boost::asio::const_buffer> buffers;

				if (length > 0) {
					// convert bytes transferred count to a hex string.
					chunk_size_ = wheel::unit::to_hex_string(length);

					// Construct chunk based on rfc2616 section 3.6.1
					buffers.push_back(boost::asio::buffer(chunk_size_));
					buffers.push_back(boost::asio::buffer(crlf));
					buffers.push_back(boost::asio::buffer(chunk_data, length));
					buffers.push_back(boost::asio::buffer(crlf));
				}

				//append last-chunk
				if (eof) {
					buffers.push_back(boost::asio::buffer(last_chunk));
					buffers.push_back(boost::asio::buffer(crlf));
				}

				return buffers;
			}

			void set_domain(const std::string& domain) {
				domain_ = domain;
			}

			std::string get_domain() {
				return domain_;
			}

			void set_path(const std::string& path) {
				path_ = path;
			}

			std::string get_path() {
				return path_;
			}

			void set_url(const std::string& url)
			{
				raw_url_ = url;
			}

			std::string get_url(const std::string& url)
			{
				return raw_url_;
			}

			void set_headers(std::pair<phr_header*, size_t> headers) {
				req_headers_ = headers;
			}

			void render_json(const nlohmann::json& json_data)
			{
				set_status_and_content(status_type::ok, json_data.dump(), res_content_type::json, content_encoding::none);
			}

			void render_string(std::string&& content)
			{
				set_status_and_content(status_type::ok, std::move(content), res_content_type::string, content_encoding::none);
			}

			std::vector<std::string> raw_content() {
				return cache_data;
			}

			void redirect(const std::string& url, bool is_forever = false)
			{
				add_header("Location", url.c_str());
				is_forever == false ? set_status_and_content(status_type::moved_temporarily) : set_status_and_content(status_type::moved_permanently);
			}

			void redirect_post(const std::string& url) {
				add_header("Location", url.c_str());
				set_status_and_content(status_type::temporary_redirect);
			}

			void reset() {
				rep_str_.clear();

				res_type_ = res_content_type::none;
				status_ = status_type::init;
				proc_continue_ = true;
				delay_ = false;
				headers_.clear();
				cache_data.clear();
				content_.clear();
			}
		private:
			std::string get_header_value(const std::string& key) const {
				phr_header* headers = req_headers_.first;
				size_t num_headers = req_headers_.second;
				for (size_t i = 0; i < num_headers; i++) {
					if (wheel::unit::iequal(headers[i].name, headers[i].name_len, key.data(), key.length()))
						return std::string(headers[i].value, headers[i].value_len);
				}

				return {};
			}

			std::string raw_url_;
			std::vector<std::pair<std::string, std::string>> headers_;
			std::vector<std::string> cache_data;
			std::string content_;
			content_type body_type_ = content_type::unknown;
			status_type status_ = status_type::init;
			bool proc_continue_ = true;
			std::string chunk_size_;

			bool delay_ = false;

			std::pair<phr_header*, size_t> req_headers_;
			std::string domain_;
			std::string path_;
			std::string rep_str_;
			std::chrono::system_clock::time_point last_time_ = std::chrono::system_clock::now();
			std::string last_date_str_;
			res_content_type res_type_;
		};
	}
}
#endif // http_response_h__