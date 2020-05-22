
// #include <iostream>
// #include "proxyEngine.h"
// #include <wheel/client.hpp>
// #include <wheel/server.hpp>
// #include"test.h"
// #include <nlohmann_json.hpp>
// #include<wheel/unit.hpp>

//  int main()
// {
// 	std::string str ="fe80::5079:92b2:9cbf:e719%11";

//  	bool is = wheel::unit::ipV6_check(str);
//  	try {
//  		nlohmann::json js;
//  		js["name"] = "123";
//  		js["sex"] = 1;
//  		js["age"] = 12;

//  		std::string json = js.dump();
//  		std::cout << json << std::endl;
//  	}
// 	catch (...) {

//  	}

//  	Test t;
//  	t.display();
//  	ProxyEngine eng;
// 	std::shared_ptr<wheel::tcp_socket::server> ptr = std::make_shared<wheel::tcp_socket::server>(std::bind(&ProxyEngine::OnMessage, &eng, std::placeholders::_1, std::placeholders::_2), 0, 8, 2, 6); //ƫ�ƺ��ֵ

//  	ptr->init(9000, 4);

// 	//std::shared_ptr<wheel::client> ptr = std::make_shared<wheel::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
//  	//	//ptr->connect("127.0.0.1", 3333);
// 	ptr->run();
//  }
// #include<iostream>
// #include"wheel/json.hpp"
// #include <config.h>

// struct person
// {
// 	std::string  name;
// 	int          age;
// };

// REFLECTION(person, name, age)
// struct one_t
// {
// 	int id;
// };
// REFLECTION(one_t, id);
// struct composit_t
// {
// 	int a;
// 	std::vector<std::string> b;
// 	int c;
// 	std::map<int, int> d;
// 	std::unordered_map<int, int> e;
// 	double f;
// 	std::list<one_t> g;
// };
// REFLECTION(composit_t, a, b, c, d, e, f, g);

// int main()
// {
// #ifdef TEST
// 	std::cout << "1123 test" << std::endl;
// #else
// 	std::cout << "1123 no" << std::endl;
// #endif

// 	auto const t = std::make_tuple(42, 'z', 3.14, 13, 0, "Hello, World!");

// 	for (std::size_t i = 0; i < std::tuple_size<decltype(t)>::value; ++i) {
// 		wheel::unit::tuple_switch(i, t, [](const auto& v) {
// 			std::cout << v << std::endl;
// 			});

// 	}
// 	person obj;
// 	const char* json = "{\"name\" : \"Boo\",	\"age\" : 28}";

// 	wheel::json::from_json(obj, json);

// 	wheel::json::string_stream sst;
// 	wheel::json::to_json(sst, obj);
// 	std::string str = sst.str();
//         std::cout<<str<<std::endl;
// 	one_t one = { 2 };
// 	composit_t composit = { 1,{ "tom", "jack" }, 3,{ { 2,3 } },{ { 5,6 } }, 5.3,{ one } };
// 	wheel::json::string_stream sst1;
// 	wheel::json::to_json(sst1, composit);
// 	str = sst1.str();
//         std::cout<<str<<std::endl;
// 	composit_t composit11;
// 	wheel::json::from_json(composit11, str.c_str());

// 	const char* str_comp = R"({"b":["tom", "jack"], "a":1, "c":3, "e":{"3":4}, "d":{"2":3,"5":6},"f":5.3,"g":[{"id":1},{"id":2}])";
// 	composit_t comp;
// 	wheel::json::from_json(comp, str_comp);
// 	std::cout << comp.a << " " << comp.f << std::endl;

// 	int i = 100;

// }


// #include <iostream>
// #include <wheel/mysql_wrap.hpp>

// int main()
// {
// 	wheel::mysql::mysql_wrap::get_intance().connect("127.0.0.1", "root", "root", "test",123);
// }

// #include <iostream>
// #include <wheel/mysql_wrap.hpp>


// struct name {
// 	std::string uer_name;
// 	int age;
// };

// REFLECTION(name, uer_name, age)

// int main()
// {

// 	std::vector<name>vec;
// 	for (int i =0;i<2;++i){
// 		name ns;
// 		ns.age = 10;
// 		ns.uer_name = "1245";
// 		vec.emplace_back(ns);
// 	}

// 	wheel::mysql::mysql_wrap::get_intance().connect("127.0.0.1", "root", "root", "test");
// 	//����ָ��һ������
// 	//wheel::mysql::mysql_wrap::get_intance().update(ns);
// 	//����ָ��n������
// 	wheel::mysql::mysql_wrap::get_intance().update(vec);
// 	//wheel::mysql::mysql_wrap::get_intance().insert(vec);

// 	//wheel::mysql::mysql_wrap::get_intance().delete_records<name>("age =20 and user_name =\"yph1111\"");
// 	//auto result1 = wheel::mysql::mysql_wrap::get_intance().query<name>("age =10");
// 	//auto result4 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<int>>("select count(1) from name");

