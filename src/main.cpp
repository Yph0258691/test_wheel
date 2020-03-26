
// #include <iostream>
// #include "proxyEngine.h"
// #include <wheel/client.hpp>
// #include <wheel/server.hpp>
// #include"test.h"
// #include <nlohmann_json.hpp>
// #include<wheel/unit.hpp>

// int main()
// {
// 	std::string str ="fe80::5079:92b2:9cbf:e719%11";

// 	bool is = wheel::unit::ipV6_check(str);
// 	try {
// 		nlohmann::json js;
// 		js["name"] = "123";
// 		js["sex"] = 1;
// 		js["age"] = 12;

// 		std::string json = js.dump();
// 		std::cout << json << std::endl;
// 	}
// 	catch (...) {

// 	}

// 	Test t;
// 	t.display();
// 	ProxyEngine eng;
// 	std::shared_ptr<wheel::tcp_socket::server> ptr = std::make_shared<wheel::tcp_socket::server>(std::bind(&ProxyEngine::OnMessage, &eng, std::placeholders::_1, std::placeholders::_2), 0, 8, 2, 6); //偏移后的值

// 	ptr->init(9000, 4);

// 	//std::shared_ptr<wheel::client> ptr = std::make_shared<wheel::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
// 	//	//ptr->connect("127.0.0.1", 3333);
// 	ptr->run();
// }
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
// 	//更新指定一条数据
// 	//wheel::mysql::mysql_wrap::get_intance().update(ns);
// 	//更新指定n条数据
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
// 	//更新指定一条数据
// 	//wheel::mysql::mysql_wrap::get_intance().update(ns);
// 	//更新指定n条数据
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

#include <iostream>
#include <wheel/entity.hpp>
#include <wheel/mysql_wrap.hpp>


struct name {
	std::string user_name;
	int age;
};

REFLECTION(name, user_name, age)

struct user {
	int id;
	std::string user_name;
	int age;
};

REFLECTION(user, id,user_name, age)


int main()
{

	std::vector<name>vec;
	for (int i =0;i<2;++i){
		name ns;
		ns.age = 10;
		ns.user_name = "1245";
		vec.emplace_back(ns);
	}

	wheel::mysql::mysql_wrap::get_intance().connect("127.0.0.1", "root", "root", "test");
	//创建数据库表
	//wheel::mysql::wheel_sql_key key{ "id" };
	//wheel::mysql::mysql_wrap::get_intance().create_table<user>(key);

	//wheel::mysql::mysql_wrap::get_intance().create_table<user>();
	wheel::mysql::wheel_sql_auto_key key1("id");
	wheel::mysql::wheel_sql_not_null key2("id");

	wheel::mysql::mysql_wrap::get_intance().create_table<user>( key1,key2);

	//更新指定一条数据
	//wheel::mysql::mysql_wrap::get_intance().update(ns);
	//更新指定n条数据
	//wheel::mysql::mysql_wrap::get_intance().update(vec);
	//wheel::mysql::mysql_wrap::get_intance().insert(vec);

	//wheel::mysql::mysql_wrap::get_intance().delete_records<name>("age =20 and user_name =\"yph1111\"");
	//auto result1 = wheel::mysql::mysql_wrap::get_intance().query<name>("select age from name where age =10 and user_name =\"yph1111\"");
	//wheel::mysql::mysql_wrap::get_intance().query("select age from name where age =10 and user_name =\"yph1111\"");
	wheel::mysql::query_result result;
	//wheel::mysql::mysql_wrap::get_intance().query("select age from name where age =10 and user_name =\"yph1111\"", result);
	//存在过程
	//wheel::mysql::mysql_wrap::get_intance().query("call test_proc(10)", result);
	//std::string sss = result.get_item_string(0, "user_name");
	//int age = result.get_item_int(0, "age");
	//auto result4 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<int>>("select count(1) from name");

	//auto result5 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<std::string,int>>("select user_name, age from name");
}

