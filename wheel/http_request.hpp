#ifndef http_request_h__
#define http_request_h__
#include <string>
#include <vector>
#include <map>
#include "htpp_define.hpp"
#include "picohttpparser.hpp"
#include "url_encode_decode.hpp"
#include "http_multipart_reader.hpp"
#include "upload_file.hpp"

namespace wheel {
	namespace http_servers {
		enum class data_proc_state : int8_t {
			data_begin,
			data_continue,
			data_end,
			data_all_end,
			data_close,
			data_error
		};

		enum parse_status {
			complete = 0,
			has_error = -1,
			not_complete = -2,
		};

		class request {
		public:
			request() {
				buffer_.resize(1024);
			}

			char* buffer(){
				return &buffer_[cur_size_];
			}
			const char* buffer(size_t size) const {
				return &buffer_[size];
			}

			size_t left_size() {
				return buffer_.size() - cur_size_;
			}

			size_t left_body_size() {
				auto size = buffer_.size();
				return left_body_len_ > size ? size : left_body_len_;
			}

			size_t current_size() const {
				return cur_size_;
			}

			size_t header_len() const {
				return header_len_;
			}

			void reduce_left_body_size(size_t size) {
				left_body_len_ -= size;
			}

			std::pair<phr_header*, size_t> get_headers() {
				if (copy_headers_.empty())
					return { headers_ , num_headers_ };

				num_headers_ = copy_headers_.size();
				for (size_t i = 0; i < num_headers_; i++) {
					headers_[i].name = copy_headers_[i].first.data();
					headers_[i].name_len = copy_headers_[i].first.size();
					headers_[i].value = copy_headers_[i].second.data();
					headers_[i].value_len = copy_headers_[i].second.size();
				}
				return { headers_ , num_headers_ };
			}

			std::string get_method() const {
				if (method_len_ != 0)
					return { method_ , method_len_ };

				return { method_str_.data(), method_str_.length() };
			}

			std::string get_url() const {
				if (method_len_ != 0)
					return { url_, url_len_ };

				return { url_str_.data(), url_str_.length() };
			}

			void set_state(data_proc_state state) {
				state_ = state;
			}

			void set_part_data(std::string&& data) {
				part_data_ = data;
			}

			std::string get_part_data() const {
				if (has_gzip_) {
					return { gzip_str_.data(), gzip_str_.length() };
				}

				return part_data_;
			}


			size_t left_body_len() const {
				size_t size = buffer_.size();
				return left_body_len_ > size ? size : left_body_len_;
			}

			bool body_finished() {
				return left_body_len_ == 0;
			}

			void handle_multipart_key_value() {
				if (multipart_form_map_.empty()) {
					return;
				}

				for (auto& pair : multipart_form_map_) {
					form_url_map_.emplace(std::string(pair.first.data(), pair.first.size()),
						std::string(pair.second.data(), pair.second.size()));
				}
			}

			std::map<std::string, std::string> get_form_url_map() const {
				return form_url_map_;
			}


			std::string get_query_value(size_t n) {
				auto get_val = [&n](auto& map) {
					auto it = map.begin();
					std::advance(it, n);
					return it->second;
				};

				if (n >= queries_.size()) {
					if (n >= form_url_map_.size())
						return {};

					return get_val(form_url_map_);
				}
				else {
					return get_val(queries_);
				}
			}

			template<typename T>
			T get_query_value(std::string key) {
				static_assert(std::is_arithmetic<T>::value);
				auto val = get_query_value(key);
				if (val.empty()) {
					throw std::logic_error("empty value");
				}

				if constexpr (std::is_same<T, int32_t>::value || std::is_same<T, uint32_t>::value ||
					std::is_same<T, bool>::value || std::is_same<T, char>::value || std::is_same<T, short>::value) {
					int r = std::atoi(val.data());
					if (val[0] != '0' && r == 0) {
						throw std::invalid_argument(std::string(val) + ": is not an integer");
					}
					return r;
				}
				else if constexpr (std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value) {
					auto r = std::atoll(val.data());
					if (val[0] != '0' && r == 0) {
						throw std::invalid_argument(std::string(val) + ": is not an integer");
					}
					return r;
				}
				else if constexpr (std::is_floating_point<T>::value) {
					char* end;
					auto f = strtof(val.data(), &end);
					if (val.back() != *(end - 1)) {
						throw std::invalid_argument(std::string(val) + ": is not a float");
					}
					return f;
				}
				else {
					throw std::invalid_argument("not support the value type");
				}
			}

