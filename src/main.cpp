
//#include <iostream>
//#include "proxyEngine.h"
//#include <wheel/client.hpp>
//#include <wheel/server.hpp>
//#include"test.h"
//#include <nlohmann_json.hpp>
//int main()
//{
//	try {
//		nlohmann::json js;
//		js["name"] = "123";
//		js["sex"] = 1;
//		js["age"] = 12;
//
//		std::string json = js.dump();
//		std::cout << json << std::endl;
//	}
//	catch (...) {
//
//	}
//
//	Test t;
//	t.display();
//	ProxyEngine eng;
//	std::shared_ptr<wheel::tcp_socket::server> ptr = std::make_shared<wheel::tcp_socket::server>(std::bind(&ProxyEngine::OnMessage, &eng, std::placeholders::_1, std::placeholders::_2), 0, 8, 2, 6); //偏移后的值
//
//	ptr->init(9000, 4);
//
//	//std::shared_ptr<wheel::client> ptr = std::make_shared<wheel::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
//	//	//ptr->connect("127.0.0.1", 3333);
//	ptr->run();
//}
#include<iostream>
#include<wheel/json.hpp>
struct person
{
	std::string  name;
	int          age;
};

REFLECTION(person, name, age)
struct one_t
{
	int id;
};
REFLECTION(one_t, id);
struct composit_t
{
	int a;
	std::vector<std::string> b;
	int c;
	std::map<int, int> d;
	std::unordered_map<int, int> e;
	double f;
	std::list<one_t> g;
};
REFLECTION(composit_t, a, b, c, d, e, f, g);

int main()
{
	auto const t = std::make_tuple(42, 'z', 3.14, 13, 0, "Hello, World!");

	for (std::size_t i = 0; i < std::tuple_size<decltype(t)>::value; ++i) {
		wheel::unit::tuple_switch(i, t, [](const auto& v) {
			std::cout << v << std::endl;
			});

	}
	person obj;
	const char* json = "{\"name\" : \"Boo\",	\"age\" : 28}";

	wheel::json::from_json(obj, json);

	wheel::json::string_stream sst;
	wheel::json::to_json(sst, obj);
	std::string str = sst.str();
        std::cout<<str<<std::endl;
	one_t one = { 2 };
	composit_t composit = { 1,{ "tom", "jack" }, 3,{ { 2,3 } },{ { 5,6 } }, 5.3,{ one } };
	wheel::json::string_stream sst1;
	wheel::json::to_json(sst1, composit);
	str = sst1.str();
        std::cout<<str<<std::endl;
	composit_t composit11;
	wheel::json::from_json(composit11, str.c_str());

	const char* str_comp = R"({"b":["tom", "jack"], "a":1, "c":3, "e":{"3":4}, "d":{"2":3,"5":6},"f":5.3,"g":[{"id":1},{"id":2}])";
	composit_t comp;
	wheel::json::from_json(comp, str_comp);
	std::cout << comp.a << " " << comp.f << std::endl;

	int i = 100;

}
