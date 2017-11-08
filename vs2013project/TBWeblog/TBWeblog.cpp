// TBWeblog.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#pragma warning(disable:4996)  // for websocketpp

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <thread>
#include <memory>
#include <typeinfo>
#include <future>
#include <sstream>

namespace wspp = websocketpp;

typedef wspp::server<wspp::config::asio> weblog_server;

using wspp::lib::placeholders::_1;
using wspp::lib::placeholders::_2;
using wspp::lib::bind;


// pull out the type of messages sent by our config
typedef weblog_server::message_ptr message_ptr;

// Define a callback to handle incoming messages
void on_message(weblog_server* s, wspp::connection_hdl hdl, message_ptr msg) {
	std::cerr << "[tbweblog]" << __FUNCTION__ << ",hdl@" << hdl.lock().get()
		<< ",message: " << msg->get_payload()
		<< std::endl;

	// check for a special command to instruct the server to stop listening so
	// it can be cleanly exited.
	if (msg->get_payload() == "stop-listening") {
		s->stop_listening();

		return;
	}

	try {
		s->send(hdl, msg->get_payload(), msg->get_opcode());
	}
	catch (const wspp::lib::error_code& e) {
		std::cerr << "Echo failed because: " << e
			<< "(" << e.message() << ")" << std::endl;
	}
}

int main() {
	using namespace std;
	using namespace wspp;
	// Create a server endpoint
	weblog_server echo_server;
	connection_hdl conn_hdl;


	std::atomic<bool> input_enable(false);//简单同步on_open事件和输入线程用的

	try {
		// Set logging settings
		echo_server.set_access_channels(log::alevel::all);
		//echo_server.clear_access_channels(log::alevel::frame_payload);

		// Initialize Asio
		echo_server.init_asio();

		// 注册opcode处理函数
		echo_server.set_open_handler([&](connection_hdl hdl)
		{
			conn_hdl = hdl;
			cerr << "[tbweblog]on_open()" << endl;
			cerr.flush();

			{
				stringstream ss;

				ss << typeid(connection_hdl).name() << endl
					<< typeid(weblog_server).name() << endl;

				echo_server.send(conn_hdl, ss.str(), wspp::frame::opcode::TEXT);
			}

			input_enable = true;
		});
		echo_server.set_close_handler([&](connection_hdl hdl)
		{
			conn_hdl.reset();
			cerr << "[tbweblog]on_close()" << endl;
			cerr.flush();
		});
		echo_server.set_fail_handler([](connection_hdl hdl)
		{
			cerr << "[tbweblog]on_fail()" << endl;
			cerr.flush();
		});

		echo_server.set_ping_handler([](connection_hdl hdl, std::string mstr)
		{
			cerr << "[tbweblog]on_ping()" << endl;
			cerr.flush();
			return true;
		});
		echo_server.set_pong_handler([](connection_hdl hdl, std::string mstr)
		{
			cerr << "[tbweblog]on_pong()" << endl;
			cerr.flush();
		});
		echo_server.set_pong_timeout_handler([](connection_hdl hdl, std::string mstr)
		{
			cerr << "[tbweblog]on_pong_timeout()" << endl;
			cerr.flush();
		});
		echo_server.set_interrupt_handler([](connection_hdl hdl)
		{
			cerr << "[tbweblog]on_interrupt()" << endl;
			cerr.flush();
		});
		echo_server.set_http_handler([](connection_hdl hdl)
		{
			cerr << "[tbweblog]on_http()" << endl;
			cerr.flush();
		});
		echo_server.set_validate_handler([](connection_hdl hdl)
		{
			cerr << "[tbweblog]on_validate()" << endl;
			cerr.flush();
			return true;
		});
		echo_server.set_message_handler(bind(&on_message, &echo_server, ::_1, ::_2));


		// Listen on port 9002
		echo_server.listen(9002);

		// Start the server accept loop
		echo_server.start_accept();

		std::thread input_work([&]()
		{
			string input;

			while (!input_enable);

			while (std::getline(cin, input))
			{
				if (input == "quit")
				{
					echo_server.stop();
				}
				echo_server.send(conn_hdl, input, wspp::frame::opcode::TEXT);
				cerr << "[CLI SendText]" << input << endl;
			}
		});
		input_work.detach();

		// Start the ASIO io_service run loop
		echo_server.run();
		cerr << "[exit after run()]" << endl;


	}
	catch (websocketpp::exception const & e) {
		std::cerr << e.what() << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << endl;
	}
	catch (...) {
		//未知的异常类型捕获
		std::cerr << "other exception" << std::endl;
	}
}
