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
/** @file checkRandomStateTest.cpp
 * @author Christoph Jentzsch <jentzsch.simulationsoftware@gmail.com>
 * @date 2015
 * Check a random test and return 0/1 for success or failure. To be used for efficiency in the random test simulation.
 */

#include <libdevcore/Common.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/Log.h>
#include <libevm/VMFactory.h>
#include "TestHelper.h"
#include "vm.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace std;
using namespace json_spirit;
using namespace dev::test;
using namespace dev;

bool doStateTest(mValue& _v);

int main(int argc, char *argv[])
{
	g_logVerbosity = 0;
	bool ret = false;

	try
	{
		mValue v;
		string s;
		for (int i = 1; i < argc; ++i)
			s += argv[i];
		if (asserts(s.length() > 0))
		{
			cout << "Content of argument is empty\n";
			return 1;
		}
		read_string(s, v);
		ret = doStateTest(v);
	}
	catch (Exception const& _e)
	{
		cout << "Failed test with Exception: " << diagnostic_information(_e) << endl;
		ret = false;
	}
	catch (std::exception const& _e)
	{
		cout << "Failed test with Exception: " << _e.what() << endl;
		ret = false;
	}
	return ret;
}

bool doStateTest(mValue& _v)
{
	eth::VMFactory::setKind(eth::VMKind::JIT);

	for (auto& i: _v.get_obj())
	{
		mObject& o = i.second.get_obj();

		assert(o.count("env") > 0);
		assert(o.count("pre") > 0);
		assert(o.count("transaction") > 0);

		ImportTest importer(o, false);

		eth::State theState = importer.m_statePre;
		bytes tx = importer.m_transaction.rlp();
		bytes output;

		try
		{
			theState.execute(lastHashes(importer.m_environment.currentBlock.number), tx, &output);
		}
		catch (Exception const& _e)
		{
			cnote << "state execution did throw an exception: " << diagnostic_information(_e);
			theState.commit();
		}
		catch (std::exception const& _e)
		{
			cnote << "state execution did throw an exception: " << _e.what();
		}

		assert(o.count("post") > 0);
		assert(o.count("out") > 0);

		//checkOutput(output, o);
		int j = 0;
		if (o["out"].type() == array_type)
			for (auto const& d: o["out"].get_array())
			{
				if (asserts(output[j] == toInt(d)))
				{
					cout << "Output byte [" << j << "] different!";
					return 1;
				}
				++j;
			}
		else if (o["out"].get_str().find("0x") == 0)
		{
			if (asserts(output == fromHex(o["out"].get_str().substr(2))))
				return 1;
		}
		else
		{
			if (asserts(output == fromHex(o["out"].get_str())))
				return 1;
		}

		//checkLog(theState.pending().size() ? theState.log(0) : LogEntries(), importer.m_environment.sub.logs);
		eth::LogEntries logs = theState.pending().size() ? theState.log(0) : eth::LogEntries();

		if (assertsEqual(logs.size(), importer.m_environment.sub.logs.size()))
			return 1;

		for (size_t i = 0; i < logs.size(); ++i)
		{
			if (assertsEqual(logs[i].address, importer.m_environment.sub.logs[i].address))
				return 1;
			if (assertsEqual(logs[i].topics, importer.m_environment.sub.logs[i].topics))
				return 1;
			if (asserts(logs[i].data == importer.m_environment.sub.logs[i].data))
				return 1;
		}

		// check addresses
#if ETH_FATDB
		auto expectedAddrs = importer.m_statePost.addresses();
		auto resultAddrs = theState.addresses();
		for (auto& expectedPair : expectedAddrs)
		{
			auto& expectedAddr = expectedPair.first;
			auto resultAddrIt = resultAddrs.find(expectedAddr);
			if (resultAddrIt == resultAddrs.end())
			{
				cout << "Missing expected address " << expectedAddr;
				return 1;
			}
			else
			{
				if (importer.m_statePost.balance(expectedAddr) !=  theState.balance(expectedAddr))
				{
					cout << expectedAddr << ": incorrect balance " << theState.balance(expectedAddr) << ", expected " << importer.m_statePost.balance(expectedAddr);
					return 1;
				}
				if (importer.m_statePost.transactionsFrom(expectedAddr) !=  theState.transactionsFrom(expectedAddr))
				{
					cout << expectedAddr << ": incorrect txCount " << theState.transactionsFrom(expectedAddr) << ", expected " << importer.m_statePost.transactionsFrom(expectedAddr);
					return 1;
				}
				if (importer.m_statePost.code(expectedAddr) != theState.code(expectedAddr))
				{
					cout << expectedAddr << ": incorrect code";
					return 1;
				}

				//checkStorage(importer.m_statePost.storage(expectedAddr), theState.storage(expectedAddr), expectedAddr);
				map<u256, u256> _resultStore = theState.storage(expectedAddr);

				for (auto&& expectedStorePair : importer.m_statePost.storage(expectedAddr))
				{
					auto& expectedStoreKey = expectedStorePair.first;
					auto resultStoreIt = _resultStore.find(expectedStoreKey);
					if (resultStoreIt == _resultStore.end())
					{
						cout << expectedAddr << ": missing store key " << expectedStoreKey << endl;
						return 1;
					}
					else
					{
						auto& expectedStoreValue = expectedStorePair.second;
						auto& resultStoreValue = resultStoreIt->second;
						if (asserts(expectedStoreValue == resultStoreValue))
						{
							cout << expectedAddr << ": store[" << expectedStoreKey << "] = " << resultStoreValue << ", expected " << expectedStoreValue << endl;
							return 1;
						}
					}
				}
				if (assertsEqual(_resultStore.size(), importer.m_statePost.storage(expectedAddr).size()))
					return 1;
				for (auto&& resultStorePair: _resultStore)
				{
					if (!importer.m_statePost.storage(expectedAddr).count(resultStorePair.first))
					{
						cout << expectedAddr << ": unexpected store key " << resultStorePair.first << endl;
						return 1;
					}
				}
			}
		}
		//checkAddresses<map<Address, u256> >(expectedAddrs, resultAddrs);
		for (auto& resultPair : resultAddrs)
		{
			auto& resultAddr = resultPair.first;
			auto expectedAddrIt = expectedAddrs.find(resultAddr);
			if (expectedAddrIt == expectedAddrs.end())
				return 1;
		}
		if(expectedAddrs != resultAddrs)
			return 1;
#endif
		if(theState.rootHash() != h256(o["postStateRoot"].get_str()), "wrong post state root")
			return 1;
	}
	return 0;
}
