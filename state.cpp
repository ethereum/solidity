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

#define FILL_TESTS

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

		ImportTest importer(o,_fillin);

		if (_fillin)
		{
			importer.code = importer.m_statePre.code(importer.m_environment.myAddress);
			importer.m_environment.code = &importer.code;
		}

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

		if (_fillin)
			importer.exportTest(output, vm.gas(), evm.state());
		else
		{
			BOOST_REQUIRE(o.count("post") > 0);
			//BOOST_REQUIRE(o.count("callcreates") > 0);
			BOOST_REQUIRE(o.count("out") > 0);
			BOOST_REQUIRE(o.count("gas") > 0);


			// check output

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

			BOOST_CHECK_EQUAL(toInt(o["gas"]), vm.gas());

			auto expectedAddrs = importer.m_statePost.addresses();
			auto resultAddrs = evm.state().addresses();
			for (auto& expectedPair : expectedAddrs)
			{
				auto& expectedAddr = expectedPair.first;
				auto resultAddrIt = resultAddrs.find(expectedAddr);
				if (resultAddrIt == resultAddrs.end())
					BOOST_ERROR("Missing expected address " << expectedAddr);
				else
				{
					BOOST_CHECK_MESSAGE(importer.m_statePost.balance(expectedAddr) ==  evm.state().balance(expectedAddr), expectedAddr << ": incorrect balance " << evm.state().balance(expectedAddr) << ", expected " << importer.m_statePost.balance(expectedAddr));
					BOOST_CHECK_MESSAGE(importer.m_statePost.transactionsFrom(expectedAddr) ==  evm.state().transactionsFrom(expectedAddr), expectedAddr << ": incorrect txCount " << evm.state().transactionsFrom(expectedAddr) << ", expected " << importer.m_statePost.transactionsFrom(expectedAddr));
					BOOST_CHECK_MESSAGE(importer.m_statePost.code(expectedAddr) == evm.state().code(expectedAddr), expectedAddr << ": incorrect code");

					auto&& expectedStore = importer.m_statePost.storage(expectedAddr);
					auto&& resultStore = evm.state().storage(expectedAddr);

					for (auto&& expectedStorePair : expectedStore)
					{
						auto& expectedStoreKey = expectedStorePair.first;
						auto resultStoreIt = resultStore.find(expectedStoreKey);
						if (resultStoreIt == resultStore.end())
							BOOST_ERROR(expectedAddr << ": missing store key " << expectedStoreKey);
						else
						{
							auto& expectedStoreValue = expectedStorePair.second;
							auto& resultStoreValue = resultStoreIt->second;
							BOOST_CHECK_MESSAGE(expectedStoreValue == resultStoreValue, expectedAddr << ": store[" << expectedStoreKey << "] = " << resultStoreValue << ", expected " << expectedStoreValue);
						}
					}
				}
			}

			for (auto& resultPair : resultAddrs)
			{
				auto& resultAddr = resultPair.first;
				auto expectedAddrIt = expectedAddrs.find(resultAddr);
				if (expectedAddrIt == expectedAddrs.end())
					BOOST_ERROR("Missing result address " << resultAddr);
			}

			BOOST_CHECK(evm.state().addresses() == importer.m_statePost.addresses());	// Just to make sure nothing missed
			//BOOST_CHECK(evm.callcreates == importer.callcreates);
		}

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

	testPath += "/statetests";

#ifdef FILL_TESTS
	try
	{
		cnote << "Populating VM tests...";
		json_spirit::mValue v;
		boost::filesystem::path p(__FILE__);
		boost::filesystem::path dir = p.parent_path();
		string s = asString(dev::contents(dir.string() + "/" + _name + "Filler.json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + dir.string() + "/" + _name + "Filler.json is empty.");
		json_spirit::read_string(s, v);
		doStateTests(v, true);
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
	dev::eth::test::executeStateTests("stSystemOperationsTest");
}

BOOST_AUTO_TEST_CASE(tmp)
{
	std::cout << "Doing systemoperationsTest in state\n";
	int currentVerbosity = g_logVerbosity;
	g_logVerbosity = 12;
	dev::eth::test::executeStateTests("tmp");
	g_logVerbosity = currentVerbosity;
}

