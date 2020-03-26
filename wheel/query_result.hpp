#ifndef query_result_h__
#define query_result_h__

#include "mysql_define.hpp"

namespace wheel {
	namespace mysql {
		class query_result
		{
			friend class mysql_wrap;
		public:
			query_result() :affected_rows_(0) {

			}

			~query_result() {

			}

		public:
			std::size_t row_count()const {
				return affected_rows_;
			}

			int get_field_index(const std::string& fieldname)const {
				std::size_t index = fieldtype_.size();
				for (std::size_t i = 0; i < fieldtype_.size(); ++i) {
					if (fieldtype_[i].name_ == fieldname) {
						index = fieldtype_[i].index_;
						break;
					}
				}

				return index;
			}

			const std::string& get_item_value(unsigned long row, unsigned int index)const {
				if (recordset_.size() <= row || fieldtype_.size() <= index)
				{
					throw std::out_of_range("ASSERT(recordset_.size() <= row || fieldtype_.size() <= index) ");
				}

				return recordset_[row][index];
			}

			const std::string& get_item_value(unsigned long row, const std::string& fieldname)const {
				const std::size_t index = get_field_index(fieldname);
				if (index == fieldtype_.size()) {
					"";
				}

				return get_item_value(row, index);
			}

			//�õ�ָ����ĳ���ֶε��ַ�������ֵ
			std::string get_item_string(unsigned long row, unsigned int index)const {
				return get_item_value(row, index);
			}

			std::string get_item_string(unsigned long row, const std::string& fieldname)const {
				return get_item_value(row, fieldname);
			}

			//�õ�ָ����ĳ���ֶε���ֵ
			double get_item_double(unsigned long row, const unsigned int index)const {

				return std::stof(get_item_value(row, index).c_str());
			}

			double get_item_double(unsigned long row, const std::string& fieldname)const {
				return  std::stof(get_item_value(row, fieldname).c_str());
			}

			//�õ�ָ����ĳ���ֶε�����ֵ
			long get_item_long(unsigned long row, const unsigned int index)const {
				return  std::stol(get_item_value(row, index).c_str());
			}

			long get_item_long(unsigned long row, const std::string& fieldname)const {
				return std::stol(get_item_value(row, fieldname).c_str());
			}

			int get_item_int(unsigned long row, const std::string& fieldname)const {
				return std::stoi(get_item_value(row, fieldname));
			}
		private:
			std::size_t affected_rows_;

			//�ֶ���Ϣ��
			field_t fieldtype_;

			std::vector<row_t> recordset_;
		};
	}
}

#endif // query_result_h__
