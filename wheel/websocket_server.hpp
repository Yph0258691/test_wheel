#ifndef websocket_server_h__
#define websocket_server_h__
#include "ws_tcp_handle.hpp"
#include <unordered_map>
#include <atomic>

namespace wheel {
	namespace websocket {
		enum wes_paser_type
		{
			json = 0,//默认是JSON
			bin = 1,
		};

		class websocket_server {
		public:
			//json 字符串
			websocket_server(const MessageEventObserver& recv_observer)
				:recv_observer_(recv_observer) {
				try
				{
					io_service_ = std::make_shared<boost::asio::io_service>();
				}
				catch (const std::exception&ex)
				{
					std::cout << ex.what() << std::endl;
					io_service_ = nullptr;
				}
			}

			//二进制流的格式
			websocket_server(const MessageEventObserver& recv_observer, int parser_type, std::size_t header_size,
				std::size_t packet_size_offset, std::size_t packet_cmd_offset)
				:recv_observer_(recv_observer), parser_type_(parser_type)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offset) {
				try
				{
					io_service_ = std::make_shared<boost::asio::io_service>();
				}
				catch (const std::exception & ex)
				{
					std::cout << ex.what() << std::endl;
					io_service_ = nullptr;
				}
			}

			~websocket_server() {
				join_all();

				ios_threads_.clear();
			}

			void init(int port, int connect_pool) {
				//accept的ios要和socket同一地址，否则会出问题
				if (io_service_ =nullptr){
					return;
				}

				try
				{
					accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
				}
				catch (const std::exception&ex){
					std::cout << ex.what() << std::endl;
					accept_ = nullptr;
				}

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
				if (io_service_ ==nullptr){
					return;
				}

				std::size_t num_threads = std::thread::hardware_concurrency();
				//子线程减一个啊 
				try{
					for (size_t c = 0;c < num_threads - 1; c++) {
						ios_threads_.emplace_back(std::make_shared<std::thread>([this]() {
							io_service_->run();
							}));
					}

				}catch (const std::exception&ex){
					std::cout << ex.what() << std::endl;
					return;
				}

				io_service_->run();
				join_all();
			}
		private:
			void make_session() {
				if (accept_ == nullptr ||io_service_ ==nullptr) {
					return;
				}

				std::shared_ptr<ws_tcp_handle> new_session = nullptr;
				try
				{
					if (parser_type_ == json) {
						new_session = std::make_shared<ws_tcp_handle>(io_service_, header_size_, packet_size_offset_, packet_cmd_offset_);
					}
					else if (parser_type_ == bin) {
						new_session = std::make_shared<ws_tcp_handle>(io_service_);
					}
					else {
						return;
					}

				}catch (const std::exception&ex)
				{
					std::cout << ex.what() << std::endl;
				}

				if (new_session ==nullptr){
					return;
				}

				new_session->register_close_observer(std::bind(&websocket_server::on_close, this, std::placeholders::_1, std::placeholders::_2));
				new_session->register_connect_observer(std::bind(&websocket_server::on_connect, this, std::placeholders::_1));
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
			
			void join_all() {
				for (auto& t : ios_threads_) {
					if (!t->joinable()) {
						t->join();
					}
				}
			}
		private:
			void on_connect(std::shared_ptr<ws_tcp_handle> handler) {
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					return;
				}

				std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
				std::time_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();

				lock_.test_and_set(std::memory_order_acquire);
				connects_.emplace(handler, timestamp);
				lock_.clear(std::memory_order_release);
			}

			void on_close(std::shared_ptr<ws_tcp_handle> handler, const boost::system::error_code& ec) {
				//tcp主动断开 boost::asio::error::connection_reset
				//websocket主动断开  boost::asio::error::connection_aborted
				lock_.test_and_set(std::memory_order_acquire);
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					connects_.erase(iter_find);
				}

				lock_.clear(std::memory_order_release);
			}
		private:
			std::unordered_map<std::shared_ptr<ws_tcp_handle>,std::time_t>connects_;
			std::unique_ptr<TCP::acceptor>accept_{};
			std::shared_ptr<boost::asio::io_service> io_service_{};
			MessageEventObserver		recv_observer_;
			int parser_type_ = json;
			std::size_t header_size_ = 0;
			std::size_t packet_size_offset_ = 0;
			std::size_t packet_cmd_offset_ = 0;
			std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
			std::vector<std::shared_ptr<std::thread>> ios_threads_;
		};
  }
}

#endif // websocket_server_h__