			std::string get_query_value(std::string key) {
				auto url = get_url();
				url = url.length() > 1 && url.back() == '/' ? url.substr(0, url.length() - 1) : url;
				std::string map_key = std::string(url.data(), url.size()) + std::string(key.data(), key.size());
				auto it = queries_.find(key);
				if (it == queries_.end()) {
					auto itf = form_url_map_.find(key);
					if (itf == form_url_map_.end())
						return {};

					if (code_utils::is_url_encode(itf->second))
					{
						auto ret = utf8_character_params_.emplace(map_key, code_utils::get_string_by_urldecode(itf->second));
						return std::string(ret.first->second.data(), ret.first->second.size());
					}
					return itf->second;
				}
				if (code_utils::is_url_encode(it->second))
				{
					auto ret = utf8_character_params_.emplace(map_key, code_utils::get_string_by_urldecode(it->second));
					return std::string(ret.first->second.data(), ret.first->second.size());
				}
				return it->second;
			}

			bool update_and_expand_size(size_t size) {
				if (update_size(size)) { //at capacity
					return true;
				}

				if (cur_size_ >= buffer_.size()) {
					resize_double();
				}

				return false;
			}
			bool parse_form_urlencoded() {
				form_url_map_.clear();

				auto body_str = body();
				form_url_map_ = parse_query(body_str);
				if (form_url_map_.empty())
					return false;

				return true;
			}

			bool is_http11() {
				return minor_version_ == 1;
			}

			bool is_chunked() const {
				return is_chunked_;
			}

			int parse_header(std::size_t last_len, size_t start) {
				if (!copy_headers_.empty()) {
					copy_headers_.clear();
				}

				num_headers_ = sizeof(headers_) / sizeof(headers_[0]);
				header_len_ = phr_parse_request(buffer_.data(), cur_size_, &method_,
					&method_len_, &url_, &url_len_,
					&minor_version_, headers_, &num_headers_, last_len);

				if (header_len_ < 0) {
					return header_len_;
				}
					
				check_gzip();
				auto header_value = get_header_value("content-length");
				if (header_value.empty()) {
					auto transfer_encoding = get_header_value("transfer-encoding");
					if (transfer_encoding == "chunked") {
						is_chunked_ = true;
					}

					body_len_ = 0;
				}
				else {
					set_body_len(atoll(header_value.data()));
				}

				auto cookie = get_header_value("cookie");
				if (!cookie.empty()) {
					cookie_str_ = std::string(cookie.data(), cookie.length());
				}

				raw_url_ ={ url_, url_len_ };
				size_t npos = raw_url_.find('/');
				if (npos == std::string::npos) {
					return -1;
				}
					
				size_t pos = raw_url_.find('?');
				if (pos != std::string::npos) {
					queries_ = parse_query(std::string{ raw_url_ }.substr(pos + 1, url_len_ - pos - 1));
					url_len_ = pos;
				}

				return header_len_;
			}
			const char* data() {
				return buffer_.data();
			}

			void set_http_type(content_type type) {
				http_type_ = type;
			}

			content_type get_content_type() const {
				return http_type_;
			}

			data_proc_state get_state() const {
				return state_;
			}

			bool at_capacity() {
				return (header_len_ + body_len_) > MaxSize;
			}

			bool at_capacity(size_t size) {
				return size > MaxSize;
			}


			bool has_recieved_all() {
				return (total_len() == current_size());
			}

			void expand_size() {
				auto total = total_len();
				auto size = buffer_.size();
				if (size == MaxSize)
					return;

				if (total < MaxSize) {
					if (total > size)
						resize(total);
				}
				else {
					resize(MaxSize);
				}
			}

			bool has_body() const {
				return body_len_ != 0 || is_chunked_;
			}

