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

// @debris disabled as tests fail with:
// unknown location(0): fatal error in "jsonrpc_setMining": std::exception: Exception -32003 : Client connector error: : libcurl error: 28
// /home/gav/Eth/cpp-ethereum/test/jsonrpc.cpp(169): last checkpoint
#if ETH_JSONRPC && 0

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/CommonIO.h>
#include <libethcore/CommonJS.h>
#include <libwebthree/WebThree.h>
#include <libweb3jsonrpc/WebThreeStubServer.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <set>
#include "JsonSpiritHeaders.h"
#include "TestHelper.h"
#include "webthreestubclient.h"

BOOST_AUTO_TEST_SUITE(jsonrpc)

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace js = json_spirit;

WebThreeDirect* web3;
unique_ptr<WebThreeStubServer> jsonrpcServer;
unique_ptr<WebThreeStubClient> jsonrpcClient;

struct Setup
{
	Setup()
	{
		static bool setup = false;
		if (setup)
			return;
		setup = true;

		dev::p2p::NetworkPreferences nprefs(30303, std::string(), false);
		web3 = new WebThreeDirect("Ethereum(++) tests", "", true, {"eth", "shh"}, nprefs);
		
		web3->setIdealPeerCount(5);
		web3->ethereum()->setForceMining(true);
		auto server = new jsonrpc::HttpServer(8080);
		jsonrpcServer = unique_ptr<WebThreeStubServer>(new WebThreeStubServer(*server, *web3, {}));
		jsonrpcServer->setIdentities({});
		jsonrpcServer->StartListening();
		auto client = new jsonrpc::HttpClient("http://localhost:8080");
		jsonrpcClient = unique_ptr<WebThreeStubClient>(new WebThreeStubClient(*client));
	}
};

string fromAscii(string _s)
{
	bytes b = asBytes(_s);
	return "0x" + toHex(b);
}

BOOST_FIXTURE_TEST_SUITE(environment, Setup)

BOOST_AUTO_TEST_CASE(jsonrpc_defaultBlock)
{
	cnote << "Testing jsonrpc defaultBlock...";
	int defaultBlock = jsonrpcClient->eth_defaultBlock();
	BOOST_CHECK_EQUAL(defaultBlock, web3->ethereum()->getDefault());
}

BOOST_AUTO_TEST_CASE(jsonrpc_gasPrice)
{
	cnote << "Testing jsonrpc gasPrice...";
	string gasPrice = jsonrpcClient->eth_gasPrice();
	BOOST_CHECK_EQUAL(gasPrice, toJS(10 * dev::eth::szabo));
}

BOOST_AUTO_TEST_CASE(jsonrpc_isListening)
{
	cnote << "Testing jsonrpc isListening...";

	web3->startNetwork();
	bool listeningOn = jsonrpcClient->eth_listening();
	BOOST_CHECK_EQUAL(listeningOn, web3->isNetworkStarted());
	
	web3->stopNetwork();
	bool listeningOff = jsonrpcClient->eth_listening();
	BOOST_CHECK_EQUAL(listeningOff, web3->isNetworkStarted());
}

BOOST_AUTO_TEST_CASE(jsonrpc_isMining)
{
	cnote << "Testing jsonrpc isMining...";

	web3->ethereum()->startMining();
	bool miningOn = jsonrpcClient->eth_mining();
	BOOST_CHECK_EQUAL(miningOn, web3->ethereum()->isMining());

	web3->ethereum()->stopMining();
	bool miningOff = jsonrpcClient->eth_mining();
	BOOST_CHECK_EQUAL(miningOff, web3->ethereum()->isMining());
}

BOOST_AUTO_TEST_CASE(jsonrpc_accounts)
{
	cnote << "Testing jsonrpc accounts...";
	std::vector <dev::KeyPair> keys = {KeyPair::create(), KeyPair::create()};
	jsonrpcServer->setAccounts(keys);
	Json::Value k = jsonrpcClient->eth_accounts();
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
	int number = jsonrpcClient->eth_number();
	BOOST_CHECK_EQUAL(number, web3->ethereum()->number() + 1);
	dev::eth::mine(*(web3->ethereum()), 1);
	int numberAfter = jsonrpcClient->eth_number();
	BOOST_CHECK_EQUAL(number + 1, numberAfter);
	BOOST_CHECK_EQUAL(numberAfter, web3->ethereum()->number() + 1);
}

BOOST_AUTO_TEST_CASE(jsonrpc_peerCount)
{
	cnote << "Testing jsonrpc peerCount...";
	int peerCount = jsonrpcClient->eth_peerCount();
	BOOST_CHECK_EQUAL(web3->peerCount(), peerCount);
}

BOOST_AUTO_TEST_CASE(jsonrpc_setListening)
{
	cnote << "Testing jsonrpc setListening...";

	jsonrpcClient->eth_setListening(true);
	BOOST_CHECK_EQUAL(web3->isNetworkStarted(), true);
	
	jsonrpcClient->eth_setListening(false);
	BOOST_CHECK_EQUAL(web3->isNetworkStarted(), false);
}

BOOST_AUTO_TEST_CASE(jsonrpc_setMining)
{
	cnote << "Testing jsonrpc setMining...";

	jsonrpcClient->eth_setMining(true);
	BOOST_CHECK_EQUAL(web3->ethereum()->isMining(), true);

	jsonrpcClient->eth_setMining(false);
	BOOST_CHECK_EQUAL(web3->ethereum()->isMining(), false);
}

