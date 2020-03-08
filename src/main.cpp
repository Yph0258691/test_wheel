#include <iostream>
#include "proxyEngine.h"
#include <wheel/client.hpp>
#include <wheel/server.hpp>

int main()
{
	ProxyEngine eng;
	std::shared_ptr<wheel::tcp_socket::server> ptr = std::make_shared<wheel::tcp_socket::server>(std::bind(&ProxyEngine::OnMessage, &eng, std::placeholders::_1,std::placeholders::_2),0,8,2,6); //偏移后的值

	ptr->init(9000,4);

	//std::shared_ptr<wheel::client> ptr = std::make_shared<wheel::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
	//	//ptr->connect("127.0.0.1", 3333);
			ptr->run();
			}
