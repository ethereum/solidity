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
/** @file checkRandomTest.cpp
 * @author Christoph Jentzsch <jentzsch.simulationsoftware@gmail.com>
 * @date 2015
 * Check a random test and return 0/1 for success or failure. To be used for efficiency in the random test simulation.
 */

#include <libdevcore/Common.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/Log.h>
#include <libevm/VMFactory.h>
#include "vm.h"
#pragma GCC diagnostic ignored "-Wunused-parameter"

using namespace std;
using namespace json_spirit;
using namespace dev::test;
using namespace dev;

bool doVMTest(mValue& v);

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
		ret = doVMTest(v);
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

bool doVMTest(mValue& v)
{
	eth::VMFactory::setKind(eth::VMKind::JIT);

	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		mObject& o = i.second.get_obj();

		assert(o.count("env") > 0);
		assert(o.count("pre") > 0);
		assert(o.count("exec") > 0);

		FakeExtVM fev;
		fev.importEnv(o["env"].get_obj());
		fev.importState(o["pre"].get_obj());

		fev.importExec(o["exec"].get_obj());
		if (fev.code.empty())
		{
			fev.thisTxCode = get<3>(fev.addresses.at(fev.myAddress));
			fev.code = fev.thisTxCode;
		}

		bytes output;
		u256 gas;
		bool vmExceptionOccured = false;
		try
		{
			auto vm = eth::VMFactory::create(fev.gas);
			output = vm->go(fev, fev.simpleTrace()).toBytes();
			gas = vm->gas();
		}
		catch (eth::VMException)
		{
			cnote << "Safe VM Exception";
			vmExceptionOccured = true;
		}
		catch (Exception const& _e)
		{
			cnote << "VM did throw an exception: " << diagnostic_information(_e);
			cnote << "Failed VM Test with Exception: " << _e.what();
			return 1;
		}
		catch (std::exception const& _e)
		{
			cnote << "VM did throw an exception: " << _e.what();
			cnote << "Failed VM Test with Exception: " << _e.what();
			return 1;
		}

		// delete null entries in storage for the sake of comparison
		for (auto  &a: fev.addresses)
		{
			vector<u256> keystoDelete;
			for (auto &s: get<2>(a.second))
			{
				if (s.second == 0)
					keystoDelete.push_back(s.first);
			}
			for (auto const key: keystoDelete )
			{
				get<2>(a.second).erase(key);
			}
		}

		if (o.count("post") > 0)	// No exceptions expected
		{
			if (asserts(!vmExceptionOccured) || asserts(o.count("post") > 0) || asserts(o.count("callcreates") > 0) || asserts(o.count("out") > 0) || asserts(o.count("gas") > 0) || asserts(o.count("logs") > 0))
				return 1;

			dev::test::FakeExtVM test;
			test.importState(o["post"].get_obj());
			test.importCallCreates(o["callcreates"].get_array());
			test.sub.logs = importLog(o["logs"].get_array());

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

			if (asserts(toInt(o["gas"]) == gas))
				return 1;

			auto& expectedAddrs = test.addresses;
			auto& resultAddrs = fev.addresses;
			for (auto&& expectedPair : expectedAddrs)
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
					auto& expectedState = expectedPair.second;
					auto& resultState = resultAddrIt->second;
					if (asserts(std::get<0>(expectedState) == std::get<0>(resultState)))
					{
						cout << expectedAddr << ": incorrect balance " << std::get<0>(resultState) << ", expected " << std::get<0>(expectedState);
						return 1;
					}
					if (asserts(std::get<1>(expectedState) == std::get<1>(resultState)))
					{
						cout << expectedAddr << ": incorrect txCount " << std::get<1>(resultState) << ", expected " << std::get<1>(expectedState);
						return 1;
					}
					if (asserts(std::get<3>(expectedState) == std::get<3>(resultState)))
					{
						cout << expectedAddr << ": incorrect code";
						return 1;
					}

					//checkStorage(std::get<2>(expectedState), std::get<2>(resultState), expectedAddr);
					for (auto&& expectedStorePair : std::get<2>(expectedState))
					{
						auto& expectedStoreKey = expectedStorePair.first;
						auto resultStoreIt = std::get<2>(resultState).find(expectedStoreKey);
						if (resultStoreIt == std::get<2>(resultState).end())
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
					if (assertsEqual(std::get<2>(resultState).size(), std::get<2>(expectedState).size()))
						return 1;
					for (auto&& resultStorePair: std::get<2>(resultState))
					{
						if (!std::get<2>(expectedState).count(resultStorePair.first))
						{
							cout << expectedAddr << ": unexpected store key " << resultStorePair.first << endl;
							return 1;
						}
					}
				}
			}

			//checkAddresses<std::map<Address, std::tuple<u256, u256, std::map<u256, u256>, bytes> > >(test.addresses, fev.addresses);
			for (auto& resultPair : fev.addresses)
			{
				auto& resultAddr = resultPair.first;
				auto expectedAddrIt = test.addresses.find(resultAddr);
				if (expectedAddrIt == test.addresses.end())
				{
					cout << "Missing result address " << resultAddr << endl;
					return 1;
				}
			}
			if (asserts(test.addresses == fev.addresses))
				return 1;

			if (asserts(test.callcreates == fev.callcreates))
				return 1;

			//checkCallCreates(fev.callcreates, test.callcreates);
			{
				if (assertsEqual(test.callcreates.size(), fev.callcreates.size()))
					return 1;

				for (size_t i = 0; i < test.callcreates.size(); ++i)
				{
					if (asserts(test.callcreates[i].data() == fev.callcreates[i].data()))
						return 1;
					if (asserts(test.callcreates[i].receiveAddress() == fev.callcreates[i].receiveAddress()))
						return 1;
					if (asserts(test.callcreates[i].gas() == fev.callcreates[i].gas()))
						return 1;
					if (asserts(test.callcreates[i].value() == fev.callcreates[i].value()))
						return 1;
				}
			}

			//checkLog(fev.sub.logs, test.sub.logs);
			{
				if (assertsEqual(fev.sub.logs.size(), test.sub.logs.size()))
					return 1;

				for (size_t i = 0; i < fev.sub.logs.size(); ++i)
				{
					if (assertsEqual(fev.sub.logs[i].address, test.sub.logs[i].address))
						return 1;
					if (assertsEqual(fev.sub.logs[i].topics, test.sub.logs[i].topics))
						return 1;
					if (asserts(fev.sub.logs[i].data == test.sub.logs[i].data))
						return 1;
				}
			}

		}
		else	// Exception expected
		{
			if (asserts(vmExceptionOccured))
				return 1;
		}
	}
	// test passed
	return 0;
}

