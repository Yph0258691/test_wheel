#ifndef http_tcp_handle_h__
#define http_tcp_handle_h__

#ifdef WHEEL_ENABLE_SSL
#include <boost/asio/ssl.hpp>
#endif
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include "unit.hpp"
#include "http_response.hpp"
#include "http_request.hpp"
#include <boost/filesystem.hpp>
#include "http_multipart_reader.hpp"
#include "uuid.h"


namespace wheel {
	namespace http_servers {
		struct ssl_configure {
			std::string cert_file;          //private cert
			std::string key_file;           //private key
			std::string passp_hrase;        //password;//私有key，是否输入密码
		};

		namespace fs = boost::filesystem;
		using http_handler = std::function<void(request&, response&)>;
		class http_tcp_handle:public std::enable_shared_from_this<http_tcp_handle> {
			typedef std::function<void(std::shared_ptr<http_tcp_handle >, const boost::system::error_code&) > CloseEventObserver;

			typedef std::function<void(std::shared_ptr<http_tcp_handle >) > ConnectEventObserver;

		public:
			http_tcp_handle() = delete;
			http_tcp_handle(const std::shared_ptr<boost::asio::io_service> ios, http_handler& handle,const std::string &static_dir,const ssl_configure& ssl_conf):http_handler_(handle)
			,ios_(ios),static_dir_(static_dir){
				socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*ios_);
				request_ = std::make_unique<request>();
				response_ = std::make_unique<response>();

#ifdef WHEEL_ENABLE_SSL
				is_ssl_ = true;
				if (!init_ssl_context(ssl_conf)){
					exit(0);
				}

#endif
				init_multipart_parser();
			}

			std::shared_ptr<boost::asio::ip::tcp::socket> get_socket()const {
				return socket_;
			}

			~http_tcp_handle() {
				close();
#ifdef WHEEL_ENABLE_SSL
				ssl_stream_ = nullptr;
#endif
				socket_ = nullptr;
			}

			void activate() {
				connect_observer_(shared_from_this());
				init();
				do_read();
			}

			void register_connect_observer(ConnectEventObserver observer) {
				connect_observer_ = observer;
			}

			void register_close_observer(CloseEventObserver observer) {
				close_observer_ = observer;
			}
			void set_multipart_begin(std::function<void(request&, std::string&)> begin) {
				multipart_begin_ = std::move(begin);
			}
		private:
			auto& socket() {
#ifdef WHEEL_ENABLE_SSL
				return *ssl_stream_;
#else
				return *socket_;
#endif	
			}

#ifdef WHEEL_ENABLE_SSL
			bool init_ssl_context(ssl_configure ssl_conf) {
				unsigned long ssl_options = boost::asio::ssl::context::default_workarounds
					| boost::asio::ssl::context::no_sslv2
					| boost::asio::ssl::context::single_dh_use;
				try {
					boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);
					ssl_context.set_options(ssl_options);
					if (!ssl_conf.passp_hrase.empty()){
						ssl_context.set_password_callback([ssl_conf](std::size_t max_length, auto& purpose) {return ssl_conf.passp_hrase; });
					}

					boost::system::error_code ec;
					if (fs::exists(ssl_conf.cert_file, ec)) {
						ssl_context.use_certificate_chain_file(std::move(ssl_conf.cert_file));
					}else {
						std::cout << "server.crt is empty" << std::endl;
					}

					if (fs::exists(ssl_conf.key_file, ec)) {
						ssl_context.use_private_key_file(std::move(ssl_conf.key_file), boost::asio::ssl::context::pem);
					}else {
						std::cout << "server.key is empty" << std::endl;
					}

					ssl_stream_ = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>>(*socket_, ssl_context);
				}
				catch (const std::exception & e) {
					std::cout << e.what() << std::endl;
					return false;
				}

				return true;
			}
#endif
			void do_read() {
				last_transfer_ = 0;
				len_ = 0;

				request_->reset();
				response_->reset();

				if (is_ssl_ && !has_shake_) {
					async_handshake();
				}else {
					async_read_some();
				}
			}


			void async_handshake() {
#ifdef WHEEL_ENABLE_SSL            
				ssl_stream_->async_handshake(boost::asio::ssl::stream_base::server,
					[this](const boost::system::error_code& error) {
					if (error) {
						release_session(boost::asio::error::make_error_code(
							static_cast<boost::asio::error::basic_errors>(error.value())));
						std::cout << error.message() << std::endl;
						return;
					}

					has_shake_ = true;
					async_read_some();
				});
#endif
			}

