#include "proxyEngine.h"
#include <wheel/native_stream.hpp>
#include <wheel/tcp_hanlde.hpp>

ProxyEngine::ProxyEngine()
{

}

ProxyEngine::~ProxyEngine()
{

}

int ProxyEngine::OnMessage(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler, std::shared_ptr<wheel::native_stream> streams)
{
	handler->get_read_parser()->set_stram_data(streams);
	int cmd = handler->get_read_parser()->get_cmd();
	int value = handler->get_read_parser()->read<int>();
	std::int16_t value1 = handler->get_read_parser()->read<std::int16_t>();
	std::string str = handler->get_read_parser()->read<std::string>();

	if (cmd ==4){
		handler->get_write_parser()->write_header(425);
		handler->get_write_parser()->write<std::int16_t>(102);
		handler->get_write_parser()->write(wheel::unit::float_to_uint32(102.1f));
		handler->get_write_parser()->write_string("yhp123545");
		handler->get_write_parser()->end();
		handler->to_send(*handler->get_write_parser()->get_native_stream());
		wheel::native_stream streamssss = *handler->get_write_parser()->get_native_stream();
		std::shared_ptr<wheel::native_stream> sptr(new wheel::native_stream(streamssss.get_data(), streamssss.get_size()));
		handler->get_read_parser()->set_stram_data(sptr);
		handler->get_read_parser()->reset_read_body();
		int cmd = handler->get_read_parser()->get_cmd();
		std::int16_t value = handler->get_read_parser()->read<std::int16_t>();
		float fa = wheel::unit::uint32_to_float(handler->get_read_parser()->read<std::uint32_t>(),1);
		std::string str = handler->get_read_parser()->read<std::string>();
	}
	return 0;
}

int ProxyEngine::OnMessage1(std::shared_ptr<wheel::websocket::ws_tcp_handle> handler, std::shared_ptr<wheel::native_stream> streams)
{
	return 0;
}
