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
/** @file jsonrpc.cpp
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2014
 */

#if ETH_JSONRPC && 1

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonJS.h>
#include <libwebthree/WebThree.h>
#include <libweb3jsonrpc/WebThreeStubServer.h>
#include <libweb3jsonrpc/CorsHttpServer.h>
#include <jsonrpc/connectors/httpserver.h>
#include <jsonrpc/connectors/httpclient.h>
#include <set>
#include "JsonSpiritHeaders.h"
#include "TestHelper.h"
#include "webthreestubclient.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace js = json_spirit;

namespace jsonrpc_tests
{

string name = "Ethereum(++) tests";
string dbPath;
auto s = set<string>{"eth", "shh"};
dev::p2p::NetworkPreferences np(30303, std::string(), false);
dev::WebThreeDirect web3(name, dbPath, true, s, np);

unique_ptr<WebThreeStubServer> jsonrpcServer;
unique_ptr<WebThreeStubClient> jsonrpcClient;

struct JsonrpcFixture  {
	JsonrpcFixture()
	{
		cnote << "setup jsonrpc";

		web3.setIdealPeerCount(5);
		web3.ethereum()->setForceMining(true);
		jsonrpcServer = unique_ptr<WebThreeStubServer>(new WebThreeStubServer(new jsonrpc::CorsHttpServer(8080), web3, {}));
		jsonrpcServer->setIdentities({});
		jsonrpcServer->StartListening();
		
		jsonrpcClient = unique_ptr<WebThreeStubClient>(new WebThreeStubClient(new jsonrpc::HttpClient("http://localhost:8080")));
	}
	~JsonrpcFixture()
	{
		cnote << "teardown jsonrpc";
	}
};

BOOST_GLOBAL_FIXTURE(JsonrpcFixture)

BOOST_AUTO_TEST_CASE(jsonrpc_defaultBlock)
{
	cnote << "Testing jsonrpc defaultBlock...";
	int defaultBlock = jsonrpcClient->defaultBlock();
	BOOST_CHECK_EQUAL(defaultBlock, web3.ethereum()->getDefault());
}

BOOST_AUTO_TEST_CASE(jsonrpc_gasPrice)
{
	cnote << "Testing jsonrpc gasPrice...";
	string gasPrice = jsonrpcClient->gasPrice();
	BOOST_CHECK_EQUAL(gasPrice, toJS(10 * dev::eth::szabo));
}

BOOST_AUTO_TEST_CASE(jsonrpc_isListening)
{
	cnote << "Testing jsonrpc isListening...";

	web3.startNetwork();
	bool listeningOn = jsonrpcClient->listening();
	BOOST_CHECK_EQUAL(listeningOn, web3.isNetworkStarted());
	
	web3.stopNetwork();
	bool listeningOff = jsonrpcClient->listening();
	BOOST_CHECK_EQUAL(listeningOff, web3.isNetworkStarted());
}

BOOST_AUTO_TEST_CASE(jsonrpc_isMining)
{
	cnote << "Testing jsonrpc isMining...";

	web3.ethereum()->startMining();
	bool miningOn = jsonrpcClient->mining();
	BOOST_CHECK_EQUAL(miningOn, web3.ethereum()->isMining());

	web3.ethereum()->stopMining();
	bool miningOff = jsonrpcClient->mining();
	BOOST_CHECK_EQUAL(miningOff, web3.ethereum()->isMining());
}

BOOST_AUTO_TEST_CASE(jsonrpc_accounts)
{
	cnote << "Testing jsonrpc accounts...";
	std::vector <dev::KeyPair> keys = {KeyPair::create(), KeyPair::create()};
	jsonrpcServer->setAccounts(keys);
	Json::Value k = jsonrpcClient->accounts();
	jsonrpcServer->setAccounts({});
	BOOST_CHECK_EQUAL(k.isArray(), true);
	BOOST_CHECK_EQUAL(k.size(),  keys.size());
	for (auto &i:k)
	{
		auto it = std::find_if(keys.begin(), keys.end(), [i](dev::KeyPair const& keyPair)
		{
			return jsToAddress(i.asString()) == keyPair.address();
		});
		BOOST_CHECK_EQUAL(it != keys.end(), true);
	}
}

BOOST_AUTO_TEST_CASE(jsonrpc_number)
{
	cnote << "Testing jsonrpc number2...";
	int number = jsonrpcClient->number();
	BOOST_CHECK_EQUAL(number, web3.ethereum()->number() + 1);
	dev::eth::mine(*(web3.ethereum()), 1);
	int numberAfter = jsonrpcClient->number();
	BOOST_CHECK_EQUAL(number + 1, numberAfter);
	BOOST_CHECK_EQUAL(numberAfter, web3.ethereum()->number() + 1);
}

BOOST_AUTO_TEST_CASE(jsonrpc_peerCount)
{
	cnote << "Testing jsonrpc peerCount...";
	int peerCount = jsonrpcClient->peerCount();
	BOOST_CHECK_EQUAL(web3.peerCount(), peerCount);
}

BOOST_AUTO_TEST_CASE(jsonrpc_setListening)
{
	cnote << "Testing jsonrpc setListening...";

	jsonrpcClient->setListening(true);
	BOOST_CHECK_EQUAL(web3.isNetworkStarted(), true);
	
	jsonrpcClient->setListening(false);
	BOOST_CHECK_EQUAL(web3.isNetworkStarted(), false);
}

BOOST_AUTO_TEST_CASE(jsonrpc_setMining)
{
	cnote << "Testing jsonrpc setMining...";

	jsonrpcClient->setMining(true);
	BOOST_CHECK_EQUAL(web3.ethereum()->isMining(), true);

	jsonrpcClient->setMining(false);
	BOOST_CHECK_EQUAL(web3.ethereum()->isMining(), false);
}

BOOST_AUTO_TEST_CASE(jsonrpc_stateAt)
{
	cnote << "Testing jsonrpc stateAt...";
	dev::KeyPair key = KeyPair::create();
	auto address = key.address();
	string stateAt = jsonrpcClient->stateAt(toJS(address), "0");
	BOOST_CHECK_EQUAL(toJS(web3.ethereum()->stateAt(address, jsToU256("0"), 0)), stateAt);
}

BOOST_AUTO_TEST_CASE(jsonrpc_transact)
{
	cnote << "Testing jsonrpc transact...";
	string coinbase = jsonrpcClient->coinbase();
	BOOST_CHECK_EQUAL(jsToAddress(coinbase), web3.ethereum()->address());
	
	dev::KeyPair key = KeyPair::create();
	auto address = key.address();
	auto receiver = KeyPair::create();
	web3.ethereum()->setAddress(address);

	coinbase = jsonrpcClient->coinbase();
	BOOST_CHECK_EQUAL(jsToAddress(coinbase), web3.ethereum()->address());
	BOOST_CHECK_EQUAL(jsToAddress(coinbase), address);
	
	jsonrpcServer->setAccounts({key});
	auto balance = web3.ethereum()->balanceAt(address, 0);
	string balanceString = jsonrpcClient->balanceAt(toJS(address));
	double countAt = jsonrpcClient->countAt(toJS(address));
	
	BOOST_CHECK_EQUAL(countAt, (double)(uint64_t)web3.ethereum()->countAt(address));
	BOOST_CHECK_EQUAL(countAt, 0);
	BOOST_CHECK_EQUAL(toJS(balance), balanceString);
	BOOST_CHECK_EQUAL(jsToDecimal(balanceString), "0");
	
	dev::eth::mine(*(web3.ethereum()), 1);
	balance = web3.ethereum()->balanceAt(address, 0);
	balanceString = jsonrpcClient->balanceAt(toJS(address));
	
	BOOST_CHECK_EQUAL(toJS(balance), balanceString);
	BOOST_CHECK_EQUAL(jsToDecimal(balanceString), "1500000000000000000");
	
	auto txAmount = balance / 2u;
	auto gasPrice = 10 * dev::eth::szabo;
	auto gas = dev::eth::c_txGas;
	
	Json::Value t;
	t["from"] = toJS(address);
	t["value"] = jsToDecimal(toJS(txAmount));
	t["to"] = toJS(receiver.address());
	t["data"] = toJS(bytes());
	t["gas"] = toJS(gas);
	t["gasPrice"] = toJS(gasPrice);
	
	jsonrpcClient->transact(t);
	jsonrpcServer->setAccounts({});
	dev::eth::mine(*(web3.ethereum()), 1);
	
	countAt = jsonrpcClient->countAt(toJS(address));
	auto balance2 = web3.ethereum()->balanceAt(receiver.address());
	string balanceString2 = jsonrpcClient->balanceAt(toJS(receiver.address()));
	
	BOOST_CHECK_EQUAL(countAt, (double)(uint64_t)web3.ethereum()->countAt(address));
	BOOST_CHECK_EQUAL(countAt, 1);
	BOOST_CHECK_EQUAL(toJS(balance2), balanceString2);
	BOOST_CHECK_EQUAL(jsToDecimal(balanceString2), "750000000000000000");
	BOOST_CHECK_EQUAL(txAmount, balance2);
}

}

#endif