			void async_read_some() {
#ifdef WHEEL_ENABLE_SSL
				if (is_ssl_ && ssl_stream_ == nullptr) {
					return;
				}
#endif

				socket().async_read_some(boost::asio::buffer(request_->buffer(), request_->left_size()), [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
					if (ec) {
						if (ec == boost::asio::error::eof || bytes_transferred ==0) {
							release_session(boost::asio::error::make_error_code(
								static_cast<boost::asio::error::basic_errors>(ec.value())));
						}

						has_shake_ = false;
						return;
					}

					handle_read(ec, bytes_transferred);
					});
			}
			void close() {
				boost::system::error_code ignored_ec;
#ifdef WHEEL_ENABLE_SSL
				if (ssl_stream_ == nullptr){
					return;
				}

				if (ssl_stream_->lowest_layer().is_open()) {
					ssl_stream_->lowest_layer().close(ignored_ec);
				}

#else
				if (socket_ == nullptr){
					return;
				}

				if (socket_->is_open()){
					socket_->close(ignored_ec);
				}
#endif
			}

			void release_session(const boost::system::error_code& ec) {
				//shared_from_this不能被调两次
				if (close_observer_ != nullptr) {
					close_observer_(shared_from_this(), ec);
				}
			}

			void do_read_head() {
				socket().async_read_some(boost::asio::buffer(request_->buffer(), request_->left_size()),
					[this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
						if (ec){
							release_session(boost::asio::error::make_error_code(
								static_cast<boost::asio::error::basic_errors>(ec.value())));
							return;
						}

						handle_read(ec, bytes_transferred);
				});
			}

