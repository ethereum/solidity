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

	bool started = false;
	unsigned result = 0;
	std::thread listener([&]()
	{
		setThreadName("other");

		Host ph("Test", NetworkPreferences(50303, "", false, true));
		auto wh = ph.registerCapability(new WhisperHost());
		ph.start();

		/// Only interested in odd packets
		auto w = wh->installWatch(BuildTopicMask("odd"));

		started = true;
		set<unsigned> received;

		for (int iterout = 0, last = 0; iterout < 200 && last < 81; ++iterout)
		{
			for (auto i: wh->checkWatch(w))
			{
				Message msg = wh->envelope(i).open(wh->fullTopic(w));
				last = RLP(msg.payload()).toInt<unsigned>();
				if (received.count(last))
					continue;
				received.insert(last);
				cnote << "New message from:" << msg.from().abridged() << RLP(msg.payload()).toInt<unsigned>();
				result += last;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	});

	while (!started)
		this_thread::sleep_for(chrono::milliseconds(50));

	Host ph("Test", NetworkPreferences(50300, "", false, true));
	shared_ptr<WhisperHost> wh = ph.registerCapability(new WhisperHost());
	this_thread::sleep_for(chrono::milliseconds(500));
	ph.start();
	this_thread::sleep_for(chrono::milliseconds(500));
	ph.connect("127.0.0.1", 50303);

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

BOOST_AUTO_TEST_CASE(forwarding)
{
	cnote << "Testing Whisper forwarding...";
	auto oldLogVerbosity = g_logVerbosity;
	g_logVerbosity = 0;

	unsigned result = 0;
	bool done = false;

	bool startedListener = false;
	std::thread listener([&]()
	{
		setThreadName("listener");

		// Host must be configured not to share peers.
		Host ph("Listner", NetworkPreferences(50303, "", false, true));
		ph.setIdealPeerCount(0);
		auto wh = ph.registerCapability(new WhisperHost());
		ph.start();

		startedListener = true;

		/// Only interested in odd packets
		auto w = wh->installWatch(BuildTopicMask("test"));

		for (int i = 0; i < 200 && !result; ++i)
		{
			for (auto i: wh->checkWatch(w))
			{
				Message msg = wh->envelope(i).open(wh->fullTopic(w));
				unsigned last = RLP(msg.payload()).toInt<unsigned>();
				cnote << "New message from:" << msg.from().abridged() << RLP(msg.payload()).toInt<unsigned>();
				result = last;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	});

	bool startedForwarder = false;
	std::thread forwarder([&]()
	{
		setThreadName("forwarder");

		while (!startedListener)
			this_thread::sleep_for(chrono::milliseconds(50));

		// Host must be configured not to share peers.
		Host ph("Forwarder", NetworkPreferences(50305, "", false, true));
		ph.setIdealPeerCount(0);
		auto wh = ph.registerCapability(new WhisperHost());
		this_thread::sleep_for(chrono::milliseconds(500));
		ph.start();

		this_thread::sleep_for(chrono::milliseconds(500));
		ph.connect("127.0.0.1", 50303);

		startedForwarder = true;

		/// Only interested in odd packets
		auto w = wh->installWatch(BuildTopicMask("test"));

		while (!done)
		{
			for (auto i: wh->checkWatch(w))
			{
				Message msg = wh->envelope(i).open(wh->fullTopic(w));
				cnote << "New message from:" << msg.from().abridged() << RLP(msg.payload()).toInt<unsigned>();
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	});

	while (!startedForwarder)
		this_thread::sleep_for(chrono::milliseconds(50));

	Host ph("Sender", NetworkPreferences(50300, "", false, true));
	ph.setIdealPeerCount(0);
	shared_ptr<WhisperHost> wh = ph.registerCapability(new WhisperHost());
	this_thread::sleep_for(chrono::milliseconds(500));
	ph.start();
	this_thread::sleep_for(chrono::milliseconds(500));
	ph.connect("127.0.0.1", 50305);

	KeyPair us = KeyPair::create();
	wh->post(us.sec(), RLPStream().append(1).out(), BuildTopic("test"));
	this_thread::sleep_for(chrono::milliseconds(250));

	listener.join();
	done = true;
	forwarder.join();
	g_logVerbosity = oldLogVerbosity;

	BOOST_REQUIRE_EQUAL(result, 1);
}

BOOST_AUTO_TEST_CASE(asyncforwarding)
{
	cnote << "Testing Whisper async forwarding...";
	auto oldLogVerbosity = g_logVerbosity;
	g_logVerbosity = 2;

	unsigned result = 0;
	bool done = false;

	bool startedForwarder = false;
	std::thread forwarder([&]()
	{
		setThreadName("forwarder");

		// Host must be configured not to share peers.
		Host ph("Forwarder", NetworkPreferences(50305, "", false, true));
		ph.setIdealPeerCount(0);
		auto wh = ph.registerCapability(new WhisperHost());
		this_thread::sleep_for(chrono::milliseconds(500));
		ph.start();

		this_thread::sleep_for(chrono::milliseconds(500));
		ph.connect("127.0.0.1", 50303);

		startedForwarder = true;

		/// Only interested in odd packets
		auto w = wh->installWatch(BuildTopicMask("test"));

		while (!done)
		{
			for (auto i: wh->checkWatch(w))
			{
				Message msg = wh->envelope(i).open(wh->fullTopic(w));
				cnote << "New message from:" << msg.from().abridged() << RLP(msg.payload()).toInt<unsigned>();
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	});

	while (!startedForwarder)
		this_thread::sleep_for(chrono::milliseconds(50));

	{
		Host ph("Sender", NetworkPreferences(50300, "", false, true));
		ph.setIdealPeerCount(0);
		shared_ptr<WhisperHost> wh = ph.registerCapability(new WhisperHost());
		this_thread::sleep_for(chrono::milliseconds(500));
		ph.start();
		this_thread::sleep_for(chrono::milliseconds(500));
		ph.connect("127.0.0.1", 50305);

		KeyPair us = KeyPair::create();
		wh->post(us.sec(), RLPStream().append(1).out(), BuildTopic("test"));
		this_thread::sleep_for(chrono::milliseconds(250));
	}

	{
		Host ph("Listener", NetworkPreferences(50300, "", false, true));
		ph.setIdealPeerCount(0);
		shared_ptr<WhisperHost> wh = ph.registerCapability(new WhisperHost());
		this_thread::sleep_for(chrono::milliseconds(500));
		ph.start();
		this_thread::sleep_for(chrono::milliseconds(500));
		ph.connect("127.0.0.1", 50305);

		/// Only interested in odd packets
		auto w = wh->installWatch(BuildTopicMask("test"));

		for (int i = 0; i < 200 && !result; ++i)
		{
			for (auto i: wh->checkWatch(w))
			{
				Message msg = wh->envelope(i).open(wh->fullTopic(w));
				unsigned last = RLP(msg.payload()).toInt<unsigned>();
				cnote << "New message from:" << msg.from().abridged() << RLP(msg.payload()).toInt<unsigned>();
				result = last;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}

	done = true;
	forwarder.join();
	g_logVerbosity = oldLogVerbosity;

	BOOST_REQUIRE_EQUAL(result, 1);
}

BOOST_AUTO_TEST_SUITE_END()
