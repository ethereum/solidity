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
/** @file TestHelper.h
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 */

#pragma once

#include <functional>

#include <boost/test/unit_test.hpp>

#include "JsonSpiritHeaders.h"
#include <libethereum/State.h>
#include <libevm/ExtVMFace.h>
#include <libtestutils/Common.h>

namespace dev
{
namespace eth
{

class Client;

void mine(Client& c, int numBlocks);
void connectClients(Client& c1, Client& c2);

}

namespace test
{

/// Make sure that no Exception is thrown during testing. If one is thrown show its info and fail the test.
/// Our version of BOOST_REQUIRE_NO_THROW()
/// @param _statenent    The statement for which to make sure no exceptions are thrown
/// @param _message       A message to act as a prefix to the expression's error information
#define ETH_TEST_REQUIRE_NO_THROW(_statement, _message)				\
	do																	\
	{																	\
		try															\
		{																\
			BOOST_TEST_PASSPOINT();										\
			_statement;												\
		}																\
		catch (boost::exception const& _e)								\
		{																\
			auto msg = std::string(_message " due to an exception thrown by " \
				BOOST_STRINGIZE(_statement) "\n") + boost::diagnostic_information(_e); \
			BOOST_CHECK_IMPL(false, msg, REQUIRE, CHECK_MSG);			\
		}																\
		catch (...)														\
		{																\
			BOOST_CHECK_IMPL(false, "Unknown exception thrown by "		\
				BOOST_STRINGIZE(_statement), REQUIRE, CHECK_MSG);		\
		}																\
	}																	\
	while (0)

/// Check if an Exception is thrown during testing. If one is thrown show its info and continue the test
/// Our version of BOOST_CHECK_NO_THROW()
/// @param _statement    The statement for which to make sure no exceptions are thrown
/// @param _message       A message to act as a prefix to the expression's error information
#define ETH_TEST_CHECK_NO_THROW(_statement, _message)					\
	do																	\
	{																	\
		try															\
		{																\
			BOOST_TEST_PASSPOINT();										\
			_statement;												\
		}																\
		catch (boost::exception const& _e)								\
		{																\
			auto msg = std::string(_message " due to an exception thrown by " \
				BOOST_STRINGIZE(_statement) "\n") + boost::diagnostic_information(_e); \
			BOOST_CHECK_IMPL(false, msg, CHECK, CHECK_MSG);				\
		}																\
		catch (...)														\
		{																\
			BOOST_CHECK_IMPL(false, "Unknown exception thrown by "		\
				BOOST_STRINGIZE(_statement), CHECK, CHECK_MSG );		\
		}																\
	}																	\
	while (0)


class ImportTest
{
public:
	ImportTest(json_spirit::mObject& _o) : m_statePre(Address(), OverlayDB(), eth::BaseState::Empty),  m_statePost(Address(), OverlayDB(), eth::BaseState::Empty), m_TestObject(_o) {}
	ImportTest(json_spirit::mObject& _o, bool isFiller);
	// imports
	void importEnv(json_spirit::mObject& _o);
	void importState(json_spirit::mObject& _o, eth::State& _state);
	void importTransaction(json_spirit::mObject& _o);
	void exportTest(bytes const& _output, eth::State const& _statePost);

	eth::State m_statePre;
	eth::State m_statePost;
	eth::ExtVMFace m_environment;
	eth::Transaction m_transaction;

private:
	json_spirit::mObject& m_TestObject;
};

class ZeroGasPricer: public eth::GasPricer
{
protected:
	u256 ask(eth::State const&) const override { return 0; }
	u256 bid(eth::TransactionPriority = eth::TransactionPriority::Medium) const override { return 0; }
};

// helping functions
u256 toInt(json_spirit::mValue const& _v);
byte toByte(json_spirit::mValue const& _v);
bytes importCode(json_spirit::mObject& _o);
bytes importData(json_spirit::mObject& _o);
bytes importByteArray(std::string const& _str);
eth::LogEntries importLog(json_spirit::mArray& _o);
json_spirit::mArray exportLog(eth::LogEntries _logs);
void checkOutput(bytes const& _output, json_spirit::mObject& _o);
void checkStorage(std::map<u256, u256> _expectedStore, std::map<u256, u256> _resultStore, Address _expectedAddr);
void checkLog(eth::LogEntries _resultLogs, eth::LogEntries _expectedLogs);
void checkCallCreates(eth::Transactions _resultCallCreates, eth::Transactions _expectedCallCreates);

void executeTests(const std::string& _name, const std::string& _testPathAppendix, std::function<void(json_spirit::mValue&, bool)> doTests);
void userDefinedTest(std::string testTypeFlag, std::function<void(json_spirit::mValue&, bool)> doTests);
RLPStream createRLPStreamFromTransactionFields(json_spirit::mObject& _tObj);
eth::LastHashes lastHashes(u256 _currentBlockNumber);
json_spirit::mObject fillJsonWithState(eth::State _state);

template<typename mapType>
void checkAddresses(mapType& _expectedAddrs, mapType& _resultAddrs)
{
	for (auto& resultPair : _resultAddrs)
	{
		auto& resultAddr = resultPair.first;
		auto expectedAddrIt = _expectedAddrs.find(resultAddr);
		if (expectedAddrIt == _expectedAddrs.end())
			BOOST_ERROR("Missing result address " << resultAddr);
	}
	BOOST_CHECK(_expectedAddrs == _resultAddrs);
}

class Options
{
public:
	bool jit = false;		///< Use JIT
	bool vmtrace = false;	///< Create EVM execution tracer // TODO: Link with log verbosity?
	bool fillTests = false; ///< Create JSON test files from execution results
	bool stats = false;		///< Execution time stats
	bool statsFull = false; ///< Output full stats - execution times for every test

	/// Test selection
	/// @{
	bool performance = false;
	bool quadratic = false;
	bool memory = false;
	bool inputLimits = false;
	bool bigData = false;
	/// @}

	/// Get reference to options
	/// The first time used, options are parsed
	static Options const& get();

private:
	Options();
	Options(Options const&) = delete;
};

/// Allows observing test execution process.
/// This class also provides methods for registering and notifying the listener
class Listener
{
public:
	virtual ~Listener() = default;

	virtual void testStarted(std::string const& _name) = 0;
	virtual void testFinished() = 0;

	static void registerListener(Listener& _listener);
	static void notifyTestStarted(std::string const& _name);
	static void notifyTestFinished();

	/// Test started/finished notification RAII helper
	class ExecTimeGuard
	{
	public:
		ExecTimeGuard(std::string const& _testName) { notifyTestStarted(_testName);	}
		~ExecTimeGuard() { notifyTestFinished(); }
		ExecTimeGuard(ExecTimeGuard const&) = delete;
		ExecTimeGuard& operator=(ExecTimeGuard) = delete;
	};
};



}
}
