#ifndef timer_h__
#define timer_h__
#include <memory>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace wheel {
	namespace unit {
		typedef std::function<void()> Trigger_type;

		class timer {
		public:
			timer(Trigger_type trigger)
			:trigger_(trigger){
				ios_ = std::make_unique<boost::asio::io_service>();
				timer_ = std::make_unique<boost::asio::steady_timer>(*ios_);
				thread_ = std::make_unique<std::thread>([this] {ios_->run();});
			}
			
			~timer() {
				if (thread_ != nullptr){
					if (thread_->joinable()) {
						thread_->join(); //�ȴ����߳��˳�
					}
				}

				timer_ = nullptr;
				thread_ = nullptr;
			}

			//��ʱ��һ������ȥִ��
			void start_timer(int seconds) {
				timer_->expires_from_now(std::chrono::seconds(seconds));
				timer_->async_wait([this](const boost::system::error_code& ec) {
					if (ec){
						return;
					}

					trigger_(); //ִ�в�������
					});
			}

			//ȡ����ʱ��
			void cancel() {
				boost::system::error_code ec;
				timer_->cancel(ec);
			}

			//ѭ����ʱ��
			void loop_timer(int seconds) {
				timer_->expires_from_now(std::chrono::seconds(seconds));
				timer_->async_wait([this,seconds](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					trigger_(); //ִ�в�������
					loop_timer(seconds);
					});
			}

		private:
			std::unique_ptr<boost::asio::io_service>ios_ = nullptr;
			std::unique_ptr<std::thread>thread_ = nullptr;
			std::unique_ptr<boost::asio::steady_timer>timer_;
			Trigger_type trigger_;
		};
	}
}
#endif // timer_h__