// 	//auto result5 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<std::string,int>>("select user_name, age from name");
// }

// #include <iostream>
// #include <wheel/mysql_wrap.hpp>


// struct name {
// 	std::string user_name;
// 	int age;
// };

// REFLECTION(name, user_name, age)

// int main()
// {

// 	std::vector<name>vec;
// 	for (int i =0;i<2;++i){
// 		name ns;
// 		ns.age = 10;
// 		ns.user_name = "1245";
// 		vec.emplace_back(ns);
// 	}

// 	wheel::mysql::mysql_wrap::get_intance().connect("127.0.0.1", "root", "root", "test");
// 	//����ָ��һ������
// 	//wheel::mysql::mysql_wrap::get_intance().update(ns);
// 	//����ָ��n������
// 	//wheel::mysql::mysql_wrap::get_intance().update(vec);
// 	//wheel::mysql::mysql_wrap::get_intance().insert(vec);

// 	//wheel::mysql::mysql_wrap::get_intance().delete_records<name>("age =20 and user_name =\"yph1111\"");
// 	//auto result1 = wheel::mysql::mysql_wrap::get_intance().query<name>("select age from name where age =10 and user_name =\"yph1111\"");
// 	//wheel::mysql::mysql_wrap::get_intance().query("select age from name where age =10 and user_name =\"yph1111\"");
// 	wheel::mysql::query_result result;
// 	wheel::mysql::mysql_wrap::get_intance().query("call test_proc(10)", result);

// 	std::string sss = result.get_item_string(0, "user_name");
// 	int age = result.get_item_int(0, "age");
// 	//auto result4 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<int>>("select count(1) from name");

// 	//auto result5 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<std::string,int>>("select user_name, age from name");
// }




// #include <iostream>
// #include <wheel/http_server.hpp>
// #include <wheel/gzip.hpp>

// int main()
// {
// 	std::string data = "111111111111111111111111assd";
// 	std::string compress_data;
// 	wheel::gzip_codec::compress(data, compress_data);
// 	using namespace wheel::http_servers;
// 	wheel::http_servers::http_server server;
// 	server.listen(9090);
// 	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
// 		res.set_status_and_content(status_type::ok, "��Ƽ��ƨ��");
// 		});


// 	server.set_http_handler<GET, POST>("/test", [](request& req, response& res) {
// 		std::string str = req.get_header_value("session");
// 		std::string name = req.get_multipart_field_name("name");
// 		std::string age = req.get_multipart_field_name("age");

// 		res.set_status_and_content(status_type::ok, "��Ƽ��ƨ��");
// 		});

// 	server.run();
// }
//
//#include <iostream>
//#include <wheel/http_server.hpp>
//
//int main()
//{
//	std::string str = "11";
//	if (!str.empty()){
//		int i = 100;
//	}
//	using namespace wheel::http_servers;
//	wheel::http_servers::http_server server;
//	//server.set_ssl_conf({ "server.crt", "server.key","1234561" });
//	server.set_ssl_conf({ "www.wheellovecplus.xyz_bundle.crt", "www.wheellovecplus.xyz.key"});
//	server.listen(9090);
//	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
//		res.set_status_and_content(status_type::ok, "hello world");
//		});
//
//
//	server.set_http_handler<GET, POST, OPTIONS>("/test", [](request& req, response& res) {
//		res.set_status_and_content(status_type::ok, "hello world");
//		});
//
//	server.run();
//}

// #define WHEEL_ENABLE_SSL
// //#define WHEEL_ENABLE_GZIP
// #include <iostream>
// #include <wheel/http_server.hpp>
// #include <wheel/encoding_conversion.hpp>

// using namespace wheel::http_servers;
// wheel::http_servers::http_server server;

// int main()
// {
// 	std::string str1234 = "我是好人我問三十多萬多無多無多無多多";

// 	std::wstring w_name = wheel::char_encoding::encoding_conversion::to_wstring(str1234);
// 	if (wheel::char_encoding::encoding_conversion::is_valid_gbk(str1234.c_str())){
// 		int i = 10;
// 	}
// 	std::cout << str1234 << std::endl;
// 	std::string wwwwww = wheel::char_encoding::encoding_conversion::to_string(w_name);


// 	//std::string zip_str;
// 	//wheel::gzip_codec::compress(str1234, zip_str);
// 	//uf8
// 	std::string s1123 = wheel::char_encoding::encoding_conversion::gbk_to_utf8(str1234);
// 	std::u16string u16_str = wheel::char_encoding::encoding_conversion::utf8_to_utf16(s1123);
// 	std::cout << "u16_str:"<<u16_str.c_str()<<std::endl;

