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
/** @file state.cpp
 * @author Christoph Jentzsch <cj@ethdev.com>
 * @date 2014
 * State test functions.
 */

#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>
#include "JsonSpiritHeaders.h"
#include <libdevcore/CommonIO.h>
#include <libethereum/BlockChain.h>
#include <libethereum/State.h>
#include <libethereum/ExtVM.h>
#include <libethereum/Defaults.h>
#include <libevm/VM.h>
#include "TestHelper.h"

using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;
using namespace dev::eth::test;

class FakeState: public State
{

};


namespace dev { namespace eth{ namespace test {



void doStateTests(json_spirit::mValue& v, bool _fillin)
{
	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("env") > 0);
		BOOST_REQUIRE(o.count("pre") > 0);
		BOOST_REQUIRE(o.count("exec") > 0);

		ImportTest importer(o,false);

		ExtVM evm(importer.m_statePre, importer.m_environment.myAddress,
				  importer.m_environment.caller, importer.m_environment.origin,
				  importer.m_environment.value, importer.m_environment.gasPrice,
				  importer.m_environment.data, importer.m_environment.code,
				  importer.getManifest());

		bytes output;
		VM vm(importer.gasExec);
		try
		{
			output = vm.go(evm, Executive::simpleTrace()).toVector();
		}
		catch (Exception const& _e)
		{
			cnote << "VM did throw an exception: " << diagnostic_information(_e);
			//BOOST_ERROR("Failed VM Test with Exception: " << e.what());
		}
		catch (std::exception const& _e)
		{
			cnote << "VM did throw an exception: " << _e.what();
			//BOOST_ERROR("Failed VM Test with Exception: " << e.what());
		}

		BOOST_REQUIRE(o.count("post") > 0);
		BOOST_REQUIRE(o.count("callcreates") > 0);
		BOOST_REQUIRE(o.count("out") > 0);
		BOOST_REQUIRE(o.count("gas") > 0);


		// check output


		//dev::test::FakeExtVM test;
		//test.importState(o["post"].get_obj());
		//test.importCallCreates(o["callcreates"].get_array());

		int j = 0;
		if (o["out"].type() == array_type)
			for (auto const& d: o["out"].get_array())
			{
				BOOST_CHECK_MESSAGE(output[j] == toInt(d), "Output byte [" << j << "] different!");
				++j;
			}
		else if (o["out"].get_str().find("0x") == 0)
			BOOST_CHECK(output == fromHex(o["out"].get_str().substr(2)));
		else
			BOOST_CHECK(output == fromHex(o["out"].get_str()));

		cout << "gas check: " << importer.gas << " " << toInt(o["gas"]) << " " << vm.gas() << endl;
		BOOST_CHECK_EQUAL(toInt(o["gas"]), vm.gas());

//		auto& expectedAddrs = test.addresses;
//		auto& resultAddrs = fev.addresses;
//		for (auto&& expectedPair : expectedAddrs)
//		{
//			auto& expectedAddr = expectedPair.first;
//			auto resultAddrIt = resultAddrs.find(expectedAddr);
//			if (resultAddrIt == resultAddrs.end())
//				BOOST_ERROR("Missing expected address " << expectedAddr);
//			else
//			{
//				auto& expectedState = expectedPair.second;
//				auto& resultState = resultAddrIt->second;
//				BOOST_CHECK_MESSAGE(std::get<0>(expectedState) == std::get<0>(resultState), expectedAddr << ": incorrect balance " << std::get<0>(resultState) << ", expected " << std::get<0>(expectedState));
//				BOOST_CHECK_MESSAGE(std::get<1>(expectedState) == std::get<1>(resultState), expectedAddr << ": incorrect txCount " << std::get<1>(resultState) << ", expected " << std::get<1>(expectedState));
//				BOOST_CHECK_MESSAGE(std::get<3>(expectedState) == std::get<3>(resultState), expectedAddr << ": incorrect code");

//				auto&& expectedStore = std::get<2>(expectedState);
//				auto&& resultStore = std::get<2>(resultState);

//				for (auto&& expectedStorePair : expectedStore)
//				{
//					auto& expectedStoreKey = expectedStorePair.first;
//					auto resultStoreIt = resultStore.find(expectedStoreKey);
//					if (resultStoreIt == resultStore.end())
//						BOOST_ERROR(expectedAddr << ": missing store key " << expectedStoreKey);
//					else
//					{
//						auto& expectedStoreValue = expectedStorePair.second;
//						auto& resultStoreValue = resultStoreIt->second;
//						BOOST_CHECK_MESSAGE(expectedStoreValue == resultStoreValue, expectedAddr << ": store[" << expectedStoreKey << "] = " << resultStoreValue << ", expected " << expectedStoreValue);
//					}
//				}
//			}
//		}

		BOOST_CHECK(evm.state().addresses() == importer.m_statePost.addresses());	// Just to make sure nothing missed
		//BOOST_CHECK(evm.callcreates == importer.callcreates);

	}
}


void executeStateTests(const string& _name)
{
	const char* ptestPath = getenv("ETHEREUM_TEST_PATH");
	string testPath;

	if (ptestPath == NULL)
	{
		cnote << " could not find environment variable ETHEREUM_TEST_PATH \n";
		testPath = "../../../tests";
	}
	else
		testPath = ptestPath;

	testPath += "/vmtests";

#ifdef FILL_TESTS
	try
	{
		cnote << "Populating VM tests...";
		json_spirit::mValue v;
		boost::filesystem::path p(__FILE__);
		boost::filesystem::path dir = p.parent_path();
		string s = asString(dev::contents(dir.string() + "/" + _name + "Filler.json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + _name + "Filler.json is empty.");
		json_spirit::read_string(s, v);
		dev::test::doStateTests(v, true);
		writeFile(testPath + "/" + _name + ".json", asBytes(json_spirit::write_string(v, true)));
	}
	catch (Exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << diagnostic_information(_e));
	}
	catch (std::exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
	}
#endif

	try
	{
		cnote << "Testing VM..." << _name;
		json_spirit::mValue v;
		string s = asString(dev::contents(testPath + "/" + _name + ".json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + testPath + "/" + _name + ".json is empty. Have you cloned the 'tests' repo branch develop and set ETHEREUM_TEST_PATH to its path?");
		json_spirit::read_string(s, v);
		doStateTests(v, false);
	}
	catch (Exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << diagnostic_information(_e));
	}
	catch (std::exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
	}

}
} } }// Namespace Close


BOOST_AUTO_TEST_CASE(vmSystemOperationsTest)
{
	std::cout << "Doing systemoperationsTest in state\n";
	int currentVerbosity = g_logVerbosity;
	g_logVerbosity = 12;
	dev::eth::test::executeStateTests("vmSystemOperationsTest");
	g_logVerbosity = currentVerbosity;
}

//BOOST_AUTO_TEST_CASE(tmp)
//{
//	std::cout << "Doing systemoperationsTest in state\n";
//	int currentVerbosity = g_logVerbosity;
//	g_logVerbosity = 12;
//	dev::eth::test::executeStateTests("tmp");
//	g_logVerbosity = currentVerbosity;
//}

