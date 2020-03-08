#ifndef timer2_h__
#define timer2_h__
#include <memory>
#include <list>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace wheel {
	namespace timer2 {

		class timer_context
		{
		public:
			typedef std::function<void(std::shared_ptr<timer2::timer_context>)> Trigger_type;

		public:
			explicit timer_context(Trigger_type trigger):trigger_(trigger){}
			virtual ~timer_context() {}

			void do_trigger(std::shared_ptr<timer2::timer_context> &ptr){
				trigger_(ptr);
			}
		private:
			Trigger_type trigger_;
		};

		typedef timer_context::Trigger_type Trigger_type;

		class timer
		{
		public:
			timer(const std::chrono::system_clock::time_point& deadline, std::shared_ptr<timer_context > contetx)
				:deadline_(deadline), contetx_(contetx), canceled_(false){

			}

			bool operator==(const timer_context& contetx)const{
				assert(contetx_ != nullptr);
				return canceled_ == false;
			}

			bool operator==(const timer_context* contetx)const{
				assert(contetx_ != nullptr);
				assert(contetx != nullptr);
				return canceled_ == false;
			}

			bool operator==(const std::shared_ptr<timer_context > contetx)const{
				assert(contetx_ != nullptr);
				assert(contetx != nullptr);
				return canceled_ == false;
			}

			bool operator==(const timer& other)const{
				assert(contetx_ != nullptr);
				assert(other.contetx_ != nullptr);
				return canceled_ == false;
			}

			bool operator < (const timer& other)const{
				return deadline_ < other.deadline_;
			}

			std::chrono::system_clock::time_point deadline_;

			std::shared_ptr<timer_context > contetx_;

			bool canceled_;
		};


		class timer_queue
		{
			typedef std::list<timer > Timers_type;

			typedef Timers_type::iterator  Timers_iterator_type;
		public:
			explicit timer_queue(boost::asio::io_service& ios) :ios_(ios), timer_(ios_),close_(false), lock_(false){

			}

			~timer_queue(void) {
				close_ = true;

				disable_alam();
			}
		public:
			int new_timer(std::shared_ptr<timer_context > contetx, long expires_from_now_millisecond) {
				if (close_ == true || expires_from_now_millisecond <= 0 || contetx == nullptr){
					return -1;
				}

				if (std::find(events_.begin(), events_.end(), contetx) == events_.end()){
					const auto deadline_timer = std::chrono::system_clock::now() + std::chrono::milliseconds(expires_from_now_millisecond);
					if (events_.empty() == true || events_.front().deadline_ > deadline_timer){
						events_.insert(events_.begin(), timer(deadline_timer, contetx));
						enable_alarm(expires_from_now_millisecond);
					}else{
						timer timer(deadline_timer, contetx);
						events_.insert(std::upper_bound(events_.begin(), events_.end(), timer), timer);
					}

					return 0;
				}
				else
				{
					return -1;
				}
			}

			int on_alarm(const boost::system::error_code& error){
				lock_ = true;

				if (close_ == true){
					lock_ = false;
					return 0;
				}

				if (error.value() == 0){
					std::chrono::system_clock::time_point pt(std::chrono::system_clock::now());

					for (Timers_iterator_type it = events_.begin(); it != events_.end();){
						if (it->canceled_ == true){
							it = events_.erase(it);
							continue;
						}

						if (it->deadline_ <= pt){
							std::shared_ptr<timer_context > contetx = it->contetx_;

							it = events_.erase(it);

							//��ֹѭ�����ã������ĵ�����ʧ�ܵ�����
							//����list�����Ժ󣬾Ͳ������������ˣ�����ӵ�Ԫ��һ������it֮�󣬲��ᵼ��itʧЧ
							//���� trigger_(contetx); �����У��п���ȡ�嶨ʱ��, ���У����ǻ����
							//Ϊ�˰�ȫ�����OnAlarm ������ events_
							contetx->do_trigger(contetx);
						}else{
							break;
						}
					}

					if (events_.empty() == false){
						enable_alarm(std::chrono::duration_cast<std::chrono::milliseconds>(events_.front().deadline_ - pt).count());
					}
				}

				lock_ = false;

				return 0;
			}


			bool cancel_timer(const timer_context& id, std::shared_ptr<timer_context >& context) {
				//ֻ��ɾ����Ԫ�أ���������������Ӱ�쵽������ʱ��
				//���ң����Ա�����ν�Ķ������������ú�ȡ����ʱ��
				Timers_iterator_type it = std::find(events_.begin(), events_.end(), id);
				if (it != events_.end()){
					context = it->contetx_;

					if (lock_ == true)
					{
						it->canceled_ = true;
					}
					else
					{
						events_.erase(it);
					}

					return true;
				}else{
					return false;
				}
			}

		private:
			void enable_alarm(long expires_from_now_milliseconds) {
				enable_alarm(std::chrono::milliseconds(expires_from_now_milliseconds));
			}

			void enable_alarm(const std::chrono::milliseconds expiry_time) {

				boost::system::error_code e;
				timer_.expires_from_now(expiry_time, e);
				timer_.async_wait(std::bind(&timer_queue::on_alarm, this, std::placeholders::_1));
			}

			void disable_alam(){
				boost::system::error_code e;
				timer_.cancel(e);
			}
		private:
			boost::asio::io_service& ios_;

			boost::asio::steady_timer	timer_;

			bool close_;

			// ��OnAlarm �����У����� events_
			bool lock_;

			Timers_type events_;
		};
	}
}//wheel
#endif // timer2_h__