// 	std::u32string u32_str1 = wheel::char_encoding::encoding_conversion::utf8_to_utf32(s1123);
// 	std::cout << "u32_str1:" << u32_str1.c_str() << std::endl;
// 	std::u32string u32_str2 = wheel::char_encoding::encoding_conversion::utf16_to_utf32(u16_str);
// 	std::cout << "u32_str2:" << u32_str2.c_str() << std::endl;




// 	std::string str12345 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(s1123);
// 	std::string test_str;
// 	test_str.resize(1024);

// 	memcpy(&test_str[0], "123", 3);

// 	memcpy(&test_str[3], "456", 3);

// 	memcpy(&test_str[6], "456", 3);

// 	std::string str = "11";
// 	if (!str.empty()){
// 		int i = 100;
// 	}

// 	//server.set_ssl_conf({ "server.crt", "server.key","1234561" });
// 	server.set_ssl_conf({ "www.wheellovecplus.xyz_bundle.crt", "www.wheellovecplus.xyz.key"});
// 	server.enable_response_time(true);
// 	server.listen(9090);
// 	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
// 		res.set_status_and_content(status_type::ok, "hello world");
// 		});


// 	server.set_http_handler<GET, POST, OPTIONS,PUT>("/test", [](request& req, response& res) {

// 	//	std::string str = req.get_header_value("Accept-Encoding");//获取包头值
// 	//	                                                    
// 	//	std::string str1 = req.get_query_value("name");//获取包体值
// 	//	std::string str23133 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(str1);
// 	//	std::string str2 = req.get_query_value("age");//获取包体值

// 	//	std::string part_data = req.get_part_data();
// 	////	std::string str3 = req.get_query_value("sex");//获取包体值
// 	//	//std::string body = req.body();
// 	//	//跨域解决
// 	//	res.add_header("Access-Control-Allow-origin", "*");
// 	//	//res.set_status_and_content(status_type::ok,"hello world");
// 	//	//res.set_status_and_content(status_type::ok, "hello world",res_content_type::string);
// 	//	std::string str4 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多");
// 	//	res.set_status_and_content(status_type::ok,std::move(str4), res_content_type::string);

// 		std::string str1 = req.get_query_value("name");//获取包体值
// 		std::string str23133 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(str1);
// 		std::string str2 = req.get_query_value("age");//获取包体值

// 		std::string part_data = req.get_part_data();
// 		std::string name1 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(req.get_query_value("name1"));
// 		std::cout << name1 << std::endl;
// 		std::string path = wheel::char_encoding::encoding_conversion::utf8_to_gbk(req.get_query_value("path1"));
// 		res.add_header("Access-Control-Allow-origin", "*");
// 		//res.set_status_and_content(status_type::ok,"hello world");
// 		//res.set_status_and_content(status_type::ok, "hello world",res_content_type::string);
// 		//std::string str4 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多abcdefg");
// 		//res.set_urlencoded_datas("name",std::move(str4));
// 		//res.set_urlencoded_datas("age", "20123454");
// 		//res.set_urlencoded_status_and_content(status_type::ok,content_encoding::none,transfer_type::chunked);

// 		//res.set_multipart_data("name","wshihaoren"); //一次性发出的数据
// 		//res.set_multipart_data("age", "123");
// 		//res.set_multipart_data("age1", "456");
// 		//res.set_multipart_data("name1",std::move(str4));
// 		//std::string str5 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多abcdefg!~*&^23455698321550/.,??><");
// 		//res.set_multipart_data("name2", std::move(str5));

// 		//res.set_mstatus_and_content(status_type::ok,transfer_type::chunked);
// 		//res.set_status_and_content(status_type::ok, std::move(str4), res_content_type::urlencoded);
// 		std::string str5 = "我是好人我問三十多萬多無多無多無多多abcdefg!~*&^23455698321550/.,??><";
// 		if (wheel::char_encoding::encoding_conversion::is_valid_gbk(str5.c_str())){
// 			std::cout << "yes gbk" << std::endl;
// 		}

// 		std::string str6 = wheel::char_encoding::encoding_conversion::gbk_to_utf8(str5);
// 		res.set_status_and_content(status_type::ok,std::move(str6),res_content_type::string,content_encoding::none,transfer_type::chunked);
// 		});

// 	server.run();
// }

#include <wheel/serialize.hpp>

namespace client
{
	struct person
	{
		std::string	name;
		int64_t		 age;
	};

	REFLECTION(person, name, age);
}

int main()
{
	client::person p1 = { "tom", 20 };
	client::person p2 = { "jack", 19 };
	client::person p3 = { "mike", 21 };

	std::vector<client::person> v{ p1, p2, p3 };
	wheel::str_stream::string_stream ss;
	wheel::serialization::to_json(ss, v);
	auto json_str = ss.str();
	std::cout << json_str << std::endl;

	std::vector<client::person> v1;
	wheel::serialization::from_json(v1, json_str.data(), json_str.length());
}


