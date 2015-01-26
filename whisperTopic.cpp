/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file whisperTopic.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */
#include <functional>
#include <boost/test/unit_test.hpp>
#include <libp2p/Host.h>
#include <libwhisper/WhisperPeer.h>
#include <libwhisper/WhisperHost.h>
using namespace std;
using namespace dev;
using namespace dev::p2p;
using namespace dev::shh;

BOOST_AUTO_TEST_SUITE(whisper)

BOOST_AUTO_TEST_CASE(topic)
{
	cnote << "Testing Whisper...";
	auto oldLogVerbosity = g_logVerbosity;
	g_logVerbosity = 0;

	Host phOther("Test", NetworkPreferences(30303, "127.0.0.1", false, true));
	auto whOther = phOther.registerCapability(new WhisperHost());
	phOther.start();
	
	Host ph("Test", NetworkPreferences(30300, "127.0.0.1", false, true));
	auto wh = ph.registerCapability(new WhisperHost());
	ph.start();
	
	this_thread::sleep_for(chrono::milliseconds(100));
	ph.addNode(phOther.id(), "127.0.0.1", 30303, 30303);
	
	this_thread::sleep_for(chrono::milliseconds(500));
	
	bool started = false;
	unsigned result = 0;
	std::thread listener([&]()
	{
		setThreadName("other");
		started = true;

		/// Only interested in odd packets
		auto w = whOther->installWatch(BuildTopicMask("odd"));

		for (int i = 0, last = 0; i < 200 && last < 81; ++i)
		{
			for (auto i: whOther->checkWatch(w))
			{
				Message msg = whOther->envelope(i).open();
				last = RLP(msg.payload()).toInt<unsigned>();
				cnote << "New message from:" << msg.from().abridged() << RLP(msg.payload()).toInt<unsigned>();
				result += last;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
		
	});

	while (!started)
		this_thread::sleep_for(chrono::milliseconds(1));
	
	KeyPair us = KeyPair::create();
	for (int i = 0; i < 10; ++i)
	{
		wh->post(us.sec(), RLPStream().append(i * i).out(), BuildTopic(i)(i % 2 ? "odd" : "even"));
		this_thread::sleep_for(chrono::milliseconds(250));
	}

	listener.join();
	g_logVerbosity = oldLogVerbosity;

	BOOST_REQUIRE_EQUAL(result, 1 + 9 + 25 + 49 + 81);
}

BOOST_AUTO_TEST_SUITE_END()
