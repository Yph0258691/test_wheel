#ifndef mysql_wrap_h__
#define mysql_wrap_h__

#include <include/mysql.h>
#include <tuple>
#include <memory>
#include <mutex>

namespace wheel {
	namespace mysql {
		class mysql_wrap {
		public:
			static mysql_wrap get_intance() {
				static std::once_flag falg;
				std::call_once(falg, []() {
					instance_.reset(new mysql_wrap);
					});

				return *instance_;
			}
			~mysql_wrap() {
				disconnect();
			}

			void disconnect() {
				if (con_ != nullptr) {
					mysql_close(con_);
					con_ = nullptr;
				}
			}

			template<typename...Args>
			bool connect(Args&&... args) {
				static_assert(sizeof...(args) > 3,"args >3");

				if (con_ != nullptr) {
					mysql_close(con_);
				}

				con_ = mysql_init(nullptr);
				if (con_ == nullptr) {
					return false;
				}

				auto tp = std::make_tuple(con_, std::forward<Args>(args)...);
				int timeout = 0;
				if constexpr (sizeof...(args) ==5){
					timeout = std::get<5>(tp);
				}

				if (timeout > 0) {
					if (mysql_options(con_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout) != 0) {
						return false;
					}
				}

				char value = 1;
				mysql_options(con_, MYSQL_OPT_RECONNECT, &value);
				mysql_options(con_, MYSQL_SET_CHARSET_NAME, "utf8");

				if (mysql_real_connect(con_, std::get<1>(tp), std::get<2>(tp), std::get<3>(tp), std::get<4>(tp),0,NULL,0) == nullptr) {
					return false;
				}

				return true;
			}
		private:
			mysql_wrap() {

			}
			mysql_wrap(const mysql_wrap&) = default;
			mysql_wrap& operator=(const mysql_wrap&) = delete;
			mysql_wrap(const mysql_wrap&&) = delete;
			mysql_wrap& operator=(const mysql_wrap&&) = delete;
		private:
			static std::unique_ptr<mysql_wrap> instance_;
			MYSQL* con_ = nullptr;
		};

		std::unique_ptr<mysql_wrap>mysql_wrap::instance_ = nullptr;
	}
}//wheel
#endif // mysql_wrap_h__
