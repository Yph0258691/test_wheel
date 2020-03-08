#ifndef client_h__
#define client_h__
#include "tcp_hanlde.hpp"
#include <chrono>

namespace wheel {
	namespace tcp_socket {
		class client
		{
		public:
			client(MessageEventObserver recv_observer, int parser_type)
				:ios_(std::make_shared<boost::asio::io_service>()), recv_observer_(recv_observer) {
				tcp_handler_ = std::make_shared<tcp_handle>(ios_, header_size_, packet_size_offset_, packet_cmd_offset_);

				if (tcp_handler_ != nullptr) {
					tcp_handler_->set_protocol_parse_type(parser_type);
				}

				timer_ = std::make_unique<boost::asio::steady_timer>(*ios_);
			}

			client(MessageEventObserver recv_observer, int parser_type
				, std::size_t header_size, std::size_t packet_size_offset, std::size_t packet_cmd_offse)
				:ios_(std::make_shared<boost::asio::io_service>()), recv_observer_(recv_observer)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offse) {
				tcp_handler_ = std::make_shared<tcp_handle>(ios_, header_size_, packet_size_offset_, packet_cmd_offset_);
				if (tcp_handler_ != nullptr) {
					tcp_handler_->set_protocol_parse_type(parser_type);
				}

				timer_ = std::make_unique<boost::asio::steady_timer>(*ios_);
			}

			~client() {

			}

			void set_reconnect(bool flag) {
				reconnent_ = flag;
			}

			int connect(std::string ip, int port) {
				if (!unit::ipAddr_check(ip) || tcp_handler_ == nullptr) {
					return -1;
				}

				server_ip_ = std::move(ip);
				server_port_ = std::move(port);
				return tcp_handler_->connect(server_ip_, server_port_, recv_observer_, std::bind(&client::on_close, this, std::placeholders::_1, std::placeholders::_2));
			}

			int send(const char* data, const std::size_t count) {
				if (tcp_handler_ == nullptr){
					return -1;
				}

				return tcp_handler_->to_send(data, count);
			}

			void run() {
				ios_->run();
			}
		private:
			void on_close(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler, const boost::system::error_code& ec) {
				if (ec) {
					//使用此接口会获取端口和ip全是0
					//boost::system::error_code e;
					//std::string ip = handler->get_socket()->remote_endpoint(e).address().to_v4().to_string();
					//int port = handler->get_socket()->remote_endpoint(e).port();

					//注意close的地方，如果先close，再获取远端的地址的ip和端口，都会变成0
					if (handler ==nullptr){
						return;
					}

					handler->close_socket();
					reconnect(handler, server_ip_, server_port_);
				}
			}

			void reconnect(std::shared_ptr<wheel::tcp_socket::tcp_handle> ptr, std::string ip, int port) {
				if (ptr == nullptr){
					return;
				}

				timer_->expires_from_now(std::chrono::seconds(3));
				timer_->async_wait([this, ptr, ip, port](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					if (ptr->get_connect_status() == wheel::tcp_socket::connectinged) {
						return;
					}

					ptr->connect(ip, port, recv_observer_, std::bind(&client::on_close, this, std::placeholders::_1, std::placeholders::_2));
					timer_->expires_from_now(std::chrono::seconds(3));
					reconnect(ptr, ip, port);
					});
			}
		private:
			bool reconnent_;
			MessageEventObserver recv_observer_;
			std::shared_ptr<boost::asio::io_service>ios_;
			std::shared_ptr<tcp_handle> tcp_handler_ = nullptr;
			std::unique_ptr<boost::asio::steady_timer> timer_;
			std::size_t header_size_ = 0;
			std::size_t packet_size_offset_ = 0;
			std::size_t packet_cmd_offset_ = 0;
			std::string server_ip_;
			int server_port_;
		};
	}
}

#endif // client_h__
