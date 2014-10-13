

#if ETH_JSONRPC && 1

#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libdevcore/CommonIO.h>
#include <libwebthree/WebThree.h>
#include <eth/EthStubServer.h>
#include <jsonrpc/connectors/httpserver.h>
#include "JsonSpiritHeaders.h"
#include "ethstubclient.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace js = json_spirit;




namespace jsonrpc_tests {

auto_ptr<EthStubServer> jsonrpcServer;


struct JsonrpcFixture  {
    JsonrpcFixture()
    {
        cnote << "setup jsonrpc";
        string name = "Ethereum(++) tests";
        string dbPath;
        dev::WebThreeDirect web3(name, dbPath);
        web3.setIdealPeerCount(5);
        jsonrpcServer = auto_ptr<EthStubServer>(new EthStubServer(new jsonrpc::HttpServer(8080), web3));
    }
    ~JsonrpcFixture()
    {
        cnote << "teardown jsonrpc";
    }
};

//BOOST_AUTO_TEST_CASE(jsonrpc_test)
//{
//    cnote << "testing jsonrpc";
//    js::mValue v;
//    string s = asString(contents("../../jsonrpc.json"));
//    BOOST_REQUIRE_MESSAGE(s.length() > 0, "Content from 'jsonrpc.json' is empty. Have you cloned the 'tests' repo branch develop?");
//    js::read_string(s, v);
//}

BOOST_GLOBAL_FIXTURE(JsonrpcFixture)

BOOST_AUTO_TEST_CASE( test_case1 )
{
//    BOOST_CHECK( i == 1 );
}

BOOST_AUTO_TEST_CASE( test_case2 )
{
//    BOOST_CHECK_EQUAL( i, 0 );
}

}

#endif



