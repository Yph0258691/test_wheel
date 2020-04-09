#ifndef ws_tcp_handle_h__
#define ws_tcp_handle_h__


#include<memory>
#include <list>
#include <functional>
#include <boost/asio.hpp>
#include "native_stream.hpp"
#include "send_buffer.hpp"
#include "bin_parser.hpp"
#include"unit.hpp"
#include "websocket_handle.hpp"
#include "picohttpparser.hpp"
#include "timer.hpp"
#include <atomic>
#include <iostream>

namespace wheel {
	namespace websocket {
		const int g_client_reconnect_seconds = 3;
		const int g_ws_heart_seconds = 3;
		using TCP = boost::asio::ip::tcp;
		using ADDRESS = boost::asio::ip::address;
		typedef std::map<std::string, std::string> HEADER_MAP;
		class ws_tcp_handle;
		typedef std::function<void(std::shared_ptr<ws_tcp_handle >, std::shared_ptr<native_stream>) > MessageEventObserver;

		typedef std::function<void(std::shared_ptr<ws_tcp_handle >, const boost::system::error_code&) > CloseEventObserver;

		typedef std::function<void(std::shared_ptr<ws_tcp_handle >) > ConnectEventObserver;
		enum
		{
			disconnect = -1,
			connectinged = 0,
		};

		class ws_tcp_handle :public std::enable_shared_from_this<ws_tcp_handle>
		{
		public:
			// bin����
			ws_tcp_handle(const std::shared_ptr<boost::asio::io_service>&ptr, std::size_t header_size,
				std::size_t packet_size_offset, std::size_t packet_cmd_offset)
				:ios_(ptr)
				, connect_status_(-1)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offset)
			{
				try
				{
					socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*ios_);
					timer_ = std::make_unique<boost::asio::steady_timer>(*ios_);
				}catch (const std::exception&ex){
					std::cout << ex.what() << std::endl;
					socket_ = nullptr;
					timer_ = nullptr;
				}

				if (protocol_parser_ == nullptr) {
					try
					{
						protocol_parser_ = create_object(0, header_size_, packet_size_offset_, packet_cmd_offset_);
					}catch (const std::exception&ex){
						std::cout << ex.what() <<std::endl;
						protocol_parser_ = nullptr;
					}
				}
			}

