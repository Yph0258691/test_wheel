#ifndef io_service_poll_h__
#define io_service_poll_h__

#include <memory>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

/***********************在 event handler 中允许执行阻塞的操作(例如数据库查询操作)*****************************************/
namespace wheel {
	class io_service_poll {
	public:
		io_service_poll():service_(std::make_shared<boost::asio::io_service>()) {
		}

		~io_service_poll() {
			stop();
		}


		std::shared_ptr<boost::asio::io_service> get_io_service()const
		{
			return service_;
		}

		void run(size_t thread_num)
		{
			for (int i = 0; i < thread_num-1; ++i) {
				threads_.emplace_back([this]() { 
					boost::system::error_code ec;
						service_->run(ec);
					});
			}

			boost::system::error_code ec;
			service_->run(ec);
		}

		io_service_poll(const io_service_poll&) = delete;
		io_service_poll& operator=(const io_service_poll&) = delete;
	private:
		void stop()
		{
			service_->stop();
			for (auto& t : threads_) {
				t.join();
			}
		}
	private:
		std::vector<std::thread>threads_;
		std::shared_ptr<boost::asio::io_service> service_;
	};
}
#endif // io_service_poll_h__
