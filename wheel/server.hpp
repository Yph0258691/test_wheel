#ifndef sever_h__
#define sever_h__

#include <memory>
#include <boost/asio.hpp>
#include "tcp_hanlde.hpp"
#include <unordered_map>
#include <atomic>

namespace wheel {
	namespace tcp_socket {
		class server
		{
		public:
			server(const MessageEventObserver& recv_observer) :recv_observer_(recv_observer) {
				try{
					io_service_ = std::make_shared<boost::asio::io_service>();
				}catch (const std::exception&ex)
				{
					std::cout << ex.what() << std::endl;
					io_service_ = nullptr;
				}
			}

			server(const MessageEventObserver& recv_observer, int parser_type, std::size_t header_size,
				std::size_t packet_size_offset, std::size_t packet_cmd_offset) 
				:recv_observer_(recv_observer), parser_type_(parser_type)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offset) {
				try {
					io_service_ = std::make_shared<boost::asio::io_service>();
				}
				catch (const std::exception & ex)
				{
					std::cout << ex.what() << std::endl;
					io_service_ = nullptr;
				}
			}

			~server() {
				join_all();

				ios_threads_.clear();
			}

			void init(int port, int connect_pool) {
				//ע��ط�Ҫͬһ��iossever����֤��һ���̶߳�����
				if (io_service_ ==nullptr){
					return;
				}

				//accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
				boost::system::error_code ec;
				accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_);
				
				//һ��Ҫ����open��������ʧ��
				accept_->open(boost::asio::ip::tcp::v4());
				//�˿ڸ���
				accept_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
				accept_->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), ec);
				accept_->listen(boost::asio::socket_base::max_connections, ec);
				if (ec) {
					std::cout << "����������ʧ��:" << ec.message() << std::endl;
					return;
				}

				if (accept_ == nullptr) {
					return;
				}

				for (int i = 0;i < connect_pool;i++) {
					make_session();
				}
			}

			void run() {
				if (io_service_ == nullptr){
					return;
				}

				std::size_t num_threads = std::thread::hardware_concurrency();
				//���̼߳�һ���� 
				try
				{
					for (size_t c = 0;c < num_threads - 1; c++) {
						ios_threads_.emplace_back(std::make_shared<std::thread>([this]() {
							boost::system::error_code ec;
							io_service_->run(ec);
							std::cout << ec.message() << std::endl;
							}));
					}
				}catch (...){
					std::cout << "expection..." << std::endl;
					return;
				}

				boost::system::error_code ec;
				io_service_->run(ec);
				std::cout << ec.message() << std::endl;
			}
		private:
			void make_session() {
				if (accept_ == nullptr) {
					return;
				}

				std::shared_ptr<tcp_handle > new_session = nullptr;
				try
				{
					new_session =std::make_shared<tcp_handle>(io_service_, header_size_, packet_size_offset_, packet_cmd_offset_);
				}
				catch (const std::exception&ex)
				{
					std::cout << ex.what() << std::endl;
					new_session = nullptr;
				}
				
				if (new_session == nullptr){
					return;
				}

				new_session->set_protocol_parse_type(parser_type_);
				new_session->register_close_observer(std::bind(&server::on_close, this, std::placeholders::_1, std::placeholders::_2));
				new_session->register_connect_observer(std::bind(&server::on_connect, this, std::placeholders::_1));
				new_session->register_recv_observer(recv_observer_);

				//��һ�����ݽ���һ��
				accept_->async_accept(*new_session->get_socket(), [this, new_session](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					new_session->activate();
					make_session();
					});
			}
			void join_all() {
				//����֮����Ҫ�ȴ�,�����̲߳�����
				for (auto& t : ios_threads_) {
					if (!t->joinable()) {
						t->join();
					}
				}
			}
		private:
			void on_connect(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler) {
				if (handler ==nullptr){
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

			void on_close(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler, const boost::system::error_code& ec) {
				//tcp�����Ͽ� boost::asio::error::connection_reset
				//websocket�����Ͽ�  boost::asio::error::connection_aborted
				while (lock_.test_and_set(std::memory_order_acquire));
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					connects_.erase(iter_find);
				}

				lock_.clear(std::memory_order_release);
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
			std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
			std::vector<std::shared_ptr<std::thread>> ios_threads_;
		};
	}
}
#endif // sever_h__
