/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Foobar is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file peer.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Peer Network test functions.
 */

#include <PeerNetwork.h>
using namespace std;
using namespace eth;
using boost::asio::ip::tcp;

int peerTest(int argc, char** argv)
{
	int port = 30303;
	PeerServer s(0, port);
	s.run();
  /*
	if (argc == 1)
	{
		boost::asio::io_service io_service;
		tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), port));
		tcp::socket socket_(io_service);
		function<void()> do_accept;
		do_accept = [&]()
		{
			acceptor_.async_accept(socket_, [&](boost::system::error_code ec)
			{
				if (!ec)
				{
					auto s = move(socket_);
					enum { max_length = 1024 };
					char data_[max_length];

					function<void()> do_read;
					do_read = [&]()
					{
						s.async_read_some(boost::asio::buffer(data_, max_length), [&](boost::system::error_code ec, std::size_t length)
						{
							if (!ec)
								boost::asio::async_write(s, boost::asio::buffer(data_, length), [&](boost::system::error_code ec, std::size_t)
								{
									if (!ec)
										do_read();
								});
						});
					};
				}
				do_accept();
			});
		};
		io_service.run();
	}
	else
	{

	}*/



/*	if (argc == 1)
	{
		PeerNetwork pn(0, 30303);
		while (true)
		{
			usleep(100000);
			pn.process();
		}
	}
	else
	{
		PeerNetwork pn(0);
		if (pn.connect("127.0.0.1", 30303))
			cout << "CONNECTED" << endl;
		while (true)
		{
			usleep(100000);
			pn.process();
		}
	}*/

	return 0;
}