			std::string get_header_value(std::string key) const {
				if (copy_headers_.empty()) {
					for (size_t i = 0; i < num_headers_; i++) {
						if (wheel::unit::iequal(headers_[i].name, headers_[i].name_len, key.data(), key.length()))
							return std::string(headers_[i].value, headers_[i].value_len);
					}

					return {};
				}

				auto it = std::find_if(copy_headers_.begin(), copy_headers_.end(), [key](auto& pair) {
					if (wheel::unit::iequal(pair.first.data(), pair.first.size(), key.data())) {
						return true;
					}

					return false;
					});

				if (it != copy_headers_.end()) {
					return (*it).second;
				}

				return {};
			}
			bool update_size(size_t size) {
				cur_size_ += size;
				if (cur_size_ > MaxSize) {
					return true;
				}

				return false;
			}
			bool has_recieved_all_part() {
				return (body_len_ == cur_size_ - header_len_);
			}

			void fit_size() {
				auto total = left_body_len_;// total_len();
				auto size = buffer_.size();
				if (size == MaxSize)
					return;

				if (total < MaxSize) {
					if (total > size)
						resize(total);
				}
				else {
					resize(MaxSize);
				}
			}

			void set_current_size(size_t size) {
				cur_size_ = size;
				if (size == 0) {
					copy_method_url_headers();
				}
			}

			void reset() {
				cur_size_ = 0;
				is_chunked_ = false;
				state_ = data_proc_state::data_begin;
				part_data_ = {};;
				queries_.clear();
				cookie_str_.clear();
				form_url_map_.clear();
				multipart_form_map_.clear();
				utf8_character_params_.clear();
				utf8_character_pathinfo_params_.clear();
				copy_headers_.clear();
				files_.clear();
			}

			void set_multipart_headers(const multipart_headers& headers) {
				for (auto pair : headers) {
					multipart_headers_[std::string(pair.first.data(), pair.first.size())] = std::string(pair.second.data(), pair.second.size());
				}
			}

			std::string get_multipart_field_name(const std::string& field_name) const {
				if (multipart_headers_.empty())
					return {};

				auto it = multipart_headers_.begin();
				auto val = it->second;

				auto pos = val.find(field_name);
				if (pos == std::string::npos) {
					return {};
				}

				auto start = val.find('"', pos) + 1;
				auto end = val.rfind('"');
				if (start == std::string::npos || end == std::string::npos || end < start) {
					return {};
				}

				auto key_name = val.substr(start, end - start);
				return key_name;
			}

			bool is_multipart_file() const {
				if (multipart_headers_.empty()) {
					return false;
				}

				bool has_content_type = (multipart_headers_.find("Content-Type") != multipart_headers_.end());
				auto it = multipart_headers_.find("Content-Disposition");
				bool has_content_disposition = (it != multipart_headers_.end());
				if (has_content_disposition) {
					if (it->second.find("filename") != std::string::npos) {
						return true;
					}

					return false;
				}

				return has_content_type || has_content_disposition;
			}


			bool open_upload_file(const std::string& filename) {
				upload_file file;
				bool r = file.open(filename);
				if (!r)
					return false;

				files_.push_back(std::move(file));
				return true;
			}

			void save_multipart_key_value(const std::string& key, const std::string& value)
			{
				if (!key.empty())
					multipart_form_map_.emplace(key, value);
			}

			void write_upload_data(const char* data, size_t size) {
				if (size == 0 || files_.empty()) {
					return;
				}

				files_.back().write(data, size);
			}

			void update_multipart_value(std::string key, const char* buf, size_t size) {
				if (!key.empty()) {
					last_multpart_key_ = key;
				}
				else {
					key = last_multpart_key_;
				}

				auto it = multipart_form_map_.find(key);
				if (it != multipart_form_map_.end()) {
					multipart_form_map_[key] += std::string(buf, size);
				}
			}


			void close_upload_file() {
				if (files_.empty()) {
					return;
				}

				files_.back().close();
			}

			upload_file* get_file() {
				if (!files_.empty())
					return &files_.back();

				return nullptr;
			}

			void set_last_len(size_t len) {
				last_len_ = len;
			}

			std::string req_buf() {
				return std::string(buffer_.data() + last_len_, total_len());
			}

