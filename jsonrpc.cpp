

#if ETH_JSONRPC && 1

#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonJS.h>
#include <libwebthree/WebThree.h>
#include <libethrpc/EthStubServer.h>
#include <jsonrpc/connectors/httpserver.h>
#include <jsonrpc/connectors/httpclient.h>
#include "JsonSpiritHeaders.h"
#include "ethstubclient.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace js = json_spirit;


namespace jsonrpc_tests {

KeyPair us;
auto_ptr<EthStubServer> jsonrpcServer;
auto_ptr<EthStubClient> jsonrpcClient;


struct JsonrpcFixture  {
    JsonrpcFixture()
    {
        cnote << "setup jsonrpc";
        string name = "Ethereum(++) tests";
        string dbPath;
        dev::WebThreeDirect web3(name, dbPath);
        web3.setIdealPeerCount(5);
        
        us = KeyPair::create();
        jsonrpcServer = auto_ptr<EthStubServer>(new EthStubServer(new jsonrpc::HttpServer(8080), web3));
        jsonrpcServer->setKeys({us});
        jsonrpcServer->StartListening();
        
        jsonrpcClient = auto_ptr<EthStubClient>(new EthStubClient(new jsonrpc::HttpClient("http://localhost:8080")));
    }
    ~JsonrpcFixture()
    {
        cnote << "teardown jsonrpc";
    }
};

BOOST_GLOBAL_FIXTURE(JsonrpcFixture)

BOOST_AUTO_TEST_CASE(jsonrpc_key)
{
    cnote << "Testing jsonrpc key...";
    Json::Value key = jsonrpcClient->key();
    BOOST_CHECK_EQUAL(key.isString(), true);
    BOOST_CHECK_EQUAL(jsToSecret(key.asString()), us.secret());
}
    
BOOST_AUTO_TEST_CASE(jsonrpc_keys)
{
    cnote << "Testing jsonrpc keys...";
    Json::Value keys = jsonrpcClient->keys();
    BOOST_CHECK_EQUAL(keys.isArray(), true);
    BOOST_CHECK_EQUAL(keys.size(),  1);
    BOOST_CHECK_EQUAL(jsToSecret(keys[0u].asString()) , us.secret());
}


}

#endif



