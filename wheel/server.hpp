#ifndef sever_h__
#define sever_h__

#include <memory>
#include <boost/asio.hpp>
#include "tcp_hanlde.hpp"
#include <unordered_map>

namespace wheel {
	namespace tcp_socket {
		class server
		{
		public:
			server(MessageEventObserver recv_observer) :io_service_(std::make_shared<boost::asio::io_service>())
				, recv_observer_(recv_observer) {

			}

			server(MessageEventObserver recv_observer, int parser_type, std::size_t header_size,
				std::size_t packet_size_offset, std::size_t packet_cmd_offset) :io_service_(std::make_shared<boost::asio::io_service>())
				, recv_observer_(recv_observer), parser_type_(parser_type)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offset) {

			}

			~server() {
			}

			void init(int port, int connect_pool) {
				accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
				if (accept_ == nullptr) {
					return;
				}

				//端口复用
				accept_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

				for (int i = 0;i < connect_pool;i++) {
					make_session();
				}
			}

			void run() {
				io_service_->run();
			}
		private:
			void make_session() {
				if (accept_ == nullptr) {
					return;
				}

				std::shared_ptr<tcp_handle > new_session = std::make_shared<tcp_handle>(io_service_, header_size_, packet_size_offset_, packet_cmd_offset_);
				new_session->set_protocol_parse_type(parser_type_);
				new_session->register_close_observer(std::bind(&server::on_close, this, std::placeholders::_1, std::placeholders::_2));
				new_session->register_connect_observer(std::bind(&server::on_connect, this, std::placeholders::_1));
				new_session->register_recv_observer(recv_observer_);

				//发一次数据接收一次
				accept_->async_accept(*new_session->get_socket(), [this, new_session](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					new_session->activate();
					make_session();
					});
			}

		private:
			void on_connect(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler) {
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					return;
				}

				std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
				std::time_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
				connects_.emplace(handler, timestamp);
			}

			void on_close(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler, const boost::system::error_code& ec) {
				//tcp主动断开 boost::asio::error::connection_reset
				//websocket主动断开  boost::asio::error::connection_aborted
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					connects_.erase(iter_find);
				}
			}
		private:
			std::unordered_map<std::shared_ptr<wheel::tcp_socket::tcp_handle>, std::time_t>connects_;
			std::unique_ptr<TCP::acceptor>accept_;
			std::shared_ptr<boost::asio::io_service> io_service_;
			MessageEventObserver		recv_observer_;
			int parser_type_ = 0;
			std::size_t header_size_ = 0;
			std::size_t packet_size_offset_ = 0;
			std::size_t packet_cmd_offset_ = 0;
		};
	}
}
#endif // sever_h__