BOOST_AUTO_TEST_CASE(jsonrpc_stateAt)
{
	cnote << "Testing jsonrpc stateAt...";
	dev::KeyPair key = KeyPair::create();
	auto address = key.address();
	string stateAt = jsonrpcClient->eth_stateAt(toJS(address), "0");
	BOOST_CHECK_EQUAL(toJS(web3->ethereum()->stateAt(address, jsToU256("0"), 0)), stateAt);
}

BOOST_AUTO_TEST_CASE(jsonrpc_transact)
{
	cnote << "Testing jsonrpc transact...";
	string coinbase = jsonrpcClient->eth_coinbase();
	BOOST_CHECK_EQUAL(jsToAddress(coinbase), web3->ethereum()->address());
	
	dev::KeyPair key = KeyPair::create();
	auto address = key.address();
	auto receiver = KeyPair::create();
	web3->ethereum()->setAddress(address);

	coinbase = jsonrpcClient->eth_coinbase();
	BOOST_CHECK_EQUAL(jsToAddress(coinbase), web3->ethereum()->address());
	BOOST_CHECK_EQUAL(jsToAddress(coinbase), address);
	
	jsonrpcServer->setAccounts({key});
	auto balance = web3->ethereum()->balanceAt(address, 0);
	string balanceString = jsonrpcClient->eth_balanceAt(toJS(address));
	double countAt = jsonrpcClient->eth_countAt(toJS(address));
	
	BOOST_CHECK_EQUAL(countAt, (double)(uint64_t)web3->ethereum()->countAt(address));
	BOOST_CHECK_EQUAL(countAt, 0);
	BOOST_CHECK_EQUAL(toJS(balance), balanceString);
	BOOST_CHECK_EQUAL(jsToDecimal(balanceString), "0");
	
	dev::eth::mine(*(web3->ethereum()), 1);
	balance = web3->ethereum()->balanceAt(address, 0);
	balanceString = jsonrpcClient->eth_balanceAt(toJS(address));
	
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
	
	jsonrpcClient->eth_transact(t);
	jsonrpcServer->setAccounts({});
	dev::eth::mine(*(web3->ethereum()), 1);
	
	countAt = jsonrpcClient->eth_countAt(toJS(address));
	auto balance2 = web3->ethereum()->balanceAt(receiver.address());
	string balanceString2 = jsonrpcClient->eth_balanceAt(toJS(receiver.address()));
	
	BOOST_CHECK_EQUAL(countAt, (double)(uint64_t)web3->ethereum()->countAt(address));
	BOOST_CHECK_EQUAL(countAt, 1);
	BOOST_CHECK_EQUAL(toJS(balance2), balanceString2);
	BOOST_CHECK_EQUAL(jsToDecimal(balanceString2), "750000000000000000");
	BOOST_CHECK_EQUAL(txAmount, balance2);
}


BOOST_AUTO_TEST_CASE(simple_contract)
{
	cnote << "Testing jsonrpc contract...";
	KeyPair kp = KeyPair::create();
	web3->ethereum()->setAddress(kp.address());
	jsonrpcServer->setAccounts({kp});

	dev::eth::mine(*(web3->ethereum()), 1);
	
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	string compiled = jsonrpcClient->eth_solidity(sourceCode);

	Json::Value create;
	create["code"] = compiled;
	string contractAddress = jsonrpcClient->eth_transact(create);
	dev::eth::mine(*(web3->ethereum()), 1);
	
	Json::Value call;
	call["to"] = contractAddress;
	call["data"] = "0x00000000000000000000000000000000000000000000000000000000000000001";
	string result = jsonrpcClient->eth_call(call);
	BOOST_CHECK_EQUAL(result, "0x0000000000000000000000000000000000000000000000000000000000000007");
}

BOOST_AUTO_TEST_CASE(contract_storage)
{
	cnote << "Testing jsonrpc contract storage...";
	KeyPair kp = KeyPair::create();
	web3->ethereum()->setAddress(kp.address());
	jsonrpcServer->setAccounts({kp});

	dev::eth::mine(*(web3->ethereum()), 1);
	
	char const* sourceCode = R"(
		contract test {
			uint hello;
			function writeHello(uint value)  returns(bool d){
				hello = value;
				return true;
			}
		}
	)";
	
	string compiled = jsonrpcClient->eth_solidity(sourceCode);
	
	Json::Value create;
	create["code"] = compiled;
	string contractAddress = jsonrpcClient->eth_transact(create);
	dev::eth::mine(*(web3->ethereum()), 1);
	
	Json::Value transact;
	transact["to"] = contractAddress;
	transact["data"] = "0x00000000000000000000000000000000000000000000000000000000000000003";
	jsonrpcClient->eth_transact(transact);
	dev::eth::mine(*(web3->ethereum()), 1);
	
	Json::Value storage = jsonrpcClient->eth_storageAt(contractAddress);
	BOOST_CHECK_EQUAL(storage.getMemberNames().size(), 1);
	// bracers are required, cause msvc couldnt handle this macro in for statement
	for (auto name: storage.getMemberNames())
	{
		BOOST_CHECK_EQUAL(storage[name].asString(), "0x03");
	}
}

BOOST_AUTO_TEST_CASE(sha3)
{
	cnote << "Testing jsonrpc sha3...";
	string testString = "multiply(uint256)";
	h256 expected = dev::sha3(testString);

	auto hexValue = fromAscii(testString);
	string result = jsonrpcClient->web3_sha3(hexValue);
	BOOST_CHECK_EQUAL(toJS(expected), result);
	BOOST_CHECK_EQUAL("0xc6888fa159d67f77c2f3d7a402e199802766bd7e8d4d1ecd2274fc920265d56a", result);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#endif
