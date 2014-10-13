

//#if ETH_JSONRPC && 1

#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonJS.h>
#include <libwebthree/WebThree.h>
#include <libethrpc/EthStubServer.h>
#include <jsonrpc/connectors/httpserver.h>
#include <jsonrpc/connectors/httpclient.h>
#include "JsonSpiritHeaders.h"
#include "TestHelper.h"
#include "ethstubclient.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace js = json_spirit;


namespace jsonrpc_tests {

string name = "Ethereum(++) tests";
string dbPath;
dev::WebThreeDirect web3(name, dbPath, true);

std::vector<dev::KeyPair> keys = {KeyPair::create()};

auto_ptr<EthStubServer> jsonrpcServer;
auto_ptr<EthStubClient> jsonrpcClient;


struct JsonrpcFixture  {
    JsonrpcFixture()
    {
        cnote << "setup jsonrpc";

        web3.setIdealPeerCount(5);
        web3.ethereum()->setForceMining(true);
        jsonrpcServer = auto_ptr<EthStubServer>(new EthStubServer(new jsonrpc::HttpServer(8080), web3));
        jsonrpcServer->setKeys(keys);
        jsonrpcServer->StartListening();
        
        jsonrpcClient = auto_ptr<EthStubClient>(new EthStubClient(new jsonrpc::HttpClient("http://localhost:8080")));
    }
    ~JsonrpcFixture()
    {
        cnote << "teardown jsonrpc";
    }
};

BOOST_GLOBAL_FIXTURE(JsonrpcFixture)

BOOST_AUTO_TEST_CASE(jsonrpc_balanceAt)
{
    cnote << "Testing jsonrpc balanceAt...";
    dev::KeyPair pair = keys[0];
//    string balance = jsonrpcClient->balanceAt(toJS(pair.address()));

}

BOOST_AUTO_TEST_CASE(jsonrpc_block)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_call)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_coinbase)
{
    cnote << "Testing jsonrpc coinbase...";
    string coinbase = jsonrpcClient->coinbase();
    BOOST_CHECK_EQUAL(jsToAddress(coinbase), web3.ethereum()->address());
}

BOOST_AUTO_TEST_CASE(jsonrpc_countAt)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_defaultBlock)
{
    cnote << "Testing jsonrpc defaultBlock...";
    int defaultBlock = jsonrpcClient->defaultBlock();
    BOOST_CHECK_EQUAL(defaultBlock, web3.ethereum()->getDefault());
}
    
BOOST_AUTO_TEST_CASE(jsonrpc_fromAscii)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_fromFixed)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_gasPrice)
{
    cnote << "Testing jsonrpc gasPrice...";
    string gasPrice = jsonrpcClient->gasPrice();
    BOOST_CHECK_EQUAL(gasPrice, toJS(10 * dev::eth::szabo));
}

BOOST_AUTO_TEST_CASE(jsonrpc_isListening)
{
    //TODO
    cnote << "Testing jsonrpc isListening...";
}

BOOST_AUTO_TEST_CASE(jsonrpc_isMining)
{
    cnote << "Testing jsonrpc isMining...";

    web3.ethereum()->startMining();
    bool miningOn = jsonrpcClient->isMining();
    BOOST_CHECK_EQUAL(miningOn, web3.ethereum()->isMining());

    web3.ethereum()->stopMining();
    bool miningOff = jsonrpcClient->isMining();
    BOOST_CHECK_EQUAL(miningOff, web3.ethereum()->isMining());
}

BOOST_AUTO_TEST_CASE(jsonrpc_key)
{
    cnote << "Testing jsonrpc key...";
    string key = jsonrpcClient->key();
    BOOST_CHECK_EQUAL(jsToSecret(key), keys[0].secret());
}
    
BOOST_AUTO_TEST_CASE(jsonrpc_keys)
{
    cnote << "Testing jsonrpc keys...";
    Json::Value k = jsonrpcClient->keys();
    BOOST_CHECK_EQUAL(k.isArray(), true);
    BOOST_CHECK_EQUAL(k.size(),  keys.size());
    for (unsigned i = 0; i < k.size(); i++)
        BOOST_CHECK_EQUAL(jsToSecret(k[i].asString()) , keys[i].secret());
}

BOOST_AUTO_TEST_CASE(jsonrpc_lll)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_messages)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_number)
{
    cnote << "Testing jsonrpc number...";
    int number = jsonrpcClient->number();
    BOOST_CHECK_EQUAL(number, web3.ethereum()->number() + 1);
}

BOOST_AUTO_TEST_CASE(jsonrpc_number2)
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
    //TODO
}

BOOST_AUTO_TEST_CASE(jsonrpc_secretToAddress)
{
    cnote << "Testing jsonrpc secretToAddress...";
    dev::KeyPair pair = dev::KeyPair::create();
    string address = jsonrpcClient->secretToAddress(toJS(pair.secret()));
    BOOST_CHECK_EQUAL(jsToAddress(address), pair.address());
}

BOOST_AUTO_TEST_CASE(jsonrpc_setListening)
{
    cnote << "Testing jsonrpc setListening...";
    //TODO
}

BOOST_AUTO_TEST_CASE(jsonrpc_setMining)
{
    cnote << "Testing jsonrpc setMining...";

    jsonrpcClient->setMining(true);
    BOOST_CHECK_EQUAL(web3.ethereum()->isMining(), true);

    jsonrpcClient->setMining(false);
    BOOST_CHECK_EQUAL(web3.ethereum()->isMining(), false);
}

BOOST_AUTO_TEST_CASE(jsonrpc_sha3)
{
    cnote << "Testing jsonrpc sha3...";
    string testString = "1234567890987654";
    string sha3 = jsonrpcClient->sha3(testString);
    BOOST_CHECK_EQUAL(jsToFixed<32>(sha3), dev::eth::sha3(jsToBytes(testString)));
}

BOOST_AUTO_TEST_CASE(jsonrpc_stateAt)
{
    
}

BOOST_AUTO_TEST_CASE(jsonrpc_toAscii)
{
    cnote << "Testing jsonrpc toAscii...";
    string testString = "1234567890987654";
    string ascii = jsonrpcClient->toAscii(testString);
    BOOST_CHECK_EQUAL(jsToBinary(testString), ascii);
    BOOST_CHECK_EQUAL(testString, jsFromBinary(ascii));     // failing!
}

BOOST_AUTO_TEST_CASE(jsonrpc_toDecimal)
{
    cnote << "Testing jsonrpc toDecimal...";
    string testString = "1234567890987654";
    string decimal = jsonrpcClient->toDecimal(testString);
    BOOST_CHECK_EQUAL(jsToDecimal(testString), decimal);
}

BOOST_AUTO_TEST_CASE(jsonrpc_toFixed)
{
    cnote << "Testing jsonrpc toFixed...";
    double testValue = 123567;
    string fixed = jsonrpcClient->toFixed(testValue);
    BOOST_CHECK_EQUAL(jsToFixed(testValue), fixed);
    BOOST_CHECK_EQUAL(testValue, jsFromFixed(fixed));
}

BOOST_AUTO_TEST_CASE(jsonrpc_transact)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_transaction)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_uncle)
{
}

BOOST_AUTO_TEST_CASE(jsonrpc_watch)
{

}




}

//#endif