			//json ����
			ws_tcp_handle(const std::shared_ptr<boost::asio::io_service>&ptr)
				:ios_(ptr)
				, connect_status_(-1)
			{
				try
				{
					socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*ios_);
					timer_ = std::make_unique<boost::asio::steady_timer>(*ios_);
				}
				catch (const std::exception&ex){
					std::cout << ex.what() << std::endl;
					socket_ = nullptr;
					timer_ = nullptr;
				}
			}

			~ws_tcp_handle() {
				set_connect_status(disconnect);
				socket_ = nullptr;
				timer_ = nullptr;
				ws_timer_heart_ = nullptr;
			}

			std::shared_ptr<TCP::socket>get_socket() const {
				return socket_;
			}

			void activate() {
				set_connect_status(connectinged);
				connect_observer_(shared_from_this());
				init();
				to_read();
			}

			int close_send_endpoint() {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				boost::system::error_code ec;
				socket_->shutdown(TCP::socket::shutdown_send, ec);
				return 0;
			}

			int close_rs_endpoint() {
				if (socket_ == nullptr ||!socket_->is_open()) {
					return -1;
				}

				boost::system::error_code ec;
				socket_->shutdown(TCP::socket::shutdown_both, ec);
				return ec.value();
			}

			int close_recv_endpoint() {
				if (socket_ == nullptr ||!socket_->is_open()) {
					return -1;
				}

				//ִ�в���֮�󣬻�ģ�����ͻ��˹ر�socket�Ĳ���
				boost::system::error_code ec;
				socket_->shutdown(TCP::socket::shutdown_receive, ec);
				return ec.value();
			}

			int close_socket() {
				if (socket_ == nullptr|| !socket_->is_open()) {
					return -1;
				}

				boost::system::error_code e;
				socket_->close(e);
				return e.value();
			}

			int to_send(const native_stream& stream) {
				return to_send(stream.buffer_.c_str(), stream.get_size());
			}

			int to_send(const char* data, const std::size_t count) {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				if (count ==0 || data == nullptr){
					return -1;
				}

				while (data_lock_.test_and_set(std::memory_order_acquire));
				//�����һ���������Ϳ��Է���ĩβ�����������õ�ǰ���ڴ棬�ﵽд���٣������ٵ�Ч��
				std::shared_ptr<send_buffer>ptr = nullptr;
				if (!send_buffers_.empty()) {
					if (!send_buffers_.back()->write(data, count)){
						try
						{
							ptr = std::make_shared<send_buffer>(data, count);
						}catch (const std::exception&ex)
						{
							std::cout << ex.what() << std::endl;
							ptr = nullptr;
						}

						if (ptr != nullptr) {
							send_buffers_.push_back(ptr);
						}
					}
				}
				else {
					
					try
					{
						ptr = std::make_shared<send_buffer>(data, count);
					}catch (const std::exception&ex)
					{
						ptr = nullptr;
						std::cout << ex.what() << std::endl;
					}

					if (ptr != nullptr) {
						send_buffers_.push_back(ptr);
					}
				}

				data_lock_.clear(std::memory_order_release);
				if (write_count_ == 0) {
					++write_count_; //1:����0����ӣ�2:���˱���Ϊ1��˵���д��� 

					socket_->async_send(boost::asio::buffer(send_buffers_.front()->data(), send_buffers_.front()->size()), std::bind(&ws_tcp_handle::on_write, this,
						std::placeholders::_1, std::placeholders::_2));
				}

				return 0;
			}

			void set_reconect_seconds(int seconds) {
				seconds_ = seconds;
			}

			int connect(std::string ip, int port,const std::string & handleshake_msg,std::string handleshake_key,MessageEventObserver recv_observer, CloseEventObserver close_observer) {
				if (!unit::ipAddr_check(ip) || socket_ == nullptr){
					return -1;
				}

				boost::system::error_code err;
				socket_->connect(TCP::endpoint(ADDRESS::from_string(ip), port), err);

				connect_status_ = err.value() == 0 ? connectinged : disconnect;

				if (err.value() == 0) {
					register_close_observer(std::move(close_observer));
					register_recv_observer(std::move(recv_observer));
					to_send(handleshake_msg.c_str(), handleshake_msg.size());
					to_read_server_data(handleshake_key);
					return 0;
				}

				return err.value();
			}

			//�ͻ���֧���첽����
			void reconect_server(std::string ip, int port, MessageEventObserver recv_observer, CloseEventObserver  close_observer) {
				if (timer_ == nullptr){
					return;
				}

				timer_->expires_from_now(std::chrono::seconds(seconds_));
				timer_->async_wait([this, ip, port, recv_observer, close_observer](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					if (connect_status_ == connectinged) {
						return;
					}

					async_connect(ip, port, recv_observer, close_observer);
					reconect_server(ip, port, recv_observer, close_observer);
					});
			}

			void register_connect_observer(ConnectEventObserver observer) {
				connect_observer_ = observer;
			}

			void register_close_observer(CloseEventObserver observer) {
				close_observer_ = observer;
			}

			void register_recv_observer(MessageEventObserver observer) {
				recv_observer_ = observer;
			}

			int get_connect_status()const {
				return connect_status_;
			}

			std::shared_ptr<stream_format>get_read_parser() {
				if (protocol_parser_ ==nullptr){
					return nullptr;
				}

				return protocol_parser_->get_read_parser();
			}

			std::shared_ptr<stream_format>get_write_parser() {
				if (protocol_parser_ == nullptr) {
					return nullptr;
				}

				return protocol_parser_->get_write_parser();
			}

			void set_ws_heart_seconds(int seconds) {
				ws_heart_seconds_ = seconds;
			}

			std::size_t get_base_receive_buffer_size() {
				if (socket_ == nullptr) {
					return 0;
				}

				boost::asio::socket_base::receive_buffer_size opt;
				socket_->get_option(opt);
				return opt.value();
			}

			std::size_t get_base_send_buffer_size() {
				if (socket_ == nullptr) {
					return 0;
				}

				boost::asio::socket_base::send_buffer_size opt;
				socket_->get_option(opt);
				return opt.value();
			}

			int set_base_receive_buffer_size(std::size_t size, bool force = false) {
				if (socket_ == nullptr) {
					return -1;
				}

				boost::system::error_code ec;

				///net.core.rmem_max, �������ϵ��ֵ�Ļ����͵����޸����ϵͳ������
				//net.core.rmem_default Ĭ�ϴ�С
				//	/proc/sys/net/ipv4/tcp_window_scaling	"1"	���� RFC 1323 ����� window scaling��Ҫ֧�ֳ��� 64KB �Ĵ��ڣ��������ø�ֵ��
				// ϵͳ���ݸ��أ���������ֵ֮�����SOCKET���ڴ�С
				// net.ipv4.tcp_wmem = 4096	16384	4194304
				// net.ipv4.tcp_rmem = 4096	87380	4194304

				// 	/proc/sys/net/ipv4/tcp_wmem	"4096 16384 131072"	Ϊ�Զ����Ŷ���ÿ�� socket ʹ�õ��ڴ档
				// 		��һ��ֵ��Ϊ socket �ķ��ͻ���������������ֽ�����
				// 		�ڶ���ֵ��Ĭ��ֵ����ֵ�ᱻ wmem_default ���ǣ�����������ϵͳ���ز��ص�����¿������������ֵ��
				// 		������ֵ�Ƿ��ͻ������ռ������ֽ�������ֵ�ᱻ wmem_max ���ǣ���

				if (force == true) {
					boost::asio::socket_base::receive_buffer_size  rs(size);
					socket_->set_option(rs, ec);
				}
				else {
					std::size_t recv_buffer_size = get_base_receive_buffer_size();
					if (ec.value() != 0 || recv_buffer_size < size)
					{
						boost::asio::socket_base::receive_buffer_size  rs(size);
						socket_->set_option(rs, ec);
					}
				}

				return ec.value();
			}

			int set_base_send_buffer_size(std::size_t send_buffer_size, bool force = false)
			{
				if (socket_ == nullptr) {
					return -1;

				}
				boost::system::error_code ec;

				///net.core.wmem_max, �������ϵ��ֵ�Ļ����͵����޸����ϵͳ������
				//net.core.wmem_default Ĭ�ϴ�С
				if (force == true) {
					boost::asio::socket_base::send_buffer_size op(send_buffer_size);
					socket_->set_option(op, ec);
				}
				else {
					std::size_t buffer_size = get_base_send_buffer_size();
					if (ec.value() != 0 || buffer_size < send_buffer_size)
					{
						boost::asio::socket_base::send_buffer_size op(send_buffer_size);
						socket_->set_option(op, ec);
					}
				}

				return ec.value();
			}

		private:
			void init() {
				if (socket_ == nullptr){
					return;
				}

				boost::system::error_code ec;
				//�ر�ţ�Ƶ��㷨(nagle�㷨),��ֹTCP�����ݰ��ڱ���ʱ�ŷ��͹�ȥ
				socket_->set_option(boost::asio::ip::tcp::no_delay(true), ec);

				//��time_wait״̬�£��ɶ˿ڶ�ʱ���������
				//Ĭ����2MSLҲ���� (RFC793�й涨MSLΪ2����)Ҳ����4����
				set_reuse_address();
			}

			void set_reuse_address() {
				if (socket_ == nullptr){
					return;
				}

				boost::system::error_code ec;
				socket_->set_option(boost::asio::socket_base::reuse_address(true), ec);
			}


			void to_read_websocket_data() {
				if (socket_ == nullptr)	{
					return;
				}

				recv_buffer_size_ = g_packet_buffer_size;
				recv_buffer_ = std::make_unique<char[]>(g_packet_buffer_size);
				socket_->async_receive(boost::asio::buffer(&recv_buffer_[0], recv_buffer_size_), [this](const boost::system::error_code& ec, size_t bytes_transferred) {
					if (ec) {
						set_connect_status(disconnect);
						close_socket();
						close_observer_(shared_from_this(), ec);
						return;
					}

					std::shared_ptr<websocket_handle> web_socket_hand = std::make_shared<websocket_handle>();

					if (web_socket_hand->parse_header(&recv_buffer_[0], bytes_transferred)) {
						std::string payload;
						ws_frame_type ret = web_socket_hand->parse_payload(&recv_buffer_[0], bytes_transferred, payload);
						handle_ws_frame(ret, std::move(payload), web_socket_hand->get_payload_length());
					}

					to_read_websocket_data();
					});
			}

			void to_read() {
				if (socket_ == nullptr){
					return;
				}

				recv_buffer_ = std::make_unique<char[]>(recv_buffer_size_);
				socket_->async_read_some(boost::asio::buffer(&recv_buffer_[0],recv_buffer_size_), [this](const boost::system::error_code ec, size_t bytes_transferred) {
					if (ec.value() == 0) {
						bool is_http = false;

						is_http = fetch_http_info(&recv_buffer_[0], bytes_transferred);
						if (is_http) {
							if (get_trans_type()) {
								std::shared_ptr<websocket_handle> web_socket_hand = std::make_shared<websocket_handle>();
								std::string msg = web_socket_hand->handle_shark_respond(get_header_info("Sec-WebSocket-Key"));
								to_send(msg.c_str(), msg.size());
								if (ws_timer_heart_ == nullptr) {
									ws_timer_heart_ = std::make_unique<wheel::unit::timer>(std::bind(&ws_tcp_handle::close_socket, this));
								}

								ws_ping();
								start_ws_heart();
								to_read_websocket_data();
								return;
							}
						}

						//�ͻ��˷��ʹ�����Ϣֱ�ӹر�
						close_observer_(shared_from_this(), boost::system::errc::make_error_code(boost::system::errc::errc_t(-1)));
					}else {
						if (this->get_connect_status() == disconnect) {
							return;
						}

						set_connect_status(disconnect);

						close_observer_(shared_from_this(), ec);
					}
					});
			}

			//��ȡwebsocket���������
			void to_read_server_data(const std::string &send_handleshake_key) {
				if (socket_ == nullptr){
					return;
				}

				recv_buffer_ = std::make_unique<char[]>(g_packet_buffer_size);
				socket_->async_read_some(boost::asio::buffer(&recv_buffer_[0], g_packet_buffer_size), [this, send_handleshake_key](const boost::system::error_code ec, size_t bytes_transferred) {
					if (ec.value() == 0) {
						//handleshake check
						std::shared_ptr<websocket_handle> web_socket_hand = std::make_shared<websocket_handle>();
						bool falg = web_socket_hand->compare_handle_shark_key(unit::find_substr(&recv_buffer_[0],"Sec-WebSocket-Accept",":"),send_handleshake_key);
						if (falg){
							if (ws_timer_heart_ == nullptr) {
								ws_timer_heart_ = std::make_unique<wheel::unit::timer>(std::bind(&ws_tcp_handle::close_socket, this));
							}

							ws_ping();
							to_read_websocket_data();
						}
					}
					else {
						if (this->get_connect_status() == disconnect) {
							return;
						}

						set_connect_status(disconnect);

						close_observer_(shared_from_this(), ec);
					}
					});
			}
			void async_connect(std::string ip, int port, const MessageEventObserver& recv_observer, const CloseEventObserver& close_observer) {
				if (socket_ == nullptr) {
					return;
				}

				socket_->async_connect(TCP::endpoint(ADDRESS::from_string(ip), port), [this, recv_observer, close_observer](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					set_connect_status(connectinged);
					register_close_observer(close_observer);
					register_recv_observer(recv_observer);
					to_read();
					});
			}
			void on_write(const boost::system::error_code& ec, std::size_t bytes_transferred) {
				--write_count_;

				if (ec) {
					//��ط�����ɾ��conencts,Ӧ��ֱ��֪ͨ���ʹ���
					return;
				}

				//��ü����ŵط�
				while (data_lock_.test_and_set(std::memory_order_acquire));
				if (send_buffers_.empty()){
					data_lock_.clear(std::memory_order_release);
					return;
				}

				std::shared_ptr<send_buffer>ptr = send_buffers_.front();
				ptr->read(bytes_transferred);

				if (ptr->size() == 0) {
					send_buffers_.pop_front();
				}

				if (!send_buffers_.empty()) {
					socket_->async_send(boost::asio::buffer(send_buffers_.front()->data(), send_buffers_.front()->size()), std::bind(&ws_tcp_handle::on_write, this,
						std::placeholders::_1, std::placeholders::_2));
					++write_count_;
				}

				data_lock_.clear(std::memory_order_release);
			}

			void set_connect_status(int status) {
				connect_status_ = status;
			}

			void send_ws_msg(std::string msg, opcode op) {
				std::shared_ptr<websocket_handle>ptr = std::make_shared<websocket_handle>();
				std::string header = ptr->format_header(msg.size(), op);
				std::string temp_msg = header + msg;
				to_send(temp_msg.c_str(), temp_msg.size());
			}
		private:
			bool fetch_http_info(const char* request, std::size_t bytes_transferred) {
				http_head_infos_.clear();

				const char* method = nullptr;
				const char* path = nullptr;
				int minor_version = 0;

				struct phr_header headers[100]{};
				std::size_t method_len = 0;
				std::size_t path_len = 0;
				std::size_t num_headers = sizeof(headers) / sizeof(headers[0]);

				int pret = phr_parse_request(request, bytes_transferred, &method, &method_len, &path, &path_len,
					&minor_version, headers, &num_headers, 0);

				if (pret > 0) {
					for (std::size_t i = 0; i < num_headers; ++i) {
						std::string key(headers[i].name, headers[i].name_len);
						std::string value(headers[i].value, headers[i].value_len);
						http_head_infos_.emplace(key, value);
					}

					return true;
				}

				return false;
			}

			std::string get_header_info(std::string key) {
				if (http_head_infos_.empty()) {
					return "";
				}

				auto iter = http_head_infos_.find(key);
				if (iter == http_head_infos_.end()) {
					return "";
				}

				return iter->second;
			}
			void ws_ping() {
				send_ws_msg("ping", opcode::ping);
			}

			void start_ws_heart() {
				if (ws_timer_heart_ == nullptr){
					return;
				}

				ws_timer_heart_->start_timer(ws_heart_seconds_);
			}

			void reset_ws_heart() {
				if (ws_timer_heart_ == nullptr){
					return;
				}

				ws_timer_heart_->cancel();
				start_ws_heart();
			}

			bool handle_ws_frame(ws_frame_type ret, std::string&& payload, size_t bytes_transferred) {
				switch (ret)
				{
				case ws_frame_type::WS_ERROR_FRAME:
					return false;
				case ws_frame_type::WS_OPENING_FRAME:
					break;
				case ws_frame_type::WS_TEXT_FRAME:
				case ws_frame_type::WS_BINARY_FRAME:
				{
					//�����ض��Ļ�������С�����账��
					if (bytes_transferred > g_packet_buffer_size){
						return true;
					}

					if (ret == ws_frame_type::WS_TEXT_FRAME)
					{
						std::shared_ptr<native_stream> stream(new native_stream(payload.c_str(), bytes_transferred));
						recv_observer_(shared_from_this(), stream);
					}
					else {
						if (protocol_parser_ == nullptr) {
							return true;
						}

						streams steams;
						protocol_parser_->read_stream(&recv_buffer_[0], bytes_transferred, steams);;
						for (const std::shared_ptr<native_stream> stream : steams) {
							recv_observer_(shared_from_this(), stream);
						}
					}
				}
				//on message
				break;
				case ws_frame_type::WS_CLOSE_FRAME:
				{
					std::shared_ptr<websocket_handle>handle_ptr = std::make_shared<websocket_handle>();
					std::string header = handle_ptr->format_header(bytes_transferred, opcode::close);
					std::string body = handle_ptr->format_close_payload(opcode::close, (char*)payload.data(), bytes_transferred);
					std::string msg = header + body;
					to_send(msg.c_str(), msg.size());
					//ֱ�ӷ��͹رգ���accpect�¹ر�,��ֹ�������
					//close_observer_(shared_from_this(),boost::system::errc::make_error_code(boost::system::errc::errc_t(10054)));
				}
				break;
				case ws_frame_type::WS_PING_FRAME:
				{
					std::shared_ptr<websocket_handle>handle_ptr = std::make_shared<websocket_handle>();
					std::string header = handle_ptr->format_header(bytes_transferred, opcode::pong);
					std::string msg = header + payload;
					to_send(msg.c_str(), msg.size());
				}
				break;
				case ws_frame_type::WS_PONG_FRAME:
					reset_ws_heart();
					ws_ping();
					break;
				default:
					break;
				}

				return true;
			}

			bool get_trans_type() {
				if (!http_head_infos_.empty()) {
					if (get_header_info("Upgrade") == "websocket") {
						return true;
					}
				}

				return false;
			}

			std::shared_ptr<IProtocol_parser>create_object(const int type,
				std::size_t header_size = 0, std::size_t packet_size_offset = 0, std::size_t packet_cmd_offset = 0) {
				if (type == 0) {
					if (header_size == 0 || packet_size_offset == 0 || packet_cmd_offset == 0) {
						return std::shared_ptr<IProtocol_parser>(new bin_parser(wheel::PACKET_HEADER_SIZE, wheel::PACKET_SIZE_OFFSET, wheel::PACKET_CMD_OFFSET));
					}

					return std::shared_ptr<IProtocol_parser>(new bin_parser(header_size, packet_size_offset, packet_cmd_offset));
				}

				return nullptr;
			}
		private:
			std::unique_ptr<boost::asio::steady_timer> timer_{};
			std::unique_ptr<wheel::unit::timer>ws_timer_heart_ = nullptr;
			int connect_status_ = disconnect;
			std::size_t header_size_;
			std::size_t packet_size_offset_;
			std::size_t packet_cmd_offset_;
			std::shared_ptr<boost::asio::io_service> ios_{};
			std::shared_ptr<TCP::socket> socket_{};
			std::list<std::shared_ptr<wheel::send_buffer>> send_buffers_;
			std::shared_ptr<IProtocol_parser>protocol_parser_ = nullptr;
			std::int32_t write_count_ = 0;
			ConnectEventObserver		connect_observer_;
			MessageEventObserver		recv_observer_;
			CloseEventObserver			close_observer_;
			//char recv_buffer_[g_packet_buffer_size] = { 0 };
			std::unique_ptr<char[]> recv_buffer_;
			int seconds_ = g_client_reconnect_seconds; //�ͻ�����������
			int ws_heart_seconds_ = g_ws_heart_seconds;
			HEADER_MAP http_head_infos_;
			std::size_t recv_buffer_size_ =1024;
			std::atomic_flag data_lock_ = ATOMIC_FLAG_INIT;
		};
	}
}

#endif // ws_tcp_handle_h__