			void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred) {
				last_transfer_ = request_->current_size();
				bool at_capacity = request_->update_and_expand_size(bytes_transferred);
				//support 3M buffer
				if (at_capacity) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				int ret = request_->parse_header(len_);

				if (ret == parse_status::has_error) {
					response_back(status_type::bad_request);
					return;
				}

				check_keep_alive();
				if (ret == parse_status::not_complete) {
					do_read_head();
				}

				else {
					constexpr int offset = 4;
					if (bytes_transferred > ret + offset) {
						std::string str(request_->data() + ret, offset);
						if (str == "GET " || str == "POST") {
							handle_pipeline(ret, bytes_transferred);
							return;
						}
					}

					request_->set_last_len(len_);
					handle_request(bytes_transferred);
				}
			}

			void handle_pipeline(int ret, std::size_t bytes_transferred) {
				response_->set_delay(true);
				request_->set_last_len(len_);
				handle_request(bytes_transferred);
				last_transfer_ += bytes_transferred;

				(len_ == 0) ? (len_ = ret) : (len_ += ret);

				auto& rep_str = response_->response_str();
				int result = 0;
				size_t left = ret;
				bool head_not_complete = false;
				bool body_not_complete = false;
				size_t left_body_len = 0;
				while (true) {
					result = request_->parse_header(len_);
					if (result == -1) {
						return;
					}

					if (result == -2) {
						head_not_complete = true;
						break;
					}

					auto total_len = request_->total_len();

					if (total_len <= (bytes_transferred - len_)) {
						request_ ->set_last_len(len_);
						handle_request(bytes_transferred);
					}

					len_ += total_len;

					if (len_ == last_transfer_) {
						break;
					}
					else if (len_ > last_transfer_) {
						auto n = len_ - last_transfer_;
						len_ -= total_len;
						if (n < request_->header_len()) {
							head_not_complete = true;
						}
						else {
							body_not_complete = true;
							left_body_len = n;
						}

						break;
					}
				}

				response_->set_delay(false);
				boost::asio::async_write(socket(), boost::asio::buffer(rep_str.data(), rep_str.size()),
					[head_not_complete, body_not_complete, left_body_len, this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
					if (head_not_complete) {
						do_read_head();
						return;
					}

					if (body_not_complete) {
						request_->set_left_body_size(left_body_len);
						do_read_body();
						return;
					}

					handle_write(ec);
				});
			}

			void handle_request(std::size_t bytes_transferred) {
				if (request_->has_body()) {

					request_->set_http_type(get_content_type());

					switch (request_->get_content_type()) {
					case content_type::string:
					case content_type::unknown:
						handle_string_body(bytes_transferred);
						break;
					case content_type::multipart:
						handle_multipart();
						break;
					case content_type::chunked:
						handle_chunked(bytes_transferred);
						break;
					case content_type::urlencoded:
						handle_form_urlencoded(bytes_transferred);
						break;
					case content_type::octet_stream:
						handle_octet_stream(bytes_transferred);
						break;
					default:
						break;
					}

					return;
				}

				handle_header_request();
			}


			//-------------octet-stream----------------//
			void handle_octet_stream(size_t bytes_transferred) {
				try {
					std::string name = static_dir_ + uuids::uuid_system_generator{}().to_short_str();
					request_->open_upload_file(name);
				}
				catch (const std::exception & ex) {
					response_back(status_type::internal_server_error, ex.what());
					return;
				}

				request_->set_state(data_proc_state::data_continue);//data
				size_t part_size = bytes_transferred - request_->header_len();
				if (part_size != 0) {
					request_->reduce_left_body_size(part_size);
					request_->set_part_data({ request_->current_part(), part_size });
					request_->write_upload_data(request_->current_part(), part_size);
				}

				if (request_->has_recieved_all()) {
					//on finish
					request_->set_state(data_proc_state::data_end);
					call_back();
					do_write();
				}
				else {
					request_->fit_size();
					request_->set_current_size(0);
					do_read_octet_stream_body();
				}
			}

			void do_read_octet_stream_body() {
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()),
					[this](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							request_->set_state(data_proc_state::data_error);
							release_session(boost::asio::error::make_error_code(
								static_cast<boost::asio::error::basic_errors>(ec.value())));
							return;
						}

						request_->set_part_data({ request_->buffer(), bytes_transferred });
						request_->write_upload_data(request_->buffer(), bytes_transferred);

						request_->reduce_left_body_size(bytes_transferred);

						if (request_->body_finished()) {
							request_->set_state(data_proc_state::data_end);
							call_back();
							do_write();
						}
						else {
							do_read_octet_stream_body();
						}
					});
			}

			//无body应答
			void handle_header_request() {

				bool r = handle_gzip();
				if (!r) {
					response_back(status_type::bad_request, "gzip uncompress error");
					return;
				}

				call_back();

				if (request_->get_content_type() == content_type::chunked)
					return;

				if (request_->get_state() == data_proc_state::data_error) {
					return;
				}

				if (!response_->need_delay()) {
					do_write();
				}
			}

			content_type get_content_type() {
				if (request_->is_chunked()) {
					return content_type::chunked;
				}

				const std::string content_type = request_->get_header_value("content-type");
				if (!content_type.empty()) {
					if (content_type.find("application/x-www-form-urlencoded") != std::string::npos) {
						return content_type::urlencoded;
					}
					else if (content_type.find("multipart/form-data") != std::string::npos) {
						auto size = content_type.find("=");
						auto bd = content_type.substr(size + 1, content_type.length() - size);
						if (bd[0] == '"' && bd[bd.length() - 1] == '"') {
							bd = bd.substr(1, bd.length() - 2);
						}
						std::string boundary(bd.data(), bd.length());
						multipart_parser_.set_boundary("\r\n--" + std::move(boundary));
						return content_type::multipart;
					}
					else if (content_type.find("application/octet-stream") != std::string::npos) {
						return content_type::octet_stream;
					}

					return content_type::string;
				}

				return content_type::unknown;
			}

			//-------------form urlencoded----------------//
			//TODO: here later will refactor the duplicate code
			void handle_form_urlencoded(size_t bytes_transferred) {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				if (request_->has_recieved_all()) {
					handle_url_urlencoded_body();
				}else {
					request_->expand_size();
					size_t part_size = bytes_transferred - request_->header_len();
					request_->reduce_left_body_size(part_size);
					do_read_form_urlencoded();
				}
			}

			//不支持chunkded
			void handle_chunked(size_t bytes_transferred) {
				int ret = request_->parse_chunked(bytes_transferred);
				if (ret == parse_status::has_error) {
					response_back(status_type::internal_server_error, "not support yet");
					return;
				}
			}

			void do_read_form_urlencoded() {
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()),
					[this](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							release_session(boost::asio::error::make_error_code(
								static_cast<boost::asio::error::basic_errors>(ec.value())));
							return;
						}

						request_->update_size(bytes_transferred);
						request_->reduce_left_body_size(bytes_transferred);

						if (request_->body_finished()) {
							handle_url_urlencoded_body();
						}
						else {
							do_read_form_urlencoded();
						}
					});
			}
			void handle_url_urlencoded_body() {
				bool success = request_->parse_form_urlencoded();

				if (!success) {
					response_back(status_type::bad_request, "form urlencoded error");
					return;
				}

				call_back();
				if (!response_->need_delay()) {
					do_write();
				}
			}

			/****************** begin handle http body data *****************/
			void handle_string_body(std::size_t bytes_transferred) {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				if (request_->has_recieved_all()) {
					handle_body();
				}else {
					request_->expand_size();

					size_t part_size = request_->current_size() - request_->header_len();
					if (part_size == -1) {
						return;
					}

					request_->reduce_left_body_size(part_size);
					do_read_body();
				}
			}

			void handle_multipart() {
				if (upload_check_) {
					bool r = (*upload_check_)(*request_,*response_);
					if (!r) {
						release_session(boost::asio::error::make_error_code(boost::asio::error::invalid_argument));
						return;
					}
				}

				bool has_error = parse_multipart(request_->header_len(), request_->current_size() - request_->header_len());

				if (has_error) {
					response_back(status_type::bad_request, "mutipart error");
					return;
				}

				if (request_->has_recieved_all_part()) {
					call_back();
					do_write();
				}
				else {
					request_->set_current_size(0);
					do_read_multipart();
				}
			}

			void do_read_multipart() {
				request_->fit_size();
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()),
					[this](boost::system::error_code ec, std::size_t length) {
						if (ec) {
							request_->set_state(data_proc_state::data_error);
							call_back();
							response_back(status_type::bad_request, "mutipart error");
							return;
						}

						bool has_error = parse_multipart(0, length);

						if (has_error) { //parse error
							keep_alive_ = false;
							response_back(status_type::bad_request, "mutipart error");
							return;
						}

						if (request_->body_finished()) {
							call_back();
							do_write();
							return;
						}

						request_->set_current_size(0);
						do_read_part_data();
					});
			}

			void do_read_part_data() {
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_size()),
					[this](boost::system::error_code ec, std::size_t length) {
						if (ec) {
							request_->set_state(data_proc_state::data_error);
							call_back();
							return;
						}

						bool has_error = parse_multipart(0, length);

						if (has_error) {
							response_back(status_type::bad_request, "mutipart error");
							return;
						}

						if (!request_->body_finished()) {
							do_read_part_data();
						}
						else {
							call_back();
							do_write();
						}
					});
			}

			bool parse_multipart(size_t size, std::size_t length) {
				if (length == 0)
					return false;

				request_->set_part_data(std::string(request_->buffer(size), length));
				std::string multipart_body = request_->get_part_data();
				size_t bufsize = multipart_body.length();

				size_t fed = 0;
				do {
					size_t ret = multipart_parser_.feed(multipart_body.data() + fed, multipart_body.length() - fed);
					fed += ret;
				} while (fed < bufsize && !multipart_parser_.stopped());

				if (multipart_parser_.has_error()) {
					request_->set_state(data_proc_state::data_error);
					return true;
				}

				request_->reduce_left_body_size(length);
				return false;
			}

			void do_read_body() {
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()),
					[this](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							release_session(boost::asio::error::make_error_code(
								static_cast<boost::asio::error::basic_errors>(ec.value())));
							return;
						}

						request_->update_size(bytes_transferred);
						request_->reduce_left_body_size(bytes_transferred);

						if (request_->body_finished()) {
							handle_body();
						}
						else {
							do_read_body();
						}
					});
			}



			void init() {
				boost::system::error_code ec;
				//关闭牛逼的算法(nagle算法),防止TCP的数据包在饱满时才发送过去
				socket_->set_option(boost::asio::ip::tcp::no_delay(true), ec);

				//有time_wait状态下，可端口短时间可以重用
				//默认是2MSL也就是 (RFC793中规定MSL为2分钟)也就是4分钟
				set_reuse_address();
			}

			void set_reuse_address() {
				if (socket_ == nullptr) {
					return;
				}

				boost::system::error_code ec;
				socket_->set_option(boost::asio::socket_base::reuse_address(true), ec);
			}

			void response_back(status_type status, std::string&& content) {
				response_->set_status_and_content(status, std::move(content));
				do_write(); //response to client
			}

			void response_back(status_type status) {
				response_->set_status_and_content(status);
				do_write(); //response to client
			}

			void do_write() {

				const std::string& rep_str = response_->response_str();
				if (rep_str.empty()) {
					this->handle_write(boost::system::error_code{});
					return;
				}

				boost::asio::async_write(socket(), boost::asio::buffer(rep_str.data(), rep_str.size()),
					[this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
						this->handle_write(ec);
				});
			}

			void handle_write(const boost::system::error_code& ec) {
				if (ec) {
					return;
				}

				if (keep_alive_) {
					do_read();
				}else {
					release_session(boost::asio::error::make_error_code(
						static_cast<boost::asio::error::basic_errors>(ec.value())));
				}
			}

			void check_keep_alive() {
				auto req_conn_hdr = request_->get_header_value("connection");
				if (request_->is_http11()) {
					keep_alive_ = req_conn_hdr.empty() || !wheel::unit::iequal(req_conn_hdr.data(), req_conn_hdr.size(), "close");
				}
				else {
					keep_alive_ = !req_conn_hdr.empty() && wheel::unit::iequal(req_conn_hdr.data(), req_conn_hdr.size(), "keep-alive");
				}
			}


			void handle_body() {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The body is too long, limitation is 3M");
					return;
				}

				bool r = handle_gzip();
				if (!r) {
					response_back(status_type::bad_request, "gzip uncompress error");
					return;
				}

				if (request_->get_content_type() == content_type::multipart) {
					bool has_error = parse_multipart(request_->header_len(), request_->current_size() - request_->header_len());
					if (has_error) {
						response_back(status_type::bad_request, "mutipart error");
						return;
					}
					do_write();
					return;
				}

				call_back();

				if (!response_->need_delay())
					do_write();
			}

			void call_back() {
				if (http_handler_ != nullptr){
					http_handler_(*request_, *response_);
				}
			}
			//-------------multipart----------------------//
			void init_multipart_parser() {
				multipart_parser_.on_part_begin = [this](const multipart_headers& headers) {
					request_->set_multipart_headers(headers);
					auto filename = request_->get_multipart_field_name("filename");
					is_multi_part_file_ = request_->is_multipart_file();
					if (filename.empty() && is_multi_part_file_) {
						request_->set_state(data_proc_state::data_error);
						response_->set_status_and_content(status_type::bad_request, "mutipart error");
						return;
					}
					if (is_multi_part_file_){
						auto ext = boost::filesystem::extension(filename);
						try {
							auto tp = std::chrono::high_resolution_clock::now();
							auto nano = tp.time_since_epoch().count();
							std::string name = static_dir_ + "/" + std::to_string(nano)
								+ std::string(ext.data(), ext.length()) + "_ing";
							if (multipart_begin_) {
								multipart_begin_(*request_, name);
							}

							request_->open_upload_file(name);
						}
						catch (const std::exception & ex) {
							request_->set_state(data_proc_state::data_error);
							response_->set_status_and_content(status_type::internal_server_error, ex.what());
							return;
						}
					}
					else {
						auto key = request_->get_multipart_field_name("name");
						request_->save_multipart_key_value(std::string(key.data(), key.size()), "");
					}
				};
				multipart_parser_.on_part_data = [this](const char* buf, size_t size) {
					if (request_->get_state() == data_proc_state::data_error) {
						return;
					}
					if (is_multi_part_file_) {
						request_->write_upload_data(buf, size);
					}
					else {
						auto key = request_->get_multipart_field_name("name");
						request_->update_multipart_value(std::move(key), buf, size);
					}
				};
				multipart_parser_.on_part_end = [this] {
					if (request_->get_state() == data_proc_state::data_error) {
						return;
					}

					if (is_multi_part_file_){
						request_->close_upload_file();
						auto pfile = request_->get_file();
						if (pfile) {
							auto old_name = pfile->get_file_path();
							auto pos = old_name.rfind("_ing");
							if (pos != std::string::npos) {
								pfile->rename_file(old_name.substr(0, old_name.length() - 4));
							}
						}
					}
				};
				multipart_parser_.on_end = [this] {
					if (request_->get_state() == data_proc_state::data_error) {
						return;
					}

					request_->handle_multipart_key_value();
				};
			}


			bool handle_gzip() {
#ifdef WHEEL_ENABLE_GZIP
				if (request_->has_gzip()) {
					return request_->uncompress();
				}
#endif
				return true;
			}

		private:
#ifdef WHEEL_ENABLE_SSL
			std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> ssl_stream_{};
#endif
			bool is_ssl_ = false;
			bool has_shake_ = false;
			//callback handler to application layer
			const std::string& static_dir_;
			const http_handler& http_handler_;
			multipart_reader multipart_parser_;
			bool keep_alive_ = false;
			std::function<bool(request&, response&)>* upload_check_ = nullptr;
			std::function<void(request&, std::string&)> multipart_begin_ = nullptr;
			std::unique_ptr<response>response_;
			ConnectEventObserver		connect_observer_;
			CloseEventObserver			close_observer_;
			std::unique_ptr<request>request_;
			std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
			std::shared_ptr<boost::asio::io_service>ios_;
			size_t len_ = 0;
			size_t last_transfer_ = 0;
			bool is_multi_part_file_;
		};
	}
}
#endif // http_tcp_handle_h__