			std::string head() {
				return std::string(buffer_.data() + last_len_, header_len_);
			}

			std::string body() {
				return std::string(buffer_.data() + last_len_ + header_len_, body_len_);
			}

			int parse_chunked(size_t bytes_transferred) {
				auto str = std::string(&buffer_[header_len_], bytes_transferred - header_len_);

				return -1;
			}

			const char* current_part() const {
				return &buffer_[header_len_];
			}
		private:
			std::string body() const {
				std::string str(&buffer_[header_len_], body_len_);
				return std::move(str);
			}

			size_t total_len() {
				return header_len_ + body_len_;
			}

			void resize_double() {
				size_t size = buffer_.size();
				resize(2 * size);
			}

			void resize(size_t size) {
				buffer_.resize(size);
			}

			void copy_method_url_headers() {
				if (method_len_ == 0)
					return;

				method_str_ = std::string(method_, method_len_);
				url_str_ = std::string(url_, url_len_);
				method_len_ = 0;
				url_len_ = 0;

				auto filename = get_multipart_field_name("filename");
				multipart_headers_.clear();
				if (!filename.empty()) {
					copy_headers_.emplace_back("filename", std::move(filename));
				}

				if (header_len_ < 0)
					return;

				for (size_t i = 0; i < num_headers_; i++) {
					copy_headers_.emplace_back(std::string(headers_[i].name, headers_[i].name_len),
						std::string(headers_[i].value, headers_[i].value_len));
				}
			}

			void check_gzip() {
				auto encoding = get_header_value("content-encoding");
				if (encoding.empty()) {
					has_gzip_ = false;
				}
				else {
					auto it = encoding.find("gzip");
					has_gzip_ = (it != std::string::npos);
				}
			}

			void set_body_len(size_t len) {
				body_len_ = len;
				left_body_len_ = body_len_;
			}

			std::map<std::string, std::string> parse_query(const std::string& str) {
				std::map<std::string, std::string> query;
				std::string key;
				std::string val;
				size_t pos = 0;
				size_t length = str.length();
				for (size_t i = 0; i < length; i++) {
					char c = str[i];
					if (c == '=') {
						key = { &str[pos], i - pos };
						wheel::unit::trim(key);
						pos = i + 1;
					}
					else if (c == '&') {
						val = { &str[pos], i - pos };
						wheel::unit::trim(val);
						pos = i + 1;
						query.emplace(key, val);
					}
				}

				if (pos == 0) {
					return {};
				}

				if ((length - pos) > 0) {
					val = { &str[pos], length - pos };
					wheel::unit::trim(val);
					query.emplace(key, val);
				}
				else if ((length - pos) == 0) {
					query.emplace(key, "");
				}

				return std::move(query);
			}
		private:
			size_t last_len_ = 0; //for pipeline, last request buffer position
			std::map<std::string, std::string> utf8_character_params_;
			std::map<std::string, std::string> utf8_character_pathinfo_params_;
			std::map<std::string, std::string> multipart_form_map_;
			std::map<std::string, std::string> form_url_map_;
			std::string last_multpart_key_;
			std::string part_data_;
			data_proc_state state_ = data_proc_state::data_begin;
			std::map<std::string, std::string> multipart_headers_;
			std::vector<std::pair<std::string, std::string>> copy_headers_;
			std::map<std::string, std::string> queries_;
			std ::string buffer_;
			size_t cur_size_ = 0;
			size_t num_headers_ = 0;
			struct phr_header headers_[32];
			const char* method_ = nullptr;
			size_t method_len_ = 0;
			const char* url_ = nullptr;
			size_t url_len_ = 0;
			int minor_version_;
			int header_len_;
			size_t body_len_;
			size_t left_body_len_ = 0;
			bool is_chunked_ = false;
			std::string raw_url_;
			std::string cookie_str_;
			std::string method_str_;
			std::string url_str_;
			bool has_gzip_ =false;
			std::string gzip_str_;
			std::vector<upload_file> files_;
			content_type http_type_ = content_type::unknown;
			constexpr const static size_t MaxSize = 3 * 1024 * 1024;
		};
	}
}
#endif // http_request_h__
