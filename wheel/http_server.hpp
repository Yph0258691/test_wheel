#ifndef http_server_h__
#define http_server_h__

#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "http_tcp_handle.hpp"
#include "http_router.hpp"

namespace wheel {
	namespace http_servers {
		class http_server {
		public:
			http_server() {
				init_conn_callback();
				ios_ = std::make_shared<boost::asio::io_service>();
			}

			~http_server() {
				join_all();

				connects_.clear();
				ios_threads_.clear();
			}

			void listen(int port) {
				if (ios_ == nullptr){
					return;
				}

				accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*ios_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
				accept_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
				
				start_accpect();
			}

			//set http handlers
			template<http_method... Is, typename Function, typename... AP>
			void set_http_handler(std::string name, Function&& f, AP&&... ap) {
				http_router_.register_handler<Is...>(name, std::forward<Function>(f), std::forward<AP>(ap)...);
			}

			void run() {
				if (ios_ ==nullptr){
					return;
				}

				std::size_t num_threads = std::thread::hardware_concurrency();
				//子线程减一个啊 
				try
				{
					for (size_t c = 0; c < num_threads - 1; c++) {
						ios_threads_.emplace_back(std::make_shared<std::thread>([this]() {
							boost::system::error_code ec;
							ios_->run(ec);
							std::cout << ec.message() << std::endl;
							}));
					}
				}
				catch (...) {
					std::cout << "expection..." << std::endl;
					return;
				}

				boost::system::error_code ec;
				ios_->run(ec);
				std::cout << ec.message() << std::endl;
			}

#ifdef WHEEL_ENABLE_SSL
			void set_ssl_conf(ssl_configure conf) {
				ssl_conf_ = std::move(conf);
			}
#endif
		private:
			void join_all() {
				//加入之后需要等待,避免线程不回收
				for (auto& t : ios_threads_) {
					if (!t->joinable()) {
						t->join();
					}
				}
			}

			void start_accpect() {
				if (accept_ == nullptr) {
					return;
				}

				std::shared_ptr<http_tcp_handle > new_session = nullptr;
				try
				{
					new_session = std::make_shared<http_tcp_handle>(ios_,http_handler_, upload_dir_, ssl_conf_);
				}
				catch (const std::exception & ex)
				{
					std::cout << ex.what() << std::endl;
					new_session = nullptr;
				}

				if (new_session == nullptr) {
					return;
				}

				//发一次数据接收一次
				accept_->async_accept(new_session->get_socket(), [this, new_session](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					new_session->register_close_observer(std::bind(&http_server::on_close, this, std::placeholders::_1, std::placeholders::_2));
					new_session->register_connect_observer(std::bind(&http_server::on_connect, this, std::placeholders::_1));
					new_session->activate();
					start_accpect();
					});
			}
		private:
			void init_conn_callback() {
				http_handler_ = [this](request& req, response& res) {
					res.set_headers(req.get_headers());
					try {
						bool success = http_router_.route(req.get_method(), req.get_url(), req, res);
						if (!success) {
							res.set_status_and_content(status_type::bad_request, "the url is not right");
						}
					}
					catch (const std::exception & ex) {
						res.set_status_and_content(status_type::internal_server_error, ex.what() + std::string(" exception in business function"));
					}
					catch (...) {
						res.set_status_and_content(status_type::internal_server_error, "unknown exception in business function");
					}
				};
			}

			void on_connect(std::shared_ptr<wheel::http_servers::http_tcp_handle> handler) {
				if (handler == nullptr) {
					return;
				}

				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					return;
				}

				std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
				std::time_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
				while (lock_.test_and_set(std::memory_order_acquire));
				connects_.emplace(handler, timestamp);

				lock_.clear(std::memory_order_release);
			}

			void on_close(std::shared_ptr<wheel::http_servers::http_tcp_handle> handler, const boost::system::error_code& ec) {
				while (lock_.test_and_set(std::memory_order_acquire));
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					connects_.erase(iter_find);
				}

				lock_.clear(std::memory_order_release);
			}

		private:
			ssl_configure ssl_conf_;
			std::string upload_dir_ = fs::absolute("www").string(); //default
			http_handler http_handler_;
			http_router http_router_;
			std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
			std::unordered_map<std::shared_ptr<wheel::http_servers::http_tcp_handle>, std::time_t>connects_;
			std::unique_ptr<boost::asio::ip::tcp::acceptor> accept_{};
			std::shared_ptr<boost::asio::io_service>ios_{};
			std::vector<std::shared_ptr<std::thread>> ios_threads_;
		};
	}
}
#endif // http_server_h__
